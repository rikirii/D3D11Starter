#pragma once
#include "Transform.h"
#include <string>

class Camera
{
	enum class ProjectionType:int {
		PERSPECTIVE,
		ORTHOGRAPHIC
	};

private:
	Transform transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projectionMatrix;

	float fovAngle; //radians
	float nearZ;
	float farZ;
	float movementSpeed; 
	float mouseSensitivity; 
	//float aspectRatio; // Width / Height
	ProjectionType currentProjection;
	


public:
	Camera(float aspectRatio, DirectX::XMFLOAT3 intPos, float fov, float movementSpeed, float mouseLookSpeed);
	~Camera();

	//Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();
	Transform& GetTransform() { return this->transform; };
	float GetFov() { return this->fovAngle; };
	const char* GetProjectionType();

	void UpdateProjectionMatrix(float aspectRatio);
	void UpdateViewMatrix();
	void Update(float deltaTime);
};

