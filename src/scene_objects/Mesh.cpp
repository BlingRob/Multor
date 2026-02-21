/// \file mesh.cpp
#include "mesh.h"

namespace Multor
{

Vertexes* BaseMesh::GetVertexes()
{
    return vertices_.get();
}

Material* BaseMesh::GetMaterial()
{
    return material_.get();
}

std::pair<BaseMesh::TexIT, BaseMesh::TexIT> BaseMesh::GetTextures()
{
    return std::make_pair<BaseMesh::TexIT>(textures_.begin(), textures_.end());
}

BaseMesh::BaseMesh(std::unique_ptr<Vertexes>                    verts,
                   std::unique_ptr<Material>                    mat,
                   std::vector<std::shared_ptr<BaseTexture> >&& texes)
{
    vertices_ = std::move(verts);
    material_ = (mat) ? std::move(mat) : std::make_unique<Material>();
    /* Textures */
    textures_ =
        std::forward<std::vector<std::shared_ptr<BaseTexture> > >(texes);
}

void BaseMesh::SetMaterial(std::unique_ptr<Material> mat)
{
    material_ = std::move(mat);
}

void BaseMesh::SetVertexes(std::unique_ptr<Vertexes> verts)
{
    vertices_ = std::move(verts);
}

void BaseMesh::AddTexture(std::shared_ptr<BaseTexture> tex)
{
    textures_.emplace_back(std::move(tex));
}

} // namespace Multor