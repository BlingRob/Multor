/// \file vertex_buffer.h

#pragma once

#include "buffer.h"

#include <memory>
#include <array>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

struct VertexBuffer
{
    VertexBuffer(
        std::unique_ptr<Buffer>                           buf,
        const VkVertexInputBindingDescription&                  binddis,
        const std::array<VkVertexInputAttributeDescription, 5>& attrdes)
        : pVertBuf_(std::move(buf)),
          bindingDescription(binddis),
          attributeDescriptions(attrdes)
    {
    }

    VkVertexInputBindingDescription                  bindingDescription {};
    std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions {};
    std::unique_ptr<Buffer>                    pVertBuf_;
};

} // namespace Multor::Vulkan
