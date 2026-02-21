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

    VkRenderPass   renderPass_          = VK_NULL_HANDLE;
    VkSwapchainKHR swapChain_           = VK_NULL_HANDLE;
    VkFormat       swapChainImageFormat_;
    VkExtent2D     swapChainExtent_;

    std::unique_ptr<Texture> depthImg_;

    std::vector<VkImage>       swapChainImages_;
    std::vector<VkImageView>   swapChainImageViews_;
    std::vector<VkFramebuffer> swapChainFramebuffers_;

    std::shared_ptr<CommandExecuter> executer_;
    std::unique_ptr<MeshFactory>     meshFactory_;

    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createFramebuffers();
};

} // namespace Multor
