/// \file vk_buffer.h

#pragma once
#ifndef VK_BUFFER_H
#define VK_BUFFER_H

#include <vulkan/vulkan.h>

namespace Multor
{

struct VulkanBuffer
{
    //VulkanBuffer(VkDevice dev, VkBuffer buf, VkDeviceMemory bufmem) :dev_(dev), buffer_(buf), bufferMemory_(bufmem) {}
    //VulkanBuffer(VulkanBuffer&& _r) :dev_(_r.dev_), buffer_(_r.buffer_), bufferMemory_(_r.bufferMemory_) {}
    VkDevice       dev_;
    VkBuffer       buffer_;
    VkDeviceMemory bufferMemory_;
    ~VulkanBuffer()
    {
        vkDestroyBuffer(dev_, buffer_, nullptr);
        vkFreeMemory(dev_, bufferMemory_, nullptr);
    }
};

} // namespace Multor

#endif // VK_BUFFER_H