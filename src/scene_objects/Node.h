/// \file node.h

#pragma once
#ifndef NODE_H
#define NODE_H

namespace Multor
{
/*
//#include "Shader.h"
#include "transformation.h"
#include "entity.h"
#include <tuple>
#include <list>

class Node:public Entity, public Transformation_interface
{
public:
	//using MIt = std::list <std::shared_ptr<Mesh>>::const_iterator;
	using NIt = std::list <std::shared_ptr<Node>>::const_iterator;
	Node();
	Node(Node&);
	Node(Node&&);
	Node& operator=(Node& right);
	Node&& operator==(Node&& right);

	//void addMesh(std::shared_ptr<Mesh> mesh);
	void addChild(std::shared_ptr<Node> child);
	//std::pair<MIt, MIt> GetMeshes();
	std::pair<NIt, NIt> GetChildren();
	//void Draw(const Shader* sh);

	void SetTransform(const std::shared_ptr <glm::mat4>);
	void SetTransform(const glm::mat4&);
	glm::mat4 GetTransform() const;

	void Translate(const glm::vec3& trans);
	void Rotate(float alph, const glm::vec3& axes);
	void Scale(const glm::vec3& coefs);
private:
	std::list<std::shared_ptr<Node>> Children;
	//std::list<std::shared_ptr<Mesh>> Meshes;
	Transformation tr;
};
*/

} // namespace Multor

#endif // NODE_H