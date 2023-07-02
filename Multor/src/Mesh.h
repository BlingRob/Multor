/// \file Mesh.h

#pragma once
#ifndef MESH_H
#define MESH_H

#include "Shader.h"
#include "Material.h"
#include "VkTexture.h"

namespace Multor
{

namespace UBOs
{
struct Transform
{
    glm::mat4 model_;
    glm::mat4 PV_;
    glm::mat3 normalMatrix_;
};

struct ViewPosition
{
    glm::vec3 viewPos_;
};
}

struct TransformUBO
{
    TransformUBO(VkDevice dev) : dev_(dev)
    {
    }

    void updateModel(std::size_t frame, const glm::mat4& newTransformMatrix);
    void updateView(std::size_t frame, const glm::vec3& newPosition);
    void updatePV(std::size_t frame, const glm::mat4& newProjectViewMatrix);

    std::vector<std::unique_ptr<VulkanBuffer> > matrixes_;
    std::vector<std::unique_ptr<VulkanBuffer> > viewPosUBO_;
    std::vector<std::unique_ptr<VulkanBuffer> > materialUBO_;

    static const VkDeviceSize MatBufObj   = sizeof(Material);
    static const VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
    static const VkDeviceSize ViewBufObj  = sizeof(UBOs::ViewPosition);

private:
    VkDevice dev_;
};

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

struct VkMesh
{
    /* Static object */
    std::unique_ptr<VertexBuffer> vertBuffer_;
    std::uint32_t                 indexesSize_;
    std::unique_ptr<VulkanBuffer> indexBuffer_;
    /* Textures */
    std::vector<std::shared_ptr<VkTexture> > textures_;
    /*  Dynamic object  */

    std::shared_ptr<vkShader> sh_;

    //std::unique_ptr<VulkanBuffer> MaterialBuffer;
    std::vector<VkDescriptorSet>  desSet_;
    std::unique_ptr<TransformUBO> tr_;
};

class VkMeshFactory : public VkTextureFactory
{
public:
    VkMeshFactory(VkDevice& dev, VkPhysicalDevice& physDev,
                  std::shared_ptr<CommandExecuter> ex)
        : VkTextureFactory(dev, physDev, std::move(ex))
    {
    }
    std::unique_ptr<VkMesh>       createMesh(std::unique_ptr<BaseMesh> mesh);
    std::unique_ptr<TransformUBO> createUBOBuffers(std::size_t nFrames);
};

} // namespace Multor

#endif // MESH_H