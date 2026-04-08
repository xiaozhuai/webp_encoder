//
// Copyright (c) 2023 xiaozhuai
//

#pragma once

#include <memory>
#include <string>
#include <vector>
#if defined(__EMSCRIPTEN__)
#include <emscripten/bind.h>
#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#endif

namespace WebpEncoder {

struct FileOptions {
    /**
     * If true, minimize the output size (slow). Implicitly
     * disables key-frame insertion.
     */
    bool minimize = true;

    /**
     * loop count (default to 0 stands for infinite loop)
     */
    int loop = 0;

    /**
     * Minimum and maximum distance between consecutive key
     * frames in the output. The library may insert some key
     * frames as needed to satisfy this criteria.
     * Note that these conditions should hold: kmax > kmin
     * and kmin >= kmax / 2 + 1. Also, if kmax <= 0, then
     * key-frame insertion is disabled; and if kmax == 1,
     * then all frames will be key-frames (kmin value does
     * not matter for these special cases).
     */
    int kmax = 0;
    int kmin = 0;

    /**
     * If true, use mixed compression mode; may choose
     */
    bool mixed = true;
};

struct FrameOptions {
    /**
     * timestamp of this frame in milliseconds.
     */
    int duration = 100;

    /**
     * Lossless encoding
     */
    bool lossless = false;

    /**
     * between 0 and 100. For lossy, 0 gives the smallest
     * size and 100 the largest. For lossless, this
     * parameter is the amount of effort put into the
     * compression: 0 is the fastest but gives larger
     * files compared to the slowest, but best, 100.
     */
    float quality = 100.0f;

    /**
     * quality/speed trade-off (0=fast, 6=slower-better)
     */
    int method = 0;

    /**
     * if true, preserve the exact RGB values under
     * transparent area. Otherwise, discard this invisible
     * RGB information for better compression.
     */
    bool exact = false;
};

class WebpEncoder final {
public:
    WebpEncoder();

    ~WebpEncoder();

    /**
     * Init encoder
     *
     * @param options       Webp file options
     * @return
     */
    bool Init(const FileOptions &options);

    /**
     * push a webp frame
     *
     * @param pixels        Pixels data in RGBA format
     * @param width         Frame width
     * @param height        Frame height
     * @param options       Frame options
     * @return
     */
    bool Push(uint8_t *pixels, int width, int height, const FrameOptions &options);

#if !defined(__EMSCRIPTEN__)
    using EncodedData = std::vector<uint8_t>;
#else
    using EncodedData = emscripten::val;
#endif
    [[nodiscard]] EncodedData Encode();

#if !defined(__EMSCRIPTEN__)
    /**
     * Write a webp file
     *
     * @param file          File path
     */
    void Write(const std::string &file);
#endif

private:
    int width_ = -1;
    int height_ = -1;
    int timestamp_ms_ = 0;
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace WebpEncoder
