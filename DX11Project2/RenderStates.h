//***************************************************************************************
// RenderStates.h by Frank Luna (C) 2011 All Rights Reserved.
//   
// Defines render state objects.  
//***************************************************************************************

#ifndef RENDERSTATES_H
#define RENDERSTATES_H

#include "d3dUtil.h"

enum RenderOptions
{
    Lighting,
    Textures,
    TexturesAndFog,
};

class RenderStates
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

	static ID3D11RasterizerState* WireframeRS;
	static ID3D11RasterizerState* NoCullRS;
	 
	static ID3D11BlendState* AlphaToCoverageBS;
	static ID3D11BlendState* TransparentBS;

    static RenderOptions m_RenderOptions;
};

#endif // RENDERSTATES_H