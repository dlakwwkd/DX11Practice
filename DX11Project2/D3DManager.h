#pragma once
#include "d3dUtil.h"
#include "Camera.h"
#include "Shader.h"
class Object;

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
    void    InitClientSize(HWND hWnd);
    void    CreateDeviceAndSwapChain(HWND hWnd);
    void    SetRenderTargets();
    void    CreateDepthStencilView();
    void    CreateRenderTargetView();
    void    SetViewport();
    bool    SetObjectList();

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

    Camera                  m_Camera;
    Shader                  m_Shader;
    std::vector<Object*>    m_ObjectList;

    int                     m_ClientWidth;
    int                     m_ClientHeight;
    UINT                    m_4xMsaaQuality;
    bool                    m_Enable4xMsaa;
};

