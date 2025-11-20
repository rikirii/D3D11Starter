#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>
#include <unordered_map>

class Material
{
private:
	const char* name;

	DirectX::XMFLOAT4 colorTint;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	// Example of unordered maps holding SRVs and Sampler for a single material
	// - Be sure to #include <unordered_map> if you use these
	//using map for foreach loop convenience
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;

	DirectX::XMFLOAT2 uvOffset;
	DirectX::XMFLOAT2 uvScale;

	/// <summary>
	///  0 - 1 roughness value. 0 = smooth (mirror-like, precise highlights), 1 = rough (matte, no highlights)
	/// </summary>
	float roughness;

public:
	Material(const char* name, DirectX::XMFLOAT4 colorTint, float roughness, Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader,
		Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader);


	// Color tint Get/Set
	DirectX::XMFLOAT4& GetColorTint();
	void SetColorTint(const DirectX::XMFLOAT4& tint);

	// Roughness getter
	float GetRoughnessValue ();

	//Get Set for vertex shaders
	Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader() ;
	void SetVertexShader(Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader);

	//Get set for pixel shaders
	Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader();
	void SetPixelShader(Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader);

	// Getter setter for UV offset and scale
	DirectX::XMFLOAT2& GetUVOffset();
	void SetUVOffset(const DirectX::XMFLOAT2& offset);
	DirectX::XMFLOAT2& GetUVScale();
	void SetUVScale(const DirectX::XMFLOAT2& scale);
	

	//Get name
	const char* GetName();


	// Get texture SRVs
	const std::unordered_map<unsigned int, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& GetTextureMap();


	//Get Set input layout
	Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout() const;
	void SetInputLayout(Microsoft::WRL::ComPtr<ID3D11InputLayout> layout);


	void LoadVertexShader();
	void LoadPixelShader();

	void AddTextureSRV(Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv, unsigned int slot);
	void AddSamplerState(Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler, unsigned int slot);

	void BindTexturesAndSamplers();
};

