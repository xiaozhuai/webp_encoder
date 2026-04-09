# webp_encoder

A cross platform C++ (win, linux, unix, macos, wasm) library that encodes sequence image frames into WebP format.

[![ci](https://github.com/xiaozhuai/webp_encoder/actions/workflows/ci.yml/badge.svg)](https://github.com/xiaozhuai/webp_encoder/actions/workflows/ci.yml)

## Online Page

See online page [https://xiaozhuai.github.io/webp_encoder/](https://xiaozhuai.github.io/webp_encoder/)

## Usage

### C++

```cpp
#include <string>
#include <vector>
#include <webp_encoder.hpp>

int main() {
    WebpEncoder::WebpEncoder encoder;

    WebpEncoder::FileOptions file_options{
            .minimize = true,
            .loop = 0,
            .kmax = 0,
            .kmin = 0,
            .mixed = true,
    };
    encoder.Init(file_options);

    WebpEncoder::FrameOptions frame_options{
            .duration = 100,
            .lossless = false,
            .quality = 100.0f,
            .method = 0,
            .exact = false,
    };

    // Push frames in RGBA format
    encoder.Push(frame_00001, width, height, frame_options);
    encoder.Push(frame_00002, width, height, frame_options);
    encoder.Push(frame_00003, width, height, frame_options);
    frame_options.duration = 200;
    encoder.Push(frame_00004, width, height, frame_options);
    // Add more frames here...

    encoder.Write("test.webp");
}
```

### Javascript

See [docs/js/main.js](docs/js/main.js)
