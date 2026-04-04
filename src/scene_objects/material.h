/// \file material.h

#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <cstdint>
//#include "assimp/scene.h"
//#include "Loaders/image_loader.h"

namespace Multor
{

enum class ShadingModel : std::uint8_t
{
    LegacyPhong = 0,
    MetallicRoughnessPBR = 1
};

struct Material
{
    Material()
        : ambient(0.0f)
        , diffuse(0.0f)
        , specular(0.0f)
        , shininess(0.0f)
        , shadingModel(ShadingModel::LegacyPhong)
        , baseColorFactor(1.0f, 1.0f, 1.0f, 1.0f)
        , metallicFactor(0.0f)
        , roughnessFactor(1.0f)
        , normalScale(1.0f)
        , aoStrength(1.0f)
        , alphaCutoff(0.5f)
        , emissiveFactor(0.0f)
    {
    }

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     shininess;

    ShadingModel shadingModel;
    glm::vec4    baseColorFactor;
    float        metallicFactor;
    float        roughnessFactor;
    float        normalScale;
    float        aoStrength;
    float        alphaCutoff;
    glm::vec3    emissiveFactor;

    void UseLegacyPhong()
    {
        shadingModel = ShadingModel::LegacyPhong;
    }

    void UseMetallicRoughnessPBR(const glm::vec4& baseColor = glm::vec4(1.0f),
                                 float metallic = 0.0f, float roughness = 1.0f)
    {
        shadingModel    = ShadingModel::MetallicRoughnessPBR;
        baseColorFactor = baseColor;
        metallicFactor  = metallic;
        roughnessFactor = roughness;
    }

    bool IsPBR() const
    {
        return shadingModel == ShadingModel::MetallicRoughnessPBR;
    }
};

} // namespace Multor

#endif // MATERIAL_H
