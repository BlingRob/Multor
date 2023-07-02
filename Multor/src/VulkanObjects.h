/// \file VulkanObjects.h

#pragma once
#ifndef VULKANOBJECTS_H
#define VULKANOBJECTS_H

#include <memory>
#include <vector>
#include <array>

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace Multor
{

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 texCoord;
    glm::vec3 aTan;
    glm::vec3 aBitan;

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

struct VkTexture
{
    VkDevice       dev_;
    VkImage        img_;
    VkImageView    view_;
    VkSampler      sampler_;
    VkDeviceMemory devMem_;
    ~VkTexture();
};

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

#endif // VULKANOBJECTS_H