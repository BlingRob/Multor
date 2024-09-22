/// \file VulkanObjects.h

#pragma once
#ifndef VK_VERTEX_BUFFER_H
#define VK_VERTEX_BUFFER_H

#include "vk_buffer.h"

#include <memory>
#include <array>

#include <vulkan/vulkan.h>

namespace Multor
{

struct VertexBuffer
{
    VertexBuffer(
        std::unique_ptr<VulkanBuffer>                           buf,
        const VkVertexInputBindingDescription&                  binddis,
        const std::array<VkVertexInputAttributeDescription, 5>& attrdes)
        : pVertBuf_(std::move(buf)),
          bindingDescription(binddis),
          attributeDescriptions(attrdes)
    {
    }

    VkVertexInputBindingDescription                  bindingDescription {};
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions {};
    std::unique_ptr<VulkanBuffer>                    pVertBuf_;
};

} // namespace Multor

#endif // VK_VERTEX_BUFFER_H