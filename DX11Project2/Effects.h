//***************************************************************************************
// Effects.h by Frank Luna (C) 2011 All Rights Reserved.
//
// Defines lightweight effect wrappers to group an effect and its variables.
// Also defines a static Effects class from which we can access all of our effects.
//***************************************************************************************

#ifndef EFFECTS_H
#define EFFECTS_H

#include "d3dUtil.h"
class Object;

#pragma region Effect
class Effect
{
public:
	Effect(ID3D11Device* device, const std::wstring& filename);
	virtual ~Effect();

    virtual void UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object) = 0;

public:
    Effect(const Effect& rhs)               = delete;
	Effect& operator=(const Effect& rhs)    = delete;

protected:
	ID3DX11Effect* m_FX;
};
#pragma endregion


#pragma region ColorEffect
class ColorEffect : public Effect
{
public:
    ColorEffect(ID3D11Device* device, const std::wstring& filename);
    virtual ~ColorEffect();

    virtual void UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object);

    void SetWorldViewProj(CXMMATRIX M)                  { m_WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }

    ID3DX11EffectTechnique*         m_ColorTech;
    ID3DX11EffectMatrixVariable*    m_WorldViewProj;
};
#pragma endregion

#pragma region BasicEffect
class BasicEffect : public Effect
{
public:
    BasicEffect(ID3D11Device* device, const std::wstring& filename);
    virtual ~BasicEffect();

    virtual void UpdateCb(ID3D11DeviceContext* context, CXMMATRIX viewProj, Object* object);

    void SetWorldViewProj(CXMMATRIX M)                  { m_WorldViewProj->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorld(CXMMATRIX M)                          { m_World->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetWorldInvTranspose(CXMMATRIX M)              { m_WorldInvTranspose->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetTexTransform(CXMMATRIX M)                   { m_TexTransform->SetMatrix(reinterpret_cast<const float*>(&M)); }
    void SetEyePosW(const XMFLOAT3& v)                  { m_EyePosW->SetRawValue(&v, 0, sizeof(XMFLOAT3)); }
    void SetFogColor(const FXMVECTOR v)                 { m_FogColor->SetFloatVector(reinterpret_cast<const float*>(&v)); }
    void SetFogStart(float f)                           { m_FogStart->SetFloat(f); }
    void SetFogRange(float f)                           { m_FogRange->SetFloat(f); }
    void SetDirLights(const DirectionalLight* lights)   { m_DirLights->SetRawValue(lights, 0, 3 * sizeof(DirectionalLight)); }
    void SetMaterial(const Material& mat)               { m_Mat->SetRawValue(&mat, 0, sizeof(Material)); }
    void SetDiffuseMap(ID3D11ShaderResourceView* tex)   { m_DiffuseMap->SetResource(tex); }

    ID3DX11EffectTechnique*         m_Light1Tech;
    ID3DX11EffectTechnique*         m_Light2Tech;
    ID3DX11EffectTechnique*         m_Light3Tech;

    ID3DX11EffectTechnique*         m_Light0TexTech;
    ID3DX11EffectTechnique*         m_Light1TexTech;
    ID3DX11EffectTechnique*         m_Light2TexTech;
    ID3DX11EffectTechnique*         m_Light3TexTech;

    ID3DX11EffectTechnique*         m_Light0TexAlphaClipTech;
    ID3DX11EffectTechnique*         m_Light1TexAlphaClipTech;
    ID3DX11EffectTechnique*         m_Light2TexAlphaClipTech;
    ID3DX11EffectTechnique*         m_Light3TexAlphaClipTech;

    ID3DX11EffectTechnique*         m_Light1FogTech;
    ID3DX11EffectTechnique*         m_Light2FogTech;
    ID3DX11EffectTechnique*         m_Light3FogTech;

    ID3DX11EffectTechnique*         m_Light0TexFogTech;
    ID3DX11EffectTechnique*         m_Light1TexFogTech;
    ID3DX11EffectTechnique*         m_Light2TexFogTech;
    ID3DX11EffectTechnique*         m_Light3TexFogTech;

    ID3DX11EffectTechnique*         m_Light0TexAlphaClipFogTech;
    ID3DX11EffectTechnique*         m_Light1TexAlphaClipFogTech;
    ID3DX11EffectTechnique*         m_Light2TexAlphaClipFogTech;
    ID3DX11EffectTechnique*         m_Light3TexAlphaClipFogTech;

    ID3DX11EffectMatrixVariable*    m_WorldViewProj;
    ID3DX11EffectMatrixVariable*    m_World;
    ID3DX11EffectMatrixVariable*    m_WorldInvTranspose;
    ID3DX11EffectMatrixVariable*    m_TexTransform;
    ID3DX11EffectVectorVariable*    m_EyePosW;
    ID3DX11EffectVectorVariable*    m_FogColor;
    ID3DX11EffectScalarVariable*    m_FogStart;
    ID3DX11EffectScalarVariable*    m_FogRange;
    ID3DX11EffectVariable*          m_DirLights;
    ID3DX11EffectVariable*          m_Mat;

    ID3DX11EffectShaderResourceVariable* m_DiffuseMap;
};
#pragma endregion


#pragma region Effects
class Effects
{
public:
	static void InitAll(ID3D11Device* device);
	static void DestroyAll();

    static ColorEffect* ColorFX;
    static BasicEffect* BasicFX;
};
#pragma endregion

#endif // EFFECTS_H