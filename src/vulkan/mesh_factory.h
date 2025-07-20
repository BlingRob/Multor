/// \file Mesh.h

#pragma once

#include "mesh.h"
#include "../scene_objects/Mesh.h"

#include "objects/vertex.h"
#include "texture_factory.h"
#include "command_executer.h"

namespace Multor::Vulkan
{

class MeshFactory : public TextureFactory
{
public:
    MeshFactory(VkDevice& dev, VkPhysicalDevice& physDev,
                  std::shared_ptr<CommandExecuter> ex)
        : TextureFactory(dev, physDev, std::move(ex))
    {
    }
    std::unique_ptr<Mesh>       createMesh(std::unique_ptr<BaseMesh> mesh);
    std::unique_ptr<TransformUBO> createUBOBuffers(std::size_t nFrames);
};

} // namespace Multor::Vulkan
