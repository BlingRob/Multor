/// \file mesh_factory.cpp

#include "mesh_factory.h"

namespace Multor::Vulkan
{

std::unique_ptr<Mesh>
MeshFactory::CreateMesh(std::unique_ptr<BaseMesh> mesh)
{
    std::unique_ptr<Mesh> vk_mesh = std::make_unique<Mesh>();

    constexpr VkDeviceSize MatBufObj   = sizeof(Material);
    constexpr VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
    constexpr VkDeviceSize ViewBufObj  = sizeof(UBOs::ViewPosition);

    vk_mesh->vertBuffer_  = CreateVertexBuffer(mesh->GetVertexes());
    vk_mesh->indexBuffer_ = CreateIndexBuffer(mesh->GetVertexes());
    vk_mesh->indexesSize_ = static_cast<std::uint32_t>(mesh->GetVertexes()->GetIndices().size());

    auto [texBegin, texEnd] = mesh->GetTextures();
    for (auto it = texBegin; it != texEnd; ++it)
        {
            if (!(*it))
                continue;

            auto images = (*it)->GetImages();
            if (images.empty() || !images[0])
                continue;

            vk_mesh->textures_.push_back(
                std::shared_ptr<Texture>(CreateTexture(images[0].get())));
        }
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
MeshFactory::CreateUBOBuffers(std::size_t nFrames)
{
    std::unique_ptr<TransformUBO> ubo = std::make_unique<TransformUBO>(dev_);
    for (std::size_t i = 0; i < nFrames; ++i)
        {
            ubo->matrixes_.push_back(CreateBuffer(
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
