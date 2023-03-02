//
// Copyright 2023 xiaozhuai
//


#include <string>
#include <fstream>
#include <vector>

#include "image.hpp"
#include "webp_encoder.hpp"

int main() {
    WebpFileOptions file_options{
            .min_size = true,
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
    encoder.Init();
    encoder.SetOptions(file_options);

    std::vector<std::string> files = {
            "docs/test/frames/frame_0.jpg",
            "docs/test/frames/frame_1.jpg",
            "docs/test/frames/frame_2.jpg",
            "docs/test/frames/frame_3.jpg",
    };

    for (const auto &file : files) {
        Image image;
        image.ReadFile(file);
        encoder.AddFrame(image.pixels, image.width, image.height, frame_options);
    }

    size_t size;
    const uint8_t *bytes = encoder.Encode(&size);

    std::ofstream out("test.webp");
    out.write(reinterpret_cast<const char *>(bytes), static_cast<std::streamsize>(size));
    out.close();

    encoder.Release();

    return 0;
}
