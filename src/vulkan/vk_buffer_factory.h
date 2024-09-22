/// \file vk_buffer_factory.h

#pragma once
#ifndef VK_BUFFER_FACTORY_H
#define VK_BUFFER_FACTORY_H

#include "vk_command_executer.h"
#include "../scene_objects/vertexes.h"
#include "../scene_objects/material.h"
#include "objects/vk_buffer.h"
#include "objects/vk_vertex_buffer.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace Multor
{

class VkBufferFactory
{
public:
    VkBufferFactory(VkDevice dev, VkPhysicalDevice PhysDev,
                    std::shared_ptr<CommandExecuter> ex)
        : dev_(dev), physDev(PhysDev), _executer(ex)
    {
    }

    std::unique_ptr<VulkanBuffer>
    createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties);
    std::unique_ptr<VertexBuffer> createVertexBuffer(Vertexes* vert);
    std::unique_ptr<VulkanBuffer> createIndexBuffer(Vertexes* vert);
    std::unique_ptr<VulkanBuffer> createUniformBuffer(VkDeviceSize bufferSize);
    std::unique_ptr<VulkanBuffer> createMaterialBuffer(Material* mat);

protected:
    VkDevice                         dev_;
    VkPhysicalDevice                 physDev;
    std::shared_ptr<CommandExecuter> _executer;
    uint32_t                         findMemoryType(uint32_t              typeFilter,
                                                    VkMemoryPropertyFlags properties);
};

} // namespace Multor

#endif // VK_BUFFER_FACTORY_H