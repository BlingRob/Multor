/// \file vk_mesh_factory.cpp

#include "vk_mesh_factory.h"

namespace Multor
{

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
