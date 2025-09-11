#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"

class Mesh
{
private:
	int numIndex;
	int numVert;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer; 
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer; 

public:
	Mesh(Vertex vertices[], unsigned int indices[], int numVert, int numIndex);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	int GetIndexCount();
	int GetVertexCount();
	void Draw(float deltaTime, float totalTime);



};

