#pragma once
#include "d3dUtil.h"
class Shader
{
private:
    struct ConstantBuffer
    {
        XMMATRIX World;
        XMMATRIX View;
        XMMATRIX Projection;
    };
public:
    Shader();
    ~Shader();

    bool    Init(ID3D11Device* device);
    void    Release();
    void    Render(ID3D11DeviceContext* context, int idxCnt, CXMMATRIX world, CXMMATRIX view, CXMMATRIX projection);

private:
    HRESULT CompileShaderFromFile(WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
    HRESULT SetupVertexShader(ID3D11Device* device);
    HRESULT SetupPixelShader(ID3D11Device* device);

private:
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader*  m_PixelShader;
    ID3D11InputLayout*  m_VertexLayout;
    ID3D11Buffer*       m_ConstantBuffer;
};

