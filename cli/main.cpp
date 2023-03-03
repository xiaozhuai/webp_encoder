//
// Copyright 2023 xiaozhuai
//


#include <string>
#include <vector>

#include "image.hpp"
#include "webp_encoder.hpp"

int main() {
    WebpFileOptions file_options{
            .minimize = true,
            .loop = 0,
            .kmax = 0,
            .kmin = 0,
            .mixed = true,
    };

    WebpFrameOptions frame_options{
            .duration = 100,
            .lossless = false,
            .quality = 100.0f,
            .method = 0,
    };

    WebpEncoder encoder;
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
        encoder.Push(image.pixels, image.width, image.height, frame_options);
    }

    encoder.Write("test.webp");

    return 0;
}
