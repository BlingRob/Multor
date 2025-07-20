/// \file buffer.h

#pragma once

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

struct Buffer
{
    //Buffer(VkDevice dev, VkBuffer buf, VkDeviceMemory bufmem) :dev_(dev), buffer_(buf), bufferMemory_(bufmem) {}
    //Buffer(Buffer&& _r) :dev_(_r.dev_), buffer_(_r.buffer_), bufferMemory_(_r.bufferMemory_) {}
    VkDevice       dev_;
    VkBuffer       buffer_;
    VkDeviceMemory bufferMemory_;
    ~Buffer()
    {
        vkDestroyBuffer(dev_, buffer_, nullptr);
        vkFreeMemory(dev_, bufferMemory_, nullptr);
    }
};

} // namespace Multor
