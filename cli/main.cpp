//
// Copyright (c) 2023 xiaozhuai
//

#include <iostream>
#include <string>
#include <vector>

#include "image.hpp"
#include "webp_encoder.hpp"

int main() {
    WebpEncoder::FileOptions file_options{true, 0, 0, 0, true};
    WebpEncoder::FrameOptions frame_options{100, false, 100.0f, 0, false};

    WebpEncoder::WebpEncoder encoder;
    if (!encoder.Init(file_options)) {
        std::cerr << "Init encoder failed" << std::endl;
        return 1;
    }

    std::vector<std::string> files = {
        "docs/frames/frame_0.jpg",
        "docs/frames/frame_1.jpg",
        "docs/frames/frame_2.jpg",
        "docs/frames/frame_3.jpg",
    };

    for (const auto &file : files) {
        Image image;
        if (!image.ReadFile(file)) {
            std::cerr << "Read image failed: " << file << std::endl;
            return 1;
        }
        std::cout << "Push image " << file << std::endl;
        if (!encoder.Push(image.pixels(), image.width(), image.height(), frame_options)) {
            std::cerr << "Push image failed: " << file << std::endl;
            return 1;
        }
    }

    std::cout << "Write test.webp" << std::endl;
    if (!encoder.Write("test.webp")) {
        std::cerr << "Write output failed: test.webp" << std::endl;
        return 1;
    }

    return 0;
}
