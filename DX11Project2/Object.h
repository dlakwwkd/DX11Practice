#pragma once
#include "d3dUtil.h"
#include "Vertex.h"
class Effect;

class Object
{
public:
    Object();
    virtual ~Object();

    int                         GetVertexOffset() const { return m_VertexOffset; }
    UINT                        GetIndexOffset() const  { return m_IndexOffset; }
    UINT                        GetIndexCount() const   { return m_IndexCount; }
    XMMATRIX                    GetWorldMatrix() const  { return XMLoadFloat4x4(&m_World); }
    XMMATRIX                    GetTexTransform() const { return XMLoadFloat4x4(&m_TexTransform); }
    ID3D11ShaderResourceView*   GetSRV() const          { return m_DiffuseMapSRV; }
    Material                    GetMaterial() const     { return m_Mat; }

    void Pick(int sx, int sy, int cw, int ch, CXMMATRIX V, CXMMATRIX P);
    void ChangeEffectAndTech(Effect* effect, ID3DX11EffectTechnique* tech)
    {
        if (!effect || !tech) return;
        m_Effect = effect;
        m_Tech = tech;
    }

    virtual bool    Init(ID3D11Device* device) = 0;
    virtual void    Release();
    virtual void    Update(float dt);
    virtual void    Render(ID3D11DeviceContext* context, CXMMATRIX viewProj);

protected:
    virtual void    CreateBuffer(ID3D11Device* device) = 0;

protected:
    ID3D11Buffer*                   m_VertexBuffer;
    ID3D11Buffer*                   m_IndexBuffer;
    int                             m_VertexOffset;
    UINT                            m_IndexOffset;
    UINT                            m_IndexCount;
    UINT                            m_PickedTriangle;

    std::vector<Vertex::Basic32>    m_MeshVertices;
    std::vector<UINT>               m_MeshIndices;

    XMFLOAT4X4                      m_World;
    XMFLOAT4X4                      m_TexTransform;
    ID3D11ShaderResourceView*       m_DiffuseMapSRV;
    Material                        m_Mat;
    Material                        m_PickedTriangleMat;

    Effect*                         m_Effect;
    ID3DX11EffectTechnique*         m_Tech;

    friend class ColorEffect;
    friend class BasicEffect;
};

