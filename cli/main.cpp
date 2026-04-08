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
    encoder.Init(file_options);

    std::vector<std::string> files = {
        "docs/frames/frame_0.jpg",
        "docs/frames/frame_1.jpg",
        "docs/frames/frame_2.jpg",
        "docs/frames/frame_3.jpg",
    };

    for (const auto &file : files) {
        Image image;
        image.ReadFile(file);
        std::cout << "Push image " << file << std::endl;
        encoder.Push(image.pixels, image.width, image.height, frame_options);
    }

    std::cout << "Write test.webp" << std::endl;
    encoder.Write("test.webp");

    return 0;
}
