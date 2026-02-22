/// \file light.cpp

#include "light.h"

#include <algorithm>
#include <utility>

namespace Multor
{

BLight::BLight(const glm::vec3& ambient, const glm::vec3& diffuse,
               const glm::vec3& specular, const glm::vec3& attenuation,
               const glm::vec4& lightVec)
    : ambient_(ambient),
      diffuse_(diffuse),
      specular_(specular),
      attenuation_(attenuation),
      lightVec_(lightVec)
{
    AcquireSlot();
}

BLight::~BLight()
{
    ReleaseSlot();
}

BLight::BLight(BLight&& other) noexcept
    : ambient_(other.ambient_),
      diffuse_(other.diffuse_),
      specular_(other.specular_),
      attenuation_(other.attenuation_),
      lightVec_(other.lightVec_),
      slotId_(other.slotId_)
{
    other.slotId_ = -1;
}

BLight& BLight::operator=(BLight&& other) noexcept
{
    if (this == &other)
        return *this;

    ReleaseSlot();

    ambient_     = other.ambient_;
    diffuse_     = other.diffuse_;
    specular_    = other.specular_;
    attenuation_ = other.attenuation_;
    lightVec_    = other.lightVec_;
    slotId_      = other.slotId_;

    other.slotId_ = -1;
    return *this;
}

LightType BLight::GetType() const
{
    return LightType::None;
}

glm::vec3 BLight::GetAmbient() const
{
    return ambient_;
}

glm::vec3 BLight::GetDiffuse() const
{
    return diffuse_;
}

glm::vec3 BLight::GetSpecular() const
{
    return specular_;
}

glm::vec3 BLight::GetAttenuation() const
{
    return attenuation_;
}

void BLight::SetAmbient(const glm::vec3& ambient)
{
    ambient_ = ambient;
}

void BLight::SetDiffuse(const glm::vec3& diffuse)
{
    diffuse_ = diffuse;
}

void BLight::SetSpecular(const glm::vec3& specular)
{
    specular_ = specular;
}

void BLight::SetAttenuation(const glm::vec3& attenuation)
{
    attenuation_ = attenuation;
}

int32_t BLight::GetLightSlot() const
{
    return slotId_;
}

bool BLight::HasLightSlot() const
{
    return slotId_ >= 0;
}

void BLight::SetVec(const glm::vec4& vec)
{
    lightVec_ = vec;
}

glm::vec4 BLight::GetVec() const
{
    return lightVec_;
}

void BLight::AcquireSlot()
{
    if (lightSlots_.empty())
        return;

    slotId_ = lightSlots_.front();
    lightSlots_.pop_front();
}

void BLight::ReleaseSlot()
{
    if (slotId_ < 0)
        return;

    lightSlots_.insert(
        std::lower_bound(lightSlots_.begin(), lightSlots_.end(), slotId_),
        slotId_);
    slotId_ = -1;
}

DirectionalLight::DirectionalLight(const glm::vec3& ambient,
                                   const glm::vec3& diffuse,
                                   const glm::vec3& specular,
                                   const glm::vec3& attenuation,
                                   const glm::vec3& direction)
    : BLight(ambient, diffuse, specular, attenuation, glm::vec4(direction, 0.0f))
{
}

void DirectionalLight::ChangeDirection(const glm::vec3& direction)
{
    SetVec(glm::vec4(direction, 0.0f));
}

glm::vec3 DirectionalLight::GetDir() const
{
    return glm::vec3(GetVec());
}

LightType DirectionalLight::GetType() const
{
    return LightType::Directional;
}

PointLight::PointLight(const glm::vec3& ambient, const glm::vec3& diffuse,
                       const glm::vec3& specular,
                       const glm::vec3& attenuation,
                       const glm::vec3& position)
    : BLight(ambient, diffuse, specular, attenuation, glm::vec4(position, 1.0f))
{
}

void PointLight::SetPos(const glm::vec3& position)
{
    SetVec(glm::vec4(position, 1.0f));
}

glm::vec3 PointLight::GetPos() const
{
    return glm::vec3(GetVec());
}

LightType PointLight::GetType() const
{
    return LightType::Point;
}

SpotLight::SpotLight(const glm::vec3& ambient, const glm::vec3& diffuse,
                     const glm::vec3& specular, const glm::vec3& attenuation,
                     const glm::vec3& position, const glm::vec3& direction,
                     float bigAngle, float smallAngle)
    : DirectionalLight(ambient, diffuse, specular, attenuation, direction),
      theta_(bigAngle),
      alpha_(smallAngle),
      position_(position)
{
}

std::pair<float, float> SpotLight::GetAngles() const
{
    return {theta_, alpha_};
}

void SpotLight::SetAngles(const std::pair<float, float>& newAngles)
{
    theta_ = newAngles.first;
    alpha_ = newAngles.second;
}

void SpotLight::SetPos(const glm::vec3& position)
{
    position_ = position;
}

glm::vec3 SpotLight::GetPos() const
{
    return position_;
}

LightType SpotLight::GetType() const
{
    return LightType::Spot;
}

} // namespace Multor
