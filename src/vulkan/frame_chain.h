/// \file frame_chain.h

#pragma once

#include "general_options.h"
#include "mesh_factory.h"

#include <algorithm>
#include <vector>
#include <array>

namespace Multor::Vulkan
{

class FrameChain : public BaseStructs
{
public:
    FrameChain(std::shared_ptr<Window>          pWnd);

    ~FrameChain();

    void CleanUpSwapChain();
    void RecreateSwapChain();

protected:
    Logging::Logger& logger_;

    VkRenderPass   renderPass;
    VkSwapchainKHR swapChain;
    VkFormat       swapChainImageFormat;
    VkExtent2D     swapChainExtent;

    std::unique_ptr<Texture> DepthImg;

    std::vector<VkImage>       swapChainImages;
    std::vector<VkImageView>   swapChainImageViews;
    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::shared_ptr<CommandExecuter> _executer;
    std::unique_ptr<MeshFactory>   MeshFac;

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
};

} // namespace Multor
