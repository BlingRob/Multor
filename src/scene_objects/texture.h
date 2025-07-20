/// \file texture.h

#pragma once
#ifndef TEXTURE_H
#define TEXTURE_H

#include "../entity.h"
#include "../utils/image.h"

#include <vector>
#include <memory>
#include <string>

#include <glm/glm.hpp>
//#include "assimp/scene.h"
//#include "Loaders/image_loader.h"

namespace Multor
{

enum class Texture_Types : uint8_t
{
    Diffuse            = 0,
    Normal             = 1,
    Specular           = 2,
    Emissive           = 3,
    Height             = 4,
    Metallic_roughness = 5,
    Ambient_occlusion  = 6,
    Skybox             = 7
};

struct BaseTexture : public Entity
{
    BaseTexture(const std::string& name, const std::string& path,
                Texture_Types                        type,
                std::vector<std::shared_ptr<Image> > images);
    virtual ~BaseTexture() {};
    //virtual bool createTexture();
    bool                                 IsCreated();
    unsigned int                         GetId();
    std::string                          GetPath();
    Texture_Types                        GetType();
    std::vector<std::shared_ptr<Image> > GetImages();
    void                                 AddImage(std::shared_ptr<Image> img);

protected:
    bool                                 created_ {false};
    unsigned int                         id_;
    std::vector<std::shared_ptr<Image> > imgs_;
    Texture_Types                        type_;
    std::string                          path_;

private:
};

} // namespace Multor

#endif // TEXTURE_H
