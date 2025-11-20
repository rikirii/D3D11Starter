#include "Material.h"
#include "PathHelpers.h"
#include "Graphics.h"

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>



Material::Material(const char* name, DirectX::XMFLOAT4 colorTint, float roughness, Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader, Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader)
{
	this->name = name;
	this->colorTint = colorTint;
	this->vertexShader = vertexShader;
	this->pixelShader = pixelShader;
	this->uvOffset = DirectX::XMFLOAT2(0.0f, 0.0f);
	this->uvScale = DirectX::XMFLOAT2(1.0f, 1.0f);
	this->roughness = roughness;
}

DirectX::XMFLOAT4& Material::GetColorTint()
{
	return this->colorTint;
}

void Material::SetColorTint(const DirectX::XMFLOAT4& tint)
{
	this->colorTint = tint;
}

float Material::GetRoughnessValue()
{
	return this->roughness;
}

Microsoft::WRL::ComPtr<ID3D11VertexShader> Material::GetVertexShader()
{
	return this->vertexShader;
}

void Material::SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader)
{
	this->vertexShader = vertexShader;
}

Microsoft::WRL::ComPtr<ID3D11PixelShader> Material::GetPixelShader()
{
	return this->pixelShader;
}

void Material::SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader)
{
	this->pixelShader = pixelShader;
}

DirectX::XMFLOAT2& Material::GetUVOffset()
{
	return this->uvOffset;
}

void Material::SetUVOffset(const DirectX::XMFLOAT2& offset)
{
	this->uvOffset = offset;
}

DirectX::XMFLOAT2& Material::GetUVScale()
{
	return this->uvScale;
}

void Material::SetUVScale(const DirectX::XMFLOAT2& scale)
{
	this->uvScale = scale;
}

const char* Material::GetName()
{
	return this->name;
}

const std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& Material::GetTextureMap()
{
	return this->textureSRVs;
}



Microsoft::WRL::ComPtr<ID3D11InputLayout> Material::GetInputLayout() const
{
	return this->inputLayout;
}

void Material::SetInputLayout(Microsoft::WRL::ComPtr<ID3D11InputLayout> layout)
{
	this->inputLayout = layout;
}

void Material::LoadVertexShader()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts

	// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters

	// Create the actual Direct3D shaders on the GPU
	D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);
	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
		vertexShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		this->vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer


	
}

void Material::LoadPixelShader()
{
	ID3DBlob* pixelShaderBlob;
	D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
	// Create the actual Direct3D shaders on the GPU
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		pixelShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		this->pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer
}

void Material::AddTextureSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv, unsigned int slot)
{
	this->textureSRVs.insert({ slot,srv });
}

void Material::AddSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, unsigned int slot)
{
	this->samplers.insert({ slot,sampler });

}

void Material::BindTexturesAndSamplers()
{

	for (auto& [id, pair] : this->textureSRVs) {
		// Binding SRVs and Samplers in C++ (the first param is the index from the shader)
		Graphics::Context->PSSetShaderResources(id, 1, pair.GetAddressOf()); // Bind srv to texture slot 0
	}

	for (auto& [id, pair] : this->samplers) {
		// Binding SRVs and Samplers in C++ (the first param is the index from the shader)
		Graphics::Context->PSSetSamplers(id, 1, pair.GetAddressOf()); // Bind srv to texture slot 0
	}
	
	

	
}
