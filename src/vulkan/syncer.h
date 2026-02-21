/// \file syncer.h

#pragma once

#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

class Syncer
{
public:
    Syncer(VkDevice& device);
    ~Syncer();

    VkSemaphore imageAvailableSemaphores_ = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphores_ = VK_NULL_HANDLE;
    VkFence     inFlightFences_           = VK_NULL_HANDLE;
    VkFence     imagesInFlight_           = VK_NULL_HANDLE;

private:
    VkDevice m_device;
};

} // namespace Multor::Vulkan
