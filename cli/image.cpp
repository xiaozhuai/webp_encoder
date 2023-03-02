//
// Copyright 2023 xiaozhuai
//

#define STB_IMAGE_IMPLEMENTATION

#include "image.hpp"

#include <stb_image.h>

Image::Image(const std::string &file) {
    ReadFile(file);
}

Image::~Image() {
    Release();
}

void Image::ReadFile(const std::string &file) {
    Release();
    int channels;
    pixels = stbi_load(file.c_str(), &width, &height, &channels, 4);
}

void Image::Release() {
    stbi_image_free(pixels);
    pixels = nullptr;
    width = 0;
    height = 0;
}

