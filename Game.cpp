#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Mesh.h"
#include "BufferStruct.h"

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
	LoadShaders();
	CreateGeometry();

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

		// Set the active vertex and pixel shaders
		//  - Once you start applying different shaders to different objects,
		//    these calls will need to happen multiple times per frame
		Graphics::Context->VSSetShader(vertexShader.Get(), 0, 0);
		Graphics::Context->PSSetShader(pixelShader.Get(), 0, 0);
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


	// Calculate the next multiple of 16 (instead of hardcoding it)
	unsigned int size = sizeof(VertexShaderData);
	size = (size + 15) / 16 * 16;

	// Describe the constant buffer
	D3D11_BUFFER_DESC cbDesc = {}; // Sets struct to all zeros
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.ByteWidth = size; // Must be a multiple of 16
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	Graphics::Device->CreateBuffer(&cbDesc, 0, constBuffer.GetAddressOf());


	Graphics::Context->VSSetConstantBuffers(
		0, // Which slot (register) to bind the buffer to?
		1, // How many are we setting right now?
		constBuffer.GetAddressOf()); // Array of buffers (or address of just one)
	

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


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// BLOBs (or Binary Large OBjects) for reading raw data from external files
	// - This is a simplified way of handling big chunks of external data
	// - Literally just a big array of bytes read from a file
	ID3DBlob* pixelShaderBlob;
	ID3DBlob* vertexShaderBlob;

	// Loading shaders
	//  - Visual Studio will compile our shaders at build time
	//  - They are saved as .cso (Compiled Shader Object) files
	//  - We need to load them when the application starts
	{
		// Read our compiled shader code files into blobs
		// - Essentially just "open the file and plop its contents here"
		// - Uses the custom FixPath() helper from Helpers.h to ensure relative paths
		// - Note the "L" before the string - this tells the compiler the string uses wide characters
		D3DReadFileToBlob(FixPath(L"PixelShader.cso").c_str(), &pixelShaderBlob);
		D3DReadFileToBlob(FixPath(L"VertexShader.cso").c_str(), &vertexShaderBlob);

		// Create the actual Direct3D shaders on the GPU
		Graphics::Device->CreatePixelShader(
			pixelShaderBlob->GetBufferPointer(),	// Pointer to blob's contents
			pixelShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			pixelShader.GetAddressOf());			// Address of the ID3D11PixelShader pointer

		Graphics::Device->CreateVertexShader(
			vertexShaderBlob->GetBufferPointer(),	// Get a pointer to the blob's contents
			vertexShaderBlob->GetBufferSize(),		// How big is that data?
			0,										// No classes in this shader
			vertexShader.GetAddressOf());			// The address of the ID3D11VertexShader pointer
	}

	// Create an input layout 
	//  - This describes the layout of data sent to a vertex shader
	//  - In other words, it describes how to interpret data (numbers) in a vertex buffer
	//  - Doing this NOW because it requires a vertex shader's byte code to verify against!
	//  - Luckily, we already have that loaded (the vertex shader blob above)
	{
		D3D11_INPUT_ELEMENT_DESC inputElements[2] = {};

		// Set up the first element - a position, which is 3 float values
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;				// Most formats are described as color channels; really it just means "Three 32-bit floats"
		inputElements[0].SemanticName = "POSITION";							// This is "POSITION" - needs to match the semantics in our vertex shader input!
		inputElements[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// How far into the vertex is this?  Assume it's after the previous element

		// Set up the second element - a color, which is 4 more float values
		inputElements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;			// 4x 32-bit floats
		inputElements[1].SemanticName = "COLOR";							// Match our vertex shader input!
		inputElements[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;	// After the previous element

		// Create the input layout, verifying our description against actual shader code
		Graphics::Device->CreateInputLayout(
			inputElements,							// An array of descriptions
			2,										// How many elements in that array?
			vertexShaderBlob->GetBufferPointer(),	// Pointer to the code of a shader that uses this layout
			vertexShaderBlob->GetBufferSize(),		// Size of the shader code that uses this layout
			inputLayout.GetAddressOf());			// Address of the resulting ID3D11InputLayout pointer
	}
}


// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	///https://rgbcolorpicker.com/0-1
	///https://colors.artyclick.com/color-name-finder/
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);
	XMFLOAT4 artyClickSkyBlue = XMFLOAT4(0.075f, 0.729f, 1.0f, 1.0f); // #13baff
	XMFLOAT4 malachite = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f); // #04cc2b
	XMFLOAT4 lavender = XMFLOAT4(0.827f, 0.827f, 1.0f, 1.0f); // #d3d3ff


	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in CPU memory
	//    over to a Direct3D-controlled data structure on the GPU (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	Vertex vertices[] =
	{
		{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
		{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
		{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	};

	// Set up indices, which tell us which vertices to use and in which order
	// - This is redundant for just 3 vertices, but will be more useful later
	// - Indices are technically not required if the vertices are in the buffer 
	//    in the correct order and each one will be used exactly once
	// - But just to see how it's done...
	unsigned int indices[] = { 0, 1, 2 };

	this->sharedMeshArray.push_back(std::make_shared<Mesh>(vertices, indices, 3, 3) );

	Vertex verticesA[] = {
		{ XMFLOAT3(-0.7f, +0.6f, +0.0f), lavender },
		{ XMFLOAT3(-0.25f, 0.8f, +0.0f), lavender },
		{ XMFLOAT3(-0.3f, +0.2f, +0.0f), lavender },
		{ XMFLOAT3(-0.8f, -0.4f, +0.0f), malachite },
	};
	unsigned int indicesA[] = { 0,1,2,0,2,3 };
	this->sharedMeshArray.push_back(std::make_shared<Mesh>(verticesA, indicesA, 4, 6 ));


	Vertex verticesB[] = {
		{ XMFLOAT3(+0.6f, +0.8f, +0.0f), malachite },
		{ XMFLOAT3(+0.75f, +0.6f, +0.0f), lavender },
		{ XMFLOAT3(+0.9f, -0.3f, +0.0f), artyClickSkyBlue },
		{ XMFLOAT3(+0.6f, -0.7f, +0.0f), malachite },
		{ XMFLOAT3(+0.5f, +0.1f, +0.0f), green },
	};

	unsigned int indicesB[] = {0,1,4,4,2,3,4,1,2};
	int size = sizeof(indicesB) / sizeof(indicesB[0]);

	this->sharedMeshArray.push_back(std::make_shared<Mesh>(verticesB, indicesB, 5, size ) );
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

	ImGui::SetNextItemOpen(true, ImGuiCond_Once);
	bool meshes_opened = ImGui::TreeNode("Meshes");

	if (meshes_opened) {

		ImGui::Spacing();

		if (ImGui::TreeNode("Mesh: Triangle")) {
			ImGui::Spacing();

			int count = 0;
			int totalVert = 0;
			int totalIndicies = 0;

			for (std::shared_ptr<Mesh> myMesh : sharedMeshArray) {
				if (myMesh->GetVertexCount() == 3) {
					count++;
					totalVert += 3;
					totalIndicies += 3;
				}
			}

			ImGui::Text("Number of 3-sided Triangles: %u", count);
			ImGui::Text("Total Verticies: %u", totalVert);
			ImGui::Text("Total Indicies: %u", totalIndicies);

			ImGui::Spacing();

			ImGui::TreePop();
		}

		ImGui::Spacing();
		if (ImGui::TreeNode("Mesh: Quad")) {

			ImGui::Spacing();

			int count = 0;
			int totalTri = 0;
			int totalVert = 0;
			int totalIndicies = 0;

			for (std::shared_ptr<Mesh> myMesh : sharedMeshArray) {
				if (myMesh->GetVertexCount() == 4) {
					count++;
					totalTri += (myMesh->GetIndexCount() / 3);
					totalVert += 4;
					totalIndicies += myMesh->GetIndexCount();
				}
			}

			ImGui::Text("Number of Quad's: %u", count);
			ImGui::Text("Number of Triangles: %u", totalTri);
			ImGui::Text("Total Verticies: %u", totalVert);
			ImGui::Text("Total Indicies: %u", totalIndicies);

			ImGui::Spacing();

			ImGui::TreePop();
		}

		ImGui::Spacing();
		if (ImGui::TreeNode("Mesh: Poly (Above 4)")) {

			ImGui::Spacing();

			int count = 0;
			int totalTri = 0;
			int totalVert = 0;
			int totalIndicies = 0;

			for (std::shared_ptr<Mesh> myMesh : sharedMeshArray) {
				if (myMesh->GetVertexCount() > 4) {
					count++;
					totalTri += (myMesh->GetIndexCount() / 3);
					totalVert += myMesh->GetVertexCount();
					totalIndicies += myMesh->GetIndexCount();
				}
			}

			ImGui::Text("Number of Polygons: %u", count);
			ImGui::Text("Number of Triangles: %u", totalTri);
			ImGui::Text("Total Verticies: %u", totalVert);
			ImGui::Text("Total Indicies: %u", totalIndicies);

			ImGui::Spacing();

			ImGui::TreePop();
		}

		ImGui::Spacing();
		if (ImGui::TreeNode("ColorTint + Offset")) {
			ImGui::Spacing();

			ImGui::ColorEdit4("Color Tint Editor", tempColorTint);

			ImGui::DragFloat3("Offset Editor",tempOffset,0.01f,-0.5f,0.5f );

			ImGui::TreePop();
		}

		ImGui::TreePop();
	}


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
		
		
		for (std::shared_ptr<Mesh> myMesh : this->sharedMeshArray) {
			VertexShaderData vsData;
			vsData.colorTint = XMFLOAT4(tempColorTint);
			vsData.offset = XMFLOAT3(tempOffset);

			D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
			Graphics::Context->Map(constBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
			memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));
			Graphics::Context->Unmap(constBuffer.Get(), 0);

			

			myMesh->Draw(deltaTime, totalTime);
		}
		
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



