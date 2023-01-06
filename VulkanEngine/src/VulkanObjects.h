/// \file VulkanObjects.h
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <array>


struct Vertex
{
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 texCoord;
    glm::vec3 aTan;
    glm::vec3 aBitan;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 5>
        getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 5>
            attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, norm);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(Vertex, aTan);

        attributeDescriptions[4].binding = 0;
        attributeDescriptions[4].location = 4;
        attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[4].offset = offsetof(Vertex, aBitan);

        return attributeDescriptions;
    }
};

struct VkTexture
{
	VkDevice _dev;
	VkImage _img;
	VkImageView _view;
	VkSampler _sampler;
    VkDeviceMemory _devMem;
	~VkTexture();
};

struct VulkanBuffer
{
	//VulkanBuffer(VkDevice dev, VkBuffer buf, VkDeviceMemory bufmem) :_dev(dev), _buffer(buf), _bufferMemory(bufmem) {}
    //VulkanBuffer(VulkanBuffer&& _r) :_dev(_r._dev), _buffer(_r._buffer), _bufferMemory(_r._bufferMemory) {}
	VkDevice _dev;
	VkBuffer _buffer;
	VkDeviceMemory _bufferMemory;
	~VulkanBuffer()
	{
		vkDestroyBuffer(_dev, _buffer, nullptr);
		vkFreeMemory(_dev, _bufferMemory, nullptr);
	}
};

struct VertexBuffer
{
    VertexBuffer(std::unique_ptr<VulkanBuffer> buf, const VkVertexInputBindingDescription& binddis, const std::array<VkVertexInputAttributeDescription, 5>& attrdes):
        _pVertBuf(std::move(buf)), bindingDescription(binddis), attributeDescriptions(attrdes){}
	
    VkVertexInputBindingDescription bindingDescription{};
	std::array<VkVertexInputAttributeDescription, 5> attributeDescriptions{};
    std::unique_ptr<VulkanBuffer> _pVertBuf;
};

