/// \file image_loader.cpp

#include "image_loader.h"

namespace Multor
{

std::shared_ptr<Image> ImageLoader::LoadTexture(const char* path)
{
    int w = 0;
    int h = 0;
    int chs = 0;
    auto* data = stbi_load(path, &w, &h, &chs, STBI_rgb_alpha);
    if (data == nullptr || w <= 0 || h <= 0)
        return std::make_shared<Image>(0, 0, 4, nullptr, STB_deleter);

    return std::make_shared<Image>(w, h, chs, data, STB_deleter);
}

std::shared_ptr<Image> ImageLoader::LoadTexture(const void* memoryPtr,
                                                int         bytes)
{
    int w = 0;
    int h = 0;
    int chs = 0;
    auto* data = stbi_load_from_memory(static_cast<const stbi_uc*>(memoryPtr),
                                       bytes, &w, &h, &chs, STBI_rgb_alpha);
    if (data == nullptr || w <= 0 || h <= 0)
        return std::make_shared<Image>(0, 0, 4, nullptr, STB_deleter);

    return std::make_shared<Image>(w, h, chs, data, STB_deleter);
}

} // namespace Multor
