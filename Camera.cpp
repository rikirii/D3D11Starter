#include "Camera.h"
#include <string>
#include "Input.h"

Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 intPos,float fov, float movementSpeed = 1, float mouseLookSpeed = 1)
{
	this->transform.SetPosition(intPos);
	this->movementSpeed = movementSpeed;
	this->mouseSensitivity = mouseLookSpeed;
	//this->aspectRatio = aspectRatio;
	this->fovAngle = DirectX::XMConvertToRadians(fov);
	this->nearZ = 0.1f;
	this->farZ = 100.0f;
	this->currentProjection = ProjectionType::PERSPECTIVE;

	this->UpdateViewMatrix();
	this->UpdateProjectionMatrix(aspectRatio);
}

Camera::~Camera()
{
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return this->viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return this->projectionMatrix;
}

const char* Camera::GetProjectionType()
{
	if (this->currentProjection == ProjectionType::PERSPECTIVE) {

		return "PERSPECTIVE";
	}
	else {
		return "ORTHOGRAPHIC";
	}
}

void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	if (currentProjection == ProjectionType::PERSPECTIVE) {
		//this->aspectRatio = aspectRatio;
		DirectX::XMMATRIX proj = DirectX::XMMatrixPerspectiveFovLH(this->fovAngle, aspectRatio, this->nearZ, this->farZ);
		DirectX::XMStoreFloat4x4(&(this->projectionMatrix), proj);
	}
	else {
		// Orthographic projection update code would go here
	}
}

void Camera::UpdateViewMatrix()
{
	DirectX::XMFLOAT3 pos = this->transform.GetPosition();
	DirectX::XMFLOAT3 forward = this->transform.GetForward();
	DirectX::XMFLOAT3 up = this->transform.GetUp();

	DirectX::XMVECTOR posVec = DirectX::XMLoadFloat3(&pos);
	DirectX::XMVECTOR forwardVec = DirectX::XMLoadFloat3(&forward);
	DirectX::XMVECTOR upVec = DirectX::XMLoadFloat3(&up);

	DirectX::XMMATRIX view = DirectX::XMMatrixLookToLH(posVec, forwardVec, upVec);
	DirectX::XMStoreFloat4x4(&(this->viewMatrix), view);
}

void Camera::Update(float deltaTime)
{
	DirectX::XMFLOAT3 moveRelative = DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 moveAbs= DirectX::XMFLOAT3(0, 0, 0);
	DirectX::XMFLOAT3 rotationDirection = DirectX::XMFLOAT3(0, 0, 0);

	// Keyboard input for movement
	// Forward and backward
	if (Input::KeyDown('W')) {
		moveRelative.z += (this->movementSpeed * deltaTime);
	}
	if (Input::KeyDown('S')) {
		moveRelative.z -= (this->movementSpeed * deltaTime);
	}

	// Left and Right
	if (Input::KeyDown('A')) {
		moveRelative.x -= (this->movementSpeed * deltaTime);
	}
	if (Input::KeyDown('D')) {
		moveRelative.x += (this->movementSpeed * deltaTime);
	}

	// Vertical movement
	if (Input::KeyDown(VK_SPACE)) {
		moveAbs.y += (this->movementSpeed * deltaTime);
	}
	if (Input::KeyDown(VK_SHIFT)) {
		moveAbs.y -= (this->movementSpeed * deltaTime);
	}

	// Mouse input for rotation
	if (Input::MouseLeftDown()) {
		float cursorMovementX = Input::GetMouseXDelta() * this->mouseSensitivity;
		float cursorMovementY = Input::GetMouseYDelta() * this->mouseSensitivity;

		rotationDirection.y += static_cast<float>(cursorMovementX) * 0.01f; // Rotate Y, Yaw (move left/right)
		rotationDirection.x += static_cast<float>(cursorMovementY) * 0.01f; // Rotate X, Pitch (move up/down)
	}
	

	// Update transform
	this->transform.MoveRelative(moveRelative.x, moveRelative.y, moveRelative.z);
	this->transform.MoveAbsolute(moveAbs.x, moveAbs.y, moveAbs.z);
	this->transform.Rotate(rotationDirection.x, rotationDirection.y, rotationDirection.z);

	DirectX::XMFLOAT3 currentRotation = this->transform.GetPitchYawRoll();

	//clamp pitch movement, prevents flipping upside down
	if (currentRotation.x > DirectX::XMConvertToRadians(90)) {
		currentRotation.x = DirectX::XMConvertToRadians(90);
	}
	else if (currentRotation.x < DirectX::XMConvertToRadians(-90)) {
		currentRotation.x = DirectX::XMConvertToRadians(-90);
	}

	this->transform.SetRotation(currentRotation);

	// Update view matrix after moving/rotating
	this->UpdateViewMatrix();
}
