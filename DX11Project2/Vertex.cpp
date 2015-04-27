#include "Vertex.h"
#include "Effects.h"

#pragma region InputLayoutDesc
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Pos[1] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
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
const D3D11_INPUT_ELEMENT_DESC InputLayoutDesc::Terrain[3] =
{
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};
#pragma endregion

#pragma region InputLayouts

ID3D11InputLayout* InputLayouts::Pos = nullptr;
ID3D11InputLayout* InputLayouts::Color = nullptr;
ID3D11InputLayout* InputLayouts::Basic32 = nullptr;
ID3D11InputLayout* InputLayouts::Terrain = nullptr;

void InputLayouts::InitAll(ID3D11Device* device)
{
	D3DX11_PASS_DESC passDesc;

    //
    // Pos
    //
    Effects::SkyFX->m_SkyTech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(device->CreateInputLayout(InputLayoutDesc::Pos, 1, passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize, &Pos));

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

    //
    // Terrain
    //
    Effects::TerrainFX->m_Light1Tech->GetPassByIndex(0)->GetDesc(&passDesc);
    HR(device->CreateInputLayout(InputLayoutDesc::Terrain, 3, passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize, &Terrain));
}

void InputLayouts::DestroyAll()
{
    ReleaseCOM(Pos);
    ReleaseCOM(Color);
    ReleaseCOM(Basic32);
    ReleaseCOM(Terrain);
}

#pragma endregion
