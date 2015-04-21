#include "Box.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "RenderStates.h"


Box::Box()
{
    m_Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 0.5f);
}


Box::~Box()
{
}

bool Box::Init(ID3D11Device* device)
{
    CreateBuffer(device);
    m_Effect = Effects::BasicFX;
    m_Tech = Effects::BasicFX->m_Light1TexTech;
    HR(D3DX11CreateShaderResourceViewFromFile(device, L"Textures/WoodCrate01.dds", 0, 0, &m_DiffuseMapSRV, 0));
    return true;
}

void Box::Release()
{
    Object::Release();
}

void Box::Update(float dt)
{
    static float t = 0.0f;
    t += dt;
    float scaleValue = 2.0f;
    float moveValue = cosf(t)*10.0f;
    XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
    XMMATRIX rotate = XMMatrixRotationX(t) * XMMatrixRotationY(-t) * XMMatrixRotationZ(t);
    XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 20.0f);
    XMMATRIX world = scale * rotate * position;
    XMStoreFloat4x4(&m_World, world);

    Object::Update(dt);
}

void Box::Render(ID3D11DeviceContext* context, CXMMATRIX viewProj)
{
    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(InputLayouts::Basic32);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    switch (RenderStates::m_RenderOptions)
    {
    case RenderOptions::Lighting:
        m_Tech = Effects::BasicFX->m_Light3Tech;
        break;
    case RenderOptions::Textures:
        m_Tech = Effects::BasicFX->m_Light3TexTech;
        break;
    case RenderOptions::TexturesAndFog:
        m_Tech = Effects::BasicFX->m_Light3TexFogTech;
        break;
    }
    Object::Render(context, viewProj);
}


void Box::CreateBuffer(ID3D11Device* device)
{
    GeometryGenerator::MeshData box;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);

    m_VertexOffset = 0;                 // Cache the vertex offsets to each object in the concatenated vertex buffer.
    m_IndexOffset = 0;                  // Cache the starting index for each object in the concatenated index buffer.
    m_IndexCount = box.Indices.size();  // Cache the index count of each object.

    UINT totalVertexCount = box.Vertices.size();
    UINT totalIndexCount = m_IndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //
    XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

    m_MeshVertices.resize(totalVertexCount);
    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        m_MeshVertices[k].Pos = box.Vertices[i].Position;
        m_MeshVertices[k].Normal = box.Vertices[i].Normal;
        m_MeshVertices[k].Tex = box.Vertices[i].TexC;

        XMVECTOR P = XMLoadFloat3(&m_MeshVertices[i].Pos);

        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }
    XMStoreFloat3(&m_MeshBox.Center, 0.5f*(vMin + vMax));
    XMStoreFloat3(&m_MeshBox.Extents, 0.5f*(vMax - vMin));

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &m_MeshVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &m_VertexBuffer));

    //
    // Pack the indices of all the meshes into one index buffer.
    //
    m_MeshIndices.insert(m_MeshIndices.end(), box.Indices.begin(), box.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT)* totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &m_MeshIndices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &m_IndexBuffer));
}
