/// \file light_ubo.cpp

#include "light_ubo.h"

#include <cstring>
#include <stdexcept>

namespace Multor::Vulkan
{

const VkDeviceSize LightsUBO::LightsBufObj = sizeof(UBOs::Lights);

namespace
{

void incrementLightCount(UBOs::Lights& dst, Multor::LightType type)
{
    switch (type)
        {
            case Multor::LightType::Directional:
                ++dst.counts_.x;
                break;
            case Multor::LightType::Point:
                ++dst.counts_.y;
                break;
            case Multor::LightType::Spot:
                ++dst.counts_.z;
                break;
            case Multor::LightType::None:
            default:
                break;
        }
    ++dst.counts_.w;
}

std::size_t resolveSlot(const Multor::BLight& light, const UBOs::Lights& dst)
{
    const int32_t slot = light.GetLightSlot();
    if (slot >= 0 && static_cast<std::size_t>(slot) < UBOs::MaxLights)
        return static_cast<std::size_t>(slot);

    for (std::size_t i = 0; i < UBOs::MaxLights; ++i)
        {
            if (dst.lights_[i].meta_.z == 0)
                return i;
        }

    return UBOs::MaxLights;
}

} // namespace

UBOs::Light PackLight(const Multor::BLight& light)
{
    UBOs::Light out {};

    out.ambient_     = glm::vec4(light.GetAmbient(), 1.0f);
    out.diffuse_     = glm::vec4(light.GetDiffuse(), 1.0f);
    out.specular_    = glm::vec4(light.GetSpecular(), 1.0f);
    out.attenuation_ = glm::vec4(light.GetAttenuation(), 0.0f);
    out.meta_.x      = static_cast<int32_t>(light.GetType());
    out.meta_.y      = light.GetLightSlot();
    out.meta_.z      = 1;

    if (const auto* point = dynamic_cast<const Multor::PointLight*>(&light))
        {
            out.lightVec_ = glm::vec4(point->GetPos(), 1.0f);
            return out;
        }

    if (const auto* spot = dynamic_cast<const Multor::SpotLight*>(&light))
        {
            out.lightVec_ = glm::vec4(spot->GetDir(), 0.0f);
            const auto [outerAngle, innerAngle] = spot->GetAngles();
            out.spotDirection_ = glm::vec4(spot->GetDir(), 0.0f);
            out.spotPosition_  = glm::vec4(spot->GetPos(), 1.0f);
            out.spotAngles_    = glm::vec4(outerAngle, innerAngle, 0.0f, 0.0f);
            return out;
        }

    if (const auto* dir =
            dynamic_cast<const Multor::DirectionalLight*>(&light))
        {
            out.lightVec_ = glm::vec4(dir->GetDir(), 0.0f);
            return out;
        }

    return out;
}

UBOs::Lights PackLights(const std::vector<const Multor::BLight*>& lights)
{
    UBOs::Lights out {};

    for (const auto* light : lights)
        {
            if (!light)
                continue;

            const std::size_t slot = resolveSlot(*light, out);
            if (slot >= UBOs::MaxLights)
                continue;

            out.lights_[slot]   = PackLight(*light);
            out.lights_[slot].meta_.y = static_cast<int32_t>(slot);
            incrementLightCount(out, light->GetType());
        }

    return out;
}

void LightsUBO::update(std::size_t frame, const UBOs::Lights& lights)
{
    if (frame >= buffers_.size() || !buffers_[frame])
        throw std::out_of_range("LightsUBO::update frame buffer is missing");

    void* data = nullptr;
    vkMapMemory(dev_, buffers_[frame]->bufferMemory_, 0, LightsBufObj, 0, &data);
    std::memcpy(data, &lights, sizeof(UBOs::Lights));
    vkUnmapMemory(dev_, buffers_[frame]->bufferMemory_);
}

} // namespace Multor::Vulkan
