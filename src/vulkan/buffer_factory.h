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
        : dev_(dev), physDev(PhysDev), _executer(ex)
    {
    }

    std::unique_ptr<Buffer>
    createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                 VkMemoryPropertyFlags properties);
    std::unique_ptr<VertexBuffer> createVertexBuffer(Vertexes* vert);
    std::unique_ptr<Buffer> createIndexBuffer(Vertexes* vert);
    std::unique_ptr<Buffer> createUniformBuffer(VkDeviceSize bufferSize);
    std::unique_ptr<Buffer> createMaterialBuffer(Material* mat);

protected:
    VkDevice                         dev_;
    VkPhysicalDevice                 physDev;
    std::shared_ptr<CommandExecuter> _executer;
    uint32_t                         findMemoryType(uint32_t              typeFilter,
                                                    VkMemoryPropertyFlags properties);
};

} // namespace Multor::Vulkan
