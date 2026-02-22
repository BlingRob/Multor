/// \file vertexes.h

#pragma once
#ifndef VERTEXES_H
#define VERTEXES_H

#include "vertex.h"

#include <vector>
#include <memory>

namespace Multor
{

class Vertexes
{
public:
    Vertexes(std::size_t size = 0, const float* positions = nullptr,
             std::vector<std::uint32_t>&& inds = std::vector<std::uint32_t>(0),
             const float*                 normals       = nullptr,
             const float*                 textureCoords = nullptr,
             const float* tangent = nullptr, const float* bitangent = nullptr);

    Vertexes(Vertexes&&);

    Vertexes&& operator=(Vertexes&&);

    void AddIndices(std::vector<std::uint32_t>&& inds);

    /*Return amount structs vertex numbers*/
    std::size_t GetSize();

    std::vector<std::uint32_t>& GetIndices();
    std::unique_ptr<Vertexes>    Clone() const;

    Vertex* GetVertexes();

private:
    std::size_t                size_;
    std::vector<Vertex>        verts_;
    std::vector<std::uint32_t> indices_;
};

} // namespace Multor

#endif // VERTEXES_H
