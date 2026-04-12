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
    NormalDelta = 12,
    ShadowFactor = 13,
    ShadowInputDelta = 14,
    ShadowNdotL = 15,
    ShadowBiasHeatmap = 16,
    ShadowVisibilityRaw = 17,
    PointShadowFace = 18,
    PointShadowDistanceRatio = 19,
    PointCompareDepth = 20
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

struct ShadowSettings
{
    float strength_ = 0.5f;
    float directionalBiasScale_ = 1.25f;
    float pointBiasScale_ = 1.4f;
    float pointNormalOffsetScale_ = 1.15f;
    float pointPcfSpreadScale_ = 1.0f;
    float minDirectionalVisibility_ = 0.45f;
    float minPointVisibility_ = 0.52f;
    float directionalPcfSpreadScale_ = 1.0f;
    float directionalNormalOffsetScale_ = 1.2f;
    float pointTerminatorNormalScale_ = 0.6f;
    float pointTerminatorGeometryScale_ = 0.25f;
    float directionalTerminatorNormalScale_ = 0.45f;
    float directionalTerminatorGeometryScale_ = 0.15f;
    float directionalRasterBiasConstant_ = 2.5f;
    float directionalRasterBiasSlope_ = 10.0f;
    float pointRasterBiasConstant_ = 1.1f;
    float pointRasterBiasSlope_ = 4.0f;
};

namespace UBOs
{

struct alignas(16) RenderOptions
{
    alignas(16) glm::ivec4 options_ {0, 0, 0, 0};
    alignas(16) glm::vec4 pbrEnvironment_ {0.18f, 0.35f, 1.0f, 1.0f};
    alignas(16) glm::vec4 pbrEnvironment2_ {1.0f, 1.0f, 1.0f, 1.0f};
    alignas(16) glm::vec4 shadowSettings0_ {0.5f, 1.25f, 1.4f, 1.15f};
    alignas(16) glm::vec4 shadowSettings1_ {1.0f, 0.45f, 0.52f, 1.0f};
    alignas(16) glm::vec4 shadowSettings2_ {1.2f, 0.6f, 0.25f, 0.45f};
    alignas(16) glm::vec4 shadowSettings3_ {0.15f, 1.0f, 0.0f, 0.0f};
};

} // namespace UBOs

struct RenderOptionsUBO
{
    explicit RenderOptionsUBO(VkDevice dev) : dev_(dev)
    {
    }

    void update(std::size_t frame, PbrDebugView debugView,
                const PbrEnvironmentSettings& pbrEnvironment,
                const ShadowSettings& shadowSettings,
                int debugShadowLightSlot);

    std::vector<std::unique_ptr<Buffer> > buffers_;

    static const VkDeviceSize BufObj;

private:
    VkDevice dev_;
};

} // namespace Multor::Vulkan
