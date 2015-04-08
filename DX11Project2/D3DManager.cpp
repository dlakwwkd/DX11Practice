#include "D3DManager.h"
#include "InputManager.h"
#include "Box.h"

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
    
    if (!SetObjectList())
        return false;

    if (!m_Shader.Init(m_Device))
        return false;

    Resize();
    return true;
}

void D3DManager::CleanupDevice()
{
    if (m_ImmediateContext)
        m_ImmediateContext->ClearState();

    m_Shader.Release();

    for (auto& object : m_ObjectList)
    {
        object->Release();
        SafeDelete(object);
    }
    m_ObjectList.clear();

    ReleaseCOM(m_DepthStencil);
    ReleaseCOM(m_DepthStencilView);
    ReleaseCOM(m_RenderTargetView);
    ReleaseCOM(m_SwapChain);
    ReleaseCOM(m_ImmediateContext);
    ReleaseCOM(m_Device);
}



void D3DManager::UpdateScene(float dt)
{
    m_Camera.Update(dt);

    auto input = InputManager::getInstance();
    if (input->GetMouseState(MK_RBUTTON))
        return;
    
    for (auto& object : m_ObjectList)
    {
        object->Update(dt);
    }
}

void D3DManager::DrawScene()
{
    float ClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f }; //red, green, blue, alpha
    m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView, ClearColor);
    m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    m_Camera.UpdateViewMatrix();
    auto view = m_Camera.View();
    auto proj = m_Camera.Proj();

    for (auto& object : m_ObjectList)
    {
        auto idxCnt = object->GetIndexCount();
        auto world = object->GetWorldMatrix();

        object->Render(m_ImmediateContext);
        m_Shader.Render(m_ImmediateContext, idxCnt, world, view, proj);
    }

    HR(m_SwapChain->Present(0, 0)); // ù��° ���� : ���� ������
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

bool D3DManager::SetObjectList()
{
    auto box = new Box();
    m_ObjectList.push_back(box);

    for (auto& object : m_ObjectList)
    {
        if (!object->Init(m_Device))
            return false;
    }
    return true;
}

