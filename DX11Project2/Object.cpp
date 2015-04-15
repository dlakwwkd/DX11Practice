#include "Object.h"
#include "Effects.h"
#include "RenderStates.h"


Object::Object()
:   m_VertexBuffer(nullptr),
    m_IndexBuffer(nullptr),
    m_VertexOffset(0),
    m_IndexOffset(0),
    m_IndexCount(0),
    m_PickedTriangle(-1),
    m_DiffuseMapSRV(nullptr),
    m_Effect(nullptr),
    m_Tech(nullptr)
{
    XMMATRIX I = XMMatrixIdentity();
    XMStoreFloat4x4(&m_World, I);
    XMStoreFloat4x4(&m_TexTransform, I);

    m_MeshBox.Center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_MeshBox.Extents = XMFLOAT3(0.0f, 0.0f, 0.0f);
    
    m_Mat.Ambient = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_Mat.Diffuse = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    m_Mat.Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);

    m_PickedTriangleMat.Ambient = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    m_PickedTriangleMat.Diffuse = XMFLOAT4(0.0f, 0.8f, 0.4f, 1.0f);
    m_PickedTriangleMat.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
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

        if (m_PickedTriangle != -1)
        {
            context->OMSetDepthStencilState(RenderStates::LessEqualDSS, 0);

            Effects::BasicFX->SetMaterial(m_PickedTriangleMat);
            m_Tech->GetPassByIndex(p)->Apply(0, context);
            context->DrawIndexed(3, 3 * m_PickedTriangle, 0);

            context->OMSetDepthStencilState(0, 0);
        }
    }
}

void Object::Pick(int sx, int sy, int cw, int ch, CXMMATRIX V, CXMMATRIX P)
{
    // Compute picking ray in view space.
    float vx = (+2.0f*sx / cw - 1.0f) / P(0, 0);
    float vy = (-2.0f*sy / ch + 1.0f) / P(1, 1);

    // Ray definition in view space.
    XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
    XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

    // Tranform ray to local space of Mesh.
    XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

    XMMATRIX W = XMLoadFloat4x4(&m_World);
    XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

    XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

    rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
    rayDir = XMVector3TransformNormal(rayDir, toLocal);

    // Make the ray direction unit length for the intersection tests.
    rayDir = XMVector3Normalize(rayDir);

    // Assume we have not picked anything yet, so init to -1.
    m_PickedTriangle = -1;
    float tmin = 0.0f;
     if (XNA::IntersectRayAxisAlignedBox(rayOrigin, rayDir, &m_MeshBox, &tmin))
     {
        // Find the nearest ray/triangle intersection.
        tmin = MathHelper::Infinity;
        for (UINT i = 0; i < m_MeshIndices.size() / 3; ++i)
        {
            // Indices for this triangle.
            UINT i0 = m_MeshIndices[i * 3 + 0];
            UINT i1 = m_MeshIndices[i * 3 + 1];
            UINT i2 = m_MeshIndices[i * 3 + 2];

            // Vertices for this triangle.
            XMVECTOR v0 = XMLoadFloat3(&m_MeshVertices[i0].Pos);
            XMVECTOR v1 = XMLoadFloat3(&m_MeshVertices[i1].Pos);
            XMVECTOR v2 = XMLoadFloat3(&m_MeshVertices[i2].Pos);

            // We have to iterate over all the triangles in order to find the nearest intersection.
            float t = 0.0f;
            if (XNA::IntersectRayTriangle(rayOrigin, rayDir, v0, v1, v2, &t))
            {
                if (t < tmin)
                {
                    // This is the new nearest picked triangle.
                    tmin = t;
                    m_PickedTriangle = i;
                }
            }
        }
    }
}
