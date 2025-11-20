#pragma once
#include <DirectXMath.h>

#define LIGHT_TYPE_DIRECTIONAL 0
#define LIGHT_TYPE_POINT 1
#define LIGHT_TYPE_SPOT 2

struct Light
{
	int type; // 0 = directional, 1 = point, 2 = 
	DirectX::XMFLOAT3 direction; // For directional and spot lights
	float range; // Effective range for point and spot lights
	DirectX::XMFLOAT3 position; // For point and spot lights
	float intensity; // Brightness of the light
	DirectX::XMFLOAT3 color; // RGBA color
	float spotInnerAngle; // Inner cone angle (in radians) – Inside this, full light!
	float spotOuterAngle; // Outer cone angle (in radians) – Between inner and outer, light falls off to zero
	DirectX::XMFLOAT2 padding; // Padding to align to 16-byte boundary
	
};