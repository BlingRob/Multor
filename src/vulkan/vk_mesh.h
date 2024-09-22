/// \file vk_mesh.h

#pragma once
#ifndef VK_MESH_H
#define VK_MESH_H

#include "objects/vk_vertex_buffer.h"
#include "structures/transform_ubo.h"
#include "VkShader.h"
#include "objects/vk_texture.h"

#include <memory>
#include <vector>

namespace Multor
{

struct VkMesh
{
    /* Static object */
    std::unique_ptr<VertexBuffer> vertBuffer_;
    std::uint32_t                 indexesSize_;
    std::unique_ptr<VulkanBuffer> indexBuffer_;
    /* Textures */
    std::vector<std::shared_ptr<VkTexture> > textures_;
    
    /*  Dynamic object  */
    std::shared_ptr<vkShader> sh_;

    //std::unique_ptr<VulkanBuffer> MaterialBuffer;
    std::vector<VkDescriptorSet>  desSet_;
    std::unique_ptr<TransformUBO> tr_;
};

} // namespace Multor

#endif // VK_MESH_H