/// \file transform_ubo.cpp

#include "transform_ubo.h"

namespace Multor
{

void TransformUBO::updateModel(std::size_t      frame,
                               const glm::mat4& newTransformMatrix)
{
    void* data;
    vkMapMemory(dev_, matrixes_[frame]->bufferMemory_, 0, TransBufObj, 0,
                &data);
    memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, model_),
           &newTransformMatrix, sizeof(glm::mat4));
    vkUnmapMemory(dev_, matrixes_[frame]->bufferMemory_);
}

void TransformUBO::updateView(std::size_t frame, const glm::vec3& newPos)
{
    void* data;
    vkMapMemory(dev_, viewPosUBO_[frame]->bufferMemory_, 0, ViewBufObj, 0,
                &data);
    std::memcpy(data, &newPos, ViewBufObj);
    vkUnmapMemory(dev_, viewPosUBO_[frame]->bufferMemory_);
}

void TransformUBO::updatePV(std::size_t      frame,
                            const glm::mat4& newProjectViewMatrix)
{
    void* data;
    vkMapMemory(dev_, matrixes_[frame]->bufferMemory_, 0, TransBufObj, 0,
                &data);
    memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, PV_),
           &newProjectViewMatrix, sizeof(glm::mat4));
    vkUnmapMemory(dev_, matrixes_[frame]->bufferMemory_);
}

} // namespace Multor