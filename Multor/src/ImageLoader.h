/// @file ImageLoader.h_
/// @brief Texture's loader

#pragma once
#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "stb_image.h"

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
    Image(Image&& img) noexcept : w_(img.w_),
                                  h_(img.h_),
                                  nrComponents_(img.nrComponents_)
    {
        std::swap(mdata_, img.mdata_);
        std::swap(deleter_, img.deleter_);
    }

    Image&& operator=(Image&& img) noexcept
    {
        w_            = img.w_;
        h_            = img.h_;
        nrComponents_ = img.nrComponents_;
        std::swap(mdata_, img.mdata_);
        std::swap(deleter_, img.deleter_);

        return std::move(*this);
    }

    Image(
        std::size_t width, std::size_t height, std::uint8_t channel = 4,
        unsigned char* data = nullptr,
        PDelFun        del =
            [](void* ptr) { delete static_cast<unsigned char*>(ptr); })
        : deleter_(del),
          w_(static_cast<int>(width)),
          h_(static_cast<int>(height)),
          nrComponents_(channel)
    {
        //texture load option
        //stbi_set_flip_vertically_on_load(true);
        if (data == nullptr)
            mdata_ = new unsigned char[width * height * channel];
        else
            mdata_ = data;
    };

    bool empty() const noexcept
    {
        return !w_ && !h_;
    }

    ~Image()
    {
        deleter_(mdata_);
    }

    unsigned char* mdata_;
    int            w_, h_, nrComponents_;

private:
    PDelFun deleter_;
};

struct ImageLoader
{
    static std::shared_ptr<Image> LoadTexture(const char* path);
    static std::shared_ptr<Image> LoadTexture(const void* memoryPtr, int width);

private:
    static inline int     w_, h_, chs_;
    static inline PDelFun STB_deleter = [](void* ptr) { stbi_image_free(ptr); };
};

} // namespace Multor

#endif // IMAGELOADER_H