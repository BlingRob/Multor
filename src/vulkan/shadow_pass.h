/// \file shadow_pass.h

#pragma once

#include "shadow_resources.h"

#include <memory>
#include <vector>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class ShadowPass
{
public:
    ShadowPass(VkDevice device, VkFormat depthFormat);
    ~ShadowPass();

    ShadowPass(const ShadowPass&) = delete;
    ShadowPass& operator=(const ShadowPass&) = delete;

    void BuildFramebuffers(const ShadowResources& resources,
                           const ShadowMapArray& directionalMaps,
                           const ShadowMapArray& pointMaps);
    void ClearFramebuffers();

    VkRenderPass GetRenderPass() const
    {
        return renderPass_;
    }

    const std::vector<VkFramebuffer>& GetDirectionalFramebuffers() const
    {
        return directionalFramebuffers_;
    }

    const std::vector<VkFramebuffer>& GetPointFramebuffers() const
    {
        return pointFramebuffers_;
    }

private:
    void createRenderPass();
    void destroyRenderPass();
    VkFramebuffer createFramebuffer(VkImageView depthView, uint32_t width,
                                    uint32_t height) const;

private:
    VkDevice    device_;
    VkFormat    depthFormat_ = VK_FORMAT_UNDEFINED;
    VkRenderPass renderPass_ = VK_NULL_HANDLE;

    std::vector<VkImageView> directionalLayerViews_;
    std::vector<VkImageView> pointLayerViews_;
    std::vector<VkFramebuffer> directionalFramebuffers_;
    std::vector<VkFramebuffer> pointFramebuffers_;
};

} // namespace Multor::Vulkan
