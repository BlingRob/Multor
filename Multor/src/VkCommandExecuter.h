/// \file VkCommandExecuter.h

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
        : dev_(dev), _pool(pool), _graphQueue(graphQueue)
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
    VkCommandPool _pool;
    VkQueue       _graphQueue;

    VkCommandBuffer beginSingleTimeCommands();
    void            endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace Multor

#endif // VKCOMMANDEXECUTER_H
