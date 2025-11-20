#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"
#include <memory>
#include <vector>


class Game
{
public:
	// Basic OOP setup
	Game();
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	//variables
	float color[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	bool demoWinVisibility = false;
	DirectX::XMFLOAT3 ambientColor;

	std::vector<Light> lights;
	std::shared_ptr<Sky> sky;
	std::shared_ptr<Mesh> skyMesh;

	//temp for assignment 4
	float tempColorTint[4] = { 1.0f, 0.5f, 0.5f, 1.0f };
	float tempOffset[3] = { 0.25f, 0.0f, 0.0f };
	DirectX::XMFLOAT4X4 tempWorldMatrix;

	std::vector<Entity > entityList;

	std::shared_ptr<Camera> currentCamera;
	std::vector < std::shared_ptr<Camera> > cameraList;

	std::vector<std::shared_ptr<Material>> materialsList;

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void GeneratingAssetsAndEntities();
	void ImGuiHelper(float deltaTime, float totalTime);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> LoadPixelShader(const wchar_t* filePath);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> LoadVertexShader(const wchar_t* filePath);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr


	// Shaders and shader-related constructs
	//Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	//Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

	
};

