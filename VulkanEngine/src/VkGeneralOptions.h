/// \file VkGeneralOptions.h
#include <vulkan/vulkan.h>
#include <vector>
#include <exception>
#include <stdexcept>
#include <optional>
#include <iostream>

#include "Window.h"

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() &&
			presentFamily.has_value();
	}
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};


struct VkBaseStructs
{
	VkBaseStructs(std::shared_ptr<Window> wnd);
	~VkBaseStructs();
protected:
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice physicDev;
	QueueFamilyIndices physicDevIndices;
	VkDevice device;
	VkQueue graphicsQueue,
		presentQueue;
	VkCommandPool commandPool;
	VkSurfaceKHR surface;
	std::shared_ptr<Window> _pwnd;

	SwapChainSupportDetails querySwapChainSupport();

private:

	void CreateInstance();
	void CreateSurface();
	void InitPhysicalDevice();
	void InitLogicalDevice();
	void createCommandPool();

	bool isDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
	std::vector<const char*> getRequiredExtensions();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool checkValidationLayerSupport();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
};

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
											 VkDebugUtilsMessageTypeFlagsEXT messageType,
									   const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, 
								const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
								const VkAllocationCallbacks* pAllocator, 
									  VkDebugUtilsMessengerEXT* pDebugMessenger);
