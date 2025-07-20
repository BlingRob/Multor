/// \file texture.h

#pragma once

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

struct Texture
{
    VkDevice       dev_;
    VkImage        img_;
    VkImageView    view_;
    VkSampler      sampler_;
    VkDeviceMemory devMem_;
    ~Texture();
};

} // namespace Multor::Vulkan
