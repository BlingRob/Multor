/// \file node.cpp
#include "node.h"

namespace Multor
{

Node::Node()
    : local_(glm::mat4(1.0f)), world_(1.0f)
{
}

void Node::addMesh(std::shared_ptr<BaseMesh> mesh)
{
    if (!mesh)
        return;
    meshes_.push_back(std::move(mesh));
}

void Node::addChild(std::shared_ptr<Node> child)
{
    if (!child)
        return;

    child->parent_ = weak_from_this();
    children_.push_back(std::move(child));
    children_.back()->UpdateWorldRecursive();
}

std::pair<Node::NIt, Node::NIt> Node::GetChildren() const
{
    return std::make_pair(children_.cbegin(), children_.cend());
}

std::pair<Node::MIt, Node::MIt> Node::GetMeshes() const
{
    return std::make_pair(meshes_.cbegin(), meshes_.cend());
}

void Node::SetTransform(const std::shared_ptr<glm::mat4> mat)
{
    if (!mat)
        return;
    SetLocalTransform(*mat);
}

void Node::SetTransform(const glm::mat4& mat)
{
    SetLocalTransform(mat);
}

glm::mat4 Node::GetTransform() const
{
    return world_;
}

void Node::Translate(const glm::vec3& trans)
{
    local_.Translate(trans);
    UpdateWorldRecursive();
}

void Node::Rotate(float alph, const glm::vec3& axes)
{
    local_.Rotate(alph, axes);
    UpdateWorldRecursive();
}

void Node::Scale(const glm::vec3& coefs)
{
    local_.Scale(coefs);
    UpdateWorldRecursive();
}

glm::mat4 Node::GetLocalTransform() const
{
    return local_.GetTransform();
}

void Node::SetLocalTransform(const glm::mat4& local)
{
    local_.SetTransform(local);
    UpdateWorldRecursive();
}

void Node::UpdateWorldRecursive()
{
    const auto parent = parent_.lock();
    if (parent)
        world_ = parent->world_ * local_.GetTransform();
    else
        world_ = local_.GetTransform();

    for (auto& child : children_)
        {
            if (child)
                child->UpdateWorldRecursive();
        }
}

} // namespace Multor
