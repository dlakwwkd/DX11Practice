#include "Box.h"
#include "GeometryGenerator.h"
#include "Vertex.h"
#include "Effects.h"


Box::Box()
{
}


Box::~Box()
{
}

bool Box::Init(ID3D11Device* device)
{
    GeometryGenerator::MeshData box;

    GeometryGenerator geoGen;
    geoGen.CreateBox(1.0f, 1.0f, 1.0f, box);
    
    m_VertexOffset  = 0;                    // Cache the vertex offsets to each object in the concatenated vertex buffer.
    m_IndexOffset   = 0;                    // Cache the starting index for each object in the concatenated index buffer.
    m_IndexCount    = box.Indices.size();   // Cache the index count of each object.

    UINT totalVertexCount   = box.Vertices.size();
    UINT totalIndexCount    = m_IndexCount;

    //
    // Extract the vertex elements we are interested in and pack the
    // vertices of all the meshes into one vertex buffer.
    //
    std::vector<Vertex::Basic32> vertices(totalVertexCount);

    UINT k = 0;
    for (size_t i = 0; i < box.Vertices.size(); ++i, ++k)
    {
        vertices[k].Pos = box.Vertices[i].Position;
        vertices[k].Normal = box.Vertices[i].Normal;
        vertices[k].Tex = box.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * totalVertexCount;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &vertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &m_VertexBuffer));

    //
    // Pack the indices of all the meshes into one index buffer.
    //
    std::vector<UINT> indices;
    indices.insert(indices.end(), box.Indices.begin(), box.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT)* totalIndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &indices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &m_IndexBuffer));

    //
    // Load Texture.
    //
    HR(D3DX11CreateShaderResourceViewFromFile(device, L"Textures/WoodCrate01.dds", 0, 0, &m_DiffuseMapSRV, 0));

    //
    // Set Effect.
    //
    m_Effect = Effects::BasicFX;
    m_Tech = Effects::BasicFX->m_Light2TexTech;
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

void Box::Render(ID3D11DeviceContext* context, CXMMATRIX viewProj)
{
    UINT stride = sizeof(Vertex::Basic32);
    UINT offset = 0;
    context->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetInputLayout(InputLayouts::Basic32);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    Object::Render(context, viewProj);
}
