/// \file VkSync.h
#pragma once
#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

class VkSyncer 
{
public:

	VkSyncer(VkDevice& device);
	~VkSyncer();

	VkSemaphore imageAvailableSemaphores;
	VkSemaphore renderFinishedSemaphores;
	VkFence inFlightFences;
	VkFence imagesInFlight;

private:
	VkDevice m_device;
};