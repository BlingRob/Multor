/// \file vk_sync.h

#pragma once
#ifndef VK_SYNC_H
#define VK_SYNC_H

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
    VkFence     inFlightFences;
    VkFence     imagesInFlight;

private:
    VkDevice m_device;
};

} // namespace Multor

#endif // VK_SYNC_H