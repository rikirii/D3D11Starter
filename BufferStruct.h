#pragma once

#include <DirectXMath.h>
#include "Lights.h"
//  DirectX::XMFLOAT4


struct  VertexShaderData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldInvTranspose;

};

struct PixelShaderData
{
	DirectX::XMFLOAT4 colorTint;
	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT3 currCamPos;
	float roughness;
	DirectX::XMFLOAT3 ambientLight;
	float time;
	Light lights[5];

	
};
