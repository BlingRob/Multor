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

    VkSemaphore imageAvailableSemaphores;
    VkSemaphore renderFinishedSemaphores;
    VkFence     inFlightFences;
    VkFence     imagesInFlight;

private:
    VkDevice m_device;
};

} // namespace Multor::Vulkan
