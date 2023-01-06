/// \file Material.cpp
#include "Material.h"

Vertexes::Vertexes(std::size_t size, const float* positions, std::vector<std::uint32_t>&& inds, const float* normals, const float* textureCoords, const float* tangent, const float* bitangent)
{
	if (size == 0)
		return;
	_size = size;
	_verts.resize(size);
	uint32_t i = 0;

	if (positions)
	{
		for (auto& vert : _verts)
			vert.pos.x = positions[i], vert.pos.y = positions[i + 1], vert.pos.z = positions[i + 2], i += 3;
		i = 0;
	}

	if (!inds.empty()) 
	{
		_indices = std::forward<std::vector<std::uint32_t>>(inds);
	}

	if (normals) 
	{
		for (auto& vert : _verts)
			vert.norm.x = normals[i], vert.norm.y = normals[i + 1], vert.norm.z = normals[i + 2], i += 3;
		i = 0;
	}

	if (textureCoords)
	{
		for (auto& vert : _verts)
			vert.texCoord.x = textureCoords[i], vert.texCoord.y = textureCoords[i + 1], i += 2;
		i = 0;
	}

	if (tangent)
	{
		for (auto& vert : _verts)
			vert.aTan.x = tangent[i], vert.aTan.y = tangent[i + 1], vert.aTan.z = tangent[i + 2], i += 3;
		i = 0;
	}

	if (bitangent)
	{
		for (auto& vert : _verts)
			vert.aBitan.x = bitangent[i], vert.aBitan.y = bitangent[i + 1], vert.aBitan.z = bitangent[i + 2], i += 3;
		i = 0;
	}
}


Vertex* Vertexes::GetVertexes() 
{
	return _verts.data();
}

std::vector<std::uint32_t>& Vertexes::GetIndices()
{
	return _indices;
}

void Vertexes::AddIndices(std::vector<std::uint32_t>&& inds)
{
	_indices = std::forward<std::vector<std::uint32_t>>(inds);
}

Vertexes::Vertexes(Vertexes&& vecs) 
{
	_verts = std::forward<std::vector<Vertex>>(vecs._verts);
	_indices = std::forward<std::vector<std::uint32_t>>(vecs._indices);
}

Vertexes&& Vertexes::operator=(Vertexes&& vecs)
{
	_verts = std::forward<std::vector<Vertex>>(vecs._verts);
	_indices = std::forward<std::vector<std::uint32_t>>(vecs._indices);

	return std::forward<Vertexes>(*this);
}

std::size_t Vertexes::GetSize()
{
	return _verts.size();
}

BaseTexture::BaseTexture(const std::string& name, const std::string& path, Texture_Types type, std::vector<std::shared_ptr<Image>> images) 
{
	SetName(name);
	_path = path;
	_type = type;
	_imgs = images;
}

bool BaseTexture::IsCreated()
{
	return Created;
}

unsigned int BaseTexture::GetId() 
{ 
	return id;
}

std::string BaseTexture::GetPath()
{ 
	return _path;
}

Texture_Types BaseTexture::GetType()
{ 
	return _type;
}

std::vector<std::shared_ptr<Image>> BaseTexture::GetImages()
{ 
	return _imgs;
}

void BaseTexture::AddImage(std::shared_ptr<Image> img) 
{
	_imgs.emplace_back(std::move(img));
}