/// \file material_ubo.h

#pragma once

#include "../../scene_objects/material.h"
#include "../../scene_objects/mesh.h"

#include <glm/glm.hpp>

namespace Multor::Vulkan::UBOs
{

struct alignas(16) MaterialData
{
    alignas(16) glm::vec4 baseColorFactor_ {1.0f, 1.0f, 1.0f, 1.0f};
    alignas(16) glm::vec4 emissiveFactor_ {0.0f, 0.0f, 0.0f, 0.0f};
    // x = metallic, y = roughness, z = normalScale, w = aoStrength
    alignas(16) glm::vec4 pbrFactors_ {0.0f, 1.0f, 1.0f, 1.0f};
    // x = shading model, y = baseColor, z = normal, w = metallic
    alignas(16) glm::ivec4 textureFlags0_ {0, 0, 0, 0};
    // x = roughness, y = metallicRoughnessPacked, z = ao, w = emissive
    alignas(16) glm::ivec4 textureFlags1_ {0, 0, 0, 0};
    // x = alphaCutoff, yzw reserved
    alignas(16) glm::vec4 misc_ {0.5f, 0.0f, 0.0f, 0.0f};
    // Legacy phong fallback
    alignas(16) glm::vec4 ambient_ {0.0f, 0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec4 diffuse_ {0.0f, 0.0f, 0.0f, 0.0f};
    alignas(16) glm::vec4 specular_ {0.0f, 0.0f, 0.0f, 0.0f};
    // x = shininess
    alignas(16) glm::vec4 phong_ {0.0f, 0.0f, 0.0f, 0.0f};
};

MaterialData PackMaterial(const Multor::Material& material,
                          const Multor::BaseMesh* mesh = nullptr);

} // namespace Multor::Vulkan::UBOs
