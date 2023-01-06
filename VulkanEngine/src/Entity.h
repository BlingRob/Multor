/// \file Entity.h
#pragma once
#include <string_view>
#include <string>

class Entity
{
	std::string _name;
	public:
        std::string_view GetName() const;
        void SetName(std::string_view name);
};