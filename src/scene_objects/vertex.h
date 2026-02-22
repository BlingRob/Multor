/// \file vertex.h

#pragma once
#ifndef VERTEX_H
#define VERTEX_H

#include "glm/glm.hpp"

namespace Multor
{

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 norm;
    glm::vec2 texCoord;
    glm::vec3 aTan;
    glm::vec3 aBitan;
};

} // namespace Multor

#endif // VERTEX_H