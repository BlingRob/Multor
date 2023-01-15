/// \file VkGeneralOptions.cpp
#include "VkGeneralOptions.h"

namespace Multor
{

VkBaseStructs::VkBaseStructs(std::shared_ptr<Window> pWnd,  std::shared_ptr<Logging::Logger> pLog):_pWnd(std::move(pWnd)), _pLogger(std::move(pLog)), physicDev(VK_NULL_HANDLE)
{
	CreateInstance();
	CreateSurface();
	InitPhysicalDevice();
	InitLogicalDevice();
	CreateCommandPool();
}

VkBaseStructs::~VkBaseStructs() 
{
	vkDestroyCommandPool(device, commandPool, nullptr), commandPool = VK_NULL_HANDLE;
	
	vkDestroySurfaceKHR(instance, surface, nullptr), surface = VK_NULL_HANDLE;
	vkDestroyDevice(device, nullptr), device = VK_NULL_HANDLE;
	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

	vkDestroyInstance(instance, nullptr);
	instance = VK_NULL_HANDLE;
}

void VkBaseStructs::CreateInstance()
{
	//If switch on debugging mode, check out main layer for it
	if (enableValidationLayers && !checkValidationLayerSupport())
		throw std::runtime_error("validation layers requested, but not available!");

	//Struct for information about application
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pNext = nullptr;
	appInfo.pApplicationName = "VulkanRenderer";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_3;

	//Fill struct describing instance
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	//Debugging layer
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		using namespace std::placeholders;
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.pNext = nullptr;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;//VK_DEBUG_UTILS_MESSAGE_SEVERITY_FLAG_BITS_MAX_ENUM_EXT; //
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;//VK_DEBUG_UTILS_MESSAGE_TYPE_FLAG_BITS_MAX_ENUM_EXT
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = _pLogger.get();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance!");
	//Create messenger for error 
	if (enableValidationLayers)
		if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS)
			throw std::runtime_error("failed to set up debug messenger!");
}

std::vector<const char*> VkBaseStructs::getRequiredExtensions()
{
	uint32_t extensionCount;
	const char** extensionNames = nullptr;
	SDL_Vulkan_GetInstanceExtensions(&extensionCount, nullptr);
	extensionNames = new const char* [extensionCount];
	SDL_Vulkan_GetInstanceExtensions(&extensionCount, extensionNames);

	std::vector<const char*> extensions(extensionNames, extensionNames + extensionCount);

	if (enableValidationLayers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}


void VkBaseStructs::InitPhysicalDevice()
{
	//find Phisical device
	uint32_t DeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &DeviceCount, nullptr);
	if (!DeviceCount)
		throw std::runtime_error("Not physical device!");

	std::vector<VkPhysicalDevice> devs(DeviceCount);
	vkEnumeratePhysicalDevices(instance, &DeviceCount, devs.data());

	physicDev = VK_NULL_HANDLE;
	for (const auto& dev : devs)
		if (isDeviceSuitable(dev))
			if (physicDevIndices = findQueueFamilies(dev); physicDevIndices.isComplete())
			{
				physicDev = dev;
				break;
			}
	if (physicDev == VK_NULL_HANDLE)
		throw std::runtime_error("Not found a suitable GPU!");
}

bool VkBaseStructs::isDeviceSuitable(VkPhysicalDevice device)
{
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	//Check can device execute required operations?
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	//Check capability surface formats 
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() &&
			!swapChainSupport.presentModes.empty() && supportedFeatures.samplerAnisotropy;
	}

	return swapChainAdequate;
}

void VkBaseStructs::InitLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

	float priority = 1.0f;
	std::vector<std::uint32_t> quequeNums;
	if (physicDevIndices.graphicsFamily.value() == physicDevIndices.presentFamily.value())
	   	quequeNums = { physicDevIndices.graphicsFamily.value() };
	else
		quequeNums = { physicDevIndices.graphicsFamily.value(), physicDevIndices.presentFamily.value() };

	for(uint32_t queueFamily: quequeNums)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.pNext = nullptr;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures devFeatures{};
	devFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &devFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());;
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	//Create logical device
	if (vkCreateDevice(physicDev, &createInfo, nullptr, &device) != VK_SUCCESS)
		std::runtime_error("Don't create physical device!");
	//Get queue from logical device
	vkGetDeviceQueue(device, physicDevIndices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, physicDevIndices.presentFamily.value(), 0, &presentQueue);
}

void VkBaseStructs::CreateSurface()
{
	if (SDL_Vulkan_CreateSurface(_pWnd->GetWindow().get(), instance, &surface) != SDL_TRUE)
		std::runtime_error("Don't create surface!");
}

void VkBaseStructs::CreateCommandPool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.pNext = nullptr;
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = physicDevIndices.graphicsFamily.value();
	poolInfo.flags = 0; // Optional
	//Create command buffer pool
	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) !=
		VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

QueueFamilyIndices VkBaseStructs::findQueueFamilies(VkPhysicalDevice device)
{
	//Find queue families and check can found queue execute graphical operations?
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

		if (presentSupport)
			indices.presentFamily = i;

		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}


bool VkBaseStructs::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}

		if (!layerFound)
			return false;
	}

	return true;
}

bool VkBaseStructs::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount = 0;

	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	for (const auto& Extension : deviceExtensions)
		if (std::find_if(availableExtensions.cbegin(), availableExtensions.cend(),
			[&Extension](const auto& pr) {return std::strcmp(Extension, pr.extensionName) == 0; })
			== availableExtensions.cend())
			return false;
	return true;
}

SwapChainSupportDetails VkBaseStructs::querySwapChainSupport() 
{
	return querySwapChainSupport(physicDev);
}

SwapChainSupportDetails VkBaseStructs::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
		func(instance, debugMessenger, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	Logging::Logger* pLog = static_cast<Logging::Logger*>(pUserData);
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		pLog->Log<Logging::File, Logging::Error>(pCallbackData->pMessage);
	break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
		pLog->Log<Logging::File, Logging::Debug>(pCallbackData->pMessage);
	break;
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
	default:
		pLog->Log<Logging::File, Logging::Info>(pCallbackData->pMessage);
	break;
	}
	//std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

} // namespace Multor