/// \file model.cpp

#include "model.h"

#include <utility>

namespace Multor
{

Model::Model(std::function<void()> resourceDeleter)
    : deleter_(std::move(resourceDeleter)), root_(std::make_shared<Node>())
{
}

Model::~Model()
{
    if (deleter_)
        deleter_();
}

Model::Model(Model&& other) noexcept
    : deleter_(std::move(other.deleter_)),
      root_(std::move(other.root_)),
      meshes_(std::move(other.meshes_)),
      transform_(other.transform_)
{
    if (!root_)
        root_ = std::make_shared<Node>();
}

Model& Model::operator=(Model&& other) noexcept
{
    if (this == &other)
        return *this;

    deleter_   = std::move(other.deleter_);
    root_      = std::move(other.root_);
    meshes_    = std::move(other.meshes_);
    transform_ = other.transform_;

    if (!root_)
        root_ = std::make_shared<Node>();
    return *this;
}

void Model::SetRoot(std::shared_ptr<Node> root)
{
    root_ = root ? std::move(root) : std::make_shared<Node>();
    SyncRootWithModelTransform();
}

std::shared_ptr<Node> Model::GetRoot() const
{
    return root_;
}

void Model::AddMesh(std::shared_ptr<BaseMesh> mesh)
{
    if (!mesh)
        return;
    meshes_.push_back(std::move(mesh));
}

std::pair<Model::MeshIt, Model::MeshIt> Model::GetMeshes() const
{
    return std::make_pair(meshes_.cbegin(), meshes_.cend());
}

void Model::SetTransform(const std::shared_ptr<glm::mat4> mat)
{
    if (!mat)
        return;
    transform_.SetTransform(mat);
    SyncRootWithModelTransform();
}

void Model::SetTransform(const glm::mat4& mat)
{
    transform_.SetTransform(mat);
    SyncRootWithModelTransform();
}

glm::mat4 Model::GetTransform() const
{
    return transform_.GetTransform();
}

void Model::Translate(const glm::vec3& trans)
{
    transform_.Translate(trans);
    SyncRootWithModelTransform();
}

void Model::Rotate(float alph, const glm::vec3& axes)
{
    transform_.Rotate(alph, axes);
    SyncRootWithModelTransform();
}

void Model::Scale(const glm::vec3& coefs)
{
    transform_.Scale(coefs);
    SyncRootWithModelTransform();
}

void Model::SyncRootWithModelTransform()
{
    if (!root_)
        return;
    root_->SetTransform(transform_.GetTransform());
}

} // namespace Multor
