/// \file buffer_factory.cpp

#include "buffer_factory.h"

#include "objects/vertex.h"

namespace Multor::Vulkan
{

uint32_t BufferFactory::findMemoryType(uint32_t              typeFilter,
                                         VkMemoryPropertyFlags properties)
{
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physDev_, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) ==
                properties)
            return i;

    throw std::runtime_error("failed to find suitable memory type!");
}

std::unique_ptr<Buffer>
BufferFactory::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                              VkMemoryPropertyFlags properties)
{
    std::unique_ptr<Buffer> buf = std::make_unique<Buffer>();
    VkBufferCreateInfo            bufferInfo {};
    bufferInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.pNext       = nullptr;
    bufferInfo.size        = size;
    bufferInfo.usage       = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(dev_, &bufferInfo, nullptr, &buf->buffer_) != VK_SUCCESS)
        throw std::runtime_error("failed to create vertex buffer!");

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(dev_, buf->buffer_, &memRequirements);

    VkMemoryAllocateInfo allocInfo {};
    allocInfo.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex =
        findMemoryType(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(dev_, &allocInfo, nullptr, &buf->bufferMemory_) !=
        VK_SUCCESS)
        throw std::runtime_error("failed to allocate vertex buffer memory!");

    vkBindBufferMemory(dev_, buf->buffer_, buf->bufferMemory_, 0);

    buf->dev_ = dev_;
    return buf;
}

std::unique_ptr<VertexBuffer>
BufferFactory::CreateVertexBuffer(Vertexes* vert)
{
    VkDeviceSize bufferSize = sizeof(Vertex) * vert->GetSize();

    std::unique_ptr<Buffer> stBuf =
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(dev_, stBuf->bufferMemory_, 0, bufferSize, 0, &data);
    std::memcpy(data, vert->GetVertexes(), bufferSize);
    vkUnmapMemory(dev_, stBuf->bufferMemory_);

    std::unique_ptr<Buffer> vertBuf = CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    executer_->CopyBuffer(stBuf->buffer_, vertBuf->buffer_, bufferSize);

    return std::unique_ptr<VertexBuffer>(
        new VertexBuffer({std::move(vertBuf), Vertex::getBindingDescription(),
                          Vertex::getAttributeDescriptions()}));
}

std::unique_ptr<Buffer> BufferFactory::CreateIndexBuffer(Vertexes* vert)
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * vert->GetIndices().size();

    std::unique_ptr<Buffer> stBuf =
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(dev_, stBuf->bufferMemory_, 0, bufferSize, 0, &data);
    std::memcpy(data, vert->GetIndices().data(), bufferSize);
    vkUnmapMemory(dev_, stBuf->bufferMemory_);

    std::unique_ptr<Buffer> IndexBuf = CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    executer_->CopyBuffer(stBuf->buffer_, IndexBuf->buffer_, bufferSize);

    return IndexBuf;
}

std::unique_ptr<Buffer>
BufferFactory::CreateUniformBuffer(VkDeviceSize bufferSize)
{
    return CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
}

std::unique_ptr<Buffer>
BufferFactory::CreateMaterialBuffer(Material* mat)
{
    VkDeviceSize bufferSize = sizeof(*mat);

    std::unique_ptr<Buffer> stBuf =
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(dev_, stBuf->bufferMemory_, 0, bufferSize, 0, &data);
    std::memcpy(data, mat, sizeof(*mat));
    vkUnmapMemory(dev_, stBuf->bufferMemory_);

    std::unique_ptr<Buffer> materialBuf = CreateBuffer(
        bufferSize,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    executer_->CopyBuffer(stBuf->buffer_, materialBuf->buffer_, bufferSize);

    return materialBuf;
}

} // namespace Multor::Vulkan
