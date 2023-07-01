/// \file VkSync.h

#pragma once
#ifndef VKSYNC_H
#define VKSYNC_H

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>

namespace Multor
{
	
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

} // namespace Multor

#endif // VKSYNC_H