#include "Object.h"
#include "Vertex.h"
#include "Effects.h"


Object::Object()
:   m_VertexBuffer(nullptr),
    m_IndexBuffer(nullptr),
    m_VertexOffset(0),
    m_IndexOffset(0),
    m_IndexCount(0),
    m_DiffuseMapSRV(nullptr),
    m_Effect(nullptr),
    m_Tech(nullptr)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&m_World, I);
    XMStoreFloat4x4(&m_TexTransform, I);
    m_Mat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Mat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
}


Object::~Object()
{
    Release();
}

void Object::Release()
{
    ReleaseCOM(m_DiffuseMapSRV);
    ReleaseCOM(m_IndexBuffer);
    ReleaseCOM(m_VertexBuffer);
}

void Object::Update(float dt)
{
}

void Object::Render(ID3D11DeviceContext* context, CXMMATRIX viewProj)
{
    m_Effect->UpdateCb(context, viewProj, this);

    D3DX11_TECHNIQUE_DESC techDesc;
    m_Tech->GetDesc(&techDesc);
    for (UINT p = 0; p < techDesc.Passes; ++p)
    {
        m_Tech->GetPassByIndex(p)->Apply(0, context);
        context->DrawIndexed(m_IndexCount, m_IndexOffset, m_VertexOffset);
    }
}
