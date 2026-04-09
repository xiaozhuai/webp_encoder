//
// Copyright (c) 2023 xiaozhuai
//

#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

class Image {
public:
    Image() = default;

    ~Image() = default;

    [[nodiscard]] bool ReadFile(const std::string &file);

    [[nodiscard]] uint8_t *pixels() { return pixels_.get(); }
    [[nodiscard]] const uint8_t *pixels() const { return pixels_.get(); }
    [[nodiscard]] int width() const { return width_; }
    [[nodiscard]] int height() const { return height_; }

private:
    std::unique_ptr<uint8_t, std::function<void(uint8_t *)>> pixels_;
    int width_ = 0;
    int height_ = 0;
};
