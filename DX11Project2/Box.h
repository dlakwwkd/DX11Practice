#pragma once
#include "Object.h"
class Box : public Object
{
public:
    Box();
    ~Box();

    virtual bool Init(ID3D11Device* device);
    virtual void Update(float dt);
};

