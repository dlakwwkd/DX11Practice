#include "Shader.h"


Shader::Shader()
:   m_VertexShader(nullptr),
    m_PixelShader(nullptr),
    m_VertexLayout(nullptr),
    m_ConstantBuffer(nullptr)
{
}

Shader::~Shader()
{
    Release();
}


bool Shader::Init(ID3D11Device* device)
{
    HR(SetupVertexShader(device));
    HR(SetupPixelShader(device));
    CreateConstantBuffer(device);
    CreateRenderState(device);
    return true;
}

void Shader::Release()
{
    ReleaseCOM(m_SolidRS);
    ReleaseCOM(m_ConstantBuffer);
    ReleaseCOM(m_VertexLayout);
    ReleaseCOM(m_PixelShader);
    ReleaseCOM(m_VertexShader);
}

void Shader::Render(ID3D11DeviceContext* context, int idxCnt, CXMMATRIX world, CXMMATRIX view, CXMMATRIX projection)
{
    XMFLOAT4 lightDirection = { XMFLOAT4(-1.0f, -1.0f, 1.0f, 1.0f) };
    XMFLOAT4 lightColor = { XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) };

    ConstantBuffer cb;
    cb.WVP = XMMatrixTranspose(world * view * projection);
    cb.World = XMMatrixTranspose(world);
    cb.lightDir = lightDirection;
    cb.lightColor = lightColor;

    context->UpdateSubresource(m_ConstantBuffer, 0, NULL, &cb, 0, 0);
    context->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    context->PSSetConstantBuffers(0, 1, &m_ConstantBuffer);

    context->RSSetState(m_SolidRS);
    context->IASetInputLayout(m_VertexLayout);
    context->VSSetShader(m_VertexShader, NULL, 0);
    context->PSSetShader(m_PixelShader, NULL, 0);
    context->DrawIndexed(idxCnt, 0, 0);
}


HRESULT Shader::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
    HRESULT hr = S_OK;

    DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
    // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
    // Setting this flag improves the shader debugging experience, but still allows 
    // the shaders to be optimized and to run exactly the way they will run in 
    // the release configuration of this program.
    dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

    ID3DBlob* pErrorBlob;
    hr = D3DX11CompileFromFile(
        szFileName, NULL, NULL,                     // shader 파일 설정
        szEntryPoint, szShaderModel, dwShaderFlags, // 컴파일 설정
        0, NULL,                                    // 쉐이더 옵션
        ppBlobOut, &pErrorBlob, NULL);              // 리턴
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        ReleaseCOM(pErrorBlob);
        return hr;
    }
    ReleaseCOM(pErrorBlob);

    return S_OK;
}

HRESULT Shader::SetupVertexShader(ID3D11Device* device)
{
    HRESULT hr = S_OK;

    // Compile the vertex shader
    ID3DBlob* pVSBlob = NULL;
    hr = CompileShaderFromFile(L"BoxRender.fx", "VS", "vs_4_0", &pVSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the vertex shader
    hr = device->CreateVertexShader(
        pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(),
        NULL, &m_VertexShader);
    if (FAILED(hr))
    {
        ReleaseCOM(pVSBlob);
        return hr;
    }

    // Define the input layout
    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = device->CreateInputLayout(layout, numElements,
        pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(),
        &m_VertexLayout);
    ReleaseCOM(pVSBlob);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

HRESULT Shader::SetupPixelShader(ID3D11Device* device)
{
    HRESULT hr = S_OK;

    // Compile the pixel shader
    ID3DBlob* pPSBlob = NULL;
    hr = CompileShaderFromFile(L"BoxRender.fx", "PS", "ps_4_0", &pPSBlob);
    if (FAILED(hr))
    {
        MessageBox(NULL,
            L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
        return hr;
    }

    // Create the pixel shader
    hr = device->CreatePixelShader(
        pPSBlob->GetBufferPointer(),
        pPSBlob->GetBufferSize(),
        NULL, &m_PixelShader);
    ReleaseCOM(pPSBlob);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void Shader::CreateConstantBuffer(ID3D11Device* device)
{
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HR(device->CreateBuffer(&bd, NULL, &m_ConstantBuffer));
}

void Shader::CreateRenderState(ID3D11Device* device)
{
    D3D11_RASTERIZER_DESC rasterizerDesc;
    ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
    rasterizerDesc.FillMode = D3D11_FILL_SOLID;		// Fill 옵션
    rasterizerDesc.CullMode = D3D11_CULL_BACK;	    // Culling 옵션
    rasterizerDesc.FrontCounterClockwise = false;	// 앞/뒷면 로직 선택
    HR(device->CreateRasterizerState(&rasterizerDesc, &m_SolidRS));
}
