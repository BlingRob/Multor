/// \file physics_types.h

#pragma once
#ifndef MULTOR_PHYSICS_TYPES_H
#define MULTOR_PHYSICS_TYPES_H

#include <cstdint>

#include <glm/glm.hpp>

namespace Multor
{

using PhysicsBodyId = std::uint64_t;

enum class RigidBodyType
{
    Static = 0,
    Dynamic,
    Kinematic
};

enum class ColliderShape
{
    Box = 0,
    Sphere,
    Capsule
};

struct ColliderDesc
{
    ColliderShape shape_ = ColliderShape::Box;
    glm::vec3 halfExtents_ {0.5f, 0.5f, 0.5f};
    float radius_ = 0.5f;
    float height_ = 1.0f;
};

struct RigidBodyDesc
{
    RigidBodyType type_ = RigidBodyType::Static;
    float mass_ = 1.0f;
    float gravityScale_ = 1.0f;
    float linearDamping_ = 0.02f;
    glm::vec3 linearVelocity_ {0.0f, 0.0f, 0.0f};
};

} // namespace Multor

#endif // MULTOR_PHYSICS_TYPES_H
