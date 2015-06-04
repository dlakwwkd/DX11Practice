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

void Land::Init(ID3D11Device* device)
{
    //CreateBuffer(device);
    CreateBufferWithLoadHeightmap(device);
    m_Effect = Effects::BasicFX;
    m_Tech = Effects::BasicFX->m_Light1TexTech;
    HR(D3DX11CreateShaderResourceViewFromFile(device, L"Textures/heightMap.jpg", 0, 0, &m_DiffuseMapSRV, 0));

    XMMATRIX grassTexScale = XMMatrixScaling(1.0f, 1.0f, 0.0f);
    XMStoreFloat4x4(&m_TexTransform, grassTexScale);

    m_Mat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Mat.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 16.0f);
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

    XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

    m_MeshVertices.resize(grid.Vertices.size());
    for (UINT i = 0; i < grid.Vertices.size(); ++i)
    {
        XMFLOAT3 p = grid.Vertices[i].Position;
        p.y = GetHillHeight(p.x, p.z);

        m_MeshVertices[i].Pos = p;
        m_MeshVertices[i].Normal = GetHillNormal(p.x, p.z);
        m_MeshVertices[i].Tex = grid.Vertices[i].TexC;

        XMVECTOR P = XMLoadFloat3(&m_MeshVertices[i].Pos);
        vMin = XMVectorMin(vMin, P);
        vMax = XMVectorMax(vMax, P);
    }
    XMStoreFloat3(&m_MeshBox.Center, 0.5f*(vMin + vMax));
    XMStoreFloat3(&m_MeshBox.Extents, 0.5f*(vMax - vMin));

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * grid.Vertices.size();
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &m_MeshVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &m_VertexBuffer));


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

void Land::CreateBufferWithLoadHeightmap(ID3D11Device* device)
{
    m_VertexCount = 257;
    m_NumVertices = 66049;
    
    LoadHeightmap();

    XMFLOAT3 vMinf3(+MathHelper::Infinity, +MathHelper::Infinity, +MathHelper::Infinity);
    XMFLOAT3 vMaxf3(-MathHelper::Infinity, -MathHelper::Infinity, -MathHelper::Infinity);
    XMVECTOR vMin = XMLoadFloat3(&vMinf3);
    XMVECTOR vMax = XMLoadFloat3(&vMaxf3);

    m_MeshVertices.resize(m_NumVertices);
    for (int z = 0; z < m_VertexCount; ++z)
    {
        for (int x = 0; x < m_VertexCount; ++x)
        {
            int idx = x + (z * (m_VertexCount));
            m_MeshVertices[idx].Pos = XMFLOAT3(x, m_Heightmap[idx], z);
            m_MeshVertices[idx].Tex = XMFLOAT2(x / (float)(m_VertexCount - 1), z / (float)(m_VertexCount - 1));
            m_MeshVertices[idx].Normal = GetHillNormal(x, z);

            XMVECTOR P = XMLoadFloat3(&m_MeshVertices[idx].Pos);
            vMin = XMVectorMin(vMin, P);
            vMax = XMVectorMax(vMax, P);
        }
    }
    XMStoreFloat3(&m_MeshBox.Center, 0.5f*(vMin + vMax));
    XMStoreFloat3(&m_MeshBox.Extents, 0.5f*(vMax - vMin));

    D3D11_BUFFER_DESC vbd;
    vbd.Usage = D3D11_USAGE_IMMUTABLE;
    vbd.ByteWidth = sizeof(Vertex::Basic32) * m_NumVertices;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = 0;
    vbd.MiscFlags = 0;
    D3D11_SUBRESOURCE_DATA vinitData;
    vinitData.pSysMem = &m_MeshVertices[0];
    HR(device->CreateBuffer(&vbd, &vinitData, &m_VertexBuffer));


    int triangleCount = (m_VertexCount - 1) * (m_VertexCount - 1) * 2;    // »ï°¢Çü °³¼ö
    m_IndexCount = triangleCount * 3;

    m_MeshIndices.resize(m_IndexCount);
    int baseIndex = 0;
    int _numVertsPerRow = m_VertexCount;
    for (int z = 0; z < _numVertsPerRow - 1; z++)
    {
        for (int x = 0; x < _numVertsPerRow - 1; x++)
        {
            m_MeshIndices[baseIndex]        =  z        * _numVertsPerRow + x;      //  0
            m_MeshIndices[baseIndex + 2]    =  z        * _numVertsPerRow + x + 1;  //  3
            m_MeshIndices[baseIndex + 1]    = (z + 1)   * _numVertsPerRow + x;      //  1

            m_MeshIndices[baseIndex + 3]    = (z + 1)   * _numVertsPerRow + x;      //  3
            m_MeshIndices[baseIndex + 5]    =  z        * _numVertsPerRow + x + 1;  //  4
            m_MeshIndices[baseIndex + 4]    = (z + 1)   * _numVertsPerRow + x + 1;  //  1

            baseIndex += 6;
        }
    }

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

void Land::LoadHeightmap()
{
    std::vector<BYTE> in(m_NumVertices);
    std::ifstream loadFile;
    loadFile.open(L"Textures/heightMap.raw", std::ios_base::binary);

    if (loadFile)
    {
        loadFile.read((char*)&in[0], (std::streamsize)in.size());
        loadFile.close();
    }

    m_Heightmap.resize(m_NumVertices);
    for (int i = 0; i < in.size(); ++i)
    {
        m_Heightmap[i] = in[i];
    }
}
