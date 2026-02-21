/// \file entity.h
/// \brief Class interface of named entity

#pragma once
#ifndef ENTITY_H
#define ENTITY_H

#include <string_view>
#include <string>

namespace Multor
{

class Entity
{
public:
    /// \brief Name getter
    std::string_view GetName() const;

    /// \brief Name setter
    void SetName(std::string_view name);

private:
    /// \brief Name
    std::string name_;
};

} // namespace Multor

#endif // ENTITY_H
