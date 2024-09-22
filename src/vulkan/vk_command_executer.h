/// \file vk_command_executer.h

#pragma once
#ifndef VKCOMMANDEXECUTER_H
#define VKCOMMANDEXECUTER_H

#include <exception>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace Multor
{

class CommandExecuter
{
public:
    CommandExecuter(VkDevice dev, VkCommandPool pool, VkQueue graphQueue)
        : dev_(dev), pool_(pool), graphQueue_(graphQueue)
    {
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void transitionImageLayout(VkImage image, VkFormat format,
                               VkImageLayout oldLayout,
                               VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width,
                           uint32_t height);

private:
    VkDevice      dev_;
    VkCommandPool pool_;
    VkQueue       graphQueue_;

    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace Multor

#endif // VKCOMMANDEXECUTER_H
