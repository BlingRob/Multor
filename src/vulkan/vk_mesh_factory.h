/// \file Mesh.h

#pragma once
#ifndef VK_MESH_FACTORY_H
#define VK_MESH_FACTORY_H

#include "vk_mesh.h"
#include "../scene_objects/Mesh.h"

#include "objects/vk_vertex.h"
#include "vk_texture_factory.h"
#include "vk_command_executer.h"

namespace Multor
{

class VkMeshFactory : public VkTextureFactory
{
public:
    VkMeshFactory(VkDevice& dev, VkPhysicalDevice& physDev,
                  std::shared_ptr<CommandExecuter> ex)
        : VkTextureFactory(dev, physDev, std::move(ex))
    {
    }
    std::unique_ptr<VkMesh>       createMesh(std::unique_ptr<BaseMesh> mesh);
    std::unique_ptr<TransformUBO> createUBOBuffers(std::size_t nFrames);
};

} // namespace Multor

#endif // VK_MESH_FACTORY_H