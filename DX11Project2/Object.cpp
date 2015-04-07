#include "Object.h"


Object::Object()
:   m_VertexBuffer(nullptr),
    m_IndexBuffer(nullptr),
    m_IndexCount(0)
{
    m_World = XMMatrixIdentity();
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
    static float t = 0.0f;
    t += dt;
    float scaleValue = sinf(t) + 2;
    float moveValue = cosf(t)*10.0f;
    XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
    XMMATRIX rotate = XMMatrixRotationX(t) * XMMatrixRotationY(-t) * XMMatrixRotationZ(t);
    XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 20.0f);
    m_World = rotate * position;
}

void Object::Render(ID3D11DeviceContext* context)
{
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
