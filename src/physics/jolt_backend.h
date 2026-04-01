/// \file jolt_backend.h

#pragma once
#ifndef MULTOR_JOLT_BACKEND_H
#define MULTOR_JOLT_BACKEND_H

#include "physics_types.h"

#include <memory>
#include <string_view>

#include <glm/glm.hpp>

namespace JPH
{
class TempAllocatorImpl;
class JobSystemThreadPool;
class PhysicsSystem;
class BroadPhaseLayerInterfaceTable;
class ObjectLayerPairFilterTable;
class ObjectVsBroadPhaseLayerFilterTable;
}

namespace Multor::Physics
{

class JoltBackend
{
public:
    using NativeBodyId = std::uint32_t;

    JoltBackend();
    ~JoltBackend();

    bool Initialize();
    bool IsInitialized() const;
    std::string_view Name() const;

    NativeBodyId CreateBody(const RigidBodyDesc& rigidBody,
                            const ColliderDesc& collider,
                            const glm::mat4& transform);
    void DestroyBody(NativeBodyId id);
    void Step(float dt);

    void SetGravity(const glm::vec3& gravity);
    glm::vec3 GetGravity() const;

    glm::vec3 GetBodyPosition(NativeBodyId id) const;
    void SetBodyPosition(NativeBodyId id, const glm::vec3& position);
    glm::vec3 GetBodyLinearVelocity(NativeBodyId id) const;
    void SetBodyLinearVelocity(NativeBodyId id, const glm::vec3& velocity);

private:
    static bool IsMovingBody(RigidBodyType type);

private:
    bool initialized_ = false;
    std::unique_ptr<JPH::TempAllocatorImpl> tempAllocator_;
    std::unique_ptr<JPH::JobSystemThreadPool> jobSystem_;
    std::unique_ptr<JPH::PhysicsSystem> physicsSystem_;
    std::unique_ptr<JPH::BroadPhaseLayerInterfaceTable> broadPhaseLayerInterface_;
    std::unique_ptr<JPH::ObjectLayerPairFilterTable> objectLayerPairFilter_;
    std::unique_ptr<JPH::ObjectVsBroadPhaseLayerFilterTable> objectVsBroadPhaseLayerFilter_;
};

} // namespace Multor::Physics

#endif // MULTOR_JOLT_BACKEND_H
