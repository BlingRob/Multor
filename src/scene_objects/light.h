/// \file light.h

#pragma once
#ifndef LIGHT_H
#define LIGHT_H

#include "../entity.h"
#include "shadow.h"

#include <glm/glm.hpp>

#include <cstdint>
#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace Multor
{

enum class LightType
{
    None = 0,
    Directional,
    Point,
    Spot
};

class BLight : public Entity
{
public:
    BLight(const glm::vec3& ambient, const glm::vec3& diffuse,
           const glm::vec3& specular, const glm::vec3& attenuation,
           const glm::vec4& lightVec);
    virtual ~BLight();

    BLight(const BLight&)            = delete;
    BLight& operator=(const BLight&) = delete;
    BLight(BLight&& other) noexcept;
    BLight& operator=(BLight&& other) noexcept;

    virtual LightType GetType() const;

    glm::vec3 GetAmbient() const;
    glm::vec3 GetDiffuse() const;
    glm::vec3 GetSpecular() const;
    glm::vec3 GetAttenuation() const;
    const Shadow* GetShadow() const;

    void SetAmbient(const glm::vec3& ambient);
    void SetDiffuse(const glm::vec3& diffuse);
    void SetSpecular(const glm::vec3& specular);
    void SetAttenuation(const glm::vec3& attenuation);
    void SetChangedCallback(std::function<void()> callback);

    int32_t GetLightSlot() const;
    bool    HasLightSlot() const;

protected:
    void      SetVec(const glm::vec4& vec);
    glm::vec4 GetVec() const;
    void      NotifyChanged();

private:
    void AcquireSlot();
    void ReleaseSlot();

private:
    glm::vec3 ambient_;
    glm::vec3 diffuse_;
    glm::vec3 specular_;
    glm::vec3 attenuation_;
    glm::vec4 lightVec_;

    int32_t slotId_ = -1;
    std::function<void()> onChanged_;

    static inline std::list<int32_t> lightSlots_ = {0, 1, 2, 3, 4, 5, 6, 7,
                                                    8, 9, 10, 11, 12, 13, 14};
protected:
    std::shared_ptr<Shadow> shadow_;
};

class DirectionalLight : public BLight
{
public:
    DirectionalLight(const glm::vec3& ambient, const glm::vec3& diffuse,
                     const glm::vec3& specular, const glm::vec3& attenuation,
                     const glm::vec3& direction);

    void      ChangeDirection(const glm::vec3& direction);
    glm::vec3 GetDir() const;
    LightType GetType() const override;
};

class PointLight : public BLight
{
public:
    PointLight(const glm::vec3& ambient, const glm::vec3& diffuse,
               const glm::vec3& specular, const glm::vec3& attenuation,
               const glm::vec3& position);

    void      SetPos(const glm::vec3& position);
    glm::vec3 GetPos() const;
    LightType GetType() const override;
};

class SpotLight : public DirectionalLight
{
public:
    SpotLight(const glm::vec3& ambient, const glm::vec3& diffuse,
              const glm::vec3& specular, const glm::vec3& attenuation,
              const glm::vec3& position, const glm::vec3& direction,
              float bigAngle = 60.0f, float smallAngle = 30.0f);

    std::pair<float, float> GetAngles() const;
    void SetAngles(const std::pair<float, float>& newAngles);

    void      SetPos(const glm::vec3& position);
    glm::vec3 GetPos() const;
    LightType GetType() const override;

private:
    float theta_;
    float alpha_;
    glm::vec3 position_;
};

} // namespace Multor

#endif // LIGHT_H
