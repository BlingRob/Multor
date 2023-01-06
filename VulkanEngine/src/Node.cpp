/// \file Node.cpp
#include "Node.h"

/*void Node::addMesh(std::shared_ptr <Mesh> mesh)
{
	Meshes.push_back(mesh);
}

void Node::addChild(std::shared_ptr<Node> child)
{
	Children.push_back(child);
}

/**void Node::Draw(const Shader* sh)
{

	for (const auto& mesh : Meshes)
	{
		tr.SendToShader(*sh);

		mesh->Draw(sh);
	}
	for (const auto& chld : Children)
		chld->Draw(sh);
}

Node::Node()
{
}

Node::Node(Node& node)
{
	Children = node.Children;
	//Meshes = node.Meshes;
	tr = node.tr;
}

Node::Node(Node&& node) 
{
	std::swap(Children, node.Children);
	std::swap(Meshes, node.Meshes);
	tr = node.tr;
}

Node& Node::operator=(Node& right)
{
	Children = right.Children;
	Meshes = right.Meshes;
	tr = right.tr;
	return *this;
}

Node&& Node::operator==(Node&& right)
{
	std::swap(Children, right.Children);
	std::swap(Meshes, right.Meshes);
	tr = right.tr;
	return std::move(*this);
}

std::pair<Node::MIt, Node::MIt> Node::GetMeshes()
{
	return std::make_pair(Meshes.cbegin(), Meshes.cend());
}

std::pair<Node::NIt, Node::NIt> Node::GetChildren()
{
	return std::make_pair(Children.cbegin(), Children.cend());
}

void Node::SetTransform(const std::shared_ptr <glm::mat4> mat)
{
	tr.SetTransform(mat);
    for(auto node: Children)
		node->SetTransform(node->GetTransform() * GetTransform());
}

void Node::SetTransform(const glm::mat4& mat)
{
	tr.SetTransform(mat);
    for(auto node: Children)
		node->SetTransform(node->GetTransform() * GetTransform());
}

glm::mat4 Node::GetTransform() const
{
	return tr.GetTransform();
}

void Node::Translate(const glm::vec3& trans)
{
	tr.Translate(trans);

    for(auto node: Children)
		node->Translate(trans);
}

void Node::Rotate(float alph, const glm::vec3& axes)
{
	tr.Rotate(alph, axes);
    for(auto node: Children)
		node->Rotate(alph,axes);
}

void Node::Scale(const glm::vec3& coefs)
{
	tr.Scale(coefs);

    for(auto node: Children)
		node->Scale(coefs);
}
*/