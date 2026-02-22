/// \file scene.cpp

#include "scene.h"

#include <functional>
#include <stdexcept>

namespace Multor
{

Scene::Scene(std::shared_ptr<PositionController> controller)
    : controller_(std::move(controller))
{
}

void Scene::SetBackGround(const glm::vec4& color)
{
    backgroundColor_ = color;
}

void Scene::SetBackGround(std::shared_ptr<Node> skyboxNode)
{
    backgroundNode_ = std::move(skyboxNode);
}

glm::vec4 Scene::GetBackGround() const
{
    return backgroundColor_;
}

std::shared_ptr<Node> Scene::GetBackGroundNode() const
{
    return backgroundNode_;
}

void Scene::AddMesh(std::shared_ptr<BaseMesh> mesh)
{
    if (!mesh)
        throw std::runtime_error("mesh is null");
    meshes_[HashName(mesh->GetName())] = std::move(mesh);
}

void Scene::AddModel(std::shared_ptr<Model> model)
{
    if (!model)
        throw std::runtime_error("model is null");
    models_[HashName(model->GetName())] = std::move(model);
}

void Scene::AddLight(std::shared_ptr<BLight> light)
{
    if (!light)
        throw std::runtime_error("light is null");
    lights_[HashName(light->GetName())] = std::move(light);
}

void Scene::AddNode(std::shared_ptr<Node> node)
{
    if (!node)
        throw std::runtime_error("node is null");
    nodes_[HashName(node->GetName())] = std::move(node);
}

void Scene::ClearLights()
{
    lights_.clear();
}

std::shared_ptr<BaseMesh> Scene::GetMesh(std::string_view name) const
{
    auto it = meshes_.find(HashName(name));
    return (it == meshes_.end()) ? nullptr : it->second;
}

std::shared_ptr<Model> Scene::GetModel(std::string_view name) const
{
    auto it = models_.find(HashName(name));
    return (it == models_.end()) ? nullptr : it->second;
}

std::shared_ptr<BLight> Scene::GetLight(std::string_view name) const
{
    auto it = lights_.find(HashName(name));
    return (it == lights_.end()) ? nullptr : it->second;
}

std::shared_ptr<Node> Scene::GetNode(std::string_view name) const
{
    auto it = nodes_.find(HashName(name));
    return (it == nodes_.end()) ? nullptr : it->second;
}

std::pair<Scene::LIt, Scene::LIt> Scene::GetLights() const
{
    return std::make_pair(lights_.cbegin(), lights_.cend());
}

std::pair<Scene::MIt, Scene::MIt> Scene::GetMeshes() const
{
    return std::make_pair(meshes_.cbegin(), meshes_.cend());
}

std::pair<Scene::ModIt, Scene::ModIt> Scene::GetModels() const
{
    return std::make_pair(models_.cbegin(), models_.cend());
}

std::pair<Scene::NIt, Scene::NIt> Scene::GetNodes() const
{
    return std::make_pair(nodes_.cbegin(), nodes_.cend());
}

void Scene::SetController(std::shared_ptr<PositionController> controller)
{
    controller_ = std::move(controller);
}

std::shared_ptr<PositionController> Scene::GetController() const
{
    return controller_;
}

std::shared_ptr<Camera> Scene::GetCamera() const
{
    return controller_ ? controller_->cam_ : nullptr;
}

SceneInformation Scene::GetInfo() const
{
    return {
        .amountMeshes_ = meshes_.size(),
        .amountModels_ = models_.size(),
        .amountLights_ = lights_.size(),
        .amountNodes_  = nodes_.size(),
    };
}

std::size_t Scene::HashName(std::string_view name)
{
    return std::hash<std::string_view> {}(name);
}

} // namespace Multor
