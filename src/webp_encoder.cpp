//
// Copyright 2023 xiaozhuai
//

#include "webp_encoder.hpp"

#include <cstdio>
#include <cstdarg>
#include <webp/encode.h>
#include <webp/mux.h>

#if defined(WEBP_ENCODER_NO_LOG)
#define LOGE(fmt, ...) do { abort(); } while (0)
#else
#define LOGE(fmt, ...) do { printf("Error: " fmt "\n", ##__VA_ARGS__); abort(); } while (0)
#endif

struct WebpHandler {
    WebPAnimEncoder *enc = nullptr;
    WebPAnimEncoderOptions anim_config;
    WebPData data;
};

#define handler_ (reinterpret_cast<WebpHandler*>(raw_handler_))

static std::string StrFormat(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

#if defined(_WIN32)
    const int sz = _vscprintf_l(fmt, c_locale(), args);
#else
    const int sz = vsnprintf(nullptr, 0, fmt, args);
#endif

    std::string output(sz, '\0');

#if defined(_WIN32)
    _vsnprintf_s_l(&output.at(0), output.size() + 1, output.size(), fmt, c_locale(), args);
#else
    va_start(args, fmt);
    vsnprintf(&output.at(0), output.size() + 1, fmt, args);
#endif
    va_end(args);

    return output;
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

WebpEncoder::~WebpEncoder() {
    Release();
}

void WebpEncoder::Release() {
    WebPDataClear(&handler_->data);
    WebPAnimEncoderDelete(handler_->enc);
    handler_->enc = nullptr;
}

bool WebpEncoder::Init(const WebpFileOptions &options) {
    raw_handler_ = new WebpHandler();

    WebPDataInit(&handler_->data);
    if (!WebPAnimEncoderOptionsInit(&handler_->anim_config)) {
        LOGE("Init encoder options failed");
        Release();
        return false;
    }

    SetLoopCount(options.loop, &handler_->data);
    handler_->anim_config.minimize_size = options.minimize;
    handler_->anim_config.kmax = options.kmax;
    handler_->anim_config.kmin = options.kmin;
    handler_->anim_config.allow_mixed = options.mixed;

    return true;
}

bool WebpEncoder::AddFrame(uint8_t *pixels, int width, int height, const WebpFrameOptions &options) {
    if (handler_->enc == nullptr) {
        width_ = width;
        height_ = height;
        handler_->enc = WebPAnimEncoderNew(width, height, &handler_->anim_config);
        if (handler_->enc == nullptr) {
            LOGE("Init encoder failed");
            return false;
        }
    }

    if (width != width_ || height != height_) {
        LOGE("Image size mismatch");
        return false;
    }

    WebPConfig config;
    WebPPicture pic;

    if (!WebPConfigInit(&config)
        || !WebPPictureInit(&pic)) {
        LOGE("Init image config failed");
        return false;
    }

    config.lossless = 1;
    if (!handler_->anim_config.allow_mixed) {
        config.lossless = options.lossless;
    }
    config.quality = options.quality;
    config.method = options.method;

    if (!WebPValidateConfig(&config)) {
        LOGE("Validate image config failed");
        return false;
    }

    pic.use_argb = true;
    pic.width = width;
    pic.height = height;


    if (!WebPPictureImportRGBA(&pic, pixels, width * 4)) {
        LOGE("Import image data failed");
        return false;
    }

    if (!WebPAnimEncoderAdd(handler_->enc, &pic, timestamp_ms_, &config)) {
        LOGE("Encode add frame failed");
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
        LOGE("Encode assemble failed");
        *size = 0;
        return nullptr;
    }

    *size = handler_->data.size;
    return handler_->data.bytes;
}
