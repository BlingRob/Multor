/// \file mesh.h

#pragma once
#ifndef MESH_H
#define MESH_H

#include "Shader.h"
#include "material.h"
#include "texture.h"
#include "vertexes.h"

namespace Multor
{

class BaseMesh : public Entity
{
public:
    using TexIT = std::vector<std::shared_ptr<BaseTexture> >::iterator;

    BaseMesh(std::unique_ptr<Vertexes>                    verts,
             std::unique_ptr<Material>                    mat = nullptr,
             std::vector<std::shared_ptr<BaseTexture> >&& texes =
                 std::vector<std::shared_ptr<BaseTexture> >(0));
    /*  Functions  */
    virtual ~BaseMesh()
    {
    }
    //void virtual setupMesh();
    //void virtual Draw(const Shader* shader) = 0;
    void SetMaterial(std::unique_ptr<Material> mat);
    void SetVertexes(std::unique_ptr<Vertexes> verts);
    void AddTexture(std::shared_ptr<BaseTexture> tex);
    std::unique_ptr<BaseMesh> Clone() const;
    //
    Vertexes*               GetVertexes();
    Material*               GetMaterial();
    std::pair<TexIT, TexIT> GetTextures();
    //
    /* Mesh's constants */
    static const std::size_t CardCoordsPerPoint     = 3;
    static const std::size_t CardCoordsPerTextPoint = 2;

protected:
    /*  Mesh Data  */
    std::unique_ptr<Vertexes> vertices_;
    std::unique_ptr<Material> material_;
    /* Textures */
    std::vector<std::shared_ptr<BaseTexture> > textures_;
};

} // namespace Multor

#endif // MESH_H
