/// \file shadow_pass.cpp

#include "shadow_pass.h"

#include <array>
#include <stdexcept>

namespace Multor::Vulkan
{

ShadowPass::ShadowPass(VkDevice device, VkFormat depthFormat)
    : device_(device), depthFormat_(depthFormat)
{
    createRenderPass();
}

ShadowPass::~ShadowPass()
{
    ClearFramebuffers();
    destroyRenderPass();
}

void ShadowPass::BuildFramebuffers(const ShadowResources& resources,
                                   const ShadowMapArray& directionalMaps,
                                   const ShadowMapArray& pointMaps)
{
    ClearFramebuffers();

    for (uint32_t layer = 0; layer < directionalMaps.layers_; ++layer)
        {
            VkImageView view =
                resources.CreateArrayLayerView(directionalMaps, layer, 1);
            directionalLayerViews_.push_back(view);
            directionalFramebuffers_.push_back(
                createFramebuffer(view, directionalMaps.width_,
                                  directionalMaps.height_));
        }

    for (uint32_t layer = 0; layer < pointMaps.layers_; ++layer)
        {
            VkImageView view = resources.CreateArrayLayerView(pointMaps, layer, 1);
            pointLayerViews_.push_back(view);
            pointFramebuffers_.push_back(
                createFramebuffer(view, pointMaps.width_, pointMaps.height_));
        }
}

void ShadowPass::ClearFramebuffers()
{
    for (auto& fb : directionalFramebuffers_)
        {
            vkDestroyFramebuffer(device_, fb, nullptr);
            fb = VK_NULL_HANDLE;
        }
    directionalFramebuffers_.clear();

    for (auto& fb : pointFramebuffers_)
        {
            vkDestroyFramebuffer(device_, fb, nullptr);
            fb = VK_NULL_HANDLE;
        }
    pointFramebuffers_.clear();

    for (auto& view : directionalLayerViews_)
        {
            vkDestroyImageView(device_, view, nullptr);
            view = VK_NULL_HANDLE;
        }
    directionalLayerViews_.clear();

    for (auto& view : pointLayerViews_)
        {
            vkDestroyImageView(device_, view, nullptr);
            view = VK_NULL_HANDLE;
        }
    pointLayerViews_.clear();
}

void ShadowPass::createRenderPass()
{
    VkAttachmentDescription depthAttachment {};
    depthAttachment.flags          = 0;
    depthAttachment.format         = depthFormat_;
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthRef {};
    depthRef.attachment = 0;
    depthRef.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 0;
    subpass.pColorAttachments       = nullptr;
    subpass.pDepthStencilAttachment = &depthRef;

    VkSubpassDependency dependency {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask =
        VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask =
        VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependency.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpInfo {};
    rpInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rpInfo.pNext           = nullptr;
    rpInfo.attachmentCount = 1;
    rpInfo.pAttachments    = &depthAttachment;
    rpInfo.subpassCount    = 1;
    rpInfo.pSubpasses      = &subpass;
    rpInfo.dependencyCount = 1;
    rpInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(device_, &rpInfo, nullptr, &renderPass_) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow render pass");
}

void ShadowPass::destroyRenderPass()
{
    if (renderPass_ != VK_NULL_HANDLE)
        {
            vkDestroyRenderPass(device_, renderPass_, nullptr);
            renderPass_ = VK_NULL_HANDLE;
        }
}

VkFramebuffer ShadowPass::createFramebuffer(VkImageView depthView, uint32_t width,
                                            uint32_t height) const
{
    VkFramebuffer fb = VK_NULL_HANDLE;
    VkFramebufferCreateInfo fbInfo {};
    fbInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fbInfo.pNext           = nullptr;
    fbInfo.renderPass      = renderPass_;
    fbInfo.attachmentCount = 1;
    fbInfo.pAttachments    = &depthView;
    fbInfo.width           = width;
    fbInfo.height          = height;
    fbInfo.layers          = 1;

    if (vkCreateFramebuffer(device_, &fbInfo, nullptr, &fb) != VK_SUCCESS)
        throw std::runtime_error("failed to create shadow framebuffer");

    return fb;
}

} // namespace Multor::Vulkan
