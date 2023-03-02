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

static void SetOptions(
        WebpEncoder &self,
        const emscripten::val &options) {
    WebpFileOptions o;
    if (options.hasOwnProperty("min_size")) {
        o.min_size = options["min_size"].as<bool>();
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
    self.SetOptions(o);
}

static void copyUint8Array(std::vector<uint8_t> &data, const emscripten::val &arr) {
    data.resize(arr["length"].as<unsigned>());
    emscripten::val memoryView{emscripten::typed_memory_view(data.size(), data.data())};
    memoryView.call<void>("set", arr);
}

static void AddFrame(
        WebpEncoder &self,
        const emscripten::val &pixels,
        int width, int height,
        const emscripten::val &options) {

    std::vector<uint8_t> native_pixels;
    copyUint8Array(native_pixels, pixels);

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

    self.AddFrame(native_pixels.data(), width, height, o);
}

static emscripten::val Encode(WebpEncoder &self) {
    size_t size;
    const uint8_t *data = self.Encode(&size);
    return emscripten::val(emscripten::typed_memory_view(size, data));
}

EMSCRIPTEN_BINDINGS(webp_encoder) {
    emscripten::class_<WebpEncoder>("WebpEncoder")
            .constructor()
            .function("Init", &WebpEncoder::Init)
            .function("Release", &WebpEncoder::Release)
            .function("SetOptions", &SetOptions)
            .function("AddFrame", &AddFrame)
            .function("Encode", &Encode);
}
