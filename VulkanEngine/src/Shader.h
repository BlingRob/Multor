/// \file Shader.h
#pragma once

#ifndef SHADER_H
#define SHADER_H
#include <string_view>
#include <string>
#include <vector>
#include <array>
#include <memory>
#include <iostream>
#include <type_traits>
//#include <glslang/Include/glslang_c_interface.h>
#include "VkShaderFactory.h"
#include <glm/glm.hpp>


class Shader
{
public:
	void setScal(std::string_view, float) const;
	void setScal(std::string_view, int) const;
	void setScal(std::string_view, bool) const;
	void setScal(std::string_view, uint64_t) const;

	void setVec(std::string_view, const glm::vec2&) const;
	void setVec(std::string_view, const glm::vec3&) const;
	void setVec(std::string_view, const glm::vec4&) const;

	void setMat(std::string_view, const glm::mat2&) const;
	void setMat(std::string_view, const glm::mat3&) const;
	void setMat(std::string_view, const glm::mat4&) const;
private:

};

#endif