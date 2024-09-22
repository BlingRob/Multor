/// \file texture.cpp

#include "texture.h"

namespace Multor
{
BaseTexture::BaseTexture(const std::string& name, const std::string& path,
                         Texture_Types                        type,
                         std::vector<std::shared_ptr<Image> > images)
{
    setName(name);
    path_ = path;
    type_ = type;
    imgs_ = images;
}

bool BaseTexture::IsCreated()
{
    return created_;
}

unsigned int BaseTexture::GetId()
{
    return id_;
}

std::string BaseTexture::GetPath()
{
    return path_;
}

Texture_Types BaseTexture::GetType()
{
    return type_;
}

std::vector<std::shared_ptr<Image>> BaseTexture::GetImages()
{
    return imgs_;
}

void BaseTexture::AddImage(std::shared_ptr<Image> img)
{
    imgs_.emplace_back(std::move(img));
}

} // namespace Multor