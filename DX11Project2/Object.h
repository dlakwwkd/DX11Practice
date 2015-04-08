#pragma once
#include "d3dUtil.h"
class Object
{
protected:
    struct Vertex
    {
        XMFLOAT3 Pos;
        XMFLOAT4 Color;

        XMFLOAT3 Normal;
    };
public:
    Object();
    virtual ~Object();

    virtual bool    Init(ID3D11Device* device) = 0;
    virtual void    Release();
    virtual void    Update(float dt);
    virtual void    Render(ID3D11DeviceContext* context);

    int             GetIndexCount() const { return m_IndexCount; }
    XMMATRIX        GetWorldMatrix()const { return XMLoadFloat4x4(&m_World); }

protected:
    ID3D11Buffer*   m_VertexBuffer; // ID3D11Buffer : 모든 버퍼 공통 인터페이스
    ID3D11Buffer*   m_IndexBuffer;
    int             m_IndexCount;

    XMFLOAT4X4      m_World;
};

