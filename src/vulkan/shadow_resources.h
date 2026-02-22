/// \file shadow_resources.h

#pragma once

#include <cstdint>
#include <memory>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

struct ShadowMapArray
{
    VkDevice       device_     = VK_NULL_HANDLE;
    VkImage        image_      = VK_NULL_HANDLE;
    VkDeviceMemory memory_     = VK_NULL_HANDLE;
    VkImageView    view_       = VK_NULL_HANDLE;
    VkSampler      sampler_    = VK_NULL_HANDLE;
    VkFormat       format_     = VK_FORMAT_UNDEFINED;
    uint32_t       width_      = 0;
    uint32_t       height_     = 0;
    uint32_t       layers_     = 0;
    bool           isCubeArray_ = false;

    ~ShadowMapArray();

    ShadowMapArray() = default;
    ShadowMapArray(const ShadowMapArray&) = delete;
    ShadowMapArray& operator=(const ShadowMapArray&) = delete;
    ShadowMapArray(ShadowMapArray&& other) noexcept;
    ShadowMapArray& operator=(ShadowMapArray&& other) noexcept;
};

class ShadowResources
{
public:
    ShadowResources(VkDevice device, VkPhysicalDevice physicalDevice);

    ShadowMapArray CreateDirectionalShadowArray(uint32_t mapSize = 1024,
                                                uint32_t layers  = 10) const;
    ShadowMapArray CreatePointShadowCubeArray(uint32_t mapSize = 1024,
                                              uint32_t lights  = 5) const;
    VkImageView CreateArrayLayerView(const ShadowMapArray& array,
                                     uint32_t baseLayer,
                                     uint32_t layerCount = 1) const;
    VkImageView CreateCubeFaceView(const ShadowMapArray& cubeArray,
                                   uint32_t lightIndex,
                                   uint32_t faceIndex) const;

    VkFormat FindDepthFormat() const;

private:
    uint32_t findMemoryType(uint32_t typeFilter,
                            VkMemoryPropertyFlags properties) const;
    VkFormat findSupportedFormat(const VkFormat* candidates, uint32_t count,
                                 VkImageTiling tiling,
                                 VkFormatFeatureFlags features) const;

    ShadowMapArray createDepthArray(uint32_t width, uint32_t height,
                                    uint32_t layers, bool cubeArray) const;
    VkSampler createShadowSampler() const;

private:
    VkDevice         device_;
    VkPhysicalDevice physicalDevice_;
};

} // namespace Multor::Vulkan
