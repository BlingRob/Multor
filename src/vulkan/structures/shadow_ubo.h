/// \file shadow_ubo.h

#pragma once

#include "../../scene_objects/light.h"

#include <array>
#include <vector>

#include <glm/glm.hpp>

namespace Multor::Vulkan::UBOs
{

constexpr std::size_t MaxPointShadowLights       = 5;
constexpr std::size_t MaxDirectionalShadowLights = 10;

struct alignas(16) DirectionalShadowEntry
{
    alignas(16) glm::mat4 lightSpace_ {};
    // x = shadow id, y = light slot, z = enabled, w = reserved
    alignas(16) glm::ivec4 meta_ {-1, -1, 0, 0};
};

struct alignas(16) DirectionalShadows
{
    // x = count
    alignas(16) glm::ivec4 counts_ {0, 0, 0, 0};
    std::array<DirectionalShadowEntry, MaxDirectionalShadowLights> entries_ {};
};

struct alignas(16) PointShadowEntry
{
    alignas(16) std::array<glm::mat4, 6> shadowMatrices_ {};
    // xyz = light position, w = far plane
    alignas(16) glm::vec4 lightPosFar_ {};
    // x = shadow id, y = light slot, z = enabled, w = reserved
    alignas(16) glm::ivec4 meta_ {-1, -1, 0, 0};
};

struct alignas(16) PointShadows
{
    // x = count
    alignas(16) glm::ivec4 counts_ {0, 0, 0, 0};
    std::array<PointShadowEntry, MaxPointShadowLights> entries_ {};
};

struct ShadowPack
{
    DirectionalShadows directional_;
    PointShadows       point_;
};

ShadowPack PackShadowData(const std::vector<const Multor::BLight*>& lights);

} // namespace Multor::Vulkan::UBOs
