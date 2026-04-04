/// \file texture.cpp

#include "texture.h"

namespace Multor
{
BaseTexture::BaseTexture(const std::string& name, const std::string& path,
                         Texture_Types                        type,
                         std::vector<std::shared_ptr<Image> > images)
{
    SetName(name);
    path_ = path;
    type_ = type;
    imgs_ = images;
}

bool BaseTexture::IsCreated() const
{
    return created_;
}

unsigned int BaseTexture::GetId() const
{
    return id_;
}

std::string BaseTexture::GetPath() const
{
    return path_;
}

Texture_Types BaseTexture::GetType() const
{
    return type_;
}

std::vector<std::shared_ptr<Image>> BaseTexture::GetImages() const
{
    return imgs_;
}

void BaseTexture::AddImage(std::shared_ptr<Image> img)
{
    imgs_.emplace_back(std::move(img));
}

} // namespace Multor
