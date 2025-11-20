#pragma once
#include <memory>
#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "Camera.h"

class Sky
{
private:
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Microsoft::WRL::ComPtr < ID3D11DepthStencilState> skyDepthBuffer;
	Microsoft::WRL::ComPtr < ID3D11RasterizerState> skyRasterizer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVS;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPS;

	std::shared_ptr<Mesh> skyMesh;

	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);

	

public:
	Sky(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler,
		Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVS,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPS,
		std::shared_ptr<Mesh> mesh
		);

	~Sky();

	void Draw(std::shared_ptr<Camera> cam);
};

