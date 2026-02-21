/// \file Transformation.cpp

#include "transformation.h"

namespace Multor
{

/*void Matrices::SendToShader(const Shader& shader)
{
	shader.setMat("transform.view", *View);
	shader.setMat("transform.projection", *Projection);
	shader.setMat("transform.PV", (*Projection) * (*View));
	//shader.setMat("invView", glm::inverse((*View)));
}*/

void Transformation::SetTransform(const std::shared_ptr<glm::mat4> matr)
{
    model_ = *matr;
    updateNormal();
}

void Transformation::SetTransform(const glm::mat4& matr)
{
    model_ = matr;
    updateNormal();
}
glm::mat4 Transformation::GetTransform() const
{
    return model_;
}

/*void Transformation::SendToShader(const Shader& shader)
{
	shader.setMat("transform.model", Model);
	//normal matrix
	shader.setMat("NormalMatrix", NormalMatrix);
}*/

void Transformation::Translate(const glm::vec3& trans)
{
    model_ = glm::translate(model_, trans);
    updateNormal();
}

void Transformation::Rotate(float alph, const glm::vec3& axes)
{
    model_ = glm::rotate(model_, alph, axes);
    updateNormal();
}
void Transformation::Scale(const glm::vec3& coefs)
{
    model_ = glm::scale(model_, coefs);
    updateNormal();
}
void Transformation::updateNormal()
{
    normalMatrix_ = glm::mat3(glm::transpose(glm::inverse(model_)));
}

/*void Position_Controller::SendToShader(const Shader& shader)
{
	Matrices::SendToShader(shader);
	shader.setVec("viewPos", cam->Position);
}*/

} // namespace Multor
