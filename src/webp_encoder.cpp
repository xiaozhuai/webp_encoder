//
// Copyright 2023 xiaozhuai
//

#include "webp_encoder.hpp"

#include <cstdio>
#include <cstdarg>
#include <fstream>
#include <functional>
#include <utility>

#include <webp/encode.h>
#include <webp/mux.h>

template<class F>
class FinalAction {
public:
    FinalAction(F f) noexcept: f_(std::move(f)), invoke_(true) {}

    FinalAction(FinalAction &&other) noexcept: f_(std::move(other.f_)), invoke_(other.invoke_) {
        other.invoke_ = false;
    }

    FinalAction(const FinalAction &) = delete;

    FinalAction &operator=(const FinalAction &) = delete;

    ~FinalAction() noexcept {
        if (invoke_) f_();
    }

private:
    F f_;
    bool invoke_;
};

template<class F>
inline FinalAction<F> _finally(const F &f) noexcept {
    return FinalAction<F>(f);
}

template<class F>
inline FinalAction<F> _finally(F &&f) noexcept {
    return FinalAction<F>(std::forward<F>(f));
}

#define concat1(a, b)       a ## b
#define concat2(a, b)       concat1(a, b)
#define _finally_object     concat2(_finally_object_, __COUNTER__)
#define finally             auto _finally_object = [&]()
#define finally2(func)      auto _finally_object = (func)

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

static bool SetLoopCount(int loop_count, WebPData *const data) {
    bool ok;
    uint32_t features;
    WebPMuxAnimParams params;
    WebPMux *mux = WebPMuxCreate(data, 1);
    if (mux == nullptr) {
        return false;
    }
    finally { WebPMuxDelete(mux); };

    if ((WebPMuxGetFeatures(mux, &features) != WEBP_MUX_OK)
        || !(features & ANIMATION_FLAG)) {
        return false;
    }

    if (WebPMuxGetAnimationParams(mux, &params) != WEBP_MUX_OK) {
        return false;
    }

    params.loop_count = loop_count;
    if (WebPMuxSetAnimationParams(mux, &params) != WEBP_MUX_OK) {
        return false;
    }

    WebPDataClear(data);

    if (WebPMuxAssemble(mux, data) != WEBP_MUX_OK) {
        return false;
    }

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
        Release();
        LOGE("Init encoder options failed");
        return false;
    }

    SetLoopCount(options.loop, &handler_->data);
    handler_->anim_config.minimize_size = options.minimize;
    handler_->anim_config.kmax = options.kmax;
    handler_->anim_config.kmin = options.kmin;
    handler_->anim_config.allow_mixed = options.mixed;

    return true;
}

bool WebpEncoder::Push(uint8_t *pixels, int width, int height, const WebpFrameOptions &options) {
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
    finally { WebPPictureFree(&pic); };

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
        return false;
    }
    timestamp_ms_ += options.duration;

    return true;
}

const uint8_t *WebpEncoder::Encode(size_t *size) {
    *size = 0;
    if (!WebPAnimEncoderAdd(handler_->enc, nullptr, timestamp_ms_, nullptr)
        || !WebPAnimEncoderAssemble(handler_->enc, &handler_->data)) {
        LOGE("Encode assemble failed");
        return nullptr;
    }

    *size = handler_->data.size;
    return handler_->data.bytes;
}

void WebpEncoder::Write(const std::string &file) {
    size_t size;
    const auto *bytes = Encode(&size);
    std::ofstream out(file);
    out.write(reinterpret_cast<const char *>(bytes), static_cast<std::streamsize>(size));
    out.close();
}
