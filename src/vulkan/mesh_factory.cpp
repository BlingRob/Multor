/// \file mesh_factory.cpp

#include "mesh_factory.h"

namespace Multor::Vulkan
{

std::unique_ptr<Mesh>
MeshFactory::createMesh(std::unique_ptr<BaseMesh> mesh)
{
    std::unique_ptr<Mesh> vk_mesh = std::make_unique<Mesh>();

    constexpr VkDeviceSize MatBufObj   = sizeof(Material);
    constexpr VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
    constexpr VkDeviceSize ViewBufObj  = sizeof(UBOs::ViewPosition);

    vk_mesh->vertBuffer_  = createVertexBuffer(mesh->GetVertexes());
    vk_mesh->indexBuffer_ = createIndexBuffer(mesh->GetVertexes());
    vk_mesh->indexesSize_ = mesh->GetVertexes()->GetIndices().size();

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

    return vk_mesh;
}

std::unique_ptr<TransformUBO>
MeshFactory::createUBOBuffers(std::size_t nFrames)
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

} // namespace Multor::Vulkan
