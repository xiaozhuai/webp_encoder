//
// Copyright 2023 xiaozhuai
//

#pragma once

#include <string>

class Image {
public:
    Image() = default;

    explicit Image(const std::string &file);

    ~Image();

    void ReadFile(const std::string &file);

    void Release();

public:
    uint8_t *pixels = nullptr;
    int width = 0;
    int height = 0;
};

