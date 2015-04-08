#include "Object.h"


Object::Object()
:   m_VertexBuffer(nullptr),
    m_IndexBuffer(nullptr),
    m_IndexCount(0)
{
    XMStoreFloat4x4(&m_World, XMMatrixIdentity());
}


Object::~Object()
{
    Release();
}

void Object::Release()
{
    ReleaseCOM(m_IndexBuffer);
    ReleaseCOM(m_VertexBuffer);
}

void Object::Update(float dt)
{
}

void Object::Render(ID3D11DeviceContext* context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
}
