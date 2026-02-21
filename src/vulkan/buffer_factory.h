/// \file buffer_factory.h

#pragma once

#include "command_executer.h"
#include "../scene_objects/vertexes.h"
#include "../scene_objects/material.h"
#include "objects/vertex_buffer.h"
#include "objects/buffer.h"

#include <memory>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class BufferFactory
{
public:
    BufferFactory(VkDevice dev, VkPhysicalDevice PhysDev,
                    std::shared_ptr<CommandExecuter> ex)
        : dev_(dev), physDev_(PhysDev), executer_(ex)
    {
    }

    std::unique_ptr<Buffer>
    CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties);
    std::unique_ptr<VertexBuffer> CreateVertexBuffer(Vertexes* vert);
    std::unique_ptr<Buffer> CreateIndexBuffer(Vertexes* vert);
    std::unique_ptr<Buffer> CreateUniformBuffer(VkDeviceSize bufferSize);
    std::unique_ptr<Buffer> CreateMaterialBuffer(Material* mat);

protected:
    VkDevice                         dev_;
    VkPhysicalDevice                 physDev_;
    std::shared_ptr<CommandExecuter> executer_;
    uint32_t                         findMemoryType(uint32_t              typeFilter,
                                                    VkMemoryPropertyFlags properties);
};

} // namespace Multor::Vulkan
