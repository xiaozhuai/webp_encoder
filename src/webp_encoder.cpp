//
// Copyright (c) 2023 xiaozhuai
//

#include "webp_encoder.hpp"

#include <webp/encode.h>
#include <webp/mux.h>

#include <fstream>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#if !defined(WEBP_ENCODER_DISABLE_EXCEPTION)
#if defined(__EMSCRIPTEN__)
EM_JS(void, throw_js_error, (const char *msg), { throw new Error(UTF8ToString(msg)); });
void throw_js_error(const std::string &msg) { throw_js_error(msg.c_str()); }
#define WEBP_ENCODER_THROW(str) throw_js_error(str)
#else
#define WEBP_ENCODER_THROW(str) throw std::runtime_error(str)
#endif
#else
#define WEBP_ENCODER_THROW(str)
#endif

namespace WebpEncoder {

struct WebpEncoder::Impl {
    std::unique_ptr<WebPAnimEncoder, std::function<void(WebPAnimEncoder *)>> enc;
    WebPAnimEncoderOptions anim_config{};
};

WebpEncoder::WebpEncoder() = default;

WebpEncoder::~WebpEncoder() = default;

bool WebpEncoder::Init(const FileOptions &options) {
    impl_ = std::make_unique<Impl>();
    width_ = -1;
    height_ = -1;
    timestamp_ms_ = 0;

    if (!WebPAnimEncoderOptionsInit(&impl_->anim_config)) {
        WEBP_ENCODER_THROW("WebPAnimEncoderOptionsInit failed");
        return false;
    }

    impl_->anim_config.anim_params.loop_count = options.loop;
    impl_->anim_config.minimize_size = options.minimize;
    impl_->anim_config.kmax = options.kmax;
    impl_->anim_config.kmin = options.kmin;
    impl_->anim_config.allow_mixed = options.mixed;

    return true;
}

bool WebpEncoder::Push(uint8_t *pixels, int width, int height, const FrameOptions &options) {
    if (impl_ == nullptr) {
        WEBP_ENCODER_THROW("WebpEncoder::Init must be called before Push");
        return false;
    }
    if (width <= 0 || height <= 0) {
        WEBP_ENCODER_THROW("width and height must be positive");
        return false;
    }
    if (pixels == nullptr) {
        WEBP_ENCODER_THROW("Pixels must not be null");
        return false;
    }

    if (impl_->enc == nullptr) {
        width_ = width;
        height_ = height;
        impl_->enc = std::unique_ptr<WebPAnimEncoder, std::function<void(WebPAnimEncoder *)>>(
            WebPAnimEncoderNew(width, height, &impl_->anim_config), WebPAnimEncoderDelete);
        if (!impl_->enc) {
            WEBP_ENCODER_THROW("WebPAnimEncoderNew failed");
            return false;
        }
    }

    if (width != width_ || height != height_) {
        WEBP_ENCODER_THROW("Image size mismatch");
        return false;
    }

    WebPConfig config;
    WebPPicture pic;

    if (!WebPConfigInit(&config)) {
        WEBP_ENCODER_THROW("WebPConfigInit failed");
        return false;
    }

    if (!WebPPictureInit(&pic)) {
        WEBP_ENCODER_THROW("WebPPictureInit failed");
        return false;
    }
    std::unique_ptr<WebPPicture, std::function<void(WebPPicture *)>> _{&pic, WebPPictureFree};

#if !defined(__wasm__)
    config.thread_level = 1;
#endif

    config.lossless = 1;
    if (!impl_->anim_config.allow_mixed) {
        config.lossless = options.lossless;
    }
    config.quality = options.quality;
    config.method = options.method;
    config.exact = options.exact;
    // #if defined(__wasm__)
    //     config.low_memory = true;
    // #endif

    if (!WebPValidateConfig(&config)) {
        WEBP_ENCODER_THROW("WebPValidateConfig config");
        return false;
    }

    pic.use_argb = true;
    pic.width = width;
    pic.height = height;

    if (!WebPPictureImportRGBA(&pic, pixels, width * 4)) {
        WEBP_ENCODER_THROW("WebPPictureImportRGBA failed");
        return false;
    }

    if (!WebPAnimEncoderAdd(impl_->enc.get(), &pic, timestamp_ms_, &config)) {
        WEBP_ENCODER_THROW("WebPAnimEncoderAdd failed, " + std::to_string(pic.error_code));
        return false;
    }
    timestamp_ms_ += options.duration;

    return true;
}

WebpEncoder::EncodedData WebpEncoder::Encode() {
    if (impl_ == nullptr) {
        WEBP_ENCODER_THROW("WebpEncoder::Init must be called before Encode");
        return {};
    }
    if (impl_->enc == nullptr) {
        WEBP_ENCODER_THROW("At least one frame must be pushed before Encode");
        return {};
    }

    if (!WebPAnimEncoderAdd(impl_->enc.get(), nullptr, timestamp_ms_, nullptr)) {
        WEBP_ENCODER_THROW("WebPAnimEncoderAdd failed");
        return {};
    }

    WebPData webp_data;
    WebPDataInit(&webp_data);
    std::unique_ptr<WebPData, std::function<void(WebPData *)>> webp_data_guard{&webp_data, WebPDataClear};

    if (!WebPAnimEncoderAssemble(impl_->enc.get(), &webp_data)) {
        WEBP_ENCODER_THROW("WebPAnimEncoderAssemble failed");
        return {};
    }

    std::unique_ptr<WebPMux, std::function<void(WebPMux *)>> mux{WebPMuxCreate(&webp_data, 0), WebPMuxDelete};
    if (mux == nullptr) {
        WEBP_ENCODER_THROW("WebPMuxCreate failed");
        return {};
    }

    if (WebPMuxAssemble(mux.get(), &webp_data) != WEBP_MUX_OK) {
        WEBP_ENCODER_THROW("WebPMuxAssemble failed");
        return {};
    }

#if !defined(__EMSCRIPTEN__)
    return {webp_data.bytes, webp_data.bytes + webp_data.size};
#else
    emscripten::val Uint8Array = emscripten::val::global("Uint8Array");
    emscripten::val arr = Uint8Array.new_(webp_data.size);
    emscripten::val view = emscripten::val(emscripten::typed_memory_view(webp_data.size, webp_data.bytes));
    arr.call<void>("set", view);
    return arr;
#endif
}

#if !defined(__EMSCRIPTEN__)
bool WebpEncoder::Write(const std::string &file) {
    auto encoded_data = Encode();
    if (encoded_data.empty()) {
        WEBP_ENCODER_THROW("Encode failed before writing output file");
        return false;
    }
    std::ofstream out(file, std::ios_base::out | std::ios_base::binary);
    if (!out.is_open()) {
        WEBP_ENCODER_THROW("Open output file failed: " + file);
        return false;
    }
    out.write(reinterpret_cast<const char *>(encoded_data.data()), static_cast<std::streamsize>(encoded_data.size()));
    if (!out.good()) {
        WEBP_ENCODER_THROW("Write output file failed: " + file);
        return false;
    }
    out.close();
    return true;
}
#endif

}  // namespace WebpEncoder
