/// \file transform_ubo.h

#pragma once

#include "../objects/buffer.h"
#include "../../scene_objects/material.h"

#include <vector>
#include <memory>

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace Multor::Vulkan
{

namespace UBOs
{
struct Transform
{
    glm::mat4 model_;
    glm::mat4 PV_;
    glm::mat3 normalMatrix_;
};

struct ViewPosition
{
    glm::vec3 viewPos_;
};
}

struct TransformUBO
{
    TransformUBO(VkDevice dev) : dev_(dev)
    {
    }

    void updateModel(std::size_t frame, const glm::mat4& newTransformMatrix);
    void updateView(std::size_t frame, const glm::vec3& newPosition);
    void updatePV(std::size_t frame, const glm::mat4& newProjectViewMatrix);

    std::vector<std::unique_ptr<Buffer> > matrixes_;
    std::vector<std::unique_ptr<Buffer> > viewPosUBO_;
    std::vector<std::unique_ptr<Buffer> > materialUBO_;

    static const VkDeviceSize MatBufObj   = sizeof(Material);
    static const VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
    static const VkDeviceSize ViewBufObj  = sizeof(UBOs::ViewPosition);

private:
    VkDevice dev_;
};

} // namespace Multor::Vulkan
