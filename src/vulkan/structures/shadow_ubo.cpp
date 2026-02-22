/// \file shadow_ubo.cpp

#include "shadow_ubo.h"

#include <stdexcept>

namespace Multor::Vulkan::UBOs
{

ShadowPack PackShadowData(const std::vector<const Multor::BLight*>& lights)
{
    ShadowPack out {};

    for (const auto* light : lights)
        {
            if (!light)
                continue;

            const Multor::Shadow* shadow = light->GetShadow();
            if (!shadow)
                continue;

            if (shadow->GetType() == Multor::ShadowType::Directional ||
                shadow->GetType() == Multor::ShadowType::Spot)
                {
                    if (out.directional_.counts_.x >=
                        static_cast<int>(MaxDirectionalShadowLights))
                        continue;

                    const auto* dirShadow =
                        dynamic_cast<const Multor::DirectionalShadow*>(shadow);
                    if (!dirShadow)
                        continue;

                    glm::vec3 dir(0.0f);
                    glm::vec3 pos(0.0f);
                    bool      hasPos = false;
                    if (const auto* sl =
                            dynamic_cast<const Multor::SpotLight*>(light))
                        {
                            dir    = sl->GetDir();
                            pos    = sl->GetPos();
                            hasPos = true;
                        }
                    else if (const auto* dl =
                                 dynamic_cast<const Multor::DirectionalLight*>(light))
                        {
                            dir = dl->GetDir();
                        }

                    auto& entry = out.directional_
                                      .entries_[static_cast<std::size_t>(
                                          out.directional_.counts_.x)];
                    entry.lightSpace_ = hasPos
                                            ? dirShadow->BuildLightSpaceMatrix(pos, dir)
                                            : dirShadow->BuildLightSpaceMatrix(dir);
                    entry.meta_ = glm::ivec4(shadow->GetId(), light->GetLightSlot(),
                                             1, 0);
                    ++out.directional_.counts_.x;
                }
            else if (shadow->GetType() == Multor::ShadowType::Point)
                {
                    if (out.point_.counts_.x >=
                        static_cast<int>(MaxPointShadowLights))
                        continue;

                    const auto* pointShadow =
                        dynamic_cast<const Multor::PointShadow*>(shadow);
                    const auto* pointLight =
                        dynamic_cast<const Multor::PointLight*>(light);
                    if (!pointShadow || !pointLight)
                        continue;

                    const glm::vec3 pos = pointLight->GetPos();
                    auto& entry = out.point_.entries_[static_cast<std::size_t>(
                        out.point_.counts_.x)];
                    entry.shadowMatrices_ = pointShadow->BuildShadowMatrices(pos);
                    entry.lightPosFar_ =
                        glm::vec4(pos, pointShadow->GetFarPlane());
                    entry.meta_ = glm::ivec4(shadow->GetId(), light->GetLightSlot(),
                                             1, 0);
                    ++out.point_.counts_.x;
                }
        }

    return out;
}

} // namespace Multor::Vulkan::UBOs
