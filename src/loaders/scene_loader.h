/// \file scene_loader.h

#pragma once
#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include "../model.h"
#include "../scene.h"
#include "../utils/image_loader.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace Multor
{

class SceneLoader
{
public:
    SceneLoader();

    bool LoadScene(std::string_view path);
    bool IsLoaded() const;
    std::string GetError() const;

    std::shared_ptr<Scene>  GetScene(std::shared_ptr<PositionController> controller = nullptr);
    std::shared_ptr<Model>  GetModel(std::uint32_t index);
    std::shared_ptr<Model>  GetModel(std::string_view name);
    std::shared_ptr<BLight> GetLight(std::uint32_t index);
    std::shared_ptr<BLight> GetLight(std::string_view name);
    std::shared_ptr<Camera> GetCamera(std::uint32_t index);

    std::uint32_t NumModels() const;
    std::uint32_t NumLights() const;
    std::uint32_t NumCameras() const;
    bool          HasLight() const;
    bool          HasCamera() const;

    static glm::mat4 ToGlm(const aiMatrix4x4& m);

private:
    std::shared_ptr<Model>  processModel(aiNode* root);
    std::shared_ptr<Node>   processNode(aiNode* node);
    std::shared_ptr<BaseMesh> processMesh(aiMesh* mesh);
    std::shared_ptr<BLight> processLight(aiLight* src, const glm::mat4& parentTransform);
    std::unique_ptr<Material> processMaterial(aiMaterial* src);
    std::vector<std::shared_ptr<BaseTexture> >
    processTextures(aiMaterial* src, aiTextureType type, Texture_Types targetType);

private:
    Assimp::Importer importer_;
    const aiScene*   scene_ = nullptr;
    bool             loaded_ = false;
    std::string      error_;
    std::string      sceneDir_;
    std::map<std::size_t, std::weak_ptr<BaseTexture> > textureCache_;
};

} // namespace Multor

#endif // SCENE_LOADER_H
