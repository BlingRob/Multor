/// \file material_ubo.cpp

#include "material_ubo.h"

namespace Multor::Vulkan::UBOs
{

namespace
{
int HasTexture(const Multor::BaseMesh* mesh, Multor::Texture_Types type)
{
    return (mesh && mesh->FindTexture(type)) ? 1 : 0;
}
}

MaterialData PackMaterial(const Multor::Material& material,
                          const Multor::BaseMesh* mesh)
{
    MaterialData out {};
    out.baseColorFactor_ = material.baseColorFactor;
    out.emissiveFactor_ =
        glm::vec4(material.emissiveFactor, 0.0f);
    out.pbrFactors_ = glm::vec4(material.metallicFactor,
                                material.roughnessFactor,
                                material.normalScale,
                                material.aoStrength);
    out.textureFlags0_ =
        glm::ivec4(static_cast<int>(material.shadingModel),
                   HasTexture(mesh, Multor::Texture_Types::Diffuse),
                   HasTexture(mesh, Multor::Texture_Types::Normal),
                   HasTexture(mesh, Multor::Texture_Types::Metallic));
    out.textureFlags1_ =
        glm::ivec4(HasTexture(mesh, Multor::Texture_Types::Roughness),
                   HasTexture(mesh, Multor::Texture_Types::Metallic_roughness),
                   HasTexture(mesh, Multor::Texture_Types::Ambient_occlusion),
                   HasTexture(mesh, Multor::Texture_Types::Emissive));
    out.misc_ = glm::vec4(material.alphaCutoff, 0.0f, 0.0f, 0.0f);

    out.ambient_ = glm::vec4(material.ambient, 0.0f);
    out.diffuse_ = glm::vec4(material.diffuse, 0.0f);
    out.specular_ = glm::vec4(material.specular, 0.0f);
    out.phong_ = glm::vec4(material.shininess, 0.0f, 0.0f, 0.0f);
    return out;
}

} // namespace Multor::Vulkan::UBOs
