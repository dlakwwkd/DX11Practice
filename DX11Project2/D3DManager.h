#pragma once
#include "d3dUtil.h"


struct Vertex
{
    XMFLOAT3 Pos;
    XMFLOAT4 Color;
};

struct ConstantBuffer
{
    XMMATRIX World;
    XMMATRIX View;
    XMMATRIX Projection;
};


class D3DManager
{
public:
    static D3DManager* getInstance()
    {
        static D3DManager d3dManager;
        return &d3dManager;
    }

    inline ID3D11Device*    GetDevice() const { return m_Device; }
    inline float            AspectRatio() const { return static_cast<float>(m_ClientWidth) / m_ClientHeight; }
    inline void             SetClientSize(int w, int h){ m_ClientWidth = w; m_ClientHeight = h; }

    bool    InitDevice(HWND hWnd);
    void    CleanupDevice();
    void    UpdateScene(float dt);
    void    DrawScene();
    void    Resize();

private:
    void    CreateDeviceAndSwapChain(HWND hWnd);
    void    SetRenderTargets();
    void    CreateDepthStencilView();
    void    CreateRenderTargetView();
    void    SetViewport();

    HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
    HRESULT SetupVertexShader();
    HRESULT SetupPixelShader();

    void    CreateBuffers();
    void    InitWVPMatrix();

private:
    D3DManager();
    ~D3DManager();

    D3D_DRIVER_TYPE         m_DriverType;
    D3D_FEATURE_LEVEL       m_FeatureLevel;
                            
    ID3D11Device*           m_Device;
    ID3D11DeviceContext*    m_ImmediateContext;
    IDXGISwapChain*         m_SwapChain;
    ID3D11Texture2D*        m_DepthStencil;
    ID3D11DepthStencilView* m_DepthStencilView;
    ID3D11RenderTargetView* m_RenderTargetView;

    ID3D11VertexShader*     m_VertexShader;
    ID3D11PixelShader*      m_PixelShader;
    ID3D11InputLayout*      m_VertexLayout;

    ID3D11Buffer*           m_VertexBuffer; // ID3D11Buffer : 모든 버퍼 공통 인터페이스
    ID3D11Buffer*           m_IndexBuffer;
    ID3D11Buffer*           m_ConstantBuffer;

    XMMATRIX                m_World;
    XMMATRIX                m_View;
    XMMATRIX                m_Projection;

    D3D11_VIEWPORT          m_ScreenViewport;
    int                     m_ClientWidth;
    int                     m_ClientHeight;
    UINT                    m_4xMsaaQuality;
    bool                    m_Enable4xMsaa;
};

