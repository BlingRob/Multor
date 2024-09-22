/// \file vk_texture.cpp

#include "vk_texture.h"

namespace Multor
{

VkTexture::~VkTexture()
{
    vkDestroySampler(dev_, sampler_, nullptr);
    vkDestroyImageView(dev_, view_, nullptr);
    vkDestroyImage(dev_, img_, nullptr);
    vkFreeMemory(dev_, devMem_, nullptr);
    sampler_ = VK_NULL_HANDLE;
    view_    = VK_NULL_HANDLE;
    img_     = VK_NULL_HANDLE;
    devMem_  = VK_NULL_HANDLE;
}

} // namespace Multor