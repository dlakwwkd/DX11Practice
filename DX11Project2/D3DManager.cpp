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
    m_ClientHeight(600)
{
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

    CreateBuffers();    // 정육면체
    InitWVPMatrix();    // 정육면체

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

void D3DManager::Resize()
{

}

void D3DManager::UpdateScene(float dt)
{
    // Update our time
    static float t = 0.0f;
    static DWORD dwTimeStart = 0;
    DWORD dwTimeCur = GetTickCount();
    if (dwTimeStart == 0)
        dwTimeStart = dwTimeCur;
    t = (dwTimeCur - dwTimeStart) / 1000.0f;

    //
    // Transform Cube
    //
    float scaleValue = sinf(t) + 1;
    float moveValue = cosf(t)*10.0f;
    XMMATRIX scale = XMMatrixScaling(scaleValue, scaleValue, scaleValue);
    XMMATRIX rotate = XMMatrixRotationZ(t);
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
        1.0f,                   // depth buffer 지울 때 채울값
        0);                     // stencil buffer 지울 때 초기값

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
    HR(m_SwapChain->Present(0, 0)); // 첫번째 인자 : 갱신 딜레이
}





//--------------------------------------------------------------------------------------
// Init 관련 함수들
//--------------------------------------------------------------------------------------

void D3DManager::CreateDeviceAndSwapChain(HWND hWnd)
{
    // Flag 설정
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
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;	// 백버퍼 포맷
    sd.BufferDesc.RefreshRate.Numerator = 60; 	        // 분자(FPS)
    sd.BufferDesc.RefreshRate.Denominator = 1; 	        // 분모(FPS)
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;	// 백버퍼 렌더링
    sd.OutputWindow = hWnd;		                        // 현재 윈도우
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;

    HR(D3D11CreateDeviceAndSwapChain(
        0,  		                // 기본 디스플레이 어댑터 사용
        D3D_DRIVER_TYPE_HARDWARE, 	// 3D 하드웨어 가속
        0,  		                // 소프트웨어 구동 안함
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
        1,    	                // 렌더 대상 개수. 장면 분할시 1 초과
        &m_RenderTargetView,  	// 렌더 타겟
        m_DepthStencilView);	// 깊이/스텐실 버퍼
}

void D3DManager::CreateRenderTargetView()
{
    ID3D11Texture2D* pBackBuffer = nullptr;

    HR(m_SwapChain->GetBuffer(
        0,                          // 후면 버퍼 색인. 여러개일때 중요. 지금은 1개이므로 0.
        __uuidof(ID3D11Texture2D),  // 버퍼 형식
        (LPVOID*)&pBackBuffer));    // 받아온 버퍼

    HR(m_Device->CreateRenderTargetView(
        pBackBuffer,
        NULL, 		            // 자원 형식 
        &m_RenderTargetView));

    pBackBuffer->Release();		// Get을 해왔으면 반드시 Release
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
    vp.Width = (FLOAT)m_ClientWidth;    // 뷰포트 너비
    vp.Height = (FLOAT)m_ClientHeight;  // 뷰포트 높이
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0; 	        // 그리기 시작 원점 x
    vp.TopLeftY = 0; 	        // 그리기 시작 원점 y
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
        szFileName, NULL, NULL,                     // shader 파일 설정
        szEntryPoint, szShaderModel, dwShaderFlags, // 컴파일 설정
        0, NULL,                                    // 쉐이더 옵션
        ppBlobOut, &pErrorBlob, NULL);              // 리턴
    if (FAILED(hr))
    {
        if (pErrorBlob != NULL)
            OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
        if (pErrorBlob) pErrorBlob->Release();
        return hr;
    }
    if (pErrorBlob) pErrorBlob->Release();

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
        pVSBlob->Release();
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
    pVSBlob->Release();
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
    pPSBlob->Release();
    if (FAILED(hr))
        return hr;

    return S_OK;
}

void D3DManager::CreateBuffers()
{
    // Create vertex buffer
    Vertex vertices[] =
    {
        { XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, -1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, 1.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f) },
        { XMFLOAT3(1.0f, -1.0f, 1.0f), XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) },
    };
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;             // 버퍼 사용 방식
    bd.ByteWidth = sizeof(Vertex)* 8;           // 버퍼 크기
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;    // 파이프라인에 연결되는 버퍼 형태
    bd.CPUAccessFlags = 0;                      // CPU접근 flag, 일반적으로 GPU를 사용하기 때문에 0을 쓴다.

    D3D11_SUBRESOURCE_DATA InitData;
    ZeroMemory(&InitData, sizeof(InitData));
    InitData.pSysMem = vertices;        // 초기화하기 위한 버퍼 배열 포인터
    HR(m_Device->CreateBuffer(
        &bd,                            // 생성할 버퍼의 정보를 담은 구조체
        &InitData,                      // 버퍼 초기화시 필요한 데이터
        &m_VertexBuffer));              // 생성된 버퍼

    // Set vertex buffer
    UINT stride = sizeof(Vertex);
    UINT offset = 0;
    m_ImmediateContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);


    // Create index buffer
    WORD indices[] =
    {
        3, 1, 0,
        2, 1, 3,

        0, 5, 4,
        1, 5, 0,

        3, 4, 7,
        0, 4, 3,

        1, 6, 5,
        2, 6, 1,

        2, 7, 6,
        3, 7, 2,

        6, 4, 5,
        7, 4, 6,
    };
    bd.Usage = D3D11_USAGE_DEFAULT;     // CPU 접근 불가, 생성후 변경 불가, GPU만 접근 가능
    bd.ByteWidth = sizeof(indices);     // 크기
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
        XM_PIDIV2,                              // Pi
        m_ClientWidth / (FLOAT)m_ClientHeight,  // aspect ratio
        0.01f, 100.0f);                         // near plane, far plane
}
