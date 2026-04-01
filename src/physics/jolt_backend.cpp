/// \file jolt_backend.cpp

#include "jolt_backend.h"

#include <Jolt/Jolt.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayerInterfaceTable.h>
#include <Jolt/Physics/Collision/ObjectLayerPairFilterTable.h>
#include <Jolt/Physics/Collision/BroadPhase/ObjectVsBroadPhaseLayerFilterTable.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/RegisterTypes.h>

#include <thread>
#include <stdexcept>

namespace Multor::Physics
{

JoltBackend::JoltBackend() = default;

namespace
{
using namespace JPH;

namespace Layers
{
static constexpr ObjectLayer NON_MOVING = 0;
static constexpr ObjectLayer MOVING = 1;
static constexpr ObjectLayer NUM_LAYERS = 2;
}

namespace BroadPhaseLayers
{
static constexpr BroadPhaseLayer NON_MOVING(0);
static constexpr BroadPhaseLayer MOVING(1);
static constexpr uint NUM_LAYERS = 2;
}

EMotionType ToMotionType(Multor::RigidBodyType type)
{
    switch (type)
    {
    case Multor::RigidBodyType::Dynamic:
        return EMotionType::Dynamic;
    case Multor::RigidBodyType::Kinematic:
        return EMotionType::Kinematic;
    case Multor::RigidBodyType::Static:
    default:
        return EMotionType::Static;
    }
}

ObjectLayer ToObjectLayer(Multor::RigidBodyType type)
{
    return type == Multor::RigidBodyType::Static ? Layers::NON_MOVING
                                                 : Layers::MOVING;
}

Vec3 ToJolt(const glm::vec3& v)
{
    return Vec3(v.x, v.y, v.z);
}

RVec3 ToJoltR(const glm::vec3& v)
{
    return RVec3(v.x, v.y, v.z);
}

glm::vec3 FromJolt(const Vec3& v)
{
    return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
}

glm::vec3 ExtractTranslation(const glm::mat4& transform)
{
    return glm::vec3(transform[3]);
}

RefConst<Shape> CreateShape(const Multor::ColliderDesc& collider)
{
    switch (collider.shape_)
    {
    case Multor::ColliderShape::Sphere:
        return new SphereShape(collider.radius_);
    case Multor::ColliderShape::Capsule:
        return new CapsuleShape(std::max(0.0f, collider.height_ * 0.5f - collider.radius_),
                                collider.radius_);
    case Multor::ColliderShape::Box:
    default:
        return new BoxShape(ToJolt(collider.halfExtents_));
    }
}
} // namespace

JoltBackend::~JoltBackend()
{
    objectVsBroadPhaseLayerFilter_.reset();
    objectLayerPairFilter_.reset();
    broadPhaseLayerInterface_.reset();
    physicsSystem_.reset();
    jobSystem_.reset();
    tempAllocator_.reset();
    if (initialized_)
    {
        JPH::UnregisterTypes();
        delete JPH::Factory::sInstance;
        JPH::Factory::sInstance = nullptr;
    }
}

bool JoltBackend::Initialize()
{
    if (initialized_)
        return true;

    JPH::RegisterDefaultAllocator();
    JPH::Factory::sInstance = new JPH::Factory();
    JPH::RegisterTypes();

    tempAllocator_ = std::make_unique<JPH::TempAllocatorImpl>(8 * 1024 * 1024);
    broadPhaseLayerInterface_ = std::make_unique<JPH::BroadPhaseLayerInterfaceTable>(
        Layers::NUM_LAYERS, BroadPhaseLayers::NUM_LAYERS);
    objectLayerPairFilter_ = std::make_unique<JPH::ObjectLayerPairFilterTable>(Layers::NUM_LAYERS);

    broadPhaseLayerInterface_->MapObjectToBroadPhaseLayer(
        Layers::NON_MOVING, BroadPhaseLayers::NON_MOVING);
    broadPhaseLayerInterface_->MapObjectToBroadPhaseLayer(
        Layers::MOVING, BroadPhaseLayers::MOVING);
    objectLayerPairFilter_->EnableCollision(Layers::NON_MOVING, Layers::MOVING);
    objectLayerPairFilter_->EnableCollision(Layers::MOVING, Layers::MOVING);
    objectVsBroadPhaseLayerFilter_ =
        std::make_unique<JPH::ObjectVsBroadPhaseLayerFilterTable>(
            *broadPhaseLayerInterface_,
            BroadPhaseLayers::NUM_LAYERS,
            *objectLayerPairFilter_,
            Layers::NUM_LAYERS);

    const uint32_t maxJobs = 1024;
    const uint32_t maxBarriers = 1024;
    const int hwThreads = std::max(1u, std::thread::hardware_concurrency());
    jobSystem_ = std::make_unique<JPH::JobSystemThreadPool>(maxJobs, maxBarriers,
                                                            hwThreads > 1 ? hwThreads - 1 : 1);

    physicsSystem_ = std::make_unique<JPH::PhysicsSystem>();
    physicsSystem_->Init(4096, 0, 4096, 4096,
                         *broadPhaseLayerInterface_,
                         *objectVsBroadPhaseLayerFilter_,
                         *objectLayerPairFilter_);

    initialized_ = true;
    return true;
}

bool JoltBackend::IsInitialized() const
{
    return initialized_;
}

std::string_view JoltBackend::Name() const
{
    return "Jolt";
}

JoltBackend::NativeBodyId JoltBackend::CreateBody(const RigidBodyDesc& rigidBody,
                                                  const ColliderDesc& collider,
                                                  const glm::mat4& transform)
{
    if (!initialized_ || !physicsSystem_)
        throw std::runtime_error("Jolt backend is not initialized");

    JPH::RefConst<JPH::Shape> shape = CreateShape(collider);
    const glm::vec3 pos = ExtractTranslation(transform);
    JPH::BodyCreationSettings settings(shape,
                                       ToJoltR(pos),
                                       JPH::Quat::sIdentity(),
                                       ToMotionType(rigidBody.type_),
                                       ToObjectLayer(rigidBody.type_));
    settings.mLinearDamping = rigidBody.linearDamping_;
    settings.mGravityFactor = rigidBody.gravityScale_;

    JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    JPH::Body* body = bodyInterface.CreateBody(settings);
    if (body == nullptr)
        throw std::runtime_error("failed to create Jolt body");

    const JPH::BodyID bodyId = body->GetID();
    bodyInterface.AddBody(bodyId,
                          IsMovingBody(rigidBody.type_) ? JPH::EActivation::Activate
                                                        : JPH::EActivation::DontActivate);
    bodyInterface.SetLinearVelocity(bodyId, ToJolt(rigidBody.linearVelocity_));
    return bodyId.GetIndexAndSequenceNumber();
}

void JoltBackend::DestroyBody(NativeBodyId id)
{
    if (!initialized_ || !physicsSystem_)
        return;
    JPH::BodyID bodyId(id);
    JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    if (bodyInterface.IsAdded(bodyId))
        bodyInterface.RemoveBody(bodyId);
    bodyInterface.DestroyBody(bodyId);
}

void JoltBackend::Step(float dt)
{
    if (!initialized_ || !physicsSystem_ || dt <= 0.0f)
        return;
    physicsSystem_->Update(dt, 1, tempAllocator_.get(), jobSystem_.get());
}

void JoltBackend::SetGravity(const glm::vec3& gravity)
{
    if (!physicsSystem_)
        return;
    physicsSystem_->SetGravity(ToJolt(gravity));
}

glm::vec3 JoltBackend::GetGravity() const
{
    if (!physicsSystem_)
        return glm::vec3(0.0f, -9.81f, 0.0f);
    return FromJolt(physicsSystem_->GetGravity());
}

glm::vec3 JoltBackend::GetBodyPosition(NativeBodyId id) const
{
    if (!initialized_ || !physicsSystem_)
        return glm::vec3(0.0f);
    const JPH::BodyID bodyId(id);
    const JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    const JPH::RVec3 pos = bodyInterface.GetCenterOfMassPosition(bodyId);
    return glm::vec3(static_cast<float>(pos.GetX()),
                     static_cast<float>(pos.GetY()),
                     static_cast<float>(pos.GetZ()));
}

void JoltBackend::SetBodyPosition(NativeBodyId id, const glm::vec3& position)
{
    if (!initialized_ || !physicsSystem_)
        return;
    JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    bodyInterface.SetPosition(JPH::BodyID(id), ToJoltR(position), JPH::EActivation::Activate);
}

glm::vec3 JoltBackend::GetBodyLinearVelocity(NativeBodyId id) const
{
    if (!initialized_ || !physicsSystem_)
        return glm::vec3(0.0f);
    const JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    return FromJolt(bodyInterface.GetLinearVelocity(JPH::BodyID(id)));
}

void JoltBackend::SetBodyLinearVelocity(NativeBodyId id, const glm::vec3& velocity)
{
    if (!initialized_ || !physicsSystem_)
        return;
    JPH::BodyInterface& bodyInterface = physicsSystem_->GetBodyInterface();
    bodyInterface.SetLinearVelocity(JPH::BodyID(id), ToJolt(velocity));
}

bool JoltBackend::IsMovingBody(RigidBodyType type)
{
    return type == RigidBodyType::Dynamic || type == RigidBodyType::Kinematic;
}

} // namespace Multor::Physics
