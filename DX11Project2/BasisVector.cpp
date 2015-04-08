#include "BasisVector.h"


BasisVector::BasisVector()
{
}


BasisVector::~BasisVector()
{
}

bool BasisVector::Init(ID3D11Device* device)
{
    Vertex vertices[] =
    {
        { XMFLOAT3(0.0f, 0.0f, 0.0f),       (const float*)&Colors::White, XMFLOAT3(0.0f, 0.0f, 0.0f) },
        { XMFLOAT3(1000000.0f, 0.0f, 0.0f), (const float*)&Colors::Red,   XMFLOAT3(1.0f, 0.0f, 0.0f) },
        { XMFLOAT3(0.0f, 1000000.0f, 0.0f), (const float*)&Colors::Green, XMFLOAT3(0.0f, 1.0f, 0.0f) },
        { XMFLOAT3(0.0f, 0.0f, 1000000.0f), (const float*)&Colors::Blue,  XMFLOAT3(0.0f, 0.0f, 1.0f) },
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;             // 버퍼 사용 방식
    bd.ByteWidth = sizeof(vertices);            // 버퍼 크기
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;    // 파이프라인에 연결되는 버퍼 형태
    bd.CPUAccessFlags = 0;                      // CPU접근 flag, 일반적으로 GPU를 사용하기 때문에 0을 쓴다.

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;
    HR(device->CreateBuffer(&bd, &InitData, &m_VertexBuffer));


    WORD indices[] =
    {
        0, 1,
        0, 2,
        0, 3,
    };
    m_IndexCount = ARRAYSIZE(indices);
    bd.Usage = D3D11_USAGE_DEFAULT;     // CPU 접근 불가, 생성후 변경 불가, GPU만 접근 가능
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    HR(device->CreateBuffer(&bd, &InitData, &m_IndexBuffer));
    return true;
}

void BasisVector::Release()
{
    Object::Release();
}

void BasisVector::Update(float dt)
{
    Object::Update(dt);
}

void BasisVector::Render(ID3D11DeviceContext* context)
{
    Object::Render(context);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
}
