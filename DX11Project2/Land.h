#pragma once
#include "Object.h"
class Land : public Object
{
public:
    Land();
    virtual ~Land();

    virtual void Init(ID3D11Device* device);
    virtual void Release();
    virtual void Update(float dt);
    virtual void Render(ID3D11DeviceContext* context, CXMMATRIX viewProj);
    virtual void CreateBuffer(ID3D11Device* device);

private:
    float       GetHillHeight(float x, float z) const { return 0.3f*(z*sinf(0.1f*x) + x*cosf(0.1f*z)); }
    XMFLOAT3    GetHillNormal(float x, float z) const
    {
        // n = (-df/dx, 1, -df/dz)
        XMFLOAT3 n(
            -0.03f*z*cosf(0.1f*x) - 0.3f*cosf(0.1f*z),
            1.0f,
            -0.3f*sinf(0.1f*x) + 0.03f*x*sinf(0.1f*z));

        XMVECTOR unitNormal = XMVector3Normalize(XMLoadFloat3(&n));
        XMStoreFloat3(&n, unitNormal);

        return n;
    }
};

