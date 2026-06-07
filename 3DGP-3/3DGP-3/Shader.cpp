#include "pch.h"
#include "Shader.h"
#include "Camera.h"

CShader::CShader()
{
    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
}

CShader::~CShader()
{
    if(m_pd3dVertexShaderBlob)
    {
        m_pd3dVertexShaderBlob->Release();
        m_pd3dVertexShaderBlob = NULL;
    }
    if(m_pd3dPixelShaderBlob)
    {
        m_pd3dPixelShaderBlob->Release();
        m_pd3dPixelShaderBlob = NULL;
    }

    if(m_ppd3dPipelineStates)
    {
        for(int i = 0; i < m_nPipelineStates; i++)
            if(m_ppd3dPipelineStates[i])
                m_ppd3dPipelineStates[i]->Release();
        delete[] m_ppd3dPipelineStates;
        m_ppd3dPipelineStates = NULL;
    }
}

D3D12_INPUT_LAYOUT_DESC CShader::CreateInputLayout()
{
    D3D12_INPUT_LAYOUT_DESC desc;
    desc.pInputElementDescs = NULL;
    desc.NumElements = 0;
    return desc;
}

D3D12_RASTERIZER_DESC CShader::CreateRasterizerState()
{
    D3D12_RASTERIZER_DESC d3dRasterizerDesc;
    ::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
    d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
    d3dRasterizerDesc.FrontCounterClockwise = FALSE;
    d3dRasterizerDesc.DepthClipEnable = TRUE;
    return d3dRasterizerDesc;
}

D3D12_BLEND_DESC CShader::CreateBlendState()
{
    D3D12_BLEND_DESC d3dBlendDesc;
    ::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
    d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    d3dBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    d3dBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
    d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
    d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
    return d3dBlendDesc;
}

D3D12_DEPTH_STENCIL_DESC CShader::CreateDepthStencilState()
{
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc;
    ::ZeroMemory(&d3dDepthStencilDesc, sizeof(D3D12_DEPTH_STENCIL_DESC));
    d3dDepthStencilDesc.DepthEnable = TRUE;
    d3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    d3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
    return d3dDepthStencilDesc;
}

D3D12_SHADER_BYTECODE CShader::CreateVertexShader()
{
    D3D12_SHADER_BYTECODE bc{ NULL, 0 };
    return bc;
}
D3D12_SHADER_BYTECODE CShader::CreatePixelShader()
{
    D3D12_SHADER_BYTECODE bc{ NULL, 0 };
    return bc;
}

D3D12_SHADER_BYTECODE CShader::CompileShaderFromFile(const WCHAR* pszFileName, LPCSTR pszShaderName,
                                                     LPCSTR pszShaderProfile,
                                                     ID3DBlob** ppd3dShaderBlob)
{
    UINT nCompileFlags = 0;
#ifdef _DEBUG
    nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

    ID3DBlob* pd3dErrorBlob = NULL;
    HRESULT hResult = ::D3DCompileFromFile(pszFileName, NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                                           pszShaderName, pszShaderProfile,
                                           nCompileFlags, 0,
                                           ppd3dShaderBlob, &pd3dErrorBlob);
    if(FAILED(hResult) && pd3dErrorBlob)
    {
        OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer());
        pd3dErrorBlob->Release();
    }

    D3D12_SHADER_BYTECODE bc{ NULL, 0 };
    if(*ppd3dShaderBlob)
    {
        bc.pShaderBytecode = (*ppd3dShaderBlob)->GetBufferPointer();
        bc.BytecodeLength = (*ppd3dShaderBlob)->GetBufferSize();
    }
    return bc;
}

void CShader::CreateShader(ID3D12Device* pd3dDevice,
                           ID3D12GraphicsCommandList* pd3dCommandList,
                           ID3D12RootSignature* pd3dRootSignature)
{
    D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc = CreateInputLayout();
    D3D12_RASTERIZER_DESC d3dRasterizerDesc = CreateRasterizerState();
    D3D12_BLEND_DESC d3dBlendDesc = CreateBlendState();
    D3D12_DEPTH_STENCIL_DESC d3dDepthStencilDesc = CreateDepthStencilState();

    D3D12_SHADER_BYTECODE d3dVS = CreateVertexShader();
    D3D12_SHADER_BYTECODE d3dPS = CreatePixelShader();

    ::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    m_d3dPipelineStateDesc.pRootSignature = pd3dRootSignature;
    m_d3dPipelineStateDesc.VS = d3dVS;
    m_d3dPipelineStateDesc.PS = d3dPS;
    m_d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;
    m_d3dPipelineStateDesc.BlendState = d3dBlendDesc;
    m_d3dPipelineStateDesc.DepthStencilState = d3dDepthStencilDesc;
    m_d3dPipelineStateDesc.InputLayout = d3dInputLayoutDesc;
    m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
    m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    m_d3dPipelineStateDesc.NumRenderTargets = 1;
    m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    m_d3dPipelineStateDesc.SampleDesc.Count = 1;
    m_d3dPipelineStateDesc.SampleDesc.Quality = 0;

    HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(
        &m_d3dPipelineStateDesc,
        __uuidof(ID3D12PipelineState),
        (void**)&m_ppd3dPipelineStates[0]);

    if(FAILED(hResult))
        OutputDebugStringA("ERROR: CreateGraphicsPipelineState failed.\n");
}

void CShader::OnPrepareRender(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if(m_ppd3dPipelineStates && m_ppd3dPipelineStates[0])
        pd3dCommandList->SetPipelineState(m_ppd3dPipelineStates[0]);
}

void CShader::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    OnPrepareRender(pd3dCommandList);
}


CIlluminatedShader::CIlluminatedShader() {}
CIlluminatedShader::~CIlluminatedShader()
{
    if(m_pd3dInputElements)
    {
        delete[] m_pd3dInputElements;
        m_pd3dInputElements = NULL;
    }
}

D3D12_INPUT_LAYOUT_DESC CIlluminatedShader::CreateInputLayout()
{
    m_nInputElements = 2;
    m_pd3dInputElements = new D3D12_INPUT_ELEMENT_DESC[m_nInputElements];

    m_pd3dInputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                               D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_pd3dInputElements[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0,
                               D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC desc;
    desc.pInputElementDescs = m_pd3dInputElements;
    desc.NumElements = m_nInputElements;
    return desc;
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreateVertexShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "VSLighting", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CIlluminatedShader::CreatePixelShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "PSLighting", "ps_5_1", &m_pd3dPixelShaderBlob);
}

void CIlluminatedShader::CreateShader(ID3D12Device* pd3dDevice,
                                      ID3D12GraphicsCommandList* pd3dCommandList,
                                      ID3D12RootSignature* pd3dRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
    m_ppd3dPipelineStates[0] = NULL;

    CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    if(m_pd3dVertexShaderBlob)
    {
        m_pd3dVertexShaderBlob->Release();
        m_pd3dVertexShaderBlob = NULL;
    }
    if(m_pd3dPixelShaderBlob)
    {
        m_pd3dPixelShaderBlob->Release();
        m_pd3dPixelShaderBlob = NULL;
    }
    if(m_pd3dInputElements)
    {
        delete[] m_pd3dInputElements;
        m_pd3dInputElements = NULL;
    }
}


D3D12_SHADER_BYTECODE CTerrainShader::CreatePixelShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "PSTerrainLine", "ps_5_1", &m_pd3dPixelShaderBlob);
}


CColorShader::CColorShader() {}
CColorShader::~CColorShader()
{
    if(m_pd3dInputElements)
    {
        delete[] m_pd3dInputElements;
        m_pd3dInputElements = NULL;
    }
}

D3D12_INPUT_LAYOUT_DESC CColorShader::CreateInputLayout()
{
    m_nInputElements = 2;
    m_pd3dInputElements = new D3D12_INPUT_ELEMENT_DESC[m_nInputElements];

    m_pd3dInputElements[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,
                               D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    m_pd3dInputElements[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
                               D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_INPUT_LAYOUT_DESC desc;
    desc.pInputElementDescs = m_pd3dInputElements;
    desc.NumElements = m_nInputElements;
    return desc;
}

D3D12_SHADER_BYTECODE CColorShader::CreateVertexShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "VSColor", "vs_5_1", &m_pd3dVertexShaderBlob);
}

D3D12_SHADER_BYTECODE CColorShader::CreatePixelShader()
{
    return CompileShaderFromFile(L"Shaders.hlsl", "PSColor", "ps_5_1", &m_pd3dPixelShaderBlob);
}

void CColorShader::CreateShader(ID3D12Device* pd3dDevice,
                                ID3D12GraphicsCommandList* pd3dCommandList,
                                ID3D12RootSignature* pd3dRootSignature)
{
    m_nPipelineStates = 1;
    m_ppd3dPipelineStates = new ID3D12PipelineState*[m_nPipelineStates];
    m_ppd3dPipelineStates[0] = NULL;

    CShader::CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    if(m_pd3dVertexShaderBlob)
    {
        m_pd3dVertexShaderBlob->Release();
        m_pd3dVertexShaderBlob = NULL;
    }
    if(m_pd3dPixelShaderBlob)
    {
        m_pd3dPixelShaderBlob->Release();
        m_pd3dPixelShaderBlob = NULL;
    }
    if(m_pd3dInputElements)
    {
        delete[] m_pd3dInputElements;
        m_pd3dInputElements = NULL;
    }
}
