/// \file VkCommandExecuter.h
#pragma once
#include <vulkan/vulkan.h>
#include <exception>
#include <stdexcept>

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
