/// \file texture_factory.h

#pragma once

#include "../utils/image.h"
#include "buffer_factory.h"
#include "objects/texture.h"

#include <tuple>
#include <exception>
#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class TextureFactory : public BufferFactory
{
public:
    TextureFactory(VkDevice dev, VkPhysicalDevice PhysDev,
                     std::shared_ptr<CommandExecuter> ex)
        : BufferFactory(dev, PhysDev, ex)
    {
    }
    Texture*                 CreateTexture(Image* img);
    std::unique_ptr<Texture> CreateDepthTexture(std::uint32_t width,
                                                  std::uint32_t height);

    VkImageView CreateImageView(VkImage image, VkFormat format,
                                VkImageAspectFlags aspectFlags);
    std::pair<VkImage, VkDeviceMemory>
                CreateImage(uint32_t width, uint32_t height, VkFormat format,
                            VkImageTiling tiling, VkImageUsageFlags usage,
                            VkMemoryPropertyFlags properties);
    VkImageView CreateTextureImageView(VkImage img);
    VkSampler   CreateTextureSampler();
    VkFormat    FindDepthFormat();

private:
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates,
                                 VkImageTiling                tiling,
                                 VkFormatFeatureFlags         features);
};

} // namespace Multor::Vulkan
