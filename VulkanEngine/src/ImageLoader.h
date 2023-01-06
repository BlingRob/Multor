/// \file ImageLoader.h
#pragma once
//Texture's loader
#include <memory>
#include <vector>
#include <string_view>
#include "stb_image.h"

using PDelFun = void(*)(void*);

struct Image
{
	Image() {}
	Image(Image& img) noexcept = default;
	Image(Image&& img) noexcept :w(img.w), h(img.h), nrComponents(img.nrComponents) { std::swap(_mdata,img._mdata); std::swap(deleter, img.deleter);}
	Image&& operator= (Image&& img) noexcept
	{
		w = img.w;
		h = img.h;
		nrComponents = img.nrComponents;
		std::swap(_mdata, img._mdata);
		std::swap(deleter, img.deleter);

		return std::move(*this);
	}
	Image(std::size_t width, std::size_t height, std::uint8_t channel = 4, unsigned char* data = nullptr, PDelFun del = [](void* ptr) {delete static_cast<unsigned char*>(ptr); }) :
		deleter(del), w(static_cast<int>(width)), h(static_cast<int>(height)), nrComponents(channel)
		{
		//texture load option
		//stbi_set_flip_vertically_on_load(true);
			if (data == nullptr)
				_mdata = new unsigned char[width * height * channel];
			else
				_mdata = data;
		};
	bool empty() noexcept
	{
		return !w && !h;
	}
	~Image()
	{
		deleter(_mdata);
	}
	unsigned char* _mdata;
	int w, h, nrComponents;
	private:
		PDelFun deleter;
};

struct ImageLoader
{
	static std::shared_ptr<Image> LoadTexture(const char* path);
	static std::shared_ptr<Image> LoadTexture(const void* memoryPtr, int width);

private:
	static inline int w, h, chs;
	static inline PDelFun STB_deleter = [](void* ptr) {stbi_image_free(ptr); };
};

