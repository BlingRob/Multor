/// \file physics_world.cpp

#include "physics_world.h"

#include <algorithm>
#include <stdexcept>

namespace Multor
{

PhysicsWorld::PhysicsWorld()
{
    backend_ = std::make_unique<Physics::JoltBackend>();
    backend_->Initialize();
    backend_->SetGravity(gravity_);
}

PhysicsWorld::BodyId PhysicsWorld::AddBody(const std::shared_ptr<Node>& node,
                                           const RigidBodyDesc& rigidBody,
                                           const ColliderDesc& collider)
{
    if (!node)
        throw std::runtime_error("physics body node is null");

    BodyEntry entry {};
    entry.id_             = nextBodyId_++;
    entry.nativeId_       = backend_->CreateBody(rigidBody, collider,
                                                 node->GetLocalTransform());
    entry.node_           = node;
    entry.rigidBody_      = rigidBody;
    entry.collider_       = collider;
    entry.linearVelocity_ = rigidBody.linearVelocity_;

    bodies_[entry.id_] = entry;
    return entry.id_;
}

void PhysicsWorld::RemoveBody(BodyId id)
{
    auto it = bodies_.find(id);
    if (it != bodies_.end() && backend_)
        backend_->DestroyBody(it->second.nativeId_);
    bodies_.erase(id);
}

void PhysicsWorld::Clear()
{
    if (backend_)
        {
            for (const auto& [id, body] : bodies_)
                backend_->DestroyBody(body.nativeId_);
        }
    bodies_.clear();
}

bool PhysicsWorld::HasBody(BodyId id) const
{
    return bodies_.find(id) != bodies_.end();
}

std::size_t PhysicsWorld::GetBodyCount() const
{
    return bodies_.size();
}

void PhysicsWorld::Step(float dt)
{
    if (!enabled_ || dt <= 0.0f)
        return;

    if (backend_)
        backend_->Step(dt);

    for (auto it = bodies_.begin(); it != bodies_.end();)
        {
            auto node = it->second.node_.lock();
            if (!node)
                {
                    it = bodies_.erase(it);
                    continue;
                }

            auto& body = it->second;
            switch (body.rigidBody_.type_)
                {
                case RigidBodyType::Static:
                    break;

                case RigidBodyType::Dynamic:
                    if (backend_)
                        {
                            const glm::vec3 position =
                                backend_->GetBodyPosition(body.nativeId_);
                            body.linearVelocity_ =
                                backend_->GetBodyLinearVelocity(body.nativeId_);
                            node->SetLocalTransform(
                                ReplaceTranslation(node->GetLocalTransform(), position));
                        }
                    break;

                case RigidBodyType::Kinematic:
                    if (backend_)
                        {
                            const glm::vec3 position =
                                ExtractTranslation(node->GetLocalTransform()) +
                                body.linearVelocity_ * dt;
                            backend_->SetBodyPosition(body.nativeId_, position);
                            backend_->SetBodyLinearVelocity(body.nativeId_,
                                                            body.linearVelocity_);
                            node->SetLocalTransform(
                                ReplaceTranslation(node->GetLocalTransform(), position));
                        }
                    break;
                }

            ++it;
        }
}

void PhysicsWorld::SetEnabled(bool enabled)
{
    enabled_ = enabled;
}

bool PhysicsWorld::IsEnabled() const
{
    return enabled_;
}

void PhysicsWorld::SetGravity(const glm::vec3& gravity)
{
    gravity_ = gravity;
    if (backend_)
        backend_->SetGravity(gravity);
}

glm::vec3 PhysicsWorld::GetGravity() const
{
    return backend_ ? backend_->GetGravity() : gravity_;
}

glm::vec3 PhysicsWorld::ExtractTranslation(const glm::mat4& transform)
{
    return glm::vec3(transform[3]);
}

glm::mat4 PhysicsWorld::ReplaceTranslation(const glm::mat4& transform,
                                           const glm::vec3& translation)
{
    glm::mat4 out = transform;
    out[3] = glm::vec4(translation, 1.0f);
    return out;
}

} // namespace Multor
