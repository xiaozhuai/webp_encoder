//
// Copyright 2023 xiaozhuai
//

#include "webp_encoder.hpp"

#include <webp/encode.h>
#include <webp/mux.h>

#define LOG(fmt, ...) do { printf(fmt "\n", ##__VA_ARGS__); } while (0)

struct WebpHandler {
    WebPAnimEncoder *enc = nullptr;
    WebPAnimEncoderOptions anim_config;
    WebPData data;
};

#define handler_ (reinterpret_cast<WebpHandler*>(raw_handler_))

WebpEncoder::~WebpEncoder() {
    Release();
}

void WebpEncoder::Release() {
    WebPDataClear(&handler_->data);
    WebPAnimEncoderDelete(handler_->enc);
    handler_->enc = nullptr;
}

bool WebpEncoder::Init() {
    raw_handler_ = new WebpHandler();

    WebPDataInit(&handler_->data);
    if (!WebPAnimEncoderOptionsInit(&handler_->anim_config)) {
        LOG("Error: Init encoder options failed");
        Release();
        return false;
    }

    return true;
}

static bool SetLoopCount(int loop_count, WebPData *const webp_data) {
    bool ok;
    WebPMuxError err;
    uint32_t features;
    WebPMuxAnimParams new_params;
    WebPMux *const mux = WebPMuxCreate(webp_data, 1);
    if (mux == nullptr) return 0;

    err = WebPMuxGetFeatures(mux, &features);
    ok = (err == WEBP_MUX_OK);
    if (!ok || !(features & ANIMATION_FLAG)) goto End;

    err = WebPMuxGetAnimationParams(mux, &new_params);
    ok = (err == WEBP_MUX_OK);
    if (ok) {
        new_params.loop_count = loop_count;
        err = WebPMuxSetAnimationParams(mux, &new_params);
        ok = (err == WEBP_MUX_OK);
    }
    if (ok) {
        WebPDataClear(webp_data);
        err = WebPMuxAssemble(mux, webp_data);
        ok = (err == WEBP_MUX_OK);
    }

    End:
    WebPMuxDelete(mux);
    return ok;
}

void WebpEncoder::SetOptions(const WebpFileOptions &options) {
    SetLoopCount(options.loop, &handler_->data);
    handler_->anim_config.minimize_size = options.min_size;
    handler_->anim_config.kmax = options.kmax;
    handler_->anim_config.kmin = options.kmin;
    handler_->anim_config.allow_mixed = options.mixed;
}

bool WebpEncoder::AddFrame(uint8_t *pixels, int width, int height, const WebpFrameOptions &options) {
    if (handler_->enc == nullptr) {
        width_ = width;
        height_ = height;
        handler_->enc = WebPAnimEncoderNew(width, height, &handler_->anim_config);
        if (handler_->enc == nullptr) {
            LOG("Error: Init encoder failed");
            return false;
        }
    }

    if (width != width_ || height != height_) {
        LOG("Error: Image size mismatch");
        return false;
    }

    WebPConfig config;
    WebPPicture pic;

    if (!WebPConfigInit(&config)
        || !WebPPictureInit(&pic)) {
        LOG("Error: Init image config failed");
        return false;
    }

    config.lossless = 1;
    if (!handler_->anim_config.allow_mixed) {
        config.lossless = options.lossless;
    }
    config.quality = options.quality;
    config.method = options.method;

    if (!WebPValidateConfig(&config)) {
        LOG("Error: Validate image config failed");
        return false;
    }

    pic.use_argb = true;
    pic.width = width;
    pic.height = height;


    if (!WebPPictureImportRGBA(&pic, pixels, width * 4)) {
        LOG("Error: Import image data failed");
        return false;
    }

    if (!WebPAnimEncoderAdd(handler_->enc, &pic, timestamp_ms_, &config)) {
        LOG("Error: Encode add frame failed");
        WebPPictureFree(&pic);
        return false;
    }
    timestamp_ms_ += options.duration;

    WebPPictureFree(&pic);

    return true;
}

const uint8_t *WebpEncoder::Encode(size_t *size) {
    if (!WebPAnimEncoderAdd(handler_->enc, nullptr, timestamp_ms_, nullptr)
        || !WebPAnimEncoderAssemble(handler_->enc, &handler_->data)) {
        LOG("Error: Encode assemble failed");
        *size = 0;
        return nullptr;
    }

    *size = handler_->data.size;
    return handler_->data.bytes;
}
