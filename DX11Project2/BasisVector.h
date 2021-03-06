#pragma once
#include "Object.h"
class BasisVector : public Object
{
public:
    BasisVector();
    virtual ~BasisVector();

    virtual void Init(ID3D11Device* device);
    virtual void Release();
    virtual void Update(float dt);
    virtual void Render(ID3D11DeviceContext* context, CXMMATRIX viewProj);
    virtual void CreateBuffer(ID3D11Device* device);
};

