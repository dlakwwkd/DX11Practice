//***************************************************************************************
// Effects.cpp by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Effects.h"
#include "Object.h"

#pragma region Effect
Effect::Effect(ID3D11Device* device, const std::wstring& filename)
	: m_FX(nullptr)
{
	std::ifstream fin(filename, std::ios::binary);

	fin.seekg(0, std::ios_base::end);
	int size = (int)fin.tellg();
	fin.seekg(0, std::ios_base::beg);
	std::vector<char> compiledShader(size);

	fin.read(&compiledShader[0], size);
	fin.close();
	
	HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 0, device, &m_FX));
}

Effect::~Effect()
{
	ReleaseCOM(m_FX);
}
#pragma endregion


#pragma region ColorEffect
ColorEffect::ColorEffect(ID3D11Device* device, const std::wstring& filename)
: Effect(device, filename)
{
    m_ColorTech     = m_FX->GetTechniqueByName("ColorTech");
    m_WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
}

ColorEffect::~ColorEffect()
{
}

void ColorEffect::UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object)
{
    XMMATRIX world = object->GetWorldMatrix();
    XMMATRIX worldViewProj = world*viewProj;

    SetWorldViewProj(worldViewProj);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
: Effect(device, filename)
{
    m_Light1Tech                = m_FX->GetTechniqueByName("Light1");
    m_Light2Tech                = m_FX->GetTechniqueByName("Light2");
    m_Light3Tech                = m_FX->GetTechniqueByName("Light3");

    m_Light0TexTech             = m_FX->GetTechniqueByName("Light0Tex");
    m_Light1TexTech             = m_FX->GetTechniqueByName("Light1Tex");
    m_Light2TexTech             = m_FX->GetTechniqueByName("Light2Tex");
    m_Light3TexTech             = m_FX->GetTechniqueByName("Light3Tex");

    m_Light0TexAlphaClipTech    = m_FX->GetTechniqueByName("Light0TexAlphaClip");
    m_Light1TexAlphaClipTech    = m_FX->GetTechniqueByName("Light1TexAlphaClip");
    m_Light2TexAlphaClipTech    = m_FX->GetTechniqueByName("Light2TexAlphaClip");
    m_Light3TexAlphaClipTech    = m_FX->GetTechniqueByName("Light3TexAlphaClip");

    m_Light1FogTech             = m_FX->GetTechniqueByName("Light1Fog");
    m_Light2FogTech             = m_FX->GetTechniqueByName("Light2Fog");
    m_Light3FogTech             = m_FX->GetTechniqueByName("Light3Fog");

    m_Light0TexFogTech          = m_FX->GetTechniqueByName("Light0TexFog");
    m_Light1TexFogTech          = m_FX->GetTechniqueByName("Light1TexFog");
    m_Light2TexFogTech          = m_FX->GetTechniqueByName("Light2TexFog");
    m_Light3TexFogTech          = m_FX->GetTechniqueByName("Light3TexFog");

    m_Light0TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light0TexAlphaClipFog");
    m_Light1TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light1TexAlphaClipFog");
    m_Light2TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light2TexAlphaClipFog");
    m_Light3TexAlphaClipFogTech = m_FX->GetTechniqueByName("Light3TexAlphaClipFog");

    m_WorldViewProj     = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
    m_World             = m_FX->GetVariableByName("gWorld")->AsMatrix();
    m_WorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
    m_TexTransform      = m_FX->GetVariableByName("gTexTransform")->AsMatrix();
    m_EyePosW           = m_FX->GetVariableByName("gEyePosW")->AsVector();
	m_FogColor          = m_FX->GetVariableByName("gFogColor")->AsVector();
	m_FogStart          = m_FX->GetVariableByName("gFogStart")->AsScalar();
	m_FogRange          = m_FX->GetVariableByName("gFogRange")->AsScalar();
    m_DirLights         = m_FX->GetVariableByName("gDirLights");
    m_Mat               = m_FX->GetVariableByName("gMaterial");
    m_DiffuseMap        = m_FX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object)
{
    XMMATRIX world = object->GetWorldMatrix();
    XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
    XMMATRIX worldViewProj = world*viewProj;

    SetWorld(world);
    SetWorldInvTranspose(worldInvTranspose);
    SetWorldViewProj(worldViewProj);
    SetTexTransform(object->GetTexTransform());
    SetMaterial(object->GetMaterial());
    SetDiffuseMap(object->GetSRV());
}
#pragma endregion

#pragma region SkyEffect
SkyEffect::SkyEffect(ID3D11Device* device, const std::wstring& filename)
: Effect(device, filename)
{
    m_SkyTech       = m_FX->GetTechniqueByName("SkyTech");
    m_WorldViewProj = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
    m_CubeMap       = m_FX->GetVariableByName("gCubeMap")->AsShaderResource();
}

SkyEffect::~SkyEffect()
{
}
#pragma endregion

#pragma region TerrainEffect
TerrainEffect::TerrainEffect(ID3D11Device* device, const std::wstring& filename)
: Effect(device, filename)
{
    m_Light1Tech    = m_FX->GetTechniqueByName("Light1");
    m_Light2Tech    = m_FX->GetTechniqueByName("Light2");
    m_Light3Tech    = m_FX->GetTechniqueByName("Light3");
    m_Light1FogTech = m_FX->GetTechniqueByName("Light1Fog");
    m_Light2FogTech = m_FX->GetTechniqueByName("Light2Fog");
    m_Light3FogTech = m_FX->GetTechniqueByName("Light3Fog");

    m_ViewProj      = m_FX->GetVariableByName("gViewProj")->AsMatrix();
    m_EyePosW       = m_FX->GetVariableByName("gEyePosW")->AsVector();
    m_FogColor      = m_FX->GetVariableByName("gFogColor")->AsVector();
    m_FogStart      = m_FX->GetVariableByName("gFogStart")->AsScalar();
    m_FogRange      = m_FX->GetVariableByName("gFogRange")->AsScalar();
    m_DirLights     = m_FX->GetVariableByName("gDirLights");
    m_Mat           = m_FX->GetVariableByName("gMaterial");

    m_MinDist               = m_FX->GetVariableByName("gMinDist")->AsScalar();
    m_MaxDist               = m_FX->GetVariableByName("gMaxDist")->AsScalar();
    m_MinTess               = m_FX->GetVariableByName("gMinTess")->AsScalar();
    m_MaxTess               = m_FX->GetVariableByName("gMaxTess")->AsScalar();
    m_TexelCellSpaceU       = m_FX->GetVariableByName("gTexelCellSpaceU")->AsScalar();
    m_TexelCellSpaceV       = m_FX->GetVariableByName("gTexelCellSpaceV")->AsScalar();
    m_WorldCellSpace        = m_FX->GetVariableByName("gWorldCellSpace")->AsScalar();
    m_WorldFrustumPlanes    = m_FX->GetVariableByName("gWorldFrustumPlanes")->AsVector();

    m_LayerMapArray = m_FX->GetVariableByName("gLayerMapArray")->AsShaderResource();
    m_BlendMap      = m_FX->GetVariableByName("gBlendMap")->AsShaderResource();
    m_HeightMap     = m_FX->GetVariableByName("gHeightMap")->AsShaderResource();
}

TerrainEffect::~TerrainEffect()
{
}
#pragma endregion


#pragma region Effects

ColorEffect*    Effects::ColorFX    = nullptr;
BasicEffect*    Effects::BasicFX    = nullptr;
SkyEffect*      Effects::SkyFX      = nullptr;
TerrainEffect*  Effects::TerrainFX  = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
    ColorFX     = new ColorEffect(device, L"FX/color.cso");
    BasicFX     = new BasicEffect(device, L"FX/Basic.cso");
    SkyFX       = new SkyEffect(device, L"FX/Sky.cso");
    TerrainFX   = new TerrainEffect(device, L"FX/Terrain.cso");
}

void Effects::DestroyAll()
{
    SafeDelete(ColorFX);
    SafeDelete(BasicFX);
    SafeDelete(SkyFX);
    SafeDelete(TerrainFX);
}
#pragma endregion