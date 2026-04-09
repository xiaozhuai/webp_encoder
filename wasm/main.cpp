//
// Copyright (c) 2023 xiaozhuai
//

#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>

#include "webp_encoder.hpp"

void throw_js_error(const std::string &msg);
#define WEBP_ENCODER_THROW(str) throw_js_error(str)

int main() { return 0; }

static bool WebpEncoder_Init(WebpEncoder::WebpEncoder &self, const emscripten::val &options) {
    WebpEncoder::FileOptions o;
    if (options.hasOwnProperty("minimize")) {
        o.minimize = options["minimize"].as<bool>();
    }
    if (options.hasOwnProperty("loop")) {
        o.loop = options["loop"].as<int>();
    }
    if (options.hasOwnProperty("kmax")) {
        o.kmax = options["kmax"].as<int>();
    }
    if (options.hasOwnProperty("kmin")) {
        o.kmin = options["kmin"].as<int>();
    }
    if (options.hasOwnProperty("mixed")) {
        o.mixed = options["mixed"].as<bool>();
    }
    return self.Init(o);
}

static bool WebpEncoder_Push(WebpEncoder::WebpEncoder &self, const emscripten::val &pixels, int width, int height,
                             const emscripten::val &options) {
    const auto size = pixels["length"].as<size_t>();
    if (width <= 0 || height <= 0) {
        throw_js_error("width and height must be positive");
        return false;
    }
    const auto expected_size = static_cast<size_t>(width) * static_cast<size_t>(height) * size_t{4};
    if (size != expected_size) {
        throw_js_error("pixels length must equal width * height * 4");
        return false;
    }

    std::vector<uint8_t> native_pixels(size);
    emscripten::val memory_view{emscripten::typed_memory_view(native_pixels.size(), native_pixels.data())};
    memory_view.call<void>("set", pixels);

    WebpEncoder::FrameOptions o;
    if (options.hasOwnProperty("duration")) {
        o.duration = options["duration"].as<int>();
    }
    if (options.hasOwnProperty("lossless")) {
        o.lossless = options["lossless"].as<bool>();
    }
    if (options.hasOwnProperty("quality")) {
        o.quality = options["quality"].as<float>();
    }
    if (options.hasOwnProperty("method")) {
        o.method = options["method"].as<int>();
    }
    if (options.hasOwnProperty("exact")) {
        o.exact = options["exact"].as<bool>();
    }
    return self.Push(native_pixels.data(), width, height, o);
}

static emscripten::val WebpEncoder_Encode(WebpEncoder::WebpEncoder &self) { return self.Encode(); }

EMSCRIPTEN_BINDINGS(WebpEncoder) {
    emscripten::class_<WebpEncoder::WebpEncoder>("WebpEncoder")
        .constructor()
        .function("init", &WebpEncoder_Init)
        .function("push", &WebpEncoder_Push)
        .function("encode", &WebpEncoder_Encode);
}
