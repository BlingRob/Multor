/// \file Mesh.cpp
#include "Mesh.h"

void TransformUBO::updateModel(std::size_t frame, const glm::mat4& newTransformMatrix)
{
	void* data;
	vkMapMemory(_dev, matrixes[frame]->_bufferMemory, 0,
		TransBufObj, 0, &data);
	memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, model), &newTransformMatrix, sizeof(glm::mat4));
	vkUnmapMemory(_dev, matrixes[frame]->_bufferMemory);
}

void TransformUBO::updateView(std::size_t frame, const glm::vec3& newPos)
{
	void* data;
	vkMapMemory(_dev, ViewPosUBO[frame]->_bufferMemory, 0,
		ViewBufObj, 0, &data);
	std::memcpy(data, &newPos, ViewBufObj);
	vkUnmapMemory(_dev, ViewPosUBO[frame]->_bufferMemory);
}

void TransformUBO::updatePV(std::size_t frame, const glm::mat4& newProjectViewMatrix)
{
	void* data;
	vkMapMemory(_dev, matrixes[frame]->_bufferMemory, 0,
		TransBufObj, 0, &data);
	memcpy(static_cast<char*>(data) + offsetof(UBOs::Transform, PV), &newProjectViewMatrix, sizeof(glm::mat4));
	vkUnmapMemory(_dev, matrixes[frame]->_bufferMemory);
}

Vertexes* BaseMesh::GetVertexes() 
{
	return vertices.get();
}

Material* BaseMesh::GetMaterial() 
{
	return material.get();
}

std::pair<BaseMesh::TexIT, BaseMesh::TexIT> BaseMesh::GetTextures() 
{
	return std::make_pair<BaseMesh::TexIT>(textures.begin(),textures.end());
}

BaseMesh::BaseMesh(std::unique_ptr<Vertexes> verts, std::unique_ptr<Material> mat, std::vector<std::shared_ptr<BaseTexture>>&& texes)
{
	vertices = std::move(verts);
	material = (mat)?std::move(mat):std::make_unique<Material>();
	/* Textures */
	textures = std::forward<std::vector<std::shared_ptr<BaseTexture>>>(texes);
}

void BaseMesh::SetMaterial(std::unique_ptr<Material> mat)
{ 
	material = std::move(mat);
}

void BaseMesh::SetVertexes(std::unique_ptr<Vertexes> verts)
{ 
	vertices = std::move(verts);
}

void BaseMesh::AddTexture(std::shared_ptr<BaseTexture> tex)
{
	textures.emplace_back(std::move(tex));
}

std::unique_ptr<VkMesh> VkMeshFactory::createMesh(std::unique_ptr<BaseMesh> mesh) 
{
	std::unique_ptr<VkMesh> Vkmesh = std::make_unique<VkMesh>();

	constexpr VkDeviceSize MatBufObj = sizeof(Material);
	constexpr VkDeviceSize TransBufObj = sizeof(UBOs::Transform);
	constexpr VkDeviceSize ViewBufObj = sizeof(UBOs::ViewPosition);

	Vkmesh->VertBuffer = createVertexBuffer(mesh->GetVertexes());
	Vkmesh->IndexBuffer = createIndexBuffer(mesh->GetVertexes());
	Vkmesh->IndexesSize = mesh->GetVertexes()->GetIndices().size();

	//Vkmesh->MaterialBuffer = createMaterialBuffer(mesh->GetMaterial());
	//uint16_t i = 0;
	//for (auto it = mesh->GetTextures(); it.first != it.second; ++it.first)
	//	Vkmesh->textures.emplace_back(CreateTexture(it.first->get()->GetImages()[i].get())),++i;
	/*
	Vkmesh->matrixes = createBuffer(TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	Vkmesh->MaterialUBO = createBuffer(MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	Vkmesh->ViewPosUBO = createBuffer(ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);*/

	return Vkmesh;
}

std::unique_ptr <TransformUBO> VkMeshFactory::createUBOBuffers(std::size_t nFrames)
{
	std::unique_ptr <TransformUBO> ubo = std::make_unique<TransformUBO>(_dev);
	for (std::size_t i = 0; i < nFrames; ++i)
	{
		ubo->matrixes.push_back(createBuffer(ubo->TransBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));
		/*
		mesh->MaterialUBO.push_back(MeshFac->createBuffer(MatBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));

		mesh->ViewPosUBO.push_back(MeshFac->createBuffer(ViewBufObj, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VK_MEMORY_PROPERTY_HOST_COHERENT_BIT));*/
		ubo->updateModel(i, glm::mat4(1.0f));
		ubo->updatePV(i, glm::mat4(1.0f));
	}

	return ubo;
}

