/// \file VkCommandExecuter.h
#pragma once
#include <exception>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace Multor
{

class CommandExecuter 
{
public:
	CommandExecuter(VkDevice dev, VkCommandPool pool, VkQueue graphQueue) :_dev(dev), _pool(pool), _graphQueue(graphQueue){}

	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
	void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

private:
	VkDevice _dev;
	VkCommandPool _pool;
	VkQueue _graphQueue;

	VkCommandBuffer beginSingleTimeCommands();
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);
};

} // namespace Multor
