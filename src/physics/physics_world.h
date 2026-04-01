/// \file physics_world.h

#pragma once
#ifndef MULTOR_PHYSICS_WORLD_H
#define MULTOR_PHYSICS_WORLD_H

#include "../scene_objects/node.h"
#include "physics_types.h"
#include "jolt_backend.h"

#include <memory>
#include <unordered_map>

#include <glm/glm.hpp>

namespace Multor
{

class PhysicsWorld
{
public:
    using BodyId = PhysicsBodyId;

    PhysicsWorld();

    BodyId AddBody(const std::shared_ptr<Node>& node,
                   const RigidBodyDesc& rigidBody,
                   const ColliderDesc& collider);
    void RemoveBody(BodyId id);
    void Clear();

    bool HasBody(BodyId id) const;
    std::size_t GetBodyCount() const;

    void Step(float dt);

    void SetEnabled(bool enabled);
    bool IsEnabled() const;

    void SetGravity(const glm::vec3& gravity);
    glm::vec3 GetGravity() const;

private:
    struct BodyEntry
    {
        BodyId id_ = 0;
        Physics::JoltBackend::NativeBodyId nativeId_ = 0;
        std::weak_ptr<Node> node_;
        RigidBodyDesc rigidBody_ {};
        ColliderDesc collider_ {};
        glm::vec3 linearVelocity_ {0.0f, 0.0f, 0.0f};
    };

    static glm::vec3 ExtractTranslation(const glm::mat4& transform);
    static glm::mat4 ReplaceTranslation(const glm::mat4& transform,
                                        const glm::vec3& translation);

private:
    std::unordered_map<BodyId, BodyEntry> bodies_;
    std::unique_ptr<Physics::JoltBackend> backend_;
    BodyId nextBodyId_ = 1;
    bool enabled_ = true;
    glm::vec3 gravity_ {0.0f, -9.81f, 0.0f};
};

} // namespace Multor

#endif // MULTOR_PHYSICS_WORLD_H
