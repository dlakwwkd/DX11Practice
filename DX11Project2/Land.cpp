#include "Land.h"
#include "Effects.h"
#include "GeometryGenerator.h"
#include "RenderStates.h"


Land::Land()
{
}


Land::~Land()
{
}

bool Land::Init(ID3D11Device* device)
{
    CreateBuffer(device);
    m_Effect = Effects::BasicFX;
    m_Tech = Effects::BasicFX->m_Light1TexTech;
    HR(D3DX11CreateShaderResourceViewFromFile(device, L"Textures/grass.dds", 0, 0, &m_DiffuseMapSRV, 0));

    XMMATRIX grassTexScale = XMMatrixScaling(5.0f, 5.0f, 0.0f);
    XMStoreFloat4x4(&m_TexTransform, grassTexScale);

    m_Mat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Mat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
    return true;
}

void Land::Release()
{
    Object::Release();
}

void Land::Update(float dt)
{
    Object::Update(dt);
}

void Land::Render(ID3D11DeviceContext* context, CXMMATRIX viewProj)
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


void Land::CreateBuffer(ID3D11Device* device)
{
    GeometryGenerator::MeshData grid;

    GeometryGenerator geoGen;

    geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);

    m_IndexCount = grid.Indices.size();

    //
    // Extract the vertex elements we are interested and apply the height function to
    // each vertex.  
    //
    m_MeshVertices.resize(grid.Vertices.size());
    for (UINT i = 0; i < grid.Vertices.size(); ++i)
    {
        XMFLOAT3 p = grid.Vertices[i].Position;

        p.y = GetHillHeight(p.x, p.z);

        m_MeshVertices[i].Pos = p;
        m_MeshVertices[i].Normal = GetHillNormal(p.x, p.z);
        m_MeshVertices[i].Tex = grid.Vertices[i].TexC;
    }

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &m_MeshVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &m_VertexBuffer));

    //
    // Pack the indices of all the meshes into one index buffer.
    //
    m_MeshIndices.insert(m_MeshIndices.end(), grid.Indices.begin(), grid.Indices.end());

    D3D11_BUFFER_DESC ibd;
    ibd.Usage = D3D11_USAGE_IMMUTABLE;
    ibd.ByteWidth = sizeof(UINT)* m_IndexCount;
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.CPUAccessFlags = 0;
    ibd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA iinitData;
    iinitData.pSysMem = &m_MeshIndices[0];
    HR(device->CreateBuffer(&ibd, &iinitData, &m_IndexBuffer));
}
