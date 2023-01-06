/// \file Entity.cpp
#include "Entity.h"

std::string_view Entity::GetName() const
{
	return std::string_view(_name);
}
void Entity::SetName(std::string_view name)
{
	_name = std::string(name);
}