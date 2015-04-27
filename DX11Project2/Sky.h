//***************************************************************************************
// Sky.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Simple class that renders a sky using a cube map.
//***************************************************************************************

#ifndef SKY_H
#define SKY_H

#include "d3dUtil.h"

class Camera;

class Sky
{
public:
	Sky(ID3D11Device* device, const std::wstring& cubemapFilename, float skySphereRadius);
	~Sky();

	ID3D11ShaderResourceView* CubeMapSRV();

	void Draw(ID3D11DeviceContext* dc, const Camera& camera);

public:
    Sky(const Sky& rhs)             = delete;
	Sky& operator=(const Sky& rhs)  = delete;

private:
	ID3D11Buffer* m_VB;
	ID3D11Buffer* m_IB;

	ID3D11ShaderResourceView* m_CubeMapSRV;

	UINT m_IndexCount;
};

#endif // SKY_H