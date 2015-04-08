#pragma once
#include "Object.h"
class Box : public Object
{
public:
    Box();
    ~Box();

    virtual bool Init(ID3D11Device* device);
    virtual void Release();
    virtual void Update(float dt);
    virtual void Render(ID3D11DeviceContext* context);
};

