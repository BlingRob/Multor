/// \file shadow.h

#pragma once
#ifndef SHADOW_H
#define SHADOW_H

#include <array>
#include <cstdint>
#include <list>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Multor
{

enum class ShadowType
{
    None = 0,
    Directional,
    Point,
    Spot
};

class Shadow
{
public:
    virtual ~Shadow() = default;

    Shadow(const Shadow&)            = delete;
    Shadow& operator=(const Shadow&) = delete;
    Shadow(Shadow&&)                 = delete;
    Shadow& operator=(Shadow&&)      = delete;

    virtual ShadowType GetType() const = 0;

    int32_t  GetId() const;
    uint32_t GetShadowMapSize() const;
    float    GetFarPlane() const;

protected:
    Shadow() = default;

    static constexpr uint32_t kShadowMapSize_ = 1024;
    static constexpr float    kFarPlane_      = 64.0f;
    static constexpr uint16_t kMaxLightsOneType_ = 5;

    int32_t id_ = -1;
};

class PointShadow : public Shadow
{
public:
    PointShadow();
    ~PointShadow() override;

    ShadowType GetType() const override;

    glm::mat4 GetProjectionMatrix() const;
    std::array<glm::mat4, 6> BuildShadowMatrices(const glm::vec3& lightPos) const;

private:
    static inline std::list<int32_t> shadowIds_ {};
    static inline bool               initializedIds_ = false;
};

class DirectionalShadow : public Shadow
{
public:
    DirectionalShadow();
    ~DirectionalShadow() override;

    ShadowType GetType() const override;

    void SetOrthoBounds(float left, float right, float bottom, float top,
                        float zNear, float zFar);
    glm::mat4 GetProjectionMatrix() const;
    glm::mat4 BuildLightSpaceMatrix(const glm::vec3& direction) const;
    glm::mat4 BuildLightSpaceMatrix(const glm::vec3& position,
                                    const glm::vec3& direction) const;
    glm::mat4 GetScaleBiasMatrix() const;

protected:
    glm::mat4 projection_ = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f,
                                       -30.0f, 30.0f);

    static inline std::list<int32_t> shadowIds_ {};
    static inline bool               initializedIds_ = false;
};

class SpotShadow : public DirectionalShadow
{
public:
    explicit SpotShadow(float outerAngleDeg = 60.0f);

    ShadowType GetType() const override;
    void SetPerspective(float outerAngleDeg, float aspect = 1.0f,
                        float zNear = 0.1f, float zFar = kFarPlane_);
};

} // namespace Multor

#endif // SHADOW_H
