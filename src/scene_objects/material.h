/// \file material.h

#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
//#include "assimp/scene.h"
//#include "Loaders/image_loader.h"

namespace Multor
{

struct Material
{
    Material() : ambient(0), diffuse(0), specular(0), shininess(0)
    {
    }
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float     shininess;
};

} // namespace Multor

#endif // MATERIAL_H
