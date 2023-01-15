/// \file VkBufferFactory.cpp
#include "VkBufferFactory.h"

namespace Multor
{

uint32_t VkBufferFactory::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physDev, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	throw std::runtime_error("failed to find suitable memory type!");
}

std::unique_ptr<VulkanBuffer> VkBufferFactory::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties)
{
	std::unique_ptr<VulkanBuffer> buf = std::make_unique<VulkanBuffer>();
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.pNext = nullptr;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(_dev, &bufferInfo, nullptr, &buf->_buffer)
		!= VK_SUCCESS)
		throw std::runtime_error("failed to create vertex buffer!");

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_dev, buf->_buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(_dev, &allocInfo, nullptr,
		&buf->_bufferMemory) != VK_SUCCESS)
		throw std::runtime_error("failed to allocate vertex buffer memory!");

	vkBindBufferMemory(_dev, buf->_buffer, buf->_bufferMemory, 0);

	buf->_dev = _dev;
	return buf;
}

std::unique_ptr<VertexBuffer> VkBufferFactory::createVertexBuffer(Vertexes* vert)
{
	VkDeviceSize bufferSize = sizeof(Vertex) * vert->GetSize();

	std::unique_ptr<VulkanBuffer> stBuf =
	createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(_dev, stBuf->_bufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, vert->GetVertexes(), bufferSize);
	vkUnmapMemory(_dev, stBuf->_bufferMemory);

	std::unique_ptr<VulkanBuffer> vertBuf = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	_executer->copyBuffer(stBuf->_buffer, vertBuf->_buffer, bufferSize);

	return std::unique_ptr<VertexBuffer>(new VertexBuffer({ std::move(vertBuf), Vertex::getBindingDescription(), Vertex::getAttributeDescriptions() }));
}

std::unique_ptr<VulkanBuffer> VkBufferFactory::createIndexBuffer(Vertexes* vert)
{
	VkDeviceSize bufferSize = sizeof(uint32_t) * vert->GetIndices().size();

	std::unique_ptr<VulkanBuffer> stBuf =
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(_dev, stBuf->_bufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, vert->GetIndices().data(), bufferSize);
	vkUnmapMemory(_dev, stBuf->_bufferMemory);

	std::unique_ptr<VulkanBuffer> IndexBuf = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_executer->copyBuffer(stBuf->_buffer, IndexBuf->_buffer, bufferSize);
	
	return IndexBuf;
}

std::unique_ptr<VulkanBuffer> VkBufferFactory::createUniformBuffer(VkDeviceSize bufferSize)
{
	return createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

std::unique_ptr<VulkanBuffer> VkBufferFactory::createMaterialBuffer(Material* mat)
{
	VkDeviceSize bufferSize = sizeof(*mat);

	std::unique_ptr<VulkanBuffer> stBuf =
		createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(_dev, stBuf->_bufferMemory, 0, bufferSize, 0, &data);
	std::memcpy(data, mat, sizeof(*mat));
	vkUnmapMemory(_dev, stBuf->_bufferMemory);

	std::unique_ptr<VulkanBuffer> MaterialBuf = createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	_executer->copyBuffer(stBuf->_buffer, MaterialBuf->_buffer, bufferSize);

	return MaterialBuf;
}

} // namespace Multor