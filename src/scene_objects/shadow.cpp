/// \file shadow.cpp

#include "shadow.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

#include <glm/gtc/matrix_transform.hpp>

namespace Multor
{

int32_t Shadow::GetId() const
{
    return id_;
}

uint32_t Shadow::GetShadowMapSize() const
{
    return kShadowMapSize_;
}

float Shadow::GetFarPlane() const
{
    return kFarPlane_;
}

PointShadow::PointShadow()
{
    if (!initializedIds_)
        {
            shadowIds_.resize(kMaxLightsOneType_);
            std::iota(shadowIds_.begin(), shadowIds_.end(), 0);
            initializedIds_ = true;
        }

    if (shadowIds_.empty())
        throw std::runtime_error("No available point shadow slots");

    id_ = shadowIds_.front();
    shadowIds_.pop_front();
}

PointShadow::~PointShadow()
{
    if (id_ >= 0)
        shadowIds_.insert(
            std::lower_bound(shadowIds_.begin(), shadowIds_.end(), id_), id_);
}

ShadowType PointShadow::GetType() const
{
    return ShadowType::Point;
}

glm::mat4 PointShadow::GetProjectionMatrix() const
{
    return glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, kFarPlane_);
}

std::array<glm::mat4, 6>
PointShadow::BuildShadowMatrices(const glm::vec3& lightPos) const
{
    const glm::mat4 proj = GetProjectionMatrix();
    return {
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(1.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, -1.0f, 0.0f)),
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0f, 0.0f, 0.0f),
                           glm::vec3(0.0f, -1.0f, 0.0f)),
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 1.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, -1.0f)),
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, -1.0f, 0.0f),
                           glm::vec3(0.0f, 0.0f, -1.0f)),
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, 1.0f),
                           glm::vec3(0.0f, -1.0f, 0.0f)),
        proj * glm::lookAt(lightPos, lightPos + glm::vec3(0.0f, 0.0f, -1.0f),
                           glm::vec3(0.0f, -1.0f, 0.0f)),
    };
}

DirectionalShadow::DirectionalShadow()
{
    if (!initializedIds_)
        {
            shadowIds_.resize(2 * kMaxLightsOneType_);
            std::iota(shadowIds_.begin(), shadowIds_.end(), 0);
            initializedIds_ = true;
        }

    if (shadowIds_.empty())
        throw std::runtime_error("No available directional shadow slots");

    id_ = shadowIds_.front();
    shadowIds_.pop_front();
}

DirectionalShadow::~DirectionalShadow()
{
    if (id_ >= 0)
        shadowIds_.insert(
            std::lower_bound(shadowIds_.begin(), shadowIds_.end(), id_), id_);
}

ShadowType DirectionalShadow::GetType() const
{
    return ShadowType::Directional;
}

void DirectionalShadow::SetOrthoBounds(float left, float right, float bottom,
                                       float top, float zNear, float zFar)
{
    projection_ = glm::ortho(left, right, bottom, top, zNear, zFar);
}

glm::mat4 DirectionalShadow::GetProjectionMatrix() const
{
    return projection_;
}

glm::mat4 DirectionalShadow::BuildLightSpaceMatrix(const glm::vec3& direction) const
{
    return BuildLightSpaceMatrix(glm::vec3(0.0f), direction);
}

glm::mat4 DirectionalShadow::BuildLightSpaceMatrix(const glm::vec3& position,
                                                   const glm::vec3& direction) const
{
    const glm::vec3 dir = glm::normalize(direction);
    glm::vec3 up        = glm::vec3(1.0f, 0.0f, 0.0f);
    if (glm::abs(glm::dot(dir, up)) > 0.99f)
        up = glm::vec3(0.0f, 1.0f, 0.0f);

    const glm::mat4 view = glm::lookAt(position, position - dir, up);
    return projection_ * view;
}

glm::mat4 DirectionalShadow::GetScaleBiasMatrix() const
{
    glm::mat4 scaleBias(0.5f);
    scaleBias[3]    = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
    scaleBias[3][3] = 1.0f;
    return scaleBias;
}

SpotShadow::SpotShadow(float outerAngleDeg)
    : DirectionalShadow()
{
    SetPerspective(outerAngleDeg);
}

ShadowType SpotShadow::GetType() const
{
    return ShadowType::Spot;
}

void SpotShadow::SetPerspective(float outerAngleDeg, float aspect, float zNear,
                                float zFar)
{
    // Slightly wider than cone edge to reduce clipping on shadow receiver.
    projection_ = glm::perspective(glm::radians(outerAngleDeg * 2.0f), aspect,
                                   zNear, zFar);
}

} // namespace Multor
