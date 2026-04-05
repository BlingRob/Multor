/// \file render_options_ubo.h

#pragma once

#include "../objects/buffer.h"

#include <glm/glm.hpp>
#include <memory>
#include <vector>

namespace Multor::Vulkan
{

enum class PbrDebugView : int
{
    Shaded = 0,
    BaseColor = 1,
    Normal = 2,
    Metallic = 3,
    Roughness = 4,
    AO = 5,
    Emissive = 6,
    EnvDiffuse = 7,
    EnvSpecular = 8,
    GeomNormal = 9,
    Tangent = 10,
    Bitangent = 11,
    NormalDelta = 12
};

struct PbrEnvironmentSettings
{
    float diffuseAmbientIntensity_ = 0.18f;
    float specularAmbientIntensity_ = 0.35f;
    float envFresnelStrength_ = 1.0f;
    float envReflectionPower_ = 1.0f;
    float roughnessAwareBlurStrength_ = 1.0f;
    float prefilterSampleRadiusScale_ = 1.0f;
    float prefilterCenterWeightScale_ = 1.0f;
    float prefilterRingWeightScale_ = 1.0f;
    float envSpecularUsesGeometricNormal_ = 1.0f;
    float directLightingUsesNormalMap_ = 1.0f;
};

namespace UBOs
{

struct alignas(16) RenderOptions
{
    alignas(16) glm::ivec4 options_ {0, 0, 0, 0};
    alignas(16) glm::vec4 pbrEnvironment_ {0.18f, 0.35f, 1.0f, 1.0f};
    alignas(16) glm::vec4 pbrEnvironment2_ {1.0f, 0.0f, 0.0f, 0.0f};
};

} // namespace UBOs

struct RenderOptionsUBO
{
    explicit RenderOptionsUBO(VkDevice dev) : dev_(dev)
    {
    }

    void update(std::size_t frame, PbrDebugView debugView,
                const PbrEnvironmentSettings& pbrEnvironment);

    std::vector<std::unique_ptr<Buffer> > buffers_;

    static const VkDeviceSize BufObj;

private:
    VkDevice dev_;
};

} // namespace Multor::Vulkan
