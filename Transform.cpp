#include "Transform.h"

Transform::Transform()
{
	this->position = DirectX::XMFLOAT3(0, 0, 0);
	this->rotation = DirectX::XMFLOAT3(0, 0, 0);
	this->scale = DirectX::XMFLOAT3(1, 1, 1);

	DirectX::XMStoreFloat4x4(&(this->worldMatrix), DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&(this->worldInverseTransposeMatrix), DirectX::XMMatrixIdentity());
}

Transform::~Transform()
{
}

void Transform::SetPosition(float x, float y, float z)
{
	this->position = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetPosition(DirectX::XMFLOAT3 position)
{
	this->position = position;
}

void Transform::SetRotation(float pitch, float yaw, float roll)
{
	this->rotation = DirectX::XMFLOAT3(pitch, yaw, roll);
}

void Transform::SetRotation(DirectX::XMFLOAT3 rotation)
{
	this->rotation = rotation;
}

void Transform::SetScale(float x, float y, float z)
{
	this->scale = DirectX::XMFLOAT3(x, y, z);
}

void Transform::SetScale(DirectX::XMFLOAT3 scale)
{
	this->scale = scale;
}



DirectX::XMFLOAT3 Transform::GetRight()
{
	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR right = DirectX::XMVector3Rotate(DirectX::XMVectorSet(1, 0, 0, 0), quat);

	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, right);

	return result;
}

DirectX::XMFLOAT3 Transform::GetUp()
{
	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR up = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 1, 0, 0), quat);

	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, up);

	return result;
}

DirectX::XMFLOAT3 Transform::GetForward()
{
	DirectX::XMVECTOR quat = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	DirectX::XMVECTOR forward = DirectX::XMVector3Rotate(DirectX::XMVectorSet(0, 0, 1, 0), quat);

	DirectX::XMFLOAT3 result;
	DirectX::XMStoreFloat3(&result, forward);

	return result;
}


DirectX::XMFLOAT3 Transform::GetPosition()
{
	return this->position;
}

DirectX::XMFLOAT3 Transform::GetPitchYawRoll()
{
	return this->rotation;
}

DirectX::XMFLOAT3 Transform::GetScale()
{
	return this->scale;
}

DirectX::XMFLOAT4X4 Transform::GetWorldMatrix()
{
	DirectX::XMMATRIX transform = DirectX::XMMatrixTranslation(this->position.x, this->position.y, this->position.z);
	DirectX::XMMATRIX scaling = DirectX::XMMatrixScaling(this->scale.x, this->scale.y, this->scale.z);
	DirectX::XMMATRIX rotate = DirectX::XMMatrixRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);

	//DirectX::XMMATRIX world = DirectX::XMMatrixMultiply(DirectX::XMMatrixMultiply(scaling,rotate),transform);
	DirectX::XMMATRIX world = scaling * rotate * transform;	

	DirectX::XMStoreFloat4x4(&(this->worldMatrix), world);

	GetWorldInverseTransposeMatrix();

	return this->worldMatrix;
}

DirectX::XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&(this->worldMatrix));

	DirectX::XMStoreFloat4x4(&(this->worldInverseTransposeMatrix), DirectX::XMMatrixInverse(0,DirectX::XMMatrixTranspose(world)));

	return this->worldInverseTransposeMatrix;
}


/// <summary>
/// Adds x,y,z values to exisitng poitiont variable. Should NOT take object's orientation into account
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
void Transform::MoveAbsolute(float x, float y, float z)
{
	DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&(this->position));
	newPos = DirectX::XMVectorAdd(newPos, DirectX::XMVectorSet(x, y, z,0));
	DirectX::XMStoreFloat3(&(this->position), newPos);

	this->GetWorldMatrix();
}

/// <summary>
/// Adds x,y,z values to exisitng poitiont variable. Should NOT take object's orientation into account
/// </summary>
/// <param name="offset"></param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 offset)
{
	DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&(this->position));
	newPos = DirectX::XMVectorAdd(newPos, DirectX::XMLoadFloat3(&offset) );
	DirectX::XMStoreFloat3(&(this->position), newPos);

	this->GetWorldMatrix();
}

void Transform::MoveRelative(float x, float y, float z)
{
	DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&(this->position));
	DirectX::XMVECTOR direction = DirectX::XMVectorSet(x, y, z,0);
	DirectX::XMVECTOR currentRotation = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	direction = DirectX::XMVector3Rotate(direction, currentRotation);
	newPos = DirectX::XMVectorAdd(newPos, direction);

	DirectX::XMStoreFloat3(&(this->position), newPos);
	this->GetWorldMatrix();
}

void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	DirectX::XMVECTOR newPos = DirectX::XMLoadFloat3(&(this->position));
	DirectX::XMVECTOR direction = DirectX::XMLoadFloat3(&offset);
	DirectX::XMVECTOR currentRotation = DirectX::XMQuaternionRotationRollPitchYaw(this->rotation.x, this->rotation.y, this->rotation.z);
	direction = DirectX::XMVector3Rotate(direction, currentRotation);
	newPos = DirectX::XMVectorAdd(newPos, direction);

	DirectX::XMStoreFloat3(&(this->position), newPos);
	this->GetWorldMatrix();
}

/// <summary>
/// Add to rotation variable
/// </summary>
/// <param name="pitch"></param>
/// <param name="yaw"></param>
/// <param name="roll"></param>
void Transform::Rotate(float pitch, float yaw, float roll)
{
	DirectX::XMVECTOR newRotate = DirectX::XMLoadFloat3(&(this->rotation));
	newRotate = DirectX::XMVectorAdd(newRotate, DirectX::XMVectorSet(pitch, yaw, roll,0));
	DirectX::XMStoreFloat3(&(this->rotation), newRotate);

	this->GetWorldMatrix();

}

void Transform::Rotate(DirectX::XMFLOAT3 rotation)
{

	DirectX::XMVECTOR newRotate = DirectX::XMLoadFloat3(&(this->rotation));
	newRotate = DirectX::XMVectorAdd(newRotate, DirectX::XMLoadFloat3(&rotation));
	DirectX::XMStoreFloat3(&(this->rotation), newRotate);

	this->GetWorldMatrix();

}


/// <summary>
/// multiply have given values
/// </summary>
/// <param name="x"></param>
/// <param name="y"></param>
/// <param name="z"></param>
void Transform::Scale(float x, float y, float z)
{
	DirectX::XMVECTOR newScale = DirectX::XMLoadFloat3(&(this->scale));
	newScale = DirectX::XMVectorMultiply(newScale, DirectX::XMVectorSet(x, y, z, 0));
	DirectX::XMStoreFloat3(&(this->scale), newScale);

	this->GetWorldMatrix();
}

void Transform::Scale(DirectX::XMFLOAT3 scale)
{

	DirectX::XMVECTOR newScale = DirectX::XMLoadFloat3(&(this->scale));
	newScale = DirectX::XMVectorMultiply(newScale, DirectX::XMLoadFloat3(&scale));
	DirectX::XMStoreFloat3(&(this->scale), newScale);

	this->GetWorldMatrix();
}
