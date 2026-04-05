/// \file mesh.h

#pragma once

#include "objects/vertex_buffer.h"
#include "structures/transform_ubo.h"
#include "structures/material_ubo.h"
#include "shader.h"
#include "objects/texture.h"

#include <memory>
#include <vector>

namespace Multor::Vulkan
{

struct Mesh
{
    /* Static object */
    std::unique_ptr<VertexBuffer> vertBuffer_;
    std::uint32_t                 indexesSize_;
    std::unique_ptr<Buffer> indexBuffer_;
    /* Textures */
    std::vector<std::shared_ptr<Texture> > textures_;
    std::shared_ptr<Texture> baseColorTex_;
    std::shared_ptr<Texture> normalTex_;
    std::shared_ptr<Texture> metallicTex_;
    std::shared_ptr<Texture> roughnessTex_;
    std::shared_ptr<Texture> aoTex_;
    std::shared_ptr<Texture> emissiveTex_;
    UBOs::MaterialData materialData_ {};
    bool castsShadows_ {true};
    
    /*  Dynamic object  */
    std::shared_ptr<Shader> sh_;

    //std::unique_ptr<Buffer> MaterialBuffer;
    std::vector<VkDescriptorSet>  desSet_;
    std::unique_ptr<TransformUBO> tr_;
};

} // namespace Multor::Vulkan
