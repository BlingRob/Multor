/// \file VkTexture.h

#pragma once
#ifndef VKTEXTURE_H
#define VKTEXTURE_H

#include "ImageLoader.h"
#include "VkBufferFactory.h"

#include <tuple>
#include <exception>
#include <stdexcept>
#include <memory>

#include <vulkan/vulkan.h>

namespace Multor
{

class VkTextureFactory:public VkBufferFactory
{
public:
	VkTextureFactory(VkDevice dev, VkPhysicalDevice PhysDev, std::shared_ptr<CommandExecuter> ex):VkBufferFactory(dev, PhysDev, ex)
	{}
	VkTexture* createTexture(Image* img);
	std::unique_ptr<VkTexture> createDepthTexture(std::size_t width, std::size_t height);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	std::pair<VkImage, VkDeviceMemory> createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling,
		VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
	VkImageView createTextureImageView(VkImage img);
	VkSampler createTextureSampler();
	VkFormat findDepthFormat();
private:
	VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
};

} // namespace Multor

#endif // VKTEXTURE_H