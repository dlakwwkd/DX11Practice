#include "Vertex.h"
#include "Effects.h"

#pragma region InputLayoutDesc

const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Color[2] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Basic32[3] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Color = nullptr;
ID3D11InputLayout* InputLayouts::Basic32 = nullptr;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

    //
    // Color
    //
    Effects::ColorFX->m_ColorTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(device->CreateInputLayout(InputLayoutDesc::Color, 2, passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize, &Color));

    //
    // Basic32
    //
    Effects::BasicFX->m_Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(device->CreateInputLayout(InputLayoutDesc::Basic32, 3, passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize, &Basic32));
}

void InputLayouts::DestroyAll()
{
    ReleaseCOM(Color);
    ReleaseCOM(Basic32);
}

#pragma endregion
