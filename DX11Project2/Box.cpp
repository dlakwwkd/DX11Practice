#include "Box.h"


Box::Box()
{
}


Box::~Box()
{
}

bool Box::Init(ID3D11Device* device)
{
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::White,  XMFLOAT3(-1.0f, -1.0f, -1.0f)},
        { XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*)&Colors::Black,  XMFLOAT3(-1.0f, +1.0f, -1.0f)},
        { XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*)&Colors::Red,    XMFLOAT3(+1.0f, +1.0f, -1.0f)},
        { XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green,  XMFLOAT3(+1.0f, -1.0f, -1.0f)},
        { XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Blue,   XMFLOAT3(-1.0f, -1.0f, +1.0f)},
        { XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*)&Colors::Yellow, XMFLOAT3(-1.0f, +1.0f, +1.0f)},
        { XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*)&Colors::Cyan,   XMFLOAT3(+1.0f, +1.0f, +1.0f)},
        { XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Magenta,XMFLOAT3(+1.0f, -1.0f, +1.0f)}
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;             // ���� ��� ���
    bd.ByteWidth = sizeof(vertices);            // ���� ũ��
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;    // ���������ο� ����Ǵ� ���� ����
    bd.CPUAccessFlags = 0;                      // CPU���� flag, �Ϲ������� GPU�� ����ϱ� ������ 0�� ����.

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;        // �ʱ�ȭ�ϱ� ���� ���� �迭 ������
    HR(device->CreateBuffer(
        &bd,                            // ������ ������ ������ ���� ����ü
        &InitData,                      // ���� �ʱ�ȭ�� �ʿ��� ������
        &m_VertexBuffer));              // ������ ����


    WORD indices[] =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };
    m_IndexCount = ARRAYSIZE(indices);
    bd.Usage = D3D11_USAGE_DEFAULT;     // CPU ���� �Ұ�, ������ ���� �Ұ�, GPU�� ���� ����
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    HR(device->CreateBuffer(&bd, &InitData, &m_IndexBuffer));
    return true;
}

void Box::Release()
{
    Object::Release();
}

void Box::Update(float dt)
{
    Object::Update(dt);
    static float t = 0.0f;
    t += dt;
    float scaleValue = sinf(t) + 2;
    float moveValue = cosf(t)*10.0f;
    XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
    XMMATRIX rotate = XMMatrixRotationX(t) * XMMatrixRotationY(-t) * XMMatrixRotationZ(t);
    XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 20.0f);
    XMMATRIX world = rotate * position;
    XMStoreFloat4x4(&m_World, world);
}

void Box::Render(ID3D11DeviceContext* context)
{
    Object::Render(context);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
