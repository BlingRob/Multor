/// \file node.h

#pragma once
#ifndef NODE_H
#define NODE_H

#include "../entity.h"
#include "../transformation.h"
#include "mesh.h"

#include <list>
#include <memory>
#include <utility>

namespace Multor
{

class Node : public Entity,
             public Transformation_interface,
             public std::enable_shared_from_this<Node>
{
public:
    using MIt = std::list<std::shared_ptr<BaseMesh> >::const_iterator;
    using NIt = std::list<std::shared_ptr<Node> >::const_iterator;

    Node();

    void addMesh(std::shared_ptr<BaseMesh> mesh);
    void addChild(std::shared_ptr<Node> child);
    std::pair<MIt, MIt> GetMeshes() const;
    std::pair<NIt, NIt> GetChildren() const;

    void      SetTransform(const std::shared_ptr<glm::mat4>) override;
    void      SetTransform(const glm::mat4&) override;
    glm::mat4 GetTransform() const override;

    void Translate(const glm::vec3& trans) override;
    void Rotate(float alph, const glm::vec3& axes) override;
    void Scale(const glm::vec3& coefs) override;

    glm::mat4 GetLocalTransform() const;
    void      SetLocalTransform(const glm::mat4& local);
    void      UpdateWorldRecursive();

private:
    std::list<std::shared_ptr<BaseMesh> > meshes_;
    std::list<std::shared_ptr<Node> > children_;
    std::weak_ptr<Node>               parent_;
    Transformation                    local_;
    glm::mat4                         world_ {1.0f};
};

} // namespace Multor

#endif // NODE_H
