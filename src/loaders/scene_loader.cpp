/// \file scene_loader.cpp

#include "scene_loader.h"

#include <filesystem>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace Multor
{
namespace
{
void collectNodeMeshes(const std::shared_ptr<Node>& node, Model& model)
{
    if (!node)
        return;

    auto meshes = node->GetMeshes();
    for (auto it = meshes.first; it != meshes.second; ++it)
        if (*it)
            model.AddMesh(*it);

    auto children = node->GetChildren();
    for (auto it = children.first; it != children.second; ++it)
        collectNodeMeshes(*it, model);
}
} // namespace

SceneLoader::SceneLoader() = default;

bool SceneLoader::LoadScene(std::string_view path)
{
    error_.clear();
    loaded_ = false;
    textureCache_.clear();
    sceneDir_.clear();

    std::filesystem::path fsPath {std::string(path)};
    if (fsPath.has_parent_path())
        sceneDir_ = fsPath.parent_path().string();

    scene_ = importer_.ReadFile(
        std::string(path),
        aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Triangulate |
            aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene_ || (scene_->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
        !scene_->mRootNode)
        {
            error_ = importer_.GetErrorString();
            scene_ = nullptr;
            return false;
        }

    loaded_ = true;
    return true;
}

bool SceneLoader::IsLoaded() const
{
    return loaded_;
}

std::string SceneLoader::GetError() const
{
    return error_;
}

std::shared_ptr<Scene>
SceneLoader::GetScene(std::shared_ptr<PositionController> controller)
{
    if (!loaded_ || !scene_)
        return nullptr;

    auto out = std::make_shared<Scene>(std::move(controller));

    if (HasCamera())
        {
            auto cam = GetCamera(0);
            if (cam && out->GetController())
                out->GetController()->cam_ = cam;
        }

    if (HasLight())
        {
            for (std::uint32_t i = 0; i < NumLights(); ++i)
                {
                    auto light = GetLight(i);
                    if (light)
                        out->AddLight(light);
                }
        }

    for (std::uint32_t i = 0; i < NumModels(); ++i)
        {
            auto model = GetModel(i);
            if (model)
                out->AddModel(model);
        }

    return out;
}

std::shared_ptr<Model> SceneLoader::GetModel(std::uint32_t index)
{
    if (!loaded_ || !scene_ || !scene_->mRootNode)
        return nullptr;
    if (index >= scene_->mRootNode->mNumChildren)
        return nullptr;
    return processModel(scene_->mRootNode->mChildren[index]);
}

std::shared_ptr<Model> SceneLoader::GetModel(std::string_view name)
{
    if (!loaded_ || !scene_ || !scene_->mRootNode)
        return nullptr;
    aiNode* node = scene_->mRootNode->FindNode(std::string(name).c_str());
    if (!node)
        return nullptr;
    return processModel(node);
}

std::shared_ptr<BLight> SceneLoader::GetLight(std::uint32_t index)
{
    if (!HasLight() || index >= scene_->mNumLights)
        return nullptr;

    aiLight* src = scene_->mLights[index];
    aiNode*  node = scene_->mRootNode ? scene_->mRootNode->FindNode(src->mName) : nullptr;
    glm::mat4 parentTransform(1.0f);
    if (node && node->mParent)
        parentTransform = ToGlm(node->mParent->mTransformation);

    return processLight(src, parentTransform);
}

std::shared_ptr<BLight> SceneLoader::GetLight(std::string_view name)
{
    if (!HasLight())
        return nullptr;

    for (std::uint32_t i = 0; i < scene_->mNumLights; ++i)
        {
            if (name == scene_->mLights[i]->mName.C_Str())
                return GetLight(i);
        }
    return nullptr;
}

std::shared_ptr<Camera> SceneLoader::GetCamera(std::uint32_t index)
{
    if (!HasCamera() || index >= scene_->mNumCameras)
        return nullptr;

    aiCamera* src = scene_->mCameras[index];
    auto cam = std::make_shared<Camera>();

    aiNode* node = scene_->mRootNode ? scene_->mRootNode->FindNode(src->mName) : nullptr;
    glm::mat4 parentTransform(1.0f);
    if (node && node->mParent)
        parentTransform = ToGlm(node->mParent->mTransformation);

    const glm::vec3 pos(src->mPosition.x, src->mPosition.y, src->mPosition.z);
    const glm::vec3 look(src->mLookAt.x, src->mLookAt.y, src->mLookAt.z);
    const glm::vec3 up(src->mUp.x, src->mUp.y, src->mUp.z);

    cam->position_ = glm::vec3(parentTransform * glm::vec4(pos, 1.0f));
    cam->front_    = glm::normalize(glm::mat3(parentTransform) * look);
    cam->worldUp_  = glm::normalize(glm::mat3(parentTransform) * up);
    cam->right_    = glm::normalize(glm::cross(cam->front_, cam->worldUp_));
    cam->up_       = glm::normalize(glm::cross(cam->right_, cam->front_));
    cam->SetName(src->mName.C_Str());
    return cam;
}

std::uint32_t SceneLoader::NumModels() const
{
    if (!loaded_ || !scene_ || !scene_->mRootNode)
        return 0;
    return scene_->mRootNode->mNumChildren;
}

std::uint32_t SceneLoader::NumLights() const
{
    return (loaded_ && scene_) ? scene_->mNumLights : 0;
}

std::uint32_t SceneLoader::NumCameras() const
{
    return (loaded_ && scene_) ? scene_->mNumCameras : 0;
}

bool SceneLoader::HasLight() const
{
    return NumLights() > 0;
}

bool SceneLoader::HasCamera() const
{
    return NumCameras() > 0;
}

glm::mat4 SceneLoader::ToGlm(const aiMatrix4x4& m)
{
    glm::mat4 out(1.0f);
    out[0] = glm::vec4(m[0][0], m[1][0], m[2][0], m[3][0]);
    out[1] = glm::vec4(m[0][1], m[1][1], m[2][1], m[3][1]);
    out[2] = glm::vec4(m[0][2], m[1][2], m[2][2], m[3][2]);
    out[3] = glm::vec4(m[0][3], m[1][3], m[2][3], m[3][3]);
    return out;
}

std::shared_ptr<Model> SceneLoader::processModel(aiNode* root)
{
    if (!root)
        return nullptr;

    auto model = std::make_shared<Model>();
    model->SetName(root->mName.C_Str());
    model->SetRoot(processNode(root));
    collectNodeMeshes(model->GetRoot(), *model);
    return model;
}

std::shared_ptr<Node> SceneLoader::processNode(aiNode* node)
{
    if (!node)
        return nullptr;

    auto cur = std::make_shared<Node>();
    cur->SetName(node->mName.C_Str());
    cur->SetLocalTransform(ToGlm(node->mTransformation));

    for (std::uint32_t i = 0; i < node->mNumMeshes; ++i)
        {
            const std::uint32_t meshIdx = node->mMeshes[i];
            if (meshIdx < scene_->mNumMeshes)
                cur->addMesh(processMesh(scene_->mMeshes[meshIdx]));
        }

    for (std::uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            auto child = processNode(node->mChildren[i]);
            if (child)
                cur->addChild(child);
        }

    return cur;
}

std::shared_ptr<BaseMesh> SceneLoader::processMesh(aiMesh* mesh)
{
    if (!mesh)
        return nullptr;

    std::vector<std::uint32_t> indices;
    indices.reserve(mesh->mNumFaces * 3u);
    for (std::uint32_t i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            for (std::uint32_t j = 0; j < face.mNumIndices; ++j)
                indices.push_back(face.mIndices[j]);
        }

    const float* positions = mesh->mVertices ? &mesh->mVertices[0].x : nullptr;
    const float* normals   = mesh->mNormals ? &mesh->mNormals[0].x : nullptr;
    const float* texcoords = (mesh->mTextureCoords[0] != nullptr)
                                 ? &mesh->mTextureCoords[0][0].x
                                 : nullptr;
    const float* tangents  = mesh->mTangents ? &mesh->mTangents[0].x : nullptr;
    const float* bitangents =
        mesh->mBitangents ? &mesh->mBitangents[0].x : nullptr;

    auto vertices = std::make_unique<Vertexes>(
        static_cast<std::size_t>(mesh->mNumVertices) *
            BaseMesh::CardCoordsPerPoint,
        positions, std::move(indices), normals, texcoords, tangents, bitangents);

    std::unique_ptr<Material> mat = std::make_unique<Material>();
    if (mesh->mMaterialIndex < scene_->mNumMaterials)
        mat = processMaterial(scene_->mMaterials[mesh->mMaterialIndex]);

    auto out = std::make_shared<BaseMesh>(std::move(vertices), std::move(mat));
    out->SetName(mesh->mName.C_Str());

    if (mesh->mMaterialIndex < scene_->mNumMaterials)
        {
            aiMaterial* srcMat = scene_->mMaterials[mesh->mMaterialIndex];
            auto appendTextures = [&out](std::vector<std::shared_ptr<BaseTexture> >&& texes)
            {
                for (auto& tex : texes)
                    if (tex)
                        out->AddTexture(tex);
            };
            appendTextures(processTextures(srcMat, aiTextureType_DIFFUSE,
                                           Texture_Types::Diffuse));
            appendTextures(processTextures(srcMat, aiTextureType_SPECULAR,
                                           Texture_Types::Specular));
            appendTextures(processTextures(srcMat, aiTextureType_NORMALS,
                                           Texture_Types::Normal));
            appendTextures(processTextures(srcMat, aiTextureType_EMISSIVE,
                                           Texture_Types::Emissive));
            appendTextures(processTextures(srcMat, aiTextureType_AMBIENT_OCCLUSION,
                                           Texture_Types::Ambient_occlusion));
        }

    return out;
}

std::shared_ptr<BLight> SceneLoader::processLight(aiLight* src,
                                                  const glm::mat4& parentTransform)
{
    if (!src)
        return nullptr;

    const glm::vec3 amb(src->mColorAmbient.r, src->mColorAmbient.g,
                        src->mColorAmbient.b);
    const glm::vec3 dif(src->mColorDiffuse.r, src->mColorDiffuse.g,
                        src->mColorDiffuse.b);
    const glm::vec3 spec(src->mColorSpecular.r, src->mColorSpecular.g,
                         src->mColorSpecular.b);
    const glm::vec3 clq(src->mAttenuationConstant, src->mAttenuationLinear,
                        src->mAttenuationQuadratic);
    const glm::vec3 pos = glm::vec3(parentTransform[3]);
    const glm::vec3 dir = glm::normalize(glm::vec3(
        src->mDirection.x, src->mDirection.y, src->mDirection.z));

    std::shared_ptr<BLight> out;
    switch (src->mType)
        {
        case aiLightSource_POINT:
            out = std::make_shared<PointLight>(amb, dif, spec, clq, pos);
            break;
        case aiLightSource_SPOT:
            out = std::make_shared<SpotLight>(
                amb, dif, spec, clq, pos, dir, src->mAngleOuterCone,
                src->mAngleInnerCone);
            break;
        case aiLightSource_DIRECTIONAL:
        default:
            out = std::make_shared<DirectionalLight>(amb, dif, spec, clq, dir);
            break;
        }
    if (out)
        out->SetName(src->mName.C_Str());
    return out;
}

std::unique_ptr<Material> SceneLoader::processMaterial(aiMaterial* src)
{
    auto out = std::make_unique<Material>();
    if (!src)
        return out;

    aiColor3D color(0.0f, 0.0f, 0.0f);
    float shininess = 0.0f;

    if (src->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS)
        out->ambient = glm::vec3(color.r, color.g, color.b);
    if (src->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS)
        out->diffuse = glm::vec3(color.r, color.g, color.b);
    if (src->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS)
        out->specular = glm::vec3(color.r, color.g, color.b);
    if (src->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS)
        out->shininess = shininess;

    return out;
}

std::vector<std::shared_ptr<BaseTexture> >
SceneLoader::processTextures(aiMaterial* src, aiTextureType type,
                             Texture_Types targetType)
{
    std::vector<std::shared_ptr<BaseTexture> > result;
    if (!src)
        return result;

    const unsigned int count = src->GetTextureCount(type);
    result.reserve(count);

    for (unsigned int i = 0; i < count; ++i)
        {
            aiString texPath;
            if (src->GetTexture(type, i, &texPath) != AI_SUCCESS)
                continue;

            const std::string pathStr = texPath.C_Str();
            const std::size_t hashKey =
                std::hash<std::string> {}(pathStr + "#" +
                                          std::to_string(static_cast<int>(targetType)));

            auto cacheIt = textureCache_.find(hashKey);
            if (cacheIt != textureCache_.end())
                {
                    if (auto cached = cacheIt->second.lock())
                        {
                            result.push_back(cached);
                            continue;
                        }
                }

            std::shared_ptr<Image> image;
            std::string resolvedPath = pathStr;

            if (scene_ && scene_->HasTextures())
                {
                    if (const aiTexture* embedded = scene_->GetEmbeddedTexture(pathStr.c_str()))
                        {
                            if (embedded->mHeight == 0)
                                {
                                    image = ImageLoader::LoadTexture(
                                        embedded->pcData,
                                        static_cast<int>(embedded->mWidth));
                                }
                            else
                                {
                                    const std::size_t byteCount =
                                        static_cast<std::size_t>(embedded->mWidth) *
                                        static_cast<std::size_t>(embedded->mHeight) * 4u;
                                    auto* copiedData = new unsigned char[byteCount];
                                    std::memcpy(copiedData, embedded->pcData, byteCount);
                                    image = std::make_shared<Image>(
                                        embedded->mWidth, embedded->mHeight, 4,
                                        copiedData);
                                }
                        }
                }

            if (!image)
                {
                    std::filesystem::path fullPath = pathStr;
                    if (fullPath.is_relative() && !sceneDir_.empty())
                        fullPath = std::filesystem::path(sceneDir_) / fullPath;
                    resolvedPath = fullPath.string();
                    image = ImageLoader::LoadTexture(resolvedPath.c_str());
                }

            if (!image || image->empty())
                continue;

            auto texture = std::make_shared<BaseTexture>(
                pathStr, resolvedPath, targetType,
                std::vector<std::shared_ptr<Image> > {image});
            textureCache_[hashKey] = texture;
            result.push_back(std::move(texture));
        }

    return result;
}

} // namespace Multor
