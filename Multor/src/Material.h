/// \file Material.h
#pragma once

#include "Entity.h"
#include "ImageLoader.h"
#include "VulkanObjects.h"

#include <array>
#include <bitset>
#include <vector>
#include <memory>

#include <glm/glm.hpp>
//#include "assimp/scene.h"
//#include "Loaders/ImageLoader.h"

namespace Multor
{

class Vertexes
{
public:
    Vertexes(std::size_t size = 0, const float* positions = nullptr, std::vector<std::uint32_t>&& inds = std::vector<std::uint32_t>(0), const float* normals = nullptr, const float* textureCoords = nullptr, const float* tangent = nullptr, const float* bitangent = nullptr);
    Vertexes(Vertexes&&);
    Vertexes&& operator=(Vertexes&&);
    void AddIndices(std::vector<std::uint32_t>&& inds);
    /*Return amount structs vertex numbers*/
    std::size_t GetSize();
    std::vector<std::uint32_t>& GetIndices();
    Vertex* GetVertexes();
private:
    std::size_t _size;
    std::vector<Vertex> _verts;
    std::vector<std::uint32_t> _indices;
};

enum class Texture_Types:uint8_t {Diffuse = 0, Normal = 1, Specular = 2, Emissive = 3, Height = 4, Metallic_roughness = 5, Ambient_occlusion = 6, Skybox = 7};

struct BaseTexture:public Entity
{
    BaseTexture(const std::string& name, const std::string& path, Texture_Types type, std::vector<std::shared_ptr<Image>> images);
    virtual ~BaseTexture() {};
    //virtual bool createTexture();
    bool IsCreated();
    unsigned int GetId();
    std::string GetPath();
    Texture_Types GetType();
    std::vector<std::shared_ptr<Image>> GetImages();
    void AddImage(std::shared_ptr<Image> img);
protected:
    bool Created{false};
    unsigned int id;
    std::vector<std::shared_ptr<Image>> _imgs;
    Texture_Types _type;
    std::string _path;
private:

};

struct Material
{
    Material():ambient(0), diffuse(0), specular(0), shininess(0){}
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

} // namespace Multor