#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "Transform.h"
#include "Entity.h"
#include "BufferStruct.h"
#include "Camera.h"
#include "WICTextureLoader.h"

#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

#include <string.h>
#include <memory>

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// The constructor is called after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
Game::Game()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	GeneratingAssetsAndEntities();



	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Ensure the pipeline knows how to interpret all the numbers stored in
		// the vertex buffer. For this course, all of your vertices will probably
		// have the same layout, so we can just set this once at startup.
		Graphics::Context->IASetInputLayout(inputLayout.Get());

		//// Set the active vertex and pixel shaders
		////  - Once you start applying different shaders to different objects,
		////    these calls will need to happen multiple times per frame
		//Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		//Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();


	DirectX::XMStoreFloat4x4(&(this->tempWorldMatrix), DirectX::XMMatrixIdentity());


	cameraList.push_back(std::make_shared<Camera>(Window::AspectRatio(), DirectX::XMFLOAT3(1, 1, -5), 45.0f, 5.0f, 1.0f));
	currentCamera = cameraList[0];

	cameraList.push_back(std::make_shared<Camera>(Window::AspectRatio(), DirectX::XMFLOAT3(1, 2, -5), 90.0f, 0.1f, 1.0f));

	

	// light setup
	ambientColor = DirectX::XMFLOAT3(0.098f, 0.184f, 0.25f); //sky blue, depending on clouds
	Light directionalLight1 = {};
	directionalLight1.type = LIGHT_TYPE_DIRECTIONAL; //directional
	directionalLight1.direction = DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f); // shining down left
	directionalLight1.color = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f); // white light
	directionalLight1.intensity = 1.0f; // full brightness

	lights.push_back(directionalLight1);

	Light pointLight1 = {};
	pointLight1.type = LIGHT_TYPE_POINT; //directional
	pointLight1.position = DirectX::XMFLOAT3(-2.0f, 6.0f, 0.0f); // 
	pointLight1.range = 20.0f; // effective range
	pointLight1.color = DirectX::XMFLOAT3(1.0f, 0.878f, 0.0f); // yellow-ish light
	pointLight1.intensity = 1.0f; // full brightness

	lights.push_back(pointLight1);

	Light directionalLight3 = {};
	directionalLight3.type = LIGHT_TYPE_DIRECTIONAL; //directional
	directionalLight3.direction = DirectX::XMFLOAT3(-1.0f, 1.0f, 0.0f); // 
	directionalLight3.color = DirectX::XMFLOAT3(1.0f, 0.671f, 0.671f); // pinkish color
	directionalLight3.intensity = 1.0f; // full brightness

	lights.push_back(directionalLight3);

	Light directionalLight4 = {};
	directionalLight4.type = LIGHT_TYPE_DIRECTIONAL; //directional
	directionalLight4.direction = DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f); // 
	directionalLight4.color = DirectX::XMFLOAT3(0.729f, 0.925f, 0.98f); // pale blue
	directionalLight4.intensity = 1.0f; // full brightness

	lights.push_back(directionalLight4);

	Light spotLight1 = {};
	spotLight1.type = LIGHT_TYPE_SPOT; //directional
	spotLight1.direction = DirectX::XMFLOAT3(-2.4f, -1.0f, 0.0f); // 
	spotLight1.position = DirectX::XMFLOAT3(13.9f,-1.0f, 5.9f); //
	spotLight1.color = DirectX::XMFLOAT3(0.8f, 0.8f, 0.8f); // light gray
	spotLight1.range = 20.0f; // effective range
	spotLight1.spotInnerAngle = XMConvertToRadians(30.0f); // 30 degree inner cone
	spotLight1.spotOuterAngle = XMConvertToRadians(45.0f); // 45 degree inner cone
	spotLight1.intensity = 1.0f; // full brightness

	lights.push_back(spotLight1);
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	//ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// Helper methods for vertex and pixel shader loading,
// Create shader objects
Microsoft::WRL::ComPtr<ID3D11PixelShader>  Game::LoadPixelShader(const wchar_t* filePath) {
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;

	// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
	D3DReadFileToBlob(FixPath(filePath).c_str(), &pixelShaderBlob);

	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;

	// Create the actual Direct3D shaders on the GPU
	Graphics::Device->CreatePixelShader(
		pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
		pixelShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

	return pixelShader;
}


Microsoft::WRL::ComPtr<ID3D11VertexShader>  Game::LoadVertexShader(const wchar_t* filePath) {
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* vertexShaderBlob;

	// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
	D3DReadFileToBlob(FixPath(filePath).c_str(), &vertexShaderBlob);

	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;

	Graphics::Device->CreateVertexShader(
		vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
		vertexShaderBlob->GetBufferSize(),		// How big is that data?
		0,										// No classes in this shader
		vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer

	return vertexShader;
}

// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::GeneratingAssetsAndEntities()
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP; // Each dimension can
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP; // have a different mode
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; // but that is uncommon
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX; // Maximum mip level
	Graphics::Device->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf());

	// creating multiple texture SRVs
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	// creating mipmaps of textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tatamiSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tatamiNormalMap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodFloorSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodFloorNormalMap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> nightSkySRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockColorSRV;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleNormalMap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockNormalMap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockyTerrainSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> rockyTerrainNormalMap;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> tideLogoSRV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> flatNormalSRV;


	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/flat_normals.png").c_str(),
		0,
		flatNormalSRV.GetAddressOf()
	);


	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Tatami/tatami_mat_diff_4k.png").c_str(),
		0,
		tatamiSRV.GetAddressOf()
	);
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Tatami/tatami_mat_nor_dx.png").c_str(),
		0,
		tatamiNormalMap.GetAddressOf()
	);

	// creating mipmaps of textures
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Wood_Floor/wood_floor_diff_4k.png").c_str(),
		0,
		woodFloorSRV.GetAddressOf()
	);
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Wood_Floor/wood_floor_nor_dx_4k.png").c_str(),
		0,
		woodFloorNormalMap.GetAddressOf()
	);


	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone.png").c_str(),
		0,
		cobbleSRV.GetAddressOf()
	);
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/cobblestone_normals.png").c_str(),
		0,
		cobbleNormalMap.GetAddressOf()
	);


	// creating mipmaps of textures
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/NightSky.jpg").c_str(),
		0,
		nightSkySRV.GetAddressOf()
	);

	// creating mipmaps of textures
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/RockColor.png").c_str(),
		0,
		rockColorSRV.GetAddressOf()
	);

	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rock.png").c_str(),
		0,
		rockSRV.GetAddressOf()
	);
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rock_normals.png").c_str(),
		0,
		rockNormalMap.GetAddressOf()
	);


	// creating mipmaps of textures
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rocky_terrain.png").c_str(),
		0,
		rockyTerrainSRV.GetAddressOf()
	);
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/rocky_terrain_nor_dx.png").c_str(),
		0,
		rockyTerrainNormalMap.GetAddressOf()
	);

	// creating mipmaps of textures
	DirectX::CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(L"../../Assets/Textures/Tide_Logo_RGB_2014.png").c_str(),
		0,
		tideLogoSRV.GetAddressOf()
	);


	///https://rgbcolorpicker.com/0-1
	///https://colors.artyclick.com/color-name-finder/
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 black = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 transparent = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);
	XMFLOAT4 white = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);



	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	Microsoft::WRL::ComPtr<ID3D11VertexShader> basicVertexShader = LoadVertexShader(L"VertexShader.cso");
	Microsoft::WRL::ComPtr<ID3D11VertexShader> skyVS = LoadVertexShader(L"SkyVS.cso");

	//pixel shaders
	Microsoft::WRL::ComPtr<ID3D11PixelShader> basicPixelShader = LoadPixelShader(L"PixelShader.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugUVPixelShader = LoadPixelShader(L"DebugUVsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> debugNormalPixelShader = LoadPixelShader(L"DebugNormalsPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> customPixelShader = LoadPixelShader(L"CustomPS.cso");
	Microsoft::WRL::ComPtr<ID3D11PixelShader> combinerPixelShader = LoadPixelShader(L"CombinerPS.cso");

	Microsoft::WRL::ComPtr<ID3D11PixelShader> skyPS = LoadPixelShader(L"SkyPS.cso");


	// creating materials
	std::shared_ptr<Material> customMat = std::make_shared<Material>("Custom Material", blue, 0.0f, basicVertexShader, customPixelShader);
	std::shared_ptr<Material> uvMat = std::make_shared<Material>("UV Material", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),0.0f, basicVertexShader, debugUVPixelShader);
	std::shared_ptr<Material> normalMat = std::make_shared<Material>("Normal Material", XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),0.0f, basicVertexShader, debugNormalPixelShader);

	// basicPixelShader
	std::shared_ptr<Material> artyClickSkyBlue = std::make_shared<Material>("Arty Click Sky Blue",XMFLOAT4(0.075f, 0.729f, 1.0f, 1.0f),1.0f, basicVertexShader, basicPixelShader); // #13baff
	std::shared_ptr<Material> vividCyan = std::make_shared<Material>("Vivid Cyan", XMFLOAT4(0.118f, 1.0f, 0.796f, 1.0f),0.5f, basicVertexShader, basicPixelShader); // 
	std::shared_ptr<Material> lavender = std::make_shared<Material>("lavender",XMFLOAT4(0.827f, 0.827f, 1.0f, 1.0f),0.3f, basicVertexShader, basicPixelShader); // #d3d3ff
	std::shared_ptr<Material> greenMat = std::make_shared<Material>("Green", green, 0.0f, basicVertexShader, basicPixelShader); // #d3d3ff

	// assigning texture SRVs and sampler states to materials
	std::shared_ptr<Material> tatamiMatOG = std::make_shared<Material>("Tatami",  transparent, 0.0f, basicVertexShader, basicPixelShader);
	
	std::shared_ptr<Material> cobbleStoneMat = std::make_shared<Material>("Cushion With Normals", transparent, 0.0f, basicVertexShader, basicPixelShader);

	std::shared_ptr<Material> nightSky = std::make_shared<Material>("Night Sky", transparent, 0.0f, basicVertexShader, basicPixelShader);
	std::shared_ptr<Material> rock = std::make_shared<Material>("Rock", transparent, 0.0f, basicVertexShader, basicPixelShader);
	std::shared_ptr<Material> rockyTerrain = std::make_shared<Material>("Rocky Terrain", transparent, 0.0f, basicVertexShader, basicPixelShader);

	// tide logo as texture material
	std::shared_ptr<Material> tideTatamiMat = std::make_shared<Material>("Tide + Tatami", transparent,0.0f, basicVertexShader, combinerPixelShader);


	tatamiMatOG->AddTextureSRV(tatamiSRV, 0);
	tatamiMatOG->AddTextureSRV(tatamiNormalMap, 1);
	tatamiMatOG->AddSamplerState(samplerState, 0);


	nightSky->AddTextureSRV(nightSkySRV, 0);
	nightSky->AddTextureSRV(flatNormalSRV, 1);
	nightSky->AddSamplerState(samplerState, 0);

	rock->AddTextureSRV(rockSRV, 0);
	rock->AddTextureSRV(rockNormalMap, 1);
	rock->AddSamplerState(samplerState, 0);

	rockyTerrain->AddTextureSRV(rockyTerrainSRV, 0);
	rockyTerrain->AddTextureSRV(rockyTerrainNormalMap, 1);
	rockyTerrain->AddSamplerState(samplerState, 0);

	//combining
	tideTatamiMat->AddTextureSRV(tideLogoSRV, 0);
	tideTatamiMat->AddTextureSRV(tatamiSRV, 1);

	tideTatamiMat->AddSamplerState(samplerState, 0);

	cobbleStoneMat->AddTextureSRV(cobbleSRV, 0);
	cobbleStoneMat->AddTextureSRV(cobbleNormalMap, 1);
	cobbleStoneMat->AddSamplerState(samplerState, 0);


	// adding to material list
	this->materialsList.push_back(artyClickSkyBlue);
	this->materialsList.push_back(vividCyan);
	this->materialsList.push_back(lavender);
	this->materialsList.push_back(customMat);
	this->materialsList.push_back(uvMat);
	this->materialsList.push_back(normalMat);
	this->materialsList.push_back(greenMat);

	this->materialsList.push_back(tatamiMatOG);
	this->materialsList.push_back(nightSky);
	this->materialsList.push_back(rock);
	this->materialsList.push_back(rockyTerrain);
	this->materialsList.push_back(tideTatamiMat);
	this->materialsList.push_back(cobbleStoneMat);



	// creating entities
	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str()), tatamiMatOG));
	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cylinder.obj").c_str()), cobbleStoneMat));

	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/helix.obj").c_str()), rockyTerrain));
	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/sphere.obj").c_str()), nightSky));

	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/torus.obj").c_str()), rock));
	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad.obj").c_str()), customMat));
	entityList.push_back(Entity(std::make_shared<Mesh>(FixPath("../../Assets/Meshes/quad_double_sided.obj").c_str()), customMat));


	

	//adjusting transforms
	entityList[0].GetTransform().MoveAbsolute(-9, 0, 8);
	entityList[1].GetTransform().MoveAbsolute(-6, 0, 8);
	entityList[2].GetTransform().MoveAbsolute(-3, 0, 8);
	entityList[3].GetTransform().MoveAbsolute(0, 0, 8);
	entityList[4].GetTransform().MoveAbsolute(3, 0, 8);
	entityList[5].GetTransform().MoveAbsolute(6, 0, 8);
	entityList[6].GetTransform().MoveAbsolute(9, 0, 8);

	skyMesh = std::make_shared<Mesh>(FixPath("../../Assets/Meshes/cube.obj").c_str());

	sky = std::make_shared<Sky>(
		FixPath(L"../../Assets/Skies/CloudsBlueSky/right.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsBlueSky/left.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsBlueSky/up.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsBlueSky/down.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsBlueSky/front.png").c_str(),
		FixPath(L"../../Assets/Skies/CloudsBlueSky/back.png").c_str(),
		samplerState,
		skyVS,
		skyPS,
		skyMesh
	);



	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	ID3DBlob* vertexShaderBlob;
	D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

	D3D11_INPUT_ELEMENT_DESC inputElements[4] = {};

	// Set up the first element - a position, which is 3 float values
	inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
	inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
	inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

	// Set up the second element - 
	inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;			// 4x 32-bit floats
	inputElements[1].SemanticName = "TEXCOORD";							// Match our vertex shader input!
	inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	// Set up the second element - a color, which is 4 more float values
	inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;			// 4x 32-bit floats
	inputElements[2].SemanticName = "NORMAL";							// Match our vertex shader input!
	inputElements[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

	// Set up the fourth element - a tangent, which is 3 more float values
	inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElements[3].SemanticName = "TANGENT";
	inputElements[3].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;

	// Create the input layout, verifying our description against actual shader code
	Graphics::Device->CreateInputLayout(
		inputElements,							// An array of descriptions
		4,										// How many elements in that array?
		vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
		vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
		inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
}



void Game::ImGuiHelper(float deltaTime, float totalTime) {
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();


	// Begin building imgui
	ImGui::Begin("Inspector"); // Everything after is part of this window

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	bool appDetail_opened = ImGui::TreeNode("App Details");

	if (appDetail_opened) {
		// Some interaction
		// Replace the %f with the next parameter, and format as a float
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		// Replace each %d with the next parameter, and format as decimal integers
		// The "x" will be printed as-is between the numbers, like so: 800x600
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());

		///Color picker for window background
		//XMFLOAT4 color(1.0f, 0.0f, 0.5f, 1.0f);
		// Can create a 3 or 4-component color editors, too!
		// - Notice the two different function names below
		//ImGui::ColorEdit3("RGB color editor", &color[0]);
		ImGui::ColorEdit4("RGBA color editor", &color[0]);


		// Mouse position
		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse pos: (%g, %g)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse pos: <INVALID>");


		// which mouse button is being pressed
		ImGui::Text("Mouse down:");
		for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDown(i)) { ImGui::SameLine(); ImGui::Text("b%d (%.02f secs)", i, io.MouseDownDuration[i]); }


		// Display what keys are being pressed
		struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
		ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;
		ImGui::Text("Keys down:");
		for (ImGuiKey key = start_key; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1)) 
		{ 
			if (funcs::IsLegacyNativeDupe(key) || !ImGui::IsKeyDown(key)) 
			continue; 
			ImGui::SameLine(); 
			ImGui::Text((key < ImGuiKey_NamedKey_BEGIN) ? "\"%s\"" : "\"%s\" %d", ImGui::GetKeyName(key), key); 
		}


		ImGui::NewLine();
		//show camera details and camera switch btn
		ImGui::Text("Camera at: (%.02f, %.02f, %.02f)", currentCamera->GetTransform().GetPosition().x, currentCamera->GetTransform().GetPosition().y, currentCamera->GetTransform().GetPosition().z);
		ImGui::Text("FOV: %.02f", currentCamera->GetFov() );
		ImGui::Text("Projection Type: %s", (currentCamera->GetProjectionType()) );

		const char* listOfCams[] = {"Camera #1", "Camera #2"};
		_int64 selectedCamIndex = std::distance(cameraList.begin(), std::find(cameraList.begin(), cameraList.end(), currentCamera) ); //store the selection as an index

		const char* combo_preview_value = listOfCams[selectedCamIndex];
		if (ImGui::BeginCombo("List of Cameras", combo_preview_value)) {
			for (int n = 0; n < IM_ARRAYSIZE(listOfCams); n++) {

				const bool is_selected = (selectedCamIndex == n);
				if (ImGui::Selectable(listOfCams[n], is_selected)) {
					selectedCamIndex = n;
				}
					
				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();

			currentCamera = cameraList[selectedCamIndex];
		}

		ImGui::NewLine();
		//dynamically change button text (round-about way0
		std::string demoVisibilityText = " ImGui Demo Window";
		char visibilityText[100];
		if (demoWinVisibility) {
			demoVisibilityText = "Hide" + demoVisibilityText;
		}
		else {
			demoVisibilityText = "Show" + demoVisibilityText;
		}

		strcpy_s(visibilityText, demoVisibilityText.c_str());
		if (demoWinVisibility)
		{
			ImGui::ShowDemoWindow();
		}

		// lambda funciton allowed?
		if (ImGui::Button(visibilityText))
		{
			// This will only execute on frames in which the button is clicked
			demoWinVisibility = !demoWinVisibility;
		}
		//==================================

		ImGui::TreePop();

	}
	ImGui::NewLine();

	if (ImGui::TreeNode("Entity List")) {
		int counter = 1;
		for (Entity & entity : this->entityList) {
			std::string enityName = "Entity #" + std::to_string(counter);

			if (ImGui::TreeNode(enityName.c_str()) ) {
				std::string matName = std::string("Material name: ") + entity.GetMaterial()->GetName();
				ImGui::Text(matName.c_str()) ;
				DirectX::XMFLOAT3 entityPos = entity.GetTransform().GetPosition();
				ImGui::Text("Position: (%.02f, %.02f, %.02f)", entityPos.x, entityPos.y, entityPos.z);

				ImGui::NewLine();
				ImGui::TreePop();
				
			}
			counter++;
		}
		
		ImGui::TreePop();
	}

	ImGui::NewLine();

	if (ImGui::TreeNode("Materials List")) {
		for (std::shared_ptr<Material> mat : this->materialsList) {

			if (ImGui::TreeNode(mat->GetName())) {

				DirectX::XMFLOAT4 tint = mat->GetColorTint();
				if (ImGui::ColorEdit4("Color Tint:", &tint.x) ) {
					mat->SetColorTint(tint);
				}
				
				DirectX::XMFLOAT2 uvOffset = mat->GetUVOffset();
				if (ImGui::DragFloat2("UV Offset:", &uvOffset.x, 0.01f)) {
					mat->SetUVOffset(uvOffset);
				}
				DirectX::XMFLOAT2 uvScale = mat->GetUVScale();
				if (ImGui::DragFloat2("UV Scale:", &uvScale.x, 0.01f)) {
					mat->SetUVScale(uvScale);
				}

				for (auto& [id,pair]: mat->GetTextureMap()) {
					std::string textureLabel = "Texture SRV ID: " + std::to_string(id);
					ImGui::Text(textureLabel.c_str());
					ImGui::Image(pair.Get(), ImVec2(100, 100));
				}

				

				ImGui::NewLine();


				ImGui::TreePop();
			}

			


		}
		ImGui::TreePop();
	}
	
	ImGui::NewLine();

	if(ImGui::TreeNode("Light List")) {
		int lightCounter = 1;
		for (Light& light : this->lights) {
			std::string lightName = "Light #" + std::to_string(lightCounter);
			if (ImGui::TreeNode(lightName.c_str())) {
				ImGui::Text("Type: %d", light.type);
				if (ImGui::DragFloat3("Position:", &light.position.x, 0.1f)) {
					light.position = light.position;
				}
				if (ImGui::DragFloat3("Direction:", &light.direction.x, 0.1f)) {
					light.direction = light.direction;
				}
				if(ImGui::ColorEdit3("Light Color:", &light.color.x)) {
					light.color = light.color;
				}
				if(ImGui::DragFloat("Intensity Slider:", &light.intensity, 0.1f, 0.0f, 10.0f)) {
					light.intensity = light.intensity;
				}
				if (light.type == LIGHT_TYPE_DIRECTIONAL) {
					ImGui::Text("Direction: (%.02f, %.02f, %.02f)", light.direction.x, light.direction.y, light.direction.z);
				}
				else if (light.type == LIGHT_TYPE_POINT) {
					ImGui::Text("Position: (%.02f, %.02f, %.02f)", light.position.x, light.position.y, light.position.z);
					ImGui::Text("Range: %.02f", light.range);
				}
				else if (light.type == LIGHT_TYPE_SPOT) {
					ImGui::Text("Position: (%.02f, %.02f, %.02f)", light.position.x, light.position.y, light.position.z);
					ImGui::Text("Direction: (%.02f, %.02f, %.02f)", light.direction.x, light.direction.y, light.direction.z);
					ImGui::Text("Range: %.02f", light.range);
					ImGui::Text("Spot Angle: %.02f", light.spotInnerAngle);
					ImGui::Text("Spot Angle: %.02f", light.spotOuterAngle);
				}
				ImGui::TreePop();
			}
			lightCounter++;
		}
		ImGui::TreePop();
	}

	ImGui::NewLine();

	

	// Draggable slider from 0-100 which reads and updates the variable number
	// - number is an integer variable, so &number is its address (a pointer)
	//ImGui::SliderInt("Choose a number", &number, 0, 100);

	//float localArray[2] = { 0.5f, 0.5f };
	//float* arrayAsPointer = new float[3];
	//XMFLOAT4 vectorStruct(10.0f, -2.0f, 99.0f, 0.1f);
	//// Provide the address of the first element when creating vector editors
	//// - Note the function names below are different!
	//// - Additional parameters allow you to set the range and drag speed
	//ImGui::DragFloat2("2-component editor", localArray);
	//ImGui::DragFloat3("3-component editor", arrayAsPointer);
	//ImGui::DragFloat4("4-component editor", &vectorStruct.x);


	

	ImGui::End(); // Ends current window
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	for (std::shared_ptr<Camera> cam : cameraList) {
		cam->UpdateProjectionMatrix(Window::AspectRatio());
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE)){
		Window::Quit();
	}


	//this->sharedMeshArray[3].GetTransform().Rotate(0, 0, (0.7f * deltaTime));

	//this->sharedMeshArray[4].GetTransform().Rotate(0, 0, (-0.7f * deltaTime));

	// Spin the 3D models
	for (Entity& e : entityList)
	{
		e.GetTransform().Rotate(0, deltaTime * 0.25f, 0);
	}

	this->currentCamera->Update(deltaTime);

	ImGuiHelper(deltaTime, totalTime);
	
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	color);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// DRAW geometry
	// - These steps are generally repeated for EACH object you draw
	// - Other Direct3D calls will also be necessary to do more complex things
	{
		
		
		for (Entity& entity : this->entityList) {

			std::shared_ptr<Material> material = entity.GetMaterial();
			// Set the active vertex and pixel shaders
			//  - Once you start applying different shaders to different objects,
			//    these calls will need to happen multiple times per frame
			Graphics::Context->VSSetShader(material->GetVertexShader().Get(), 0, 0);
			Graphics::Context->PSSetShader(material->GetPixelShader().Get(), 0, 0);


			VertexShaderData vsData{};
			/*this->tempWorldMatrix = entity.GetTransform().GetWorldMatrix();
			DirectX::XMMATRIX transposeMatrix = DirectX::XMLoadFloat4x4(&tempWorldMatrix);
			transposeMatrix = DirectX::XMMatrixTranspose(transposeMatrix);
			DirectX::XMStoreFloat4x4(&vsData.world, transposeMatrix);*/
			vsData.world = entity.GetTransform().GetWorldMatrix();
			vsData.view = currentCamera->GetViewMatrix();
			vsData.projection = currentCamera->GetProjectionMatrix();
			vsData.worldInvTranspose = entity.GetTransform().GetWorldInverseTransposeMatrix();
			
			Graphics::FillAndBindNextConstantBuffer(&vsData, sizeof(VertexShaderData), D3D11_VERTEX_SHADER, 0);

			

			

			// second constant buffer for pixel shader
			PixelShaderData psData{};
			psData.colorTint = material->GetColorTint();
			psData.uvOffset = material->GetUVOffset();
			psData.uvScale = material->GetUVScale();
			psData.roughness = material->GetRoughnessValue();
			psData.currCamPos = currentCamera->GetTransform().GetPosition();
			psData.ambientLight = ambientColor;
			psData.time = totalTime;
			
			memcpy(&psData.lights, &lights[0], sizeof(Light) * (int)lights.size());

			Graphics::FillAndBindNextConstantBuffer(&psData, sizeof(PixelShaderData), D3D11_PIXEL_SHADER, 0);


		
			
			entity.GetMaterial()->BindTexturesAndSamplers();
			entity.Draw(deltaTime, totalTime);
		}
		
		sky->Draw(currentCamera);
	}

	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}



