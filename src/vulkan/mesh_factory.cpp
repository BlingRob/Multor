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

    auto uploadTexture = [this](const std::shared_ptr<BaseTexture>& tex)
        -> std::shared_ptr<Texture>
    {
        if (!tex)
            return nullptr;
        auto images = tex->GetImages();
        if (images.empty() || !images[0])
            return nullptr;
        return std::shared_ptr<Texture>(CreateTexture(images[0].get()));
    };

    vk_mesh->baseColorTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Diffuse));
    vk_mesh->normalTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Normal));
    vk_mesh->metallicTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Metallic));
    if (!vk_mesh->metallicTex_)
        vk_mesh->metallicTex_ =
            uploadTexture(mesh->FindTexture(Texture_Types::Metallic_roughness));
    vk_mesh->roughnessTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Roughness));
    if (!vk_mesh->roughnessTex_)
        vk_mesh->roughnessTex_ =
            uploadTexture(mesh->FindTexture(Texture_Types::Metallic_roughness));
    vk_mesh->aoTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Ambient_occlusion));
    vk_mesh->emissiveTex_ = uploadTexture(mesh->FindTexture(Texture_Types::Emissive));
    vk_mesh->materialData_ =
        mesh->GetMaterial() ? UBOs::PackMaterial(*mesh->GetMaterial(), mesh.get())
                            : UBOs::PackMaterial(Material {});
    vk_mesh->castsShadows_ = mesh->CastsShadows();

    for (const auto& tex : {vk_mesh->baseColorTex_, vk_mesh->normalTex_,
                            vk_mesh->metallicTex_, vk_mesh->roughnessTex_,
                            vk_mesh->aoTex_, vk_mesh->emissiveTex_})
        if (tex)
            vk_mesh->textures_.push_back(tex);
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
MeshFactory::CreateUBOBuffers(const UBOs::MaterialData& material, std::size_t nFrames)
{
    std::unique_ptr<TransformUBO> ubo = std::make_unique<TransformUBO>(dev_);
    for (std::size_t i = 0; i < nFrames; ++i)
        {
            ubo->matrixes_.push_back(CreateBuffer(
                ubo->TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            ubo->viewPosUBO_.push_back(CreateBuffer(
                ubo->ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            ubo->materialUBO_.push_back(CreateBuffer(
                ubo->MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
            ubo->updateModel(i, glm::mat4(1.0f));
            ubo->updatePV(i, glm::mat4(1.0f));
            ubo->updateView(i, glm::vec3(0.0f));
            ubo->updateMaterial(i, material);
        }

    return ubo;
}

} // namespace Multor::Vulkan
