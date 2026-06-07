#include "pch.h"
#include "UI_Manager.h"
#include <vector>

CUI_Manager* CUI_Manager::s_pInstance = nullptr;

CUI_Manager* CUI_Manager::Get_Instance()
{
    if(!s_pInstance)
        s_pInstance = new CUI_Manager;
    return s_pInstance;
}

void CUI_Manager::Destroy_Instance()
{
    if(s_pInstance)
    {
        delete s_pInstance;
        s_pInstance = nullptr;
    }
}

namespace
{
    struct UIVtx
    {
        XMFLOAT2 pos;
        XMFLOAT4 col;
    };

    void AppendBoxOutline(std::vector<UIVtx>& outVertices, float x0, float y0, float x1, float y1, XMFLOAT4 color)
    {
        outVertices.push_back({ { x0, y0 }, color });
        outVertices.push_back({ { x1, y0 }, color });
        outVertices.push_back({ { x1, y0 }, color });
        outVertices.push_back({ { x1, y1 }, color });
        outVertices.push_back({ { x1, y1 }, color });
        outVertices.push_back({ { x0, y1 }, color });
        outVertices.push_back({ { x0, y1 }, color });
        outVertices.push_back({ { x0, y0 }, color });
    }

    ID3D12Resource* CreateStaticVB(ID3D12Device* pd3dDevice, const void* pData, UINT nBytes,
                                   D3D12_VERTEX_BUFFER_VIEW& outView, UINT stride)
    {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = nBytes;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ID3D12Resource* pBuffer = nullptr;
        pd3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                            __uuidof(ID3D12Resource), (void**)&pBuffer);

        void* pMapped = nullptr;
        pBuffer->Map(0, nullptr, &pMapped);
        memcpy(pMapped, pData, nBytes);
        pBuffer->Unmap(0, nullptr);

        outView.BufferLocation = pBuffer->GetGPUVirtualAddress();
        outView.SizeInBytes = nBytes;
        outView.StrideInBytes = stride;
        return pBuffer;
    }

    ID3D12Resource* CreateDynamicVB(ID3D12Device* pd3dDevice, UINT nBytes,
                                    D3D12_VERTEX_BUFFER_VIEW& outView, UINT stride, void** ppMapped)
    {
        D3D12_HEAP_PROPERTIES heapProps = {};
        heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC bufferDesc = {};
        bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        bufferDesc.Width = nBytes;
        bufferDesc.Height = 1;
        bufferDesc.DepthOrArraySize = 1;
        bufferDesc.MipLevels = 1;
        bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
        bufferDesc.SampleDesc.Count = 1;
        bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ID3D12Resource* pBuffer = nullptr;
        pd3dDevice->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &bufferDesc,
                                            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
                                            __uuidof(ID3D12Resource), (void**)&pBuffer);

        pBuffer->Map(0, nullptr, ppMapped);
        outView.BufferLocation = pBuffer->GetGPUVirtualAddress();
        outView.SizeInBytes = nBytes;
        outView.StrideInBytes = stride;
        return pBuffer;
    }
} // namespace

void CUI_Manager::Initialize(ID3D12Device* pd3dDevice,
                             ID3D12GraphicsCommandList*,
                             ID3D12RootSignature* pd3dRootSignature)
{
    if(m_pPSOLine)
        return;

    UINT flags = 0;
#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob *pVS = nullptr, *pPS = nullptr, *pErr = nullptr;
    // Shaders.hlsl #includes "Light.hlsl", so a real include handler is required.
    D3DCompileFromFile(L"Shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                       "VSUIMain", "vs_5_1", flags, 0, &pVS, &pErr);
    if(pErr)
    {
        OutputDebugStringA((char*)pErr->GetBufferPointer());
        pErr->Release();
        pErr = nullptr;
    }
    D3DCompileFromFile(L"Shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
                       "PSUIMain", "ps_5_1", flags, 0, &pPS, &pErr);
    if(pErr)
    {
        OutputDebugStringA((char*)pErr->GetBufferPointer());
        pErr->Release();
    }

    if(!pVS || !pPS)
    {
        if(pVS)
            pVS->Release();
        if(pPS)
            pPS->Release();
        return;
    }

    D3D12_INPUT_ELEMENT_DESC elems[2] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 8, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    };

    D3D12_RASTERIZER_DESC raster = {};
    raster.FillMode = D3D12_FILL_MODE_SOLID;
    raster.CullMode = D3D12_CULL_MODE_NONE;
    raster.DepthClipEnable = TRUE;

    D3D12_DEPTH_STENCIL_DESC ds = {};
    ds.DepthEnable = FALSE;
    ds.StencilEnable = FALSE;

    D3D12_BLEND_DESC blend = {};
    blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso = {};
    pso.pRootSignature = pd3dRootSignature;
    pso.VS = { pVS->GetBufferPointer(), pVS->GetBufferSize() };
    pso.PS = { pPS->GetBufferPointer(), pPS->GetBufferSize() };
    pso.InputLayout = { elems, 2 };
    pso.RasterizerState = raster;
    pso.BlendState = blend;
    pso.DepthStencilState = ds;
    pso.SampleMask = UINT_MAX;
    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    pso.NumRenderTargets = 1;
    pso.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    pso.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    pso.SampleDesc.Count = 1;

    pd3dDevice->CreateGraphicsPipelineState(&pso, __uuidof(ID3D12PipelineState), (void**)&m_pPSOLine);

    pso.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pd3dDevice->CreateGraphicsPipelineState(&pso, __uuidof(ID3D12PipelineState), (void**)&m_pPSOTriangle);

    pVS->Release();
    pPS->Release();

    {
        const XMFLOAT4 lime = { 0.2f, 1.0f, 0.3f, 1.0f };
        UIVtx crosshairVertices[4] = {
            { { -0.013f, 0.000f }, lime },
            { { +0.013f, 0.000f }, lime },
            { { 0.000f, -0.023f }, lime },
            { { 0.000f, +0.023f }, lime },
        };
        m_nCrosshairVerts = 4;
        m_pVBCrosshair = CreateStaticVB(pd3dDevice, crosshairVertices, sizeof(crosshairVertices), m_vbCrosshair, sizeof(UIVtx));
    }

    {
        std::vector<UIVtx> outlineVertices;
        AppendBoxOutline(outlineVertices, -0.95f, -0.92f, -0.55f, -0.86f, XMFLOAT4(1, 1, 1, 1));
        m_nHPOutlineVerts = (UINT)outlineVertices.size();
        m_pVBHPOutline = CreateStaticVB(pd3dDevice, outlineVertices.data(),
                                        (UINT)(outlineVertices.size() * sizeof(UIVtx)), m_vbHPOutline, sizeof(UIVtx));
    }

    {
        std::vector<UIVtx> outlineVertices;
        AppendBoxOutline(outlineVertices, -0.30f, 0.86f, 0.30f, 0.92f, XMFLOAT4(1, 1, 1, 1));
        m_nEnemyHPOutlineVerts = (UINT)outlineVertices.size();
        m_pVBEnemyHPOutline = CreateStaticVB(pd3dDevice, outlineVertices.data(),
                                             (UINT)(outlineVertices.size() * sizeof(UIVtx)), m_vbEnemyHPOutline, sizeof(UIVtx));
    }

    m_pVBHPFill = CreateDynamicVB(pd3dDevice, sizeof(UIVtx) * 6, m_vbHPFill, sizeof(UIVtx), &m_pMappedHPFill);
    m_pVBEnemyHPFill = CreateDynamicVB(pd3dDevice, sizeof(UIVtx) * 6, m_vbEnemyHPFill, sizeof(UIVtx), &m_pMappedEnemyHPFill);
}

void CUI_Manager::RenderCrosshair(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if(!m_pPSOLine)
        return;
    pd3dCommandList->SetPipelineState(m_pPSOLine);
    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbCrosshair);
    pd3dCommandList->DrawInstanced(m_nCrosshairVerts, 1, 0, 0);
}

void CUI_Manager::RenderHPBar(ID3D12GraphicsCommandList* pd3dCommandList, int currentHP, int maxHP)
{
    if(!m_pPSOTriangle || maxHP <= 0)
        return;
    if(currentHP < 0)
        currentHP = 0;
    if(currentHP > maxHP)
        currentHP = maxHP;

    float ratio = (float)currentHP / (float)maxHP;
    const float x0 = -0.95f, x1Max = -0.55f, y0 = -0.92f, y1 = -0.86f;
    const float x1 = x0 + (x1Max - x0) * ratio;

    XMFLOAT4 color;
    if(ratio > 0.5f)
        color = XMFLOAT4(0.2f, 1.0f, 0.3f, 1.0f);
    else if(ratio > 0.25f)
        color = XMFLOAT4(1.0f, 0.85f, 0.2f, 1.0f);
    else
        color = XMFLOAT4(1.0f, 0.25f, 0.25f, 1.0f);

    UIVtx* pVertices = (UIVtx*)m_pMappedHPFill;
    pVertices[0] = { { x0, y0 }, color };
    pVertices[1] = { { x1, y0 }, color };
    pVertices[2] = { { x1, y1 }, color };
    pVertices[3] = { { x0, y0 }, color };
    pVertices[4] = { { x1, y1 }, color };
    pVertices[5] = { { x0, y1 }, color };

    if(currentHP > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbHPFill);
        pd3dCommandList->DrawInstanced(6, 1, 0, 0);
    }

    pd3dCommandList->SetPipelineState(m_pPSOLine);
    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbHPOutline);
    pd3dCommandList->DrawInstanced(m_nHPOutlineVerts, 1, 0, 0);
}

void CUI_Manager::RenderEnemyHPBar(ID3D12GraphicsCommandList* pd3dCommandList, int currentHP, int maxHP)
{
    if(!m_pPSOTriangle || maxHP <= 0)
        return;
    if(currentHP < 0)
        currentHP = 0;
    if(currentHP > maxHP)
        currentHP = maxHP;

    float ratio = (float)currentHP / (float)maxHP;
    const float x0 = -0.30f, x1Max = 0.30f, y0 = 0.86f, y1 = 0.92f;
    const float x1 = x0 + (x1Max - x0) * ratio;

    // Enemy bar drains red->dark red as it depletes.
    XMFLOAT4 color = XMFLOAT4(0.85f, 0.20f, 0.20f, 1.0f);

    UIVtx* pVertices = (UIVtx*)m_pMappedEnemyHPFill;
    pVertices[0] = { { x0, y0 }, color };
    pVertices[1] = { { x1, y0 }, color };
    pVertices[2] = { { x1, y1 }, color };
    pVertices[3] = { { x0, y0 }, color };
    pVertices[4] = { { x1, y1 }, color };
    pVertices[5] = { { x0, y1 }, color };

    if(currentHP > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbEnemyHPFill);
        pd3dCommandList->DrawInstanced(6, 1, 0, 0);
    }

    pd3dCommandList->SetPipelineState(m_pPSOLine);
    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbEnemyHPOutline);
    pd3dCommandList->DrawInstanced(m_nEnemyHPOutlineVerts, 1, 0, 0);
}

void CUI_Manager::Release()
{
    if(m_pVBHPFill)
    {
        m_pVBHPFill->Unmap(0, nullptr);
        m_pVBHPFill->Release();
        m_pVBHPFill = nullptr;
        m_pMappedHPFill = nullptr;
    }
    if(m_pVBEnemyHPFill)
    {
        m_pVBEnemyHPFill->Unmap(0, nullptr);
        m_pVBEnemyHPFill->Release();
        m_pVBEnemyHPFill = nullptr;
        m_pMappedEnemyHPFill = nullptr;
    }
    if(m_pVBEnemyHPOutline)
    {
        m_pVBEnemyHPOutline->Release();
        m_pVBEnemyHPOutline = nullptr;
    }
    if(m_pVBHPOutline)
    {
        m_pVBHPOutline->Release();
        m_pVBHPOutline = nullptr;
    }
    if(m_pVBCrosshair)
    {
        m_pVBCrosshair->Release();
        m_pVBCrosshair = nullptr;
    }
    if(m_pPSOLine)
    {
        m_pPSOLine->Release();
        m_pPSOLine = nullptr;
    }
    if(m_pPSOTriangle)
    {
        m_pPSOTriangle->Release();
        m_pPSOTriangle = nullptr;
    }
}
