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
        pbrEnvironment.roughnessAwareBlurStrength_, 0.0f, 0.0f, 0.0f);

    void* data = nullptr;
    vkMapMemory(dev_, buffers_[frame]->bufferMemory_, 0, BufObj, 0, &data);
    std::memcpy(data, &options, sizeof(options));
    vkUnmapMemory(dev_, buffers_[frame]->bufferMemory_);
}

} // namespace Multor::Vulkan
