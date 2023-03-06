//
// Copyright 2023 xiaozhuai
//

#include <iostream>
#include <memory>
#include <emscripten/bind.h>

#include "webp_encoder.hpp"

int main() {
    return 0;
}

static bool WebpEncoder_Init(
        WebpEncoder &self,
        const emscripten::val &options) {
    WebpFileOptions o;
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

static inline void WebpEncoder_Release(WebpEncoder &self) {
    self.Release();
}

static bool WebpEncoder_Push(
        WebpEncoder &self,
        const emscripten::val &pixels,
        int width, int height,
        const emscripten::val &options) {

    std::vector<uint8_t> native_pixels;
    native_pixels.resize(pixels["length"].as<unsigned>());
    emscripten::val memoryView{emscripten::typed_memory_view(native_pixels.size(), native_pixels.data())};
    memoryView.call<void>("set", pixels);

    WebpFrameOptions o;
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

    return self.Push(native_pixels.data(), width, height, o);
}

static emscripten::val WebpEncoder_Encode(WebpEncoder &self) {
    size_t size;
    const uint8_t *data = self.Encode(&size);
    return emscripten::val(emscripten::typed_memory_view(size, data));
}

EMSCRIPTEN_BINDINGS(webp_encoder) {
    emscripten::class_<WebpEncoder>("WebpEncoder")
            .constructor()
            .function("init", &WebpEncoder_Init)
            .function("release", &WebpEncoder_Release)
            .function("push", &WebpEncoder_Push)
            .function("encode", &WebpEncoder_Encode);
}
