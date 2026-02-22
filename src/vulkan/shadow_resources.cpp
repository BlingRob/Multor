/// \file shadow_resources.cpp

#include "shadow_resources.h"

#include <array>
#include <stdexcept>
#include <utility>

namespace Multor::Vulkan
{

ShadowMapArray::~ShadowMapArray()
{
    if (device_ == VK_NULL_HANDLE)
        return;

    if (sampler_ != VK_NULL_HANDLE)
        vkDestroySampler(device_, sampler_, nullptr);
    if (view_ != VK_NULL_HANDLE)
        vkDestroyImageView(device_, view_, nullptr);
    if (image_ != VK_NULL_HANDLE)
        vkDestroyImage(device_, image_, nullptr);
    if (memory_ != VK_NULL_HANDLE)
        vkFreeMemory(device_, memory_, nullptr);

    sampler_ = VK_NULL_HANDLE;
    view_    = VK_NULL_HANDLE;
    image_   = VK_NULL_HANDLE;
    memory_  = VK_NULL_HANDLE;
    device_  = VK_NULL_HANDLE;
}

ShadowMapArray::ShadowMapArray(ShadowMapArray&& other) noexcept
{
    *this = std::move(other);
}

ShadowMapArray& ShadowMapArray::operator=(ShadowMapArray&& other) noexcept
{
    if (this == &other)
        return *this;

    if (device_ != VK_NULL_HANDLE)
        {
            if (sampler_ != VK_NULL_HANDLE)
                vkDestroySampler(device_, sampler_, nullptr);
            if (view_ != VK_NULL_HANDLE)
                vkDestroyImageView(device_, view_, nullptr);
            if (image_ != VK_NULL_HANDLE)
                vkDestroyImage(device_, image_, nullptr);
            if (memory_ != VK_NULL_HANDLE)
                vkFreeMemory(device_, memory_, nullptr);
        }

    device_      = other.device_;
    image_       = other.image_;
    memory_      = other.memory_;
    view_        = other.view_;
    sampler_     = other.sampler_;
    format_      = other.format_;
    width_       = other.width_;
    height_      = other.height_;
    layers_      = other.layers_;
    isCubeArray_ = other.isCubeArray_;

    other.device_      = VK_NULL_HANDLE;
    other.image_       = VK_NULL_HANDLE;
    other.memory_      = VK_NULL_HANDLE;
    other.view_        = VK_NULL_HANDLE;
    other.sampler_     = VK_NULL_HANDLE;
    other.format_      = VK_FORMAT_UNDEFINED;
    other.width_       = 0;
    other.height_      = 0;
    other.layers_      = 0;
    other.isCubeArray_ = false;

    return *this;
}

ShadowResources::ShadowResources(VkDevice device, VkPhysicalDevice physicalDevice)
    : device_(device), physicalDevice_(physicalDevice)
{
}

ShadowMapArray ShadowResources::CreateDirectionalShadowArray(uint32_t mapSize,
                                                             uint32_t layers) const
{
    return createDepthArray(mapSize, mapSize, layers, false);
}

ShadowMapArray ShadowResources::CreatePointShadowCubeArray(uint32_t mapSize,
                                                           uint32_t lights) const
{
    return createDepthArray(mapSize, mapSize, lights * 6u, true);
}

VkImageView ShadowResources::CreateArrayLayerView(const ShadowMapArray& array,
                                                  uint32_t baseLayer,
                                                  uint32_t layerCount) const
{
    if (array.image_ == VK_NULL_HANDLE)
        throw std::runtime_error("shadow array image is null");
    if (baseLayer + layerCount > array.layers_)
        throw std::out_of_range("shadow array layer view is out of range");

    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext    = nullptr;
    viewInfo.image    = array.image_;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format   = array.format_;
    viewInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = baseLayer;
    viewInfo.subresourceRange.layerCount     = layerCount;

    VkImageView view = VK_NULL_HANDLE;
    if (vkCreateImageView(device_, &viewInfo, nullptr, &view) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow array layer view");

    return view;
}

VkImageView ShadowResources::CreateCubeFaceView(const ShadowMapArray& cubeArray,
                                                uint32_t lightIndex,
                                                uint32_t faceIndex) const
{
    if (!cubeArray.isCubeArray_)
        throw std::runtime_error("CreateCubeFaceView requires cube shadow array");
    if (faceIndex >= 6)
        throw std::out_of_range("cube face index must be in [0,5]");

    const uint32_t baseLayer = lightIndex * 6u + faceIndex;
    return CreateArrayLayerView(cubeArray, baseLayer, 1);
}

VkFormat ShadowResources::FindDepthFormat() const
{
    constexpr std::array<VkFormat, 3> candidates = {
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D24_UNORM_S8_UINT,
    };

    return findSupportedFormat(candidates.data(),
                               static_cast<uint32_t>(candidates.size()),
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT |
                                   VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
}

uint32_t ShadowResources::findMemoryType(uint32_t typeFilter,
                                         VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties memProperties {};
    vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((typeFilter & (1u << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                    properties)
                return i;
        }

    throw std::runtime_error("failed to find suitable memory type for shadow");
}

VkFormat ShadowResources::findSupportedFormat(const VkFormat* candidates,
                                              uint32_t count,
                                              VkImageTiling tiling,
                                              VkFormatFeatureFlags features) const
{
    for (uint32_t i = 0; i < count; ++i)
        {
            VkFormatProperties props {};
            vkGetPhysicalDeviceFormatProperties(physicalDevice_, candidates[i],
                                                &props);

            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features)
                return candidates[i];

            if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                (props.optimalTilingFeatures & features) == features)
                return candidates[i];
        }

    throw std::runtime_error("failed to find supported depth format for shadow");
}

ShadowMapArray ShadowResources::createDepthArray(uint32_t width, uint32_t height,
                                                 uint32_t layers,
                                                 bool cubeArray) const
{
    ShadowMapArray out {};
    out.device_      = device_;
    out.width_       = width;
    out.height_      = height;
    out.layers_      = layers;
    out.isCubeArray_ = cubeArray;
    out.format_      = FindDepthFormat();

    VkImageCreateInfo imageInfo {};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.pNext         = nullptr;
    imageInfo.flags         = cubeArray ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.format        = out.format_;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = layers;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage         = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT |
                      VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    if (vkCreateImage(device_, &imageInfo, nullptr, &out.image_) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow image array");

    VkMemoryRequirements memRequirements {};
    vkGetImageMemoryRequirements(device_, out.image_, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType           = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext           = nullptr;
    allocInfo.allocationSize  = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(
        memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &out.memory_) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate shadow image memory");

    if (vkBindImageMemory(device_, out.image_, out.memory_, 0) != VK_SUCCESS)
        throw std::runtime_error("failed to bind shadow image memory");

    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext    = nullptr;
    viewInfo.image    = out.image_;
    viewInfo.viewType =
        cubeArray ? VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : VK_IMAGE_VIEW_TYPE_2D_ARRAY;
    viewInfo.format                              = out.format_;
    viewInfo.subresourceRange.aspectMask         = VK_IMAGE_ASPECT_DEPTH_BIT;
    viewInfo.subresourceRange.baseMipLevel       = 0;
    viewInfo.subresourceRange.levelCount         = 1;
    viewInfo.subresourceRange.baseArrayLayer     = 0;
    viewInfo.subresourceRange.layerCount         = layers;
    viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (vkCreateImageView(device_, &viewInfo, nullptr, &out.view_) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow image view");

    out.sampler_ = createShadowSampler();
    return out;
}

VkSampler ShadowResources::createShadowSampler() const
{
    VkSampler sampler = VK_NULL_HANDLE;

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.pNext        = nullptr;
    samplerInfo.magFilter    = VK_FILTER_LINEAR;
    samplerInfo.minFilter    = VK_FILTER_LINEAR;
    samplerInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    samplerInfo.mipLodBias   = 0.0f;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy    = 1.0f;
    samplerInfo.compareEnable    = VK_TRUE;
    samplerInfo.compareOp        = VK_COMPARE_OP_LESS_OR_EQUAL;
    samplerInfo.minLod           = 0.0f;
    samplerInfo.maxLod           = 0.0f;
    samplerInfo.borderColor      = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;

    if (vkCreateSampler(device_, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow sampler");

    return sampler;
}

} // namespace Multor::Vulkan
