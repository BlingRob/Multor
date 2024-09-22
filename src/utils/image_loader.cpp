/// \file image_loader.cpp

#include "image_loader.h"

namespace Multor
{

std::shared_ptr<Image> ImageLoader::LoadTexture(const char* path)
{
    return std::make_shared<Image>(
        w_, h_, chs_, stbi_load(path, &w_, &h_, &chs_, STBI_rgb_alpha),
        STB_deleter);
}

std::shared_ptr<Image> ImageLoader::LoadTexture(const void* memoryPtr,
                                                int         bytes)
{
    return std::make_shared<Image>(
        w_, h_, chs_,
        stbi_load_from_memory(static_cast<const stbi_uc*>(memoryPtr), bytes,
                              &w_, &h_, &chs_, STBI_rgb_alpha),
        STB_deleter);
}

} // namespace Multor
