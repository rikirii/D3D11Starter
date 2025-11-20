#include "Entity.h"
#include "Graphics.h"

Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->transform = Transform();
	this->myMesh = mesh;
	this->material = material;
}

Entity::~Entity()
{
	// As the Mesh object is not instantiated by this class, 
	// it should NOT be deleted by this class!
	// In general, a class shouldn't delete an object it didn't create
}

std::shared_ptr<Mesh> Entity::GetMesh()
{
	return this->myMesh;
}

Transform& Entity::GetTransform()
{
	return this->transform;
}

std::shared_ptr<Material> Entity::GetMaterial()
{
	return this->material;
}

void Entity::SetMaterial(std::shared_ptr<Material> mat)
{
	this->material = mat;
}

void Entity::Draw(float deltaTime, float totalTime)
{
	

	this->myMesh->Draw();
}


