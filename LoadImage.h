// Function that loads an image from a file
#pragma once
#include "stb_image.h"
#include <string>
#include <vector>
#include <cstdint>
#include <stdexcept>

struct Pixel { uint8_t r, g, b, a; };

struct Image {
    int width;
    int height;
    std::vector<Pixel> data; // RGBA format
};

inline Image loadImage(const std::string& filename) {
    Image img;
    int channels;
    stbi_uc* data = stbi_load(filename.c_str(), &img.width, &img.height, &channels, 4);
    if (!data) {
        throw std::runtime_error("Failed to load image");
    }
    img.data.reserve(img.width * img.height);
    for (int i = 0; i < img.width * img.height * 4; i += 4) {
        Pixel pixel = { data[i], data[i + 1], data[i + 2], data[i + 3] };
        img.data.push_back(pixel);
    }
    stbi_image_free(data);
    return img;
}
