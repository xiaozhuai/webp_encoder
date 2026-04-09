//
// Copyright (c) 2023 xiaozhuai
//

#define STB_IMAGE_IMPLEMENTATION

#include "image.hpp"

#include <stb_image.h>

bool Image::ReadFile(const std::string &file) {
    pixels_.reset();
    width_ = 0;
    height_ = 0;
    int channels = 0;
    auto p = stbi_load(file.c_str(), &width_, &height_, &channels, 4);
    if (!p) {
        return false;
    }
    pixels_ = {p, stbi_image_free};
    return true;
}
