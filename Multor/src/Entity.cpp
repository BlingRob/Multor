/// \file Entity.cpp
/// \brief Class realization of named entity 

#include "Entity.h"

namespace Multor
{

std::string_view Entity::getName() const
{
	return std::string_view(name_);
}

void Entity::setName(std::string_view name)
{
	name_ = std::string(name);
}

} // namespace Multor