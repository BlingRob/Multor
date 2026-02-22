/// \file model.h

#pragma once
#ifndef MODEL_H
#define MODEL_H

#include "entity.h"
#include "scene_objects/mesh.h"
#include "scene_objects/node.h"
#include "transformation.h"

#include <functional>
#include <list>
#include <memory>
#include <utility>

namespace Multor
{

class Model : public Entity, public Transformation_interface
{
public:
    using MeshIt = std::list<std::shared_ptr<BaseMesh> >::const_iterator;

    explicit Model(std::function<void()> resourceDeleter = []() {});
    ~Model();

    Model(const Model&)            = delete;
    Model& operator=(const Model&) = delete;
    Model(Model&&) noexcept;
    Model& operator=(Model&&) noexcept;

    void SetRoot(std::shared_ptr<Node> root);
    std::shared_ptr<Node> GetRoot() const;

    void AddMesh(std::shared_ptr<BaseMesh> mesh);
    std::pair<MeshIt, MeshIt> GetMeshes() const;

    void      SetTransform(const std::shared_ptr<glm::mat4>) override;
    void      SetTransform(const glm::mat4&) override;
    glm::mat4 GetTransform() const override;

    void Translate(const glm::vec3& trans) override;
    void Rotate(float alph, const glm::vec3& axes) override;
    void Scale(const glm::vec3& coefs) override;

private:
    void SyncRootWithModelTransform();

private:
    std::function<void()>             deleter_;
    std::shared_ptr<Node>             root_;
    std::list<std::shared_ptr<BaseMesh> > meshes_;
    Transformation                    transform_;
};

} // namespace Multor

#endif // MODEL_H
