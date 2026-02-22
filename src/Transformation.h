/// \file transformation.h

#pragma once
#ifndef TRANSFORMATION_H
#define TRANSFORMATION_H

// #include "Shader.h"
#include "camera.h"

#include <memory>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Multor
{

struct Matrices
{
    Matrices()
    {
        projection_ = std::make_shared<glm::mat4>(1.0f);
        view_       = std::make_shared<glm::mat4>(1.0f);
    }
    std::shared_ptr<glm::mat4> projection_, view_;
    //void SendToShader(const Shader& shader);
};

struct PositionController : public Matrices
{
    PositionController()
    {
        dt_  = 0.0f;
        cam_ = std::make_shared<Camera>(glm::vec3(10.0f, 10.0f, 10.0f));
    }

    //void SendToShader(const Shader& shader);

    std::shared_ptr<Camera> cam_;
    //delta time
    float dt_;
};

class Transformation
{
public:
    Transformation(glm::mat4 mod = glm::mat4(1.0f))
        : model_(mod),
          normalMatrix_(glm::mat3(glm::transpose(glm::inverse(model_))))
    {
    }
    Transformation(std::shared_ptr<glm::mat4> mod)
        : model_(*mod),
          normalMatrix_(glm::mat3(glm::transpose(glm::inverse(model_))))
    {
    }
    //void SendToShader(const Shader& shader);

    void      SetTransform(const std::shared_ptr<glm::mat4>);
    void      SetTransform(const glm::mat4&);
    glm::mat4 GetTransform() const;

    void Translate(const glm::vec3& trans);
    void Rotate(float alph, const glm::vec3& axes);
    void Scale(const glm::vec3& coefs);

private:
    glm::mat4 model_;
    glm::mat3 normalMatrix_;
    void      updateNormal();
};

class Transformation_interface
{
public:
    virtual void      SetTransform(const std::shared_ptr<glm::mat4>) = 0;
    virtual void      SetTransform(const glm::mat4&)                 = 0;
    virtual glm::mat4 GetTransform() const                           = 0;

    virtual void Translate(const glm::vec3& trans)         = 0;
    virtual void Rotate(float alph, const glm::vec3& axes) = 0;
    virtual void Scale(const glm::vec3& coefs)             = 0;
};

} // namespace Multor

#endif // TRANSFORMATION_H
