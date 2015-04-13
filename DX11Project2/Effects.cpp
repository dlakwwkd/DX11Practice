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
    XMMATRIX world = XMLoadFloat4x4(&object->m_World);
    XMMATRIX worldViewProj = world*viewProj;

    SetWorldViewProj(worldViewProj);
}
#pragma endregion

#pragma region BasicEffect
BasicEffect::BasicEffect(ID3D11Device* device, const std::wstring& filename)
: Effect(device, filename)
{
    m_Light1Tech    = m_FX->GetTechniqueByName("Light1");
    m_Light2Tech    = m_FX->GetTechniqueByName("Light2");
    m_Light3Tech    = m_FX->GetTechniqueByName("Light3");

    m_Light0TexTech = m_FX->GetTechniqueByName("Light0Tex");
    m_Light1TexTech = m_FX->GetTechniqueByName("Light1Tex");
    m_Light2TexTech = m_FX->GetTechniqueByName("Light2Tex");
    m_Light3TexTech = m_FX->GetTechniqueByName("Light3Tex");

    m_WorldViewProj     = m_FX->GetVariableByName("gWorldViewProj")->AsMatrix();
    m_World             = m_FX->GetVariableByName("gWorld")->AsMatrix();
    m_WorldInvTranspose = m_FX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
    m_TexTransform      = m_FX->GetVariableByName("gTexTransform")->AsMatrix();
    m_EyePosW           = m_FX->GetVariableByName("gEyePosW")->AsVector();
    m_DirLights         = m_FX->GetVariableByName("gDirLights");
    m_Mat               = m_FX->GetVariableByName("gMaterial");
    m_DiffuseMap        = m_FX->GetVariableByName("gDiffuseMap")->AsShaderResource();
}

BasicEffect::~BasicEffect()
{
}

void BasicEffect::UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object)
{
    XMMATRIX world = XMLoadFloat4x4(&object->m_World);
    XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world);
    XMMATRIX worldViewProj = world*viewProj;

    SetWorld(world);
    SetWorldInvTranspose(worldInvTranspose);
    SetWorldViewProj(worldViewProj);
    SetTexTransform(XMLoadFloat4x4(&object->m_TexTransform));
    SetMaterial(object->m_Mat);
    SetDiffuseMap(object->m_DiffuseMapSRV);
}
#pragma endregion


#pragma region Effects

ColorEffect* Effects::ColorFX = nullptr;
BasicEffect* Effects::BasicFX = nullptr;

void Effects::InitAll(ID3D11Device* device)
{
    ColorFX = new ColorEffect(device, L"FX/color.cso");
    BasicFX = new BasicEffect(device, L"FX/Basic.cso");
}

void Effects::DestroyAll()
{
    SafeDelete(ColorFX);
    SafeDelete(BasicFX);
}
#pragma endregion