/// \file render_options_ubo.cpp

#include "render_options_ubo.h"

#include <cstring>

namespace Multor::Vulkan
{

const VkDeviceSize RenderOptionsUBO::BufObj = sizeof(UBOs::RenderOptions);

void RenderOptionsUBO::update(std::size_t frame, PbrDebugView debugView,
                              const PbrEnvironmentSettings& pbrEnvironment)
{
    UBOs::RenderOptions options {};
    options.options_.x = static_cast<int>(debugView);
    options.pbrEnvironment_ = glm::vec4(
        pbrEnvironment.diffuseAmbientIntensity_,
        pbrEnvironment.specularAmbientIntensity_,
        pbrEnvironment.envFresnelStrength_,
        pbrEnvironment.envReflectionPower_);
    options.pbrEnvironment2_ = glm::vec4(
        pbrEnvironment.roughnessAwareBlurStrength_,
        pbrEnvironment.prefilterSampleRadiusScale_,
        pbrEnvironment.prefilterCenterWeightScale_,
        pbrEnvironment.prefilterRingWeightScale_);
    options.options_.y =
        pbrEnvironment.envSpecularUsesGeometricNormal_ >= 0.5f ? 1 : 0;
    options.options_.z =
        pbrEnvironment.directLightingUsesNormalMap_ >= 0.5f ? 1 : 0;

    void* data = nullptr;
    vkMapMemory(dev_, buffers_[frame]->bufferMemory_, 0, BufObj, 0, &data);
    std::memcpy(data, &options, sizeof(options));
    vkUnmapMemory(dev_, buffers_[frame]->bufferMemory_);
}

} // namespace Multor::Vulkan
