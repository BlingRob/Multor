/// \file Mesh.cpp
#include "Mesh.h"

namespace Multor
{

void TransformUBO::updateModel(std::size_t      frame,
                               const glm::mat4& newTransformMatrix)
{
    void* data;
    vkMapMemory(dev_, matrixes_[frame]->bufferMemory_, 0, TransBufObj, 0,
                &data);
    memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, model_),
           &newTransformMatrix, sizeof(glm::mat4));
    vkUnmapMemory(dev_, matrixes_[frame]->bufferMemory_);
}

void TransformUBO::updateView(std::size_t frame, const glm::vec3& newPos)
{
    void* data;
    vkMapMemory(dev_, viewPosUBO_[frame]->bufferMemory_, 0, ViewBufObj, 0,
                &data);
    std::memcpy(data, &newPos, ViewBufObj);
    vkUnmapMemory(dev_, viewPosUBO_[frame]->bufferMemory_);
}

void TransformUBO::updatePV(std::size_t      frame,
                            const glm::mat4& newProjectViewMatrix)
{
    void* data;
    vkMapMemory(dev_, matrixes_[frame]->bufferMemory_, 0, TransBufObj, 0,
                &data);
    memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, PV_),
           &newProjectViewMatrix, sizeof(glm::mat4));
    vkUnmapMemory(dev_, matrixes_[frame]->bufferMemory_);
}

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

std::unique_ptr<VkMesh>
VkMeshFactory::createMesh(std::unique_ptr<BaseMesh> mesh)
{
    std::unique_ptr<VkMesh> Vkmesh = std::make_unique<VkMesh>();

    constexpr VkDeviceSize MatBufObj   = sizeof(Material);
    constexpr VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
    constexpr VkDeviceSize ViewBufObj  = sizeof(UBOs::ViewPosition);

    Vkmesh->vertBuffer_  = createVertexBuffer(mesh->GetVertexes());
    Vkmesh->indexBuffer_ = createIndexBuffer(mesh->GetVertexes());
    Vkmesh->indexesSize_ = mesh->GetVertexes()->GetIndices().size();

    //Vkmesh->MaterialBuffer = createMaterialBuffer(mesh->GetMaterial());
    //uint16_t i = 0;
    //for (auto it = mesh->GetTextures(); it.first != it.second; ++it.first)
    //	Vkmesh->textures_.emplace_back(CreateTexture(it.first->get()->GetImages()[i].get())),++i;
    /*
	Vkmesh->matrixes_ = createBuffer(TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	Vkmesh->materialUBO_ = createBuffer(MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	Vkmesh->viewPosUBO_ = createBuffer(ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);*/

    return Vkmesh;
}

std::unique_ptr<TransformUBO>
VkMeshFactory::createUBOBuffers(std::size_t nFrames)
{
    std::unique_ptr<TransformUBO> ubo = std::make_unique<TransformUBO>(dev_);
    for (std::size_t i = 0; i < nFrames; ++i)
        {
            ubo->matrixes_.push_back(createBuffer(
                ubo->TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            /*
		mesh->materialUBO_.push_back(MeshFac->createBuffer(MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		mesh->viewPosUBO_.push_back(MeshFac->createBuffer(ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));*/
            ubo->updateModel(i, glm::mat4(1.0f));
            ubo->updatePV(i, glm::mat4(1.0f));
        }

    return ubo;
}

} // namespace Multor