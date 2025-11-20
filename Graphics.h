#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <d3d11_1.h>
#include <string>
#include <wrl/client.h>
#include <d3d11shadertracing.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

namespace Graphics
{
	// --- GLOBAL VARS ---

	// Primary D3D11 API objects
	inline Microsoft::WRL::ComPtr<ID3D11Device> Device;
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> Context;
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> SwapChain;

	// Rendering buffers
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> BackBufferRTV;
	inline Microsoft::WRL::ComPtr<ID3D11DepthStencilView> DepthBufferDSV;

	// Debug Layer
	inline Microsoft::WRL::ComPtr<ID3D11InfoQueue> InfoQueue;


	// --- Ring Constant Buffer ---
	// The D3D11.1 version of the Context object
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext1> Context1;
	// The one and only very large constant buffer (our GPU memory "heap")
	inline Microsoft::WRL::ComPtr<ID3D11Buffer> ConstantBufferHeap;
	// Size of the constant buffer heap (measured in bytes)
	inline unsigned int cbHeapSizeInBytes;
	// Position of the next unused portion of the heap
	inline unsigned int cbHeapOffsetInBytes;


	// --- FUNCTIONS ---

	// Getters
	bool VsyncState();
	std::wstring APIName();

	// General functions
	HRESULT Initialize(unsigned int windowWidth, unsigned int windowHeight, HWND windowHandle, bool vsyncIfPossible);
	void ShutDown();
	void ResizeBuffers(unsigned int width, unsigned int height);

	// Ring Constant Buffer functions
	void FillAndBindNextConstantBuffer(
		void* data,
		unsigned int dataSizeInBytes,
		D3D11_SHADER_TYPE shaderType,
		unsigned int registerSlot);


	// Debug Layer
	void PrintDebugMessages();



	

}