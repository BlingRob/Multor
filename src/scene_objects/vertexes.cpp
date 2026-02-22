/// \file vertexes.cpp

#include "vertexes.h"

namespace Multor
{

Vertexes::Vertexes(std::size_t size, const float* positions,
                   std::vector<std::uint32_t>&& inds, const float* normals,
                   const float* textureCoords, const float* tangent,
                   const float* bitangent)
{
    if (size == 0)
        return;

    size_ = size;
    verts_.resize(size);
    uint32_t i = 0;

    if (positions)
        {
            for (auto& vert : verts_)
                vert.pos.x = positions[i], vert.pos.y = positions[i + 1],
                vert.pos.z = positions[i + 2], i += 3;
            i = 0;
        }

    if (!inds.empty())
        {
            indices_ = std::forward<std::vector<std::uint32_t> >(inds);
        }

    if (normals)
        {
            for (auto& vert : verts_)
                vert.norm.x = normals[i], vert.norm.y = normals[i + 1],
                vert.norm.z = normals[i + 2], i += 3;
            i = 0;
        }

    if (textureCoords)
        {
            for (auto& vert : verts_)
                vert.texCoord.x = textureCoords[i],
                vert.texCoord.y = textureCoords[i + 1], i += 2;
            i = 0;
        }

    if (tangent)
        {
            for (auto& vert : verts_)
                vert.aTan.x = tangent[i], vert.aTan.y = tangent[i + 1],
                vert.aTan.z = tangent[i + 2], i += 3;
            i = 0;
        }

    if (bitangent)
        {
            for (auto& vert : verts_)
                vert.aBitan.x = bitangent[i], vert.aBitan.y = bitangent[i + 1],
                vert.aBitan.z = bitangent[i + 2], i += 3;
            i = 0;
        }
}

Vertex* Vertexes::GetVertexes()
{
    return verts_.data();
}

std::vector<std::uint32_t>& Vertexes::GetIndices()
{
    return indices_;
}

std::unique_ptr<Vertexes> Vertexes::Clone() const
{
    auto out = std::make_unique<Vertexes>();
    out->size_   = size_;
    out->verts_  = verts_;
    out->indices_ = indices_;
    return out;
}

void Vertexes::AddIndices(std::vector<std::uint32_t>&& inds)
{
    indices_ = std::forward<std::vector<std::uint32_t> >(inds);
}

Vertexes::Vertexes(Vertexes&& vecs)
{
    verts_   = std::forward<std::vector<Vertex> >(vecs.verts_);
    indices_ = std::forward<std::vector<std::uint32_t> >(vecs.indices_);
}

Vertexes&& Vertexes::operator=(Vertexes&& vecs)
{
    verts_   = std::forward<std::vector<Vertex> >(vecs.verts_);
    indices_ = std::forward<std::vector<std::uint32_t> >(vecs.indices_);

    return std::forward<Vertexes>(*this);
}

std::size_t Vertexes::GetSize()
{
    return verts_.size();
}

} // namespace Multor
