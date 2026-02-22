/// \file light_ubo.h

#pragma once

#include "../objects/buffer.h"
#include "../../scene_objects/light.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

namespace UBOs
{

constexpr std::size_t MaxLights = 16;

struct alignas(16) Light
{
    // xyz = position or direction, w = 0 (direction) / 1 (position)
    alignas(16) glm::vec4 lightVec_ {};
    alignas(16) glm::vec4 ambient_ {};
    alignas(16) glm::vec4 diffuse_ {};
    alignas(16) glm::vec4 specular_ {};
    // xyz = attenuation (constant/linear/quadratic), w = reserved
    alignas(16) glm::vec4 attenuation_ {};
    // x = type, y = slot, z = enabled(0/1), w = reserved
    alignas(16) glm::ivec4 meta_ {0, -1, 0, 0};
    // Spot-only data:
    // spotDirection_.xyz, spotPosition_.xyz, spotAngles_.xy (outer, inner)
    alignas(16) glm::vec4 spotDirection_ {};
    alignas(16) glm::vec4 spotPosition_ {};
    alignas(16) glm::vec4 spotAngles_ {};
};

struct alignas(16) Lights
{
    // x = directional count, y = point count, z = spot count, w = total
    alignas(16) glm::ivec4 counts_ {0, 0, 0, 0};
    std::array<Light, MaxLights> lights_ {};
};

} // namespace UBOs

UBOs::Light PackLight(const Multor::BLight& light);
UBOs::Lights PackLights(const std::vector<const Multor::BLight*>& lights);

struct LightsUBO
{
    explicit LightsUBO(VkDevice dev) : dev_(dev)
    {
    }

    void update(std::size_t frame, const UBOs::Lights& lights);

    std::vector<std::unique_ptr<Buffer> > buffers_;

    static const VkDeviceSize LightsBufObj;

private:
    VkDevice dev_;
};

} // namespace Multor::Vulkan
