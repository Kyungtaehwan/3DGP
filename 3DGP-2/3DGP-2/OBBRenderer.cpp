#include "pch.h"
#include "OBBRenderer.h"
#include <D3Dcompiler.h>

COBBRenderer* COBBRenderer::s_pInstance = nullptr;
bool          COBBRenderer::s_bShow     = false;

COBBRenderer* COBBRenderer::Get_Instance()
{
    if (!s_pInstance) s_pInstance = new COBBRenderer;
    return s_pInstance;
}

void COBBRenderer::Destroy_Instance()
{
    if (s_pInstance) { delete s_pInstance; s_pInstance = nullptr; }
}

// ---------------------------------------------------------------
// Init
// ---------------------------------------------------------------
void COBBRenderer::Init(ID3D12Device* pd3dDevice,
                        ID3D12GraphicsCommandList* pd3dCommandList,
                        ID3D12RootSignature* pd3dRootSignature)
{
    // -- Compile shaders --
    UINT flags = 0;
#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob* pVS = nullptr, * pPS = nullptr, * pErr = nullptr;

    D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr,
                       "VSMain", "vs_5_1", flags, 0, &pVS, &pErr);
    if (pErr) { OutputDebugStringA((char*)pErr->GetBufferPointer()); pErr->Release(); pErr = nullptr; }

    D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr,
                       "PSMain", "ps_5_1", flags, 0, &pPS, &pErr);
    if (pErr) { OutputDebugStringA((char*)pErr->GetBufferPointer()); pErr->Release(); }

    // -- Input layout (same as CObjectShader) --
    D3D12_INPUT_ELEMENT_DESC inputElems[2] = {};
    inputElems[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0,
                      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElems[1] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
                      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    // -- Rasterizer: no culling, solid (lines ignore fill mode) --
    D3D12_RASTERIZER_DESC raster = {};
    raster.FillMode              = D3D12_FILL_MODE_SOLID;
    raster.CullMode              = D3D12_CULL_MODE_NONE;
    raster.FrontCounterClockwise = FALSE;
    raster.DepthClipEnable       = TRUE;

    // -- Depth stencil: read depth, no write so OBBs don't block scene --
    D3D12_DEPTH_STENCIL_DESC ds = {};
    ds.DepthEnable    = TRUE;
    ds.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
    ds.DepthFunc      = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    // -- Blend: default opaque --
    D3D12_BLEND_DESC blend = {};
    blend.RenderTarget[0].BlendEnable            = FALSE;
    blend.RenderTarget[0].RenderTargetWriteMask  = D3D12_COLOR_WRITE_ENABLE_ALL;

    // -- PSO --
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature        = pd3dRootSignature;
    psoDesc.VS                    = { pVS->GetBufferPointer(), pVS->GetBufferSize() };
    psoDesc.PS                    = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
    psoDesc.InputLayout           = { inputElems, 2 };
    psoDesc.RasterizerState       = raster;
    psoDesc.BlendState            = blend;
    psoDesc.DepthStencilState     = ds;
    psoDesc.SampleMask            = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    psoDesc.NumRenderTargets      = 1;
    psoDesc.RTVFormats[0]         = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.DSVFormat             = DXGI_FORMAT_D24_UNORM_S8_UINT;
    psoDesc.SampleDesc.Count      = 1;

    pd3dDevice->CreateGraphicsPipelineState(&psoDesc,
        __uuidof(ID3D12PipelineState), (void**)&m_pPSO);

    pVS->Release();
    pPS->Release();

    // -- Upload vertex buffer (persistently mapped) --
    UINT vbBytes = sizeof(CVertex) * MAX_VERTICES;

    D3D12_HEAP_PROPERTIES heapUp = {};
    heapUp.Type = D3D12_HEAP_TYPE_UPLOAD;

    D3D12_RESOURCE_DESC resDesc = {};
    resDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    resDesc.Width              = vbBytes;
    resDesc.Height             = 1;
    resDesc.DepthOrArraySize   = 1;
    resDesc.MipLevels          = 1;
    resDesc.Format             = DXGI_FORMAT_UNKNOWN;
    resDesc.SampleDesc.Count   = 1;
    resDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    pd3dDevice->CreateCommittedResource(
        &heapUp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        __uuidof(ID3D12Resource), (void**)&m_pVB);

    m_pVB->Map(0, nullptr, (void**)&m_pMappedVB);

    m_vbView.BufferLocation = m_pVB->GetGPUVirtualAddress();
    m_vbView.SizeInBytes    = vbBytes;
    m_vbView.StrideInBytes  = sizeof(CVertex);

    // -- Identity world matrix constant buffer (b1) --
    UINT cbBytes = (sizeof(XMFLOAT4X4) + 255) & ~255;
    resDesc.Width = cbBytes;

    pd3dDevice->CreateCommittedResource(
        &heapUp, D3D12_HEAP_FLAG_NONE, &resDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
        __uuidof(ID3D12Resource), (void**)&m_pCBWorld);

    m_pCBWorld->Map(0, nullptr, &m_pMappedCBWorld);

    // Store transposed identity (HLSL column-major convention)
    XMFLOAT4X4 identity;
    XMStoreFloat4x4(&identity, XMMatrixTranspose(XMMatrixIdentity()));
    memcpy(m_pMappedCBWorld, &identity, sizeof(XMFLOAT4X4));
}

// ---------------------------------------------------------------
// BeginFrame / AddOBB / Render
// ---------------------------------------------------------------
void COBBRenderer::BeginFrame()
{
    m_nVerts = 0;
}

void COBBRenderer::AddOBB(const BoundingOrientedBox& obb, XMFLOAT4 color)
{
    if (m_nVerts + 24 > MAX_VERTICES) return;

    XMFLOAT3 c[8];
    obb.GetCorners(c);

    // 12 edges of the box as line list
    static const int edges[12][2] = {
        {0,1},{1,2},{2,3},{3,0},   // front face (z+)
        {4,5},{5,6},{6,7},{7,4},   // back face  (z-)
        {0,4},{1,5},{2,6},{3,7}    // connecting sides
    };

    for (auto& e : edges)
    {
        m_pMappedVB[m_nVerts++] = CVertex(c[e[0]], color);
        m_pMappedVB[m_nVerts++] = CVertex(c[e[1]], color);
    }
}

void COBBRenderer::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_nVerts == 0 || !m_pPSO) return;

    pd3dCommandList->SetPipelineState(m_pPSO);

    // Bind identity world matrix to b1
    pd3dCommandList->SetGraphicsRootConstantBufferView(
        1, m_pCBWorld->GetGPUVirtualAddress());

    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbView);
    pd3dCommandList->DrawInstanced(m_nVerts, 1, 0, 0);
}

// ---------------------------------------------------------------
// Release
// ---------------------------------------------------------------
void COBBRenderer::Release()
{
    if (m_pVB)
    {
        m_pVB->Unmap(0, nullptr);
        m_pVB->Release();
        m_pVB       = nullptr;
        m_pMappedVB = nullptr;
    }
    if (m_pCBWorld)
    {
        m_pCBWorld->Unmap(0, nullptr);
        m_pCBWorld->Release();
        m_pCBWorld       = nullptr;
        m_pMappedCBWorld = nullptr;
    }
    if (m_pPSO) { m_pPSO->Release(); m_pPSO = nullptr; }
}
