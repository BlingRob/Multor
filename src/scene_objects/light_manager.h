/// \file light_manager.h

#pragma once
#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include "light.h"

#include <memory>
#include <vector>

namespace Multor
{

class LightManager
{
public:
    using LightPtr = std::shared_ptr<BLight>;

    void Add(LightPtr light);
    void Clear();

    const std::vector<LightPtr>& GetAll() const;

private:
    std::vector<LightPtr> lights_;
};

} // namespace Multor

#endif // LIGHT_MANAGER_H
