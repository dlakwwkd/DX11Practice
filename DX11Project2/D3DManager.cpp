#include "D3DManager.h"


D3DManager::D3DManager()
:   m_DriverType(D3D_DRIVER_TYPE_HARDWARE),
    m_FeatureLevel(D3D_FEATURE_LEVEL_11_0),
    m_Device(nullptr),
    m_ImmediateContext(nullptr),
    m_SwapChain(nullptr),
    m_DepthStencil(nullptr),
    m_DepthStencilView(nullptr),
    m_RenderTargetView(nullptr),
    m_VertexShader(nullptr),
    m_PixelShader(nullptr),
    m_VertexLayout(nullptr),
    m_VertexBuffer(nullptr),
    m_IndexBuffer(nullptr),
    m_ConstantBuffer(nullptr),
    m_ClientWidth(800),
    m_ClientHeight(600),
    m_4xMsaaQuality(0),
	m_Enable4xMsaa(false)
{
    ZeroMemory(&m_ScreenViewport, sizeof(D3D11_VIEWPORT));
}


D3DManager::~D3DManager()
{
    CleanupDevice();
}

bool D3DManager::InitDevice(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    m_ClientWidth = rc.right - rc.left;
    m_ClientHeight = rc.bottom - rc.top;
    
    CreateDeviceAndSwapChain(hWnd);
    SetRenderTargets();
    SetViewport();

    HR(SetupVertexShader());
    HR(SetupPixelShader());

    CreateBuffers();    // ������ü
    InitWVPMatrix();    // ������ü

    return true;
}

void D3DManager::CleanupDevice()
{
    if (m_ImmediateContext)
        m_ImmediateContext->ClearState();

    ReleaseCOM(m_ConstantBuffer);
    ReleaseCOM(m_VertexBuffer);
    ReleaseCOM(m_IndexBuffer);
    ReleaseCOM(m_VertexLayout);
    ReleaseCOM(m_VertexShader);
    ReleaseCOM(m_PixelShader);
    ReleaseCOM(m_DepthStencil);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_SwapChain);
    ReleaseCOM(m_ImmediateContext);
    ReleaseCOM(m_Device);
}



void D3DManager::UpdateScene(float dt)
{
    //
    // Transform Cube
    //
    static float t = 0.0f;
    t += dt;
    float scaleValue = sinf(t) + 1;
    float moveValue = cosf(t)*10.0f;
    XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
    XMMATRIX rotate = XMMatrixRotationX(t) * XMMatrixRotationY(-t) * XMMatrixRotationZ(t);
    XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 10.0f);
    m_World = scale * rotate * position;

    //
    // Clear the back buffer
    //
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red, green, blue, alpha
    m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, ClearColor);

    //
    // Clear the depth buffer to 1.0 (max depth)
    //
    m_ImmediateContext->ClearDepthStencilView(
        m_DepthStencilView,     // Clear target
        D3D11_CLEAR_DEPTH,      // Clear flag (depth, stencil)
        1.0f,                   // depth buffer ���� �� ä�ﰪ
        0);                     // stencil buffer ���� �� �ʱⰪ

    //
    // Update variables for the cube
    //
    ConstantBuffer cb1;
    cb1.World = XMMatrixTranspose(m_World);
    cb1.View = XMMatrixTranspose(m_View);
    cb1.Projection = XMMatrixTranspose(m_Projection);
    m_ImmediateContext->UpdateSubresource(m_ConstantBuffer, 0, NULL, &cb1, 0, 0);
}


void D3DManager::DrawScene()
{
    //
    // Render the cube
    //
    m_ImmediateContext->VSSetShader(m_VertexShader, NULL, 0);
    m_ImmediateContext->VSSetConstantBuffers(0, 1, &m_ConstantBuffer);
    m_ImmediateContext->PSSetShader(m_PixelShader, NULL, 0);
    m_ImmediateContext->DrawIndexed(36, 0, 0);

    //
    // Present our back buffer to our front buffer
    //
    HR(m_SwapChain->Present(0, 0)); // ù��° ���� : ���� ������
}



void D3DManager::Resize()
{
    assert(m_ImmediateContext);
    assert(m_Device);
    assert(m_SwapChain);

    // Release the old views, as they hold references to the buffers we
    // will be destroying.  Also release the old depth/stencil buffer.

    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_DepthStencil);


    // Resize the swap chain and recreate the render target view.

    HR(m_SwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    ID3D11Texture2D* backBuffer;
    HR(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
    HR(m_Device->CreateRenderTargetView(backBuffer, 0, &m_RenderTargetView));
    ReleaseCOM(backBuffer);

    // Create the depth/stencil buffer and view.

    D3D11_TEXTURE2D_DESC depthStencilDesc;

    depthStencilDesc.Width = m_ClientWidth;
    depthStencilDesc.Height = m_ClientHeight;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

    // Use 4X MSAA? --must match swap chain MSAA values.
    if (m_Enable4xMsaa)
    {
        depthStencilDesc.SampleDesc.Count = 4;
        depthStencilDesc.SampleDesc.Quality = m_4xMsaaQuality - 1;
    }
    // No MSAA
    else
    {
        depthStencilDesc.SampleDesc.Count = 1;
        depthStencilDesc.SampleDesc.Quality = 0;
    }

    depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthStencilDesc.CPUAccessFlags = 0;
    depthStencilDesc.MiscFlags = 0;

    HR(m_Device->CreateTexture2D(&depthStencilDesc, 0, &m_DepthStencil));
    HR(m_Device->CreateDepthStencilView(m_DepthStencil, 0, &m_DepthStencilView));


    // Bind the render target view and depth/stencil view to the pipeline.

    m_ImmediateContext->OMSetRenderTargets(1, &m_RenderTargetView, m_DepthStencilView);


    // Set the viewport transform.

    m_ScreenViewport.TopLeftX = 0;
    m_ScreenViewport.TopLeftY = 0;
    m_ScreenViewport.Width = static_cast<float>(m_ClientWidth);
    m_ScreenViewport.Height = static_cast<float>(m_ClientHeight);
    m_ScreenViewport.MinDepth = 0.0f;
    m_ScreenViewport.MaxDepth = 1.0f;

    m_ImmediateContext->RSSetViewports(1, &m_ScreenViewport);

    m_Projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, AspectRatio(), 0.01f, 100.0f);
}


//--------------------------------------------------------------------------------------
// Init ���� �Լ���
//--------------------------------------------------------------------------------------

void D3DManager::CreateDeviceAndSwapChain(HWND hWnd)
{
    // Flag ����
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);

    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 1;
    sd.BufferDesc.Width = m_ClientWidth;
    sd.BufferDesc.Height = m_ClientHeight;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// ����� ����
    sd.BufferDesc.RefreshRate.Numerator = 60; 	        // ����(FPS)
    sd.BufferDesc.RefreshRate.Denominator = 1; 	        // �и�(FPS)
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// ����� ������
    sd.OutputWindow = hWnd;		                        // ���� ������
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    HR(D3D11CreateDeviceAndSwapChain(
        0,  		                // �⺻ ���÷��� ����� ���
        D3D_DRIVER_TYPE_HARDWARE, 	// 3D �ϵ���� ����
        0,  		                // ����Ʈ���� ���� ����
        createDeviceFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,          // SDK version
        &sd,                        // Swap chain description
        &m_SwapChain,
        &m_Device,
        &m_FeatureLevel,
        &m_ImmediateContext));
}

void D3DManager::SetRenderTargets()
{
    CreateRenderTargetView();
    CreateDepthStencilView();

    m_ImmediateContext->OMSetRenderTargets(
        1,    	                // ���� ��� ����. ��� ���ҽ� 1 �ʰ�
        &m_RenderTargetView,  	// ���� Ÿ��
        m_DepthStencilView);	// ����/���ٽ� ����
}

void D3DManager::CreateRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    HR(m_SwapChain->GetBuffer(
        0,                          // �ĸ� ���� ����. �������϶� �߿�. ������ 1���̹Ƿ� 0.
        __uuidof(ID3D11Texture2D),  // ���� ����
        (LPVOID*)&pBackBuffer));    // �޾ƿ� ����

    HR(m_Device->CreateRenderTargetView(
        pBackBuffer,
        NULL, 		            // �ڿ� ���� 
        &m_RenderTargetView));

    ReleaseCOM(pBackBuffer);	// Get�� �ؿ����� �ݵ�� Release
}

void D3DManager::CreateDepthStencilView()
{
    // Create the depth stencil texture
    D3D11_TEXTURE2D_DESC descDepth;
    ZeroMemory(&descDepth, sizeof(descDepth));
    descDepth.Width = m_ClientWidth;
    descDepth.Height = m_ClientHeight;
    descDepth.MipLevels = 1;
    descDepth.ArraySize = 1;
    descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    descDepth.SampleDesc.Count = 1;
    descDepth.SampleDesc.Quality = 0;
    descDepth.Usage = D3D11_USAGE_DEFAULT;
    descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    descDepth.CPUAccessFlags = 0;
    descDepth.MiscFlags = 0;
    HR(m_Device->CreateTexture2D(&descDepth, NULL, &m_DepthStencil));

    // Create the depth stencil view
    D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
    ZeroMemory(&descDSV, sizeof(descDSV));
    descDSV.Format = descDepth.Format;
    descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    descDSV.Texture2D.MipSlice = 0;
    HR(m_Device->CreateDepthStencilView(m_DepthStencil, &descDSV, &m_DepthStencilView));
}

void D3DManager::SetViewport()
{
    D3D11_VIEWPORT vp;
    vp.Width = (FLOAT)m_ClientWidth;    // ����Ʈ �ʺ�
    vp.Height = (FLOAT)m_ClientHeight;  // ����Ʈ ����
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0; 	        // �׸��� ���� ���� x
    vp.TopLeftY = 0; 	        // �׸��� ���� ���� y
    m_ImmediateContext->RSSetViewports(1, &vp);
}

HRESULT D3DManager::CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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
        szFileName, NULL, NULL,                     // shader ���� ����
        szEntryPoint, szShaderModel, dwShaderFlags, // ������ ����
        0, NULL,                                    // ���̴� �ɼ�
        ppBlobOut, &pErrorBlob, NULL);              // ����
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

HRESULT D3DManager::SetupVertexShader()
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
    hr = m_Device->CreateVertexShader(
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
    };
    UINT numElements = ARRAYSIZE(layout);

    // Create the input layout
    hr = m_Device->CreateInputLayout(layout, numElements,
        pVSBlob->GetBufferPointer(),
        pVSBlob->GetBufferSize(),
        &m_VertexLayout);
    ReleaseCOM(pVSBlob);
    if (FAILED(hr))
        return hr;

    // Set the input layout
    m_ImmediateContext->IASetInputLayout(m_VertexLayout);

    return S_OK;
}

HRESULT D3DManager::SetupPixelShader()
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
    hr = m_Device->CreatePixelShader(
        pPSBlob->GetBufferPointer(),
        pPSBlob->GetBufferSize(),
        NULL, &m_PixelShader);
    ReleaseCOM(pPSBlob);
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void D3DManager::CreateBuffers()
{
    // Create vertex buffer
    Vertex vertices[] =
    {
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f), (const float*)&Colors::White   },
		{ XMFLOAT3(-1.0f, +1.0f, -1.0f), (const float*)&Colors::Black   },
		{ XMFLOAT3(+1.0f, +1.0f, -1.0f), (const float*)&Colors::Red     },
		{ XMFLOAT3(+1.0f, -1.0f, -1.0f), (const float*)&Colors::Green   },
		{ XMFLOAT3(-1.0f, -1.0f, +1.0f), (const float*)&Colors::Blue    },
		{ XMFLOAT3(-1.0f, +1.0f, +1.0f), (const float*)&Colors::Yellow  },
		{ XMFLOAT3(+1.0f, +1.0f, +1.0f), (const float*)&Colors::Cyan    },
		{ XMFLOAT3(+1.0f, -1.0f, +1.0f), (const float*)&Colors::Magenta }
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;             // ���� ��� ���
    bd.ByteWidth = sizeof(vertices);            // ���� ũ��
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;    // ���������ο� ����Ǵ� ���� ����
    bd.CPUAccessFlags = 0;                      // CPU���� flag, �Ϲ������� GPU�� ����ϱ� ������ 0�� ����.

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;        // �ʱ�ȭ�ϱ� ���� ���� �迭 ������
    HR(m_Device->CreateBuffer(
        &bd,                            // ������ ������ ������ ���� ����ü
        &InitData,                      // ���� �ʱ�ȭ�� �ʿ��� ������
        &m_VertexBuffer));              // ������ ����

    // Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_ImmediateContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);


    // Create index buffer
    WORD indices[] =
    {
        // front face
        0, 1, 2,
        0, 2, 3,

        // back face
        4, 6, 5,
        4, 7, 6,

        // left face
        4, 5, 1,
        4, 1, 0,

        // right face
        3, 2, 6,
        3, 6, 7,

        // top face
        1, 5, 6,
        1, 6, 2,

        // bottom face
        4, 0, 3,
        4, 3, 7
    };
    bd.Usage = D3D11_USAGE_DEFAULT;     // CPU ���� �Ұ�, ������ ���� �Ұ�, GPU�� ���� ����
    bd.ByteWidth = sizeof(indices);     // ũ��
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;
    InitData.pSysMem = indices;
    HR(m_Device->CreateBuffer(&bd, &InitData, &m_IndexBuffer));

    // Set index buffer
    m_ImmediateContext->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R16_UINT, 0);

    // Set primitive topology
    m_ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


    // Create the constant buffer
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(ConstantBuffer);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = 0;
    HR(m_Device->CreateBuffer(&bd, NULL, &m_ConstantBuffer));
}

void D3DManager::InitWVPMatrix()
{
    // Initialize the world matrix
    m_World = XMMatrixIdentity();

    // Initialize the view matrix
    XMVECTOR Eye = XMVectorSet(0.0f, 1.0f, -5.0f, 0.0f);
    XMVECTOR At = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    m_View = XMMatrixLookAtLH(Eye, At, Up);

    // Initialize the projection matrix
    m_Projection = XMMatrixPerspectiveFovLH(
        XM_PIDIV2,      // Pi
        AspectRatio(),  // aspect ratio
        0.01f, 100.0f); // near plane, far plane
}
