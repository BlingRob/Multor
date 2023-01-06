/// \file VulkanObjects.cpp
#include "VulkanObjects.h"

VkTexture::~VkTexture()
{
	vkDestroySampler(_dev, _sampler, nullptr);
	vkDestroyImageView(_dev, _view, nullptr);
	vkDestroyImage(_dev, _img, nullptr);
	vkFreeMemory(_dev, _devMem, nullptr);
	_sampler = VK_NULL_HANDLE; 
	_view = VK_NULL_HANDLE;
	_img = VK_NULL_HANDLE;
	_devMem = VK_NULL_HANDLE;
}