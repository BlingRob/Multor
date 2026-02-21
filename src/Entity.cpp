/// \file Entity.cpp
/// \brief Class realization of named entity

#include "entity.h"

namespace Multor
{

std::string_view Entity::GetName() const
{
    return std::string_view(name_);
}

void Entity::SetName(std::string_view name)
{
    name_ = std::string(name);
}

} // namespace Multor
