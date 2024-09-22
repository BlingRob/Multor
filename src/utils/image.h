/// @file image.h
/// @brief Class of image

#pragma once
#ifndef IMAGE_H
#define IMAGE_H

#include "stb_image.h"

#include <cstdint>
#include <memory>
#include <vector>
#include <string_view>

namespace Multor
{

using PDelFun = void (*)(void*);

struct Image
{
    Image()
    {
    }
    Image(Image& img) noexcept = default;
    Image(Image&& img) noexcept;

    Image&& operator=(Image&& img) noexcept;

    Image(
        std::size_t width, std::size_t height, std::uint8_t channel = 4,
        unsigned char* data = nullptr,
        PDelFun        del =
            [](void* ptr) { delete static_cast<unsigned char*>(ptr); });

    bool empty() const noexcept;

    ~Image();

    unsigned char* mdata_;
    int            w_, h_, nrComponents_;

private:
    PDelFun deleter_;
};


} // namespace Multor

#endif // IMAGE_H