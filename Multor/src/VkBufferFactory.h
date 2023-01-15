/// \file VkBufferFactory.h
#pragma once

#include "Material.h"
#include "VkCommandExecuter.h"
#include "VulkanObjects.h"

#include <memory>
#include <exception>
#include <stdexcept>
#include <cstring>

#include <vulkan/vulkan.h>

namespace Multor
{

class VkBufferFactory
{
public:
	VkBufferFactory(VkDevice dev, VkPhysicalDevice PhysDev, std::shared_ptr<CommandExecuter> ex):_dev(dev), physDev(PhysDev),
																								 _executer(ex){}

	std::unique_ptr<VulkanBuffer> createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
	std::unique_ptr<VertexBuffer> createVertexBuffer(Vertexes* vert);
	std::unique_ptr<VulkanBuffer> createIndexBuffer(Vertexes* vert);
	std::unique_ptr<VulkanBuffer> createUniformBuffer(VkDeviceSize bufferSize);
	std::unique_ptr<VulkanBuffer> createMaterialBuffer(Material* mat);

protected:

	VkDevice _dev;
	VkPhysicalDevice physDev;
	std::shared_ptr<CommandExecuter> _executer;
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
};

} // namespace Multor