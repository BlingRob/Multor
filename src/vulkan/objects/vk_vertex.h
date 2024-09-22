/// \file vk_vertex.h

#pragma once
#ifndef VK_VERTEX_H
#define VK_VERTEX_H

#include "../../scene_objects/vertex.h"

#include <array>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Multor
{

struct VkVertex : Vertex
{
    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription {};
        bindingDescription.binding   = 0;
        bindingDescription.stride    = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 5>
    getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 5>
            attributeDescriptions {};
        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, norm);

        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding  = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset   = offsetof(Vertex, aTan);

        attributeDescriptions[4].binding  = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset   = offsetof(Vertex, aBitan);

        return attributeDescriptions;
    }
};

} // namespace Multor

#endif // VK_VERTEX_H