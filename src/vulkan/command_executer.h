/// \file command_executer.h

#pragma once

#include <exception>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class CommandExecuter
{
public:
    CommandExecuter(VkDevice dev, VkCommandPool pool, VkQueue graphQueue)
        : dev_(dev), pool_(pool), graphQueue_(graphQueue)
    {
    }

    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void TransitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
    void TransitionImageLayoutLayers(VkImage image, VkFormat format,
                                     VkImageLayout oldLayout,
                                     VkImageLayout newLayout,
                                     uint32_t baseArrayLayer,
                                     uint32_t layerCount);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

private:
    VkDevice      dev_;
    VkCommandPool pool_;
    VkQueue       graphQueue_;

    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace Multor::Vulkan

