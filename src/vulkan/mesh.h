/// \file mesh.h

#pragma once

#include "objects/vertex_buffer.h"
#include "structures/transform_ubo.h"
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
    
    /*  Dynamic object  */
    std::shared_ptr<Shader> sh_;

    //std::unique_ptr<Buffer> MaterialBuffer;
    std::vector<VkDescriptorSet>  desSet_;
    std::unique_ptr<TransformUBO> tr_;
};

} // namespace Multor::Vulkan
