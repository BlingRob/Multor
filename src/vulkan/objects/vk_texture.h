/// \file vk_texture.h

#pragma once
#ifndef VK_TEXTURE_H
#define VK_TEXTURE_H

#include <vulkan/vulkan.h>

namespace Multor
{

struct VkTexture
{
    VkDevice       dev_;
    VkImage        img_;
    VkImageView    view_;
    VkSampler      sampler_;
    VkDeviceMemory devMem_;
    ~VkTexture();
};

} // namespace Multor

#endif // VK_TEXTURE_H