/// \file VkSync.cpp
#include "VkSync.h"

namespace Multor
{

VkSyncer::VkSyncer(VkDevice& device):m_device(device), imagesInFlight(VK_NULL_HANDLE)
{
	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreInfo.pNext = nullptr;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.pNext = nullptr;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(device, &semaphoreInfo, nullptr,
						  &imageAvailableSemaphores) != VK_SUCCESS ||
		vkCreateSemaphore(device, &semaphoreInfo, nullptr,
						  &renderFinishedSemaphores) != VK_SUCCESS ||
		vkCreateFence(	  device, &fenceInfo, nullptr, &inFlightFences)
						  != VK_SUCCESS)
			throw std::runtime_error("failed to create semaphores!");
}

VkSyncer::~VkSyncer() 
{
	vkDestroySemaphore(m_device, imageAvailableSemaphores, nullptr);
	imageAvailableSemaphores = VK_NULL_HANDLE;
	vkDestroySemaphore(m_device, renderFinishedSemaphores, nullptr);
	renderFinishedSemaphores = VK_NULL_HANDLE;
	vkDestroyFence(m_device, inFlightFences, nullptr);
	inFlightFences = VK_NULL_HANDLE;
}

} // namespace Multor