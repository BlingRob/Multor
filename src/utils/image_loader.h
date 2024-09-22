/// @file image_loader.h
/// @brief Texture's loader

#pragma once
#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "stb_image.h"
#include "image.h"


namespace Multor
{

using PDelFun = void (*)(void*);

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