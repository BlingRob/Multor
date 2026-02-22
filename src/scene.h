/// \file scene.h

#pragma once
#ifndef SCENE_H
#define SCENE_H

#include "scene_objects/light.h"
#include "scene_objects/mesh.h"
#include "scene_objects/node.h"
#include "model.h"
#include "transformation.h"

#include <cstddef>
#include <map>
#include <memory>
#include <string_view>
#include <utility>

#include <glm/glm.hpp>

namespace Multor
{

struct SceneInformation
{
    std::size_t amountMeshes_ = 0;
    std::size_t amountModels_ = 0;
    std::size_t amountLights_ = 0;
    std::size_t amountNodes_  = 0;
};

class Scene
{
public:
    using LIt = std::map<std::size_t, std::shared_ptr<BLight> >::const_iterator;
    using MIt = std::map<std::size_t, std::shared_ptr<BaseMesh> >::const_iterator;
    using NIt = std::map<std::size_t, std::shared_ptr<Node> >::const_iterator;
    using ModIt = std::map<std::size_t, std::shared_ptr<Model> >::const_iterator;

    Scene(std::shared_ptr<PositionController> controller = nullptr);

    void SetBackGround(const glm::vec4& color);
    void SetBackGround(std::shared_ptr<Node> skyboxNode);
    glm::vec4 GetBackGround() const;
    std::shared_ptr<Node> GetBackGroundNode() const;

    void AddMesh(std::shared_ptr<BaseMesh> mesh);
    void AddModel(std::shared_ptr<Model> model);
    void AddLight(std::shared_ptr<BLight> light);
    void AddNode(std::shared_ptr<Node> node);
    void ClearLights();

    std::shared_ptr<BaseMesh> GetMesh(std::string_view name) const;
    std::shared_ptr<Model> GetModel(std::string_view name) const;
    std::shared_ptr<BLight> GetLight(std::string_view name) const;
    std::shared_ptr<Node> GetNode(std::string_view name) const;

    std::pair<LIt, LIt> GetLights() const;
    std::pair<MIt, MIt> GetMeshes() const;
    std::pair<ModIt, ModIt> GetModels() const;
    std::pair<NIt, NIt> GetNodes() const;

    void SetController(std::shared_ptr<PositionController> controller);
    std::shared_ptr<PositionController> GetController() const;
    std::shared_ptr<Camera> GetCamera() const;

    SceneInformation GetInfo() const;

private:
    static std::size_t HashName(std::string_view name);

private:
    std::shared_ptr<PositionController> controller_;

    std::map<std::size_t, std::shared_ptr<BaseMesh> > meshes_;
    std::map<std::size_t, std::shared_ptr<Model> >    models_;
    std::map<std::size_t, std::shared_ptr<BLight> >   lights_;
    std::map<std::size_t, std::shared_ptr<Node> >     nodes_;

    glm::vec4 backgroundColor_ {0.0f, 0.0f, 0.0f, 1.0f};
    std::shared_ptr<Node> backgroundNode_;
};

} // namespace Multor

#endif // SCENE_H
