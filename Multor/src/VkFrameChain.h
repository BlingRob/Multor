/// \file VkFrameChain.h

#pragma once
#ifndef VKFRAMECHAIN_H
#define VKFRAMECHAIN_H

#include "VkGeneralOptions.h"
#include "Mesh.h"

#include <algorithm>
#include <vector>
#include <array>

namespace Multor
{

class FrameChain : public VkBaseStructs
{
public:
    FrameChain(std::shared_ptr<Window>          pWnd,
               std::shared_ptr<Logging::Logger> pLog)
        : VkBaseStructs(std::move(pWnd), std::move(pLog))
    {
        _executer = std::make_shared<CommandExecuter>(device, commandPool,
                                                      graphicsQueue);
        MeshFac = std::make_unique<VkMeshFactory>(device, physicDev, _executer);
        createSwapChain();
        createImageViews();
        createRenderPass();
        DepthImg = MeshFac->createDepthTexture(swapChainExtent.width,
                                               swapChainExtent.height);
        createFramebuffers();
    }

    ~FrameChain()
    {
        MeshFac.reset();
        _executer.reset();
        CleanUpSwapChain();
    }

    void CleanUpSwapChain();
    void RecreateSwapChain();

protected:
    VkRenderPass   renderPass;
    VkSwapchainKHR swapChain;
    VkFormat       swapChainImageFormat;
    VkExtent2D     swapChainExtent;

    std::unique_ptr<VkTexture> DepthImg;

    std::vector<VkImage>       swapChainImages;
    std::vector<VkImageView>   swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::shared_ptr<CommandExecuter> _executer;
    std::unique_ptr<VkMeshFactory>   MeshFac;

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
};

} // namespace Multor

#endif // VKFRAMECHAIN_H