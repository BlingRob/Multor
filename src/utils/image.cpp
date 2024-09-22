/// \file image.cpp

#include "image.h"

namespace Multor
{

    Image::Image(Image&& img) noexcept : 
                                  w_(img.w_),
                                  h_(img.h_),
                                  nrComponents_(img.nrComponents_)
    {
        std::swap(mdata_, img.mdata_);
        std::swap(deleter_, img.deleter_);
    }

    Image&& Image::operator=(Image&& img) noexcept
    {
        w_            = img.w_;
        h_            = img.h_;
        nrComponents_ = img.nrComponents_;
        std::swap(mdata_, img.mdata_);
        std::swap(deleter_, img.deleter_);

        return std::move(*this);
    }

    Image::Image(
        std::size_t width, std::size_t height, std::uint8_t channel,
        unsigned char* data,
        PDelFun        del)
        : deleter_(del),
          w_(static_cast<int>(width)),
          h_(static_cast<int>(height)),
          nrComponents_(channel)
    {
        //texture load option
        //stbi_set_flip_vertically_on_load(true);
        if (data == nullptr)
            mdata_ = new unsigned char[width * height * channel];
        else
            mdata_ = data;
    }

    bool Image::empty() const noexcept
    {
        return !w_ && !h_;
    }

    Image::~Image()
    {
        deleter_(mdata_);
    }

} // namespace Multor
