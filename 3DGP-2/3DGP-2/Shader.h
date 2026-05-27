#pragma once

class CCamera;

class CShader
{
public:
    CShader();
    virtual ~CShader();

    virtual D3D12_INPUT_LAYOUT_DESC   CreateInputLayout();
    virtual D3D12_RASTERIZER_DESC     CreateRasterizerState();
    virtual D3D12_BLEND_DESC          CreateBlendState();
    virtual D3D12_DEPTH_STENCIL_DESC  CreateDepthStencilState();
    virtual D3D12_SHADER_BYTECODE     CreateVertexShader();
    virtual D3D12_SHADER_BYTECODE     CreatePixelShader();

    D3D12_SHADER_BYTECODE CompileShaderFromFile(
        const WCHAR* pszFileName, LPCSTR pszShaderName, LPCSTR pszShaderProfile,
        ID3DBlob** ppd3dShaderBlob);

    virtual void CreateShader(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature);

    virtual void OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

protected:
    ID3DBlob* m_pd3dVertexShaderBlob = NULL;
    ID3DBlob* m_pd3dPixelShaderBlob  = NULL;

    int                   m_nPipelineStates   = 0;
    ID3D12PipelineState** m_ppd3dPipelineStates = NULL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC m_d3dPipelineStateDesc;
};


class CObjectShader : public CShader
{
public:
    CObjectShader();
    virtual ~CObjectShader();

    virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout()  override;
    virtual D3D12_SHADER_BYTECODE   CreateVertexShader() override;
    virtual D3D12_SHADER_BYTECODE   CreatePixelShader()  override;

    virtual void CreateShader(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature) override;

private:
    D3D12_INPUT_ELEMENT_DESC* m_pd3dInputElements = NULL;
    UINT                      m_nInputElements     = 0;
};
