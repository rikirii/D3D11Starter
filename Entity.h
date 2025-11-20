#pragma once
#include <iostream>
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

class Entity
{
private:
	Transform transform;
	std::shared_ptr<Mesh> myMesh;
	std::shared_ptr<Material> material;

public:
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);
	~Entity();

	std::shared_ptr<Mesh> GetMesh();
	Transform& GetTransform();

	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> mat);

	void Draw(float deltaTime, float totalTime);
};

