#include "D3DManager.h"
#include "InputManager.h"
#include "Effects.h"
#include "Vertex.h"
#include "RenderStates.h"
#include "BasisVector.h"
#include "Box.h"
#include "Land.h"

#define MAX_OBJECT_NUM 100

D3DManager::D3DManager()
:   m_DriverType(D3D_DRIVER_TYPE_HARDWARE),
    m_FeatureLevel(D3D_FEATURE_LEVEL_11_0),
    m_Device(nullptr),
    m_ImmediateContext(nullptr),
    m_SwapChain(nullptr),
    m_DepthStencil(nullptr),
    m_DepthStencilView(nullptr),
    m_RenderTargetView(nullptr),
    m_ClientWidth(800),
    m_ClientHeight(600),
    m_4xMsaaQuality(0),
	m_Enable4xMsaa(false)
{
    m_ObjectList.reserve(MAX_OBJECT_NUM);
}


D3DManager::~D3DManager()
{
    CleanupDevice();
}

bool D3DManager::InitDevice(HWND hWnd)
{
    InitClientSize(hWnd);

    CreateDeviceAndSwapChain(hWnd);
    SetRenderTargets();
    SetViewport();
    SetLight();

    Effects::InitAll(m_Device);
    InputLayouts::InitAll(m_Device);
    RenderStates::InitAll(m_Device);

    if (!SetObjectList())
        return false;

    Resize();
    return true;
}

void D3DManager::CleanupDevice()
{
    if (m_ImmediateContext)
        m_ImmediateContext->ClearState();

    for (auto& object : m_ObjectList)
    {
        object->Release();
        SafeDelete(object);
    }
    m_ObjectList.clear();

    RenderStates::DestroyAll();
    InputLayouts::DestroyAll();
    Effects::DestroyAll();

    ReleaseCOM(m_DepthStencil);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_SwapChain);
    ReleaseCOM(m_ImmediateContext);
    ReleaseCOM(m_Device);
}



void D3DManager::Update(float dt)
{
    m_Camera.Update(dt);

    auto input = InputManager::getInstance();
    if (input->GetMouseState(MK_RBUTTON))
        return;
    if (input->GetKeyState('1'))
        RenderStates::m_RenderOptions = RenderOptions::Lighting;
    if (input->GetKeyState('2'))
        RenderStates::m_RenderOptions = RenderOptions::Textures;
    if (input->GetKeyState('3'))
        RenderStates::m_RenderOptions = RenderOptions::TexturesAndFog;
    
    auto pos = input->GetMousePos();
    auto view = m_Camera.View();
    auto proj = m_Camera.Proj();
    for (auto& object : m_ObjectList)
    {
        object->Pick(pos.x, pos.y, m_ClientWidth, m_ClientHeight, view, proj);
        object->Update(dt);
    }
}

void D3DManager::Render()
{
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red, green, blue, alpha
    m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, ClearColor);
    m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    auto viewProj = m_Camera.ViewProj();
    auto eyePos = m_Camera.GetPosition();

    Effects::BasicFX->SetDirLights(m_DirLights);
    Effects::BasicFX->SetEyePosW(eyePos);
    Effects::BasicFX->SetFogColor(Colors::Silver);
    Effects::BasicFX->SetFogStart(50.0f);
    Effects::BasicFX->SetFogRange(150.0f);

    for (auto& object : m_ObjectList)
    {
        object->Render(m_ImmediateContext, viewProj);
    }
    HR(m_SwapChain->Present(0, 0)); // 첫번째 인자 : 갱신 딜레이
}


void D3DManager::Resize()
{
    assert(m_ImmediateContext);
    assert(m_Device);
    assert(m_SwapChain);

    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_DepthStencil);

    HR(m_SwapChain->ResizeBuffers(1, m_ClientWidth, m_ClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
    SetRenderTargets();
    SetViewport();

    m_Camera.SetLens(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
}



void D3DManager::InitClientSize(HWND hWnd)
{
    RECT rc;
    GetClientRect(hWnd, &rc);
    m_ClientWidth = rc.right - rc.left;
    m_ClientHeight = rc.bottom - rc.top;
}

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

    ReleaseCOM(pBackBuffer);	// Get을 해왔으면 반드시 Release
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

void D3DManager::SetLight()
{
    m_DirLights[0].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLights[0].Diffuse = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLights[0].Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
    m_DirLights[0].Direction = XMFLOAT3(0.57735f, -0.57735f, 0.57735f);

    m_DirLights[1].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[1].Diffuse = XMFLOAT4(0.20f, 0.20f, 0.20f, 1.0f);
    m_DirLights[1].Specular = XMFLOAT4(0.25f, 0.25f, 0.25f, 1.0f);
    m_DirLights[1].Direction = XMFLOAT3(-0.57735f, -0.57735f, 0.57735f);

    m_DirLights[2].Ambient = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[2].Diffuse = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    m_DirLights[2].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    m_DirLights[2].Direction = XMFLOAT3(0.0f, -0.707f, -0.707f);
}

bool D3DManager::SetObjectList()
{
    auto bv = new BasisVector();
    auto box = new Box();
    auto land = new Land();
    m_ObjectList.push_back(bv);
    m_ObjectList.push_back(box);
    m_ObjectList.push_back(land);

    for (auto& object : m_ObjectList)
    {
        if (!object->Init(m_Device))
            return false;
    }
    return true;
}

