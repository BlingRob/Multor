/// \file VkFrameChain.cpp
#include "VkFrameChain.h"

namespace Multor
{

VkPresentModeKHR chooseSwapPresentMode(
    const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    if (std::find(availablePresentModes.cbegin(), availablePresentModes.cend(),
                  VK_PRESENT_MODE_MAILBOX_KHR) != availablePresentModes.cend())
        return VK_PRESENT_MODE_MAILBOX_KHR;
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
                availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                return availableFormat;
        }
    return availableFormats[0];
}

VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != UINT32_MAX)
        return capabilities.currentExtent;
    else
        {
            SDL_DisplayMode DM;
            SDL_GetCurrentDisplayMode(0, &DM);

            VkExtent2D actualExtent = {static_cast<uint32_t>(DM.w),
                                       static_cast<uint32_t>(DM.h)};

            actualExtent.width  = std::clamp(actualExtent.width,
                                             capabilities.minImageExtent.width,
                                             capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(
                actualExtent.height, capabilities.minImageExtent.height,
                capabilities.maxImageExtent.height);

            return actualExtent;
        }
}

void FrameChain::createSwapChain()
{
    //Looking for physical device parametrs - formats
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    //Choose needing surface format
    VkSurfaceFormatKHR surfaceFormat =
        chooseSwapSurfaceFormat(swapChainSupport.formats);
    //Set present mode
    VkPresentModeKHR presentMode =
        chooseSwapPresentMode(swapChainSupport.presentModes);
    //Set resolution of window
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    //Set amount swap buffers
    uint32_t imageCount =
        std::min(swapChainSupport.capabilities.minImageCount + 1u,
                 std::max(swapChainSupport.capabilities.maxImageCount, 0u));

    VkSwapchainCreateInfoKHR createInfo {};
    createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.pNext            = nullptr;
    createInfo.surface          = surface;
    createInfo.minImageCount    = imageCount;
    createInfo.imageFormat      = surfaceFormat.format;
    createInfo.imageColorSpace  = surfaceFormat.colorSpace;
    createInfo.imageExtent      = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {physicDevIndices.graphicsFamily.value(),
                                     physicDevIndices.presentFamily.value()};

    if (queueFamilyIndices[0] != queueFamilyIndices[1])
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
    else
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;       // Optional
            createInfo.pQueueFamilyIndices   = nullptr; // Optional
        }
    createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode    = presentMode;
    createInfo.clipped        = VK_TRUE;
    createInfo.oldSwapchain   = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) !=
        VK_SUCCESS)
        throw std::runtime_error("Failed to create swap chain!");
    //Get window's images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount,
                            swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent      = extent;
}

void FrameChain::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); ++i)
        //Create "descriptor" of window's buffer images
        swapChainImageViews[i] =
            MeshFac->createImageView(swapChainImages[i], swapChainImageFormat,
                                     VK_IMAGE_ASPECT_COLOR_BIT);
}

void FrameChain::createRenderPass()
{
    //Options for output images
    VkAttachmentDescription colorAttachment {};
    colorAttachment.format         = swapChainImageFormat;
    colorAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    //Set first index for colour output
    VkAttachmentReference colorAttachmentRef {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    //Depth attachment
    VkAttachmentDescription depthAttachment {};
    depthAttachment.format         = MeshFac->findDepthFormat();
    depthAttachment.samples        = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp        = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef {};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout =
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    //Execute one pass
    VkSubpassDescription subpass {};
    subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount    = 1;
    subpass.pColorAttachments       = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    //Orientation on colour output
    VkSubpassDependency dependency {};
    dependency.srcSubpass   = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass   = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                              VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

    //Collect everything and create Render pass
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment,
                                                          depthAttachment};
    VkRenderPassCreateInfo                 renderPassInfo {};
    renderPassInfo.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext           = nullptr;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments    = attachments.data();
    renderPassInfo.subpassCount    = 1;
    renderPassInfo.pSubpasses      = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies   = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to create render pass!");
}

void FrameChain::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    //bind and create framebuffer
    for (std::size_t i = 0; i < swapChainImageViews.size(); ++i)
        {
            std::array<VkImageView, 2> attachments = {swapChainImageViews[i],
                                                      DepthImg->view_};

            VkFramebufferCreateInfo framebufferInfo {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.pNext = nullptr;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount =
                static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width        = swapChainExtent.width;
            framebufferInfo.height       = swapChainExtent.height;
            framebufferInfo.layers       = 1;

            if (vkCreateFramebuffer(device, &framebufferInfo, nullptr,
                                    &swapChainFramebuffers[i]) != VK_SUCCESS)
                throw std::runtime_error("failed to create framebuffer!");
        }
}

void FrameChain::CleanUpSwapChain()
{
    vkDeviceWaitIdle(device);

    DepthImg.reset();

    for (auto& framebuffer : swapChainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr), framebuffer = 0;

    vkDestroyRenderPass(device, renderPass, nullptr),
        renderPass = VK_NULL_HANDLE;

    for (auto& imageView : swapChainImageViews)
        vkDestroyImageView(device, imageView, nullptr),
            imageView = VK_NULL_HANDLE;

    vkDestroySwapchainKHR(device, swapChain, nullptr),
        swapChain = VK_NULL_HANDLE;
}

void FrameChain::RecreateSwapChain()
{
    vkDeviceWaitIdle(device);

    CleanUpSwapChain();

    createSwapChain();
    createImageViews();
    createRenderPass();
    DepthImg = MeshFac->createDepthTexture(swapChainExtent.width,
                                           swapChainExtent.height);
    createFramebuffers();
}

} // namespace Multor