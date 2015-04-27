//***************************************************************************************
// Terrain.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Class that renders a terrain using hardware tessellation and multitexturing.
//***************************************************************************************

#ifndef TERRAIN_H
#define TERRAIN_H

#include "d3dUtil.h"

class Camera;
struct DirectionalLight;

class Terrain
{
public:
	struct InitInfo
	{
		std::wstring HeightMapFilename;
		std::wstring LayerMapFilename0;
		std::wstring LayerMapFilename1;
		std::wstring LayerMapFilename2;
		std::wstring LayerMapFilename3;
		std::wstring LayerMapFilename4;
		std::wstring BlendMapFilename;
		float HeightScale;
		UINT HeightmapWidth;
		UINT HeightmapHeight;
		float CellSpacing;
	};

public:
	Terrain();
	~Terrain();

	float GetWidth()const;
	float GetDepth()const;
	float GetHeight(float x, float z)const;

	XMMATRIX GetWorld()const;
	void SetWorld(CXMMATRIX M);

	void Init(ID3D11Device* device, ID3D11DeviceContext* dc, const InitInfo& initInfo);

	void Draw(ID3D11DeviceContext* dc, const Camera& cam, DirectionalLight lights[3]);

private:
	void LoadHeightmap();
	void Smooth();
	bool InBounds(int i, int j);
	float Average(int i, int j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(UINT i, UINT j);
	void BuildQuadPatchVB(ID3D11Device* device);
	void BuildQuadPatchIB(ID3D11Device* device);
	void BuildHeightmapSRV(ID3D11Device* device);

private:

	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices.  Use 64 so that if we tessellate all the way 
	// to 64, we use all the data from the heightmap.  
	static const int CellsPerPatch = 64;

	ID3D11Buffer* m_QuadPatchVB;
	ID3D11Buffer* m_QuadPatchIB;

	ID3D11ShaderResourceView* m_LayerMapArraySRV;
	ID3D11ShaderResourceView* m_BlendMapSRV;
	ID3D11ShaderResourceView* m_HeightMapSRV;

	InitInfo m_Info;

	UINT m_NumPatchVertices;
	UINT m_NumPatchQuadFaces;

	UINT m_NumPatchVertRows;
	UINT m_NumPatchVertCols;

	XMFLOAT4X4 m_World;

	Material m_Mat;

	std::vector<XMFLOAT2> m_PatchBoundsY;
	std::vector<float> m_Heightmap;
};

#endif // TERRAIN_H