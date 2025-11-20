#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include "Vertex.h"
#include <vector>


class Mesh
{
private:
	int numIndex;
	int numVert;

	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer; 
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer; 

	void CreateDirect3DBuffer(Vertex* vertexArr, unsigned int* indexArr, int numVert, int numIndex);

public:
	Mesh(Vertex vertices[], unsigned int indices[], int numVert, int numIndex);
	Mesh(const char* objFilePath);
	~Mesh();

	Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D11Buffer> GetIndexBuffer();

	int GetIndexCount();
	int GetVertexCount();
	void Draw();

	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);

};

