/// \file VkTexture.cpp
#include "VkTexture.h"

namespace Multor
{

VkImageView VkTextureFactory::createImageView(VkImage image, VkFormat format,
                                              VkImageAspectFlags aspectFlags)
{
    VkImageViewCreateInfo viewInfo {};
    viewInfo.sType    = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.pNext    = nullptr;
    viewInfo.image    = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format   = format;
    viewInfo.subresourceRange.aspectMask     = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel   = 0;
    viewInfo.subresourceRange.levelCount     = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount     = 1;

    VkImageView imageView;
    if (vkCreateImageView(dev_, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create texture image view!");

    return imageView;
}

VkSampler VkTextureFactory::createTextureSampler()
{
    VkSampler textureSampler;

    VkSamplerCreateInfo samplerInfo {};
    samplerInfo.sType            = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter        = VK_FILTER_LINEAR;
    samplerInfo.minFilter        = VK_FILTER_LINEAR;
    samplerInfo.addressModeU     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW     = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    VkPhysicalDeviceProperties properties {};
    vkGetPhysicalDeviceProperties(physDev, &properties);
    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor   = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable           = VK_FALSE;
    samplerInfo.compareOp               = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode              = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias              = 0.0f;
    samplerInfo.minLod                  = 0.0f;
    samplerInfo.maxLod                  = 0.0f;
    samplerInfo.anisotropyEnable        = VK_FALSE;
    samplerInfo.maxAnisotropy           = 1.0f;

    if (vkCreateSampler(dev_, &samplerInfo, nullptr, &textureSampler) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create texture sampler!");
    return textureSampler;
}

VkTexture* VkTextureFactory::createTexture(Image* img)
{
    //std::shared_ptr<Image> img = ImageLoader::LoadTexture("A:/VulkanEngine/build/matrix.jpg");

    if (img->empty())
        throw std::runtime_error("failed to image load!");

    VkDeviceSize imageSize = img->w_ * img->h_ * 4;

    std::unique_ptr<VulkanBuffer> stBuf =
        createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(dev_, stBuf->bufferMemory_, 0, imageSize, 0, &data);
    std::memcpy(data, img->mdata_, static_cast<size_t>(imageSize));
    vkUnmapMemory(dev_, stBuf->bufferMemory_);

    std::pair<VkImage, VkDeviceMemory> texture = createImage(
        img->w_, img->h_, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    _executer->transitionImageLayout(texture.first, VK_FORMAT_R8G8B8A8_SRGB,
                                     VK_IMAGE_LAYOUT_UNDEFINED,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    _executer->copyBufferToImage(stBuf->buffer_, texture.first,
                                 static_cast<uint32_t>(img->w_),
                                 static_cast<uint32_t>(img->h_));
    _executer->transitionImageLayout(texture.first, VK_FORMAT_R8G8B8A8_SRGB,
                                     VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    VkTexture* val = new VkTexture;
    val->dev_      = dev_;
    val->img_      = texture.first;
    val->view_     = createTextureImageView(texture.first);
    val->sampler_  = createTextureSampler();
    val->devMem_   = texture.second;
    return val;
}

std::unique_ptr<VkTexture>
VkTextureFactory::createDepthTexture(std::size_t width, std::size_t height)
{
    std::unique_ptr<VkTexture> depthTex = std::make_unique<VkTexture>();
    depthTex->dev_                      = dev_;

    VkFormat depthFormat = findDepthFormat();
    auto [depth, depthMemory] =
        createImage(width, height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    depthTex->view_ =
        createImageView(depth, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    depthTex->img_    = depth;
    depthTex->devMem_ = depthMemory;

    _executer->transitionImageLayout(
        depthTex->img_, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

    return depthTex;
}

std::pair<VkImage, VkDeviceMemory>
VkTextureFactory::createImage(uint32_t width, uint32_t height, VkFormat format,
                              VkImageTiling tiling, VkImageUsageFlags usage,
                              VkMemoryPropertyFlags properties)
{
    std::pair<VkImage, VkDeviceMemory> image;

    VkImageCreateInfo imageInfo {};
    imageInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType     = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width  = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth  = 1;
    imageInfo.mipLevels     = 1;
    imageInfo.arrayLayers   = 1;
    imageInfo.format        = format;
    imageInfo.tiling        = tiling;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage         = usage;
    imageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(dev_, &imageInfo, nullptr, &image.first) != VK_SUCCESS)
        throw std::runtime_error("failed to create image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(dev_, image.first, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(dev_, &allocInfo, nullptr, &image.second) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate image memory!");

    vkBindImageMemory(dev_, image.first, image.second, 0);
    return image;
}

VkImageView VkTextureFactory::createTextureImageView(VkImage img)
{
    return createImageView(img, VK_FORMAT_R8G8B8A8_SRGB,
                           VK_IMAGE_ASPECT_COLOR_BIT);
}

VkFormat
VkTextureFactory::findSupportedFormat(const std::vector<VkFormat>& candidates,
                                      VkImageTiling                tiling,
                                      VkFormatFeatureFlags         features)
{
    for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physDev, format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR &&
                (props.linearTilingFeatures & features) == features)
                return format;
            else if (tiling == VK_IMAGE_TILING_OPTIMAL &&
                     (props.optimalTilingFeatures & features) == features)
                return format;
        }

    throw std::runtime_error("failed to find supported format!");
}

VkFormat VkTextureFactory::findDepthFormat()
{
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT,
                                VK_FORMAT_D32_SFLOAT_S8_UINT,
                                VK_FORMAT_D24_UNORM_S8_UINT},
                               VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

} // namespace Multor