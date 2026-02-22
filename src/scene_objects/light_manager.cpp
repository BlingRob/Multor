/// \file light_manager.cpp

#include "light_manager.h"

#include <stdexcept>

namespace Multor
{

void LightManager::Add(LightPtr light)
{
    if (!light)
        throw std::runtime_error("LightManager::Add got null light");
    lights_.push_back(std::move(light));
}

void LightManager::Clear()
{
    lights_.clear();
}

const std::vector<LightManager::LightPtr>& LightManager::GetAll() const
{
    return lights_;
}

} // namespace Multor
