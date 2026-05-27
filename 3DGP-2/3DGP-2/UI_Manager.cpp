#include "pch.h"
#include "UI_Manager.h"
#include "Camera.h"
#include <D3Dcompiler.h>
#include <vector>

CUI_Manager* CUI_Manager::s_pInstance = nullptr;

CUI_Manager* CUI_Manager::Get_Instance()
{
    if (!s_pInstance) s_pInstance = new CUI_Manager;
    return s_pInstance;
}

void CUI_Manager::Destroy_Instance()
{
    if (s_pInstance) { delete s_pInstance; s_pInstance = nullptr; }
}

namespace
{
    struct UIVertexLocal
    {
        XMFLOAT2 pos;
        XMFLOAT4 col;
    };

    void AppendBoxOutline(std::vector<UIVertexLocal>& out,
                          float x0, float y0, float x1, float y1,
                          XMFLOAT4 c)
    {
        out.push_back({ { x0, y0 }, c }); out.push_back({ { x1, y0 }, c });  // bottom
        out.push_back({ { x1, y0 }, c }); out.push_back({ { x1, y1 }, c });  // right
        out.push_back({ { x1, y1 }, c }); out.push_back({ { x0, y1 }, c });  // top
        out.push_back({ { x0, y1 }, c }); out.push_back({ { x0, y0 }, c });  // left
    }

    void AppendBoxDiagonals(std::vector<UIVertexLocal>& out,
                            float x0, float y0, float x1, float y1,
                            XMFLOAT4 c)
    {
        out.push_back({ { x0, y0 }, c }); out.push_back({ { x1, y1 }, c });
        out.push_back({ { x0, y1 }, c }); out.push_back({ { x1, y0 }, c });
    }

    void AppendFilledQuad(std::vector<UIVertexLocal>& out,
                          float x0, float y0, float x1, float y1,
                          XMFLOAT4 c)
    {
        out.push_back({ { x0, y0 }, c });
        out.push_back({ { x1, y0 }, c });
        out.push_back({ { x1, y1 }, c });
        out.push_back({ { x0, y0 }, c });
        out.push_back({ { x1, y1 }, c });
        out.push_back({ { x0, y1 }, c });
    }


    void AppendDigitOne(std::vector<UIVertexLocal>& out,
                        float cx, float cy, float halfH, XMFLOAT4 c)
    {
        float hw = halfH * 0.45f;
        // flag (top-left diagonal -> top of stem)
        out.push_back({ { cx - hw,        cy + halfH * 0.55f }, c });
        out.push_back({ { cx,             cy + halfH         }, c });
        // stem
        out.push_back({ { cx,             cy + halfH         }, c });
        out.push_back({ { cx,             cy - halfH         }, c });
        // base bar
        out.push_back({ { cx - hw,        cy - halfH         }, c });
        out.push_back({ { cx + hw,        cy - halfH         }, c });
    }

    void AppendDigitTwo(std::vector<UIVertexLocal>& out,
                        float cx, float cy, float halfH, XMFLOAT4 c)
    {
        float hw = halfH * 0.45f;
        // top bar
        out.push_back({ { cx - hw, cy + halfH }, c });
        out.push_back({ { cx + hw, cy + halfH }, c });
        // right shoulder (top to mid)
        out.push_back({ { cx + hw, cy + halfH }, c });
        out.push_back({ { cx + hw, cy         }, c });
        // diagonal (mid-right -> bottom-left)
        out.push_back({ { cx + hw, cy         }, c });
        out.push_back({ { cx - hw, cy - halfH }, c });
        // bottom bar
        out.push_back({ { cx - hw, cy - halfH }, c });
        out.push_back({ { cx + hw, cy - halfH }, c });
    }

    void AppendCheckMark(std::vector<UIVertexLocal>& out,
                         float cx, float cy, float size, XMFLOAT4 c)
    {
        float w = size;
        float h = size * 0.75f;
        // top-left -> bottom-mid
        out.push_back({ { cx - w,        cy + h * 0.1f }, c });
        out.push_back({ { cx - w * 0.2f, cy - h        }, c });
        // bottom-mid -> top-right
        out.push_back({ { cx - w * 0.2f, cy - h        }, c });
        out.push_back({ { cx + w,        cy + h        }, c });
    }

    void AppendX(std::vector<UIVertexLocal>& out,
                 float cx, float cy, float halfSize, XMFLOAT4 c)
    {
        // top-left -> bottom-right
        out.push_back({ { cx - halfSize, cy + halfSize }, c });
        out.push_back({ { cx + halfSize, cy - halfSize }, c });
        // bottom-left -> top-right
        out.push_back({ { cx - halfSize, cy - halfSize }, c });
        out.push_back({ { cx + halfSize, cy + halfSize }, c });
    }

    void AppendUnderscore(std::vector<UIVertexLocal>& out,
                          float cx, float cy, float halfW, XMFLOAT4 c)
    {
        out.push_back({ { cx - halfW, cy }, c });
        out.push_back({ { cx + halfW, cy }, c });
    }

    ID3D12Resource* CreateStaticUploadVB(ID3D12Device* pd3dDevice,
                                         const void* pData, UINT nBytes,
                                         D3D12_VERTEX_BUFFER_VIEW& outView,
                                         UINT stride)
    {
        D3D12_HEAP_PROPERTIES heapUp = {};
        heapUp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Width            = nBytes;
        resDesc.Height           = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels        = 1;
        resDesc.Format           = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        ID3D12Resource* pBuf = nullptr;
        pd3dDevice->CreateCommittedResource(
            &heapUp, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            __uuidof(ID3D12Resource), (void**)&pBuf);

        void* pMapped = nullptr;
        pBuf->Map(0, nullptr, &pMapped);
        memcpy(pMapped, pData, nBytes);
        pBuf->Unmap(0, nullptr);

        outView.BufferLocation = pBuf->GetGPUVirtualAddress();
        outView.SizeInBytes    = nBytes;
        outView.StrideInBytes  = stride;
        return pBuf;
    }
}

void CUI_Manager::Init(ID3D12Device* pd3dDevice,
                       ID3D12GraphicsCommandList* pd3dCommandList,
                       ID3D12RootSignature* pd3dRootSignature)
{
    // -- Compile UI shaders --
    UINT flags = 0;
#ifdef _DEBUG
    flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
    ID3DBlob* pVS = nullptr, * pPS = nullptr, * pErr = nullptr;

    D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr,
                       "VSUIMain", "vs_5_1", flags, 0, &pVS, &pErr);
    if (pErr) { OutputDebugStringA((char*)pErr->GetBufferPointer()); pErr->Release(); pErr = nullptr; }

    D3DCompileFromFile(L"Shaders.hlsl", nullptr, nullptr,
                       "PSUIMain", "ps_5_1", flags, 0, &pPS, &pErr);
    if (pErr) { OutputDebugStringA((char*)pErr->GetBufferPointer()); pErr->Release(); }

    D3D12_INPUT_ELEMENT_DESC inputElems[2] = {};
    inputElems[0] = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0,  0,
                      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
    inputElems[1] = { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0,  8,
                      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

    D3D12_RASTERIZER_DESC raster = {};
    raster.FillMode        = D3D12_FILL_MODE_SOLID;
    raster.CullMode        = D3D12_CULL_MODE_NONE;
    raster.DepthClipEnable = TRUE;

    D3D12_DEPTH_STENCIL_DESC ds = {};
    ds.DepthEnable   = FALSE;
    ds.StencilEnable = FALSE;

    D3D12_BLEND_DESC blend = {};
    blend.RenderTarget[0].BlendEnable           = FALSE;
    blend.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

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
        __uuidof(ID3D12PipelineState), (void**)&m_pPSOLine);

    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pd3dDevice->CreateGraphicsPipelineState(&psoDesc,
        __uuidof(ID3D12PipelineState), (void**)&m_pPSOTriangle);

    pVS->Release();
    pPS->Release();


    {
        const XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
        UIVertexLocal verts[4] =
        {
            { { -0.012f,  0.000f }, white },
            { { +0.012f,  0.000f }, white },
            { {  0.000f, -0.020f }, white },
            { {  0.000f, +0.020f }, white },
        };
        m_nCrosshairVerts = 4;
        m_pVBCrosshair = CreateStaticUploadVB(pd3dDevice, verts,
                                              sizeof(verts), m_vbViewCrosshair,
                                              sizeof(UIVertexLocal));
    }

    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 darkGreen = { 0.10f, 0.40f, 0.15f, 1.0f };
        const XMFLOAT4 darkBlue  = { 0.10f, 0.20f, 0.50f, 1.0f };

        AppendFilledQuad(v, -0.70f, -0.50f, -0.10f, +0.50f, darkGreen);  // Stage 1
        AppendFilledQuad(v, +0.10f, -0.50f, +0.70f, +0.50f, darkBlue);   // Stage 2

        m_nMenuFillVerts = (UINT)v.size();
        m_pVBMenuFill = CreateStaticUploadVB(pd3dDevice, v.data(),
                                             (UINT)(v.size() * sizeof(UIVertexLocal)),
                                             m_vbViewMenuFill, sizeof(UIVertexLocal));
    }

    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 green = { 0.4f, 1.0f, 0.5f, 1.0f };
        const XMFLOAT4 blue  = { 0.5f, 0.7f, 1.0f, 1.0f };
        const XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };

        AppendBoxOutline(v, -0.70f, -0.50f, -0.10f, +0.50f, green);
        AppendBoxOutline(v, +0.10f, -0.50f, +0.70f, +0.50f, blue);

        AppendDigitOne(v, -0.40f, 0.0f, 0.30f, white);   // centered in Stage 1
        AppendDigitTwo(v, +0.40f, 0.0f, 0.30f, white);   // centered in Stage 2

        m_nMenuLineVerts = (UINT)v.size();
        m_pVBMenuLine = CreateStaticUploadVB(pd3dDevice, v.data(),
                                             (UINT)(v.size() * sizeof(UIVertexLocal)),
                                             m_vbViewMenuLine, sizeof(UIVertexLocal));
    }


    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 darkYellow = { 0.55f, 0.45f, 0.10f, 1.0f };
        AppendFilledQuad(v, -0.55f, -0.30f, +0.55f, +0.30f, darkYellow);

        m_nClearFillVerts = (UINT)v.size();
        m_pVBClearFill = CreateStaticUploadVB(pd3dDevice, v.data(),
                                              (UINT)(v.size() * sizeof(UIVertexLocal)),
                                              m_vbViewClearFill, sizeof(UIVertexLocal));
    }


    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 yellow = { 1.0f, 0.95f, 0.2f, 1.0f };
        const XMFLOAT4 white  = { 1.0f, 1.0f,  1.0f, 1.0f };

        AppendBoxOutline(v, -0.55f, -0.30f, +0.55f, +0.30f, yellow);
        AppendCheckMark (v, 0.0f, 0.0f, 0.20f, white);

        m_nClearLineVerts = (UINT)v.size();
        m_pVBClearLine = CreateStaticUploadVB(pd3dDevice, v.data(),
                                              (UINT)(v.size() * sizeof(UIVertexLocal)),
                                              m_vbViewClearLine, sizeof(UIVertexLocal));
    }


    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 darkRed = { 0.55f, 0.10f, 0.10f, 1.0f };
        AppendFilledQuad(v, -0.55f, -0.30f, +0.55f, +0.30f, darkRed);

        m_nGameOverFillVerts = (UINT)v.size();
        m_pVBGameOverFill = CreateStaticUploadVB(pd3dDevice, v.data(),
                                                 (UINT)(v.size() * sizeof(UIVertexLocal)),
                                                 m_vbViewGameOverFill, sizeof(UIVertexLocal));
    }


    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 brightRed = { 1.0f, 0.30f, 0.30f, 1.0f };
        const XMFLOAT4 white     = { 1.0f, 1.0f,  1.0f,  1.0f };

        AppendBoxOutline(v, -0.55f, -0.30f, +0.55f, +0.30f, brightRed);

        const float xHalf = 0.10f;

        AppendX(v, -0.22f, +0.02f, xHalf, white);
        AppendX(v, +0.22f, +0.02f, xHalf, white);
        AppendUnderscore(v, 0.0f, -0.10f, 0.06f, white);

        m_nGameOverLineVerts = (UINT)v.size();
        m_pVBGameOverLine = CreateStaticUploadVB(pd3dDevice, v.data(),
                                                 (UINT)(v.size() * sizeof(UIVertexLocal)),
                                                 m_vbViewGameOverLine, sizeof(UIVertexLocal));
    }


    {
        std::vector<UIVertexLocal> v;
        const XMFLOAT4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
        // NDC rect: x in [-0.95, -0.55], y in [-0.92, -0.86]
        AppendBoxOutline(v, -0.95f, -0.92f, -0.55f, -0.86f, white);
        m_nHPOutlineVerts = (UINT)v.size();
        m_pVBHPOutline = CreateStaticUploadVB(pd3dDevice, v.data(),
                                              (UINT)(v.size() * sizeof(UIVertexLocal)),
                                              m_vbViewHPOutline, sizeof(UIVertexLocal));
    }

    {
        UINT bytes = sizeof(UIVertexLocal) * 6;

        D3D12_HEAP_PROPERTIES heapUp = {};
        heapUp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Width            = bytes;
        resDesc.Height           = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels        = 1;
        resDesc.Format           = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        pd3dDevice->CreateCommittedResource(
            &heapUp, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            __uuidof(ID3D12Resource), (void**)&m_pVBHPFill);

        m_pVBHPFill->Map(0, nullptr, &m_pMappedHPFill);

        m_vbViewHPFill.BufferLocation = m_pVBHPFill->GetGPUVirtualAddress();
        m_vbViewHPFill.SizeInBytes    = bytes;
        m_vbViewHPFill.StrideInBytes  = sizeof(UIVertexLocal);
    }


    {
        D3D12_HEAP_PROPERTIES heapUp = {};
        heapUp.Type = D3D12_HEAP_TYPE_UPLOAD;

        D3D12_RESOURCE_DESC resDesc = {};
        resDesc.Dimension        = D3D12_RESOURCE_DIMENSION_BUFFER;
        resDesc.Width            = 256;
        resDesc.Height           = 1;
        resDesc.DepthOrArraySize = 1;
        resDesc.MipLevels        = 1;
        resDesc.Format           = DXGI_FORMAT_UNKNOWN;
        resDesc.SampleDesc.Count = 1;
        resDesc.Layout           = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

        pd3dDevice->CreateCommittedResource(
            &heapUp, D3D12_HEAP_FLAG_NONE, &resDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            __uuidof(ID3D12Resource), (void**)&m_pCBDummy);
    }
}


void CUI_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_pPSOLine || !pCamera) return;
    if (pCamera->GetMode() != FIRST_PERSON_CAMERA) return;

    pd3dCommandList->SetPipelineState(m_pPSOLine);
    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewCrosshair);
    pd3dCommandList->DrawInstanced(m_nCrosshairVerts, 1, 0, 0);
}

void CUI_Manager::RenderMenu(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (!m_pPSOLine || !m_pPSOTriangle) return;

    if (m_pCBDummy)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddr = m_pCBDummy->GetGPUVirtualAddress();
        pd3dCommandList->SetGraphicsRootConstantBufferView(0, gpuAddr);
        pd3dCommandList->SetGraphicsRootConstantBufferView(1, gpuAddr);
    }

    if (m_nMenuFillVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewMenuFill);
        pd3dCommandList->DrawInstanced(m_nMenuFillVerts, 1, 0, 0);
    }

    if (m_nMenuLineVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOLine);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewMenuLine);
        pd3dCommandList->DrawInstanced(m_nMenuLineVerts, 1, 0, 0);
    }
}

void CUI_Manager::RenderClearOverlay(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (!m_pPSOLine || !m_pPSOTriangle) return;

    if (m_nClearFillVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewClearFill);
        pd3dCommandList->DrawInstanced(m_nClearFillVerts, 1, 0, 0);
    }

    if (m_nClearLineVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOLine);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewClearLine);
        pd3dCommandList->DrawInstanced(m_nClearLineVerts, 1, 0, 0);
    }
}

void CUI_Manager::RenderGameOverOverlay(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (!m_pPSOLine || !m_pPSOTriangle) return;

    if (m_nGameOverFillVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewGameOverFill);
        pd3dCommandList->DrawInstanced(m_nGameOverFillVerts, 1, 0, 0);
    }

    if (m_nGameOverLineVerts > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOLine);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewGameOverLine);
        pd3dCommandList->DrawInstanced(m_nGameOverLineVerts, 1, 0, 0);
    }
}

void CUI_Manager::RenderHPBar(ID3D12GraphicsCommandList* pd3dCommandList,
                              int currentHP, int maxHP)
{
    if (!m_pPSOLine || !m_pPSOTriangle) return;
    if (maxHP <= 0) return;
    if (currentHP < 0)     currentHP = 0;
    if (currentHP > maxHP) currentHP = maxHP;

    float ratio = (float)currentHP / (float)maxHP;

    const float x0    = -0.95f;
    const float x1Max = -0.55f;
    const float y0    = -0.92f;
    const float y1    = -0.86f;
    const float x1    = x0 + (x1Max - x0) * ratio;

    XMFLOAT4 col;
    if (ratio > 0.5f) col = { 0.2f, 1.0f, 0.3f, 1.0f };
    else if (ratio > 0.25f) col = { 1.0f, 0.85f, 0.2f, 1.0f };
    else col = { 1.0f, 0.25f, 0.25f, 1.0f };


    UIVertexLocal* p = (UIVertexLocal*)m_pMappedHPFill;
    p[0] = { { x0, y0 }, col };
    p[1] = { { x1, y0 }, col };
    p[2] = { { x1, y1 }, col };
    p[3] = { { x0, y0 }, col };
    p[4] = { { x1, y1 }, col };
    p[5] = { { x0, y1 }, col };

    if (m_pCBDummy)
    {
        D3D12_GPU_VIRTUAL_ADDRESS gpuAddr = m_pCBDummy->GetGPUVirtualAddress();
        pd3dCommandList->SetGraphicsRootConstantBufferView(0, gpuAddr);
        pd3dCommandList->SetGraphicsRootConstantBufferView(1, gpuAddr);
    }

    if (currentHP > 0)
    {
        pd3dCommandList->SetPipelineState(m_pPSOTriangle);
        pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewHPFill);
        pd3dCommandList->DrawInstanced(6, 1, 0, 0);
    }

    pd3dCommandList->SetPipelineState(m_pPSOLine);
    pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
    pd3dCommandList->IASetVertexBuffers(0, 1, &m_vbViewHPOutline);
    pd3dCommandList->DrawInstanced(m_nHPOutlineVerts, 1, 0, 0);
}

void CUI_Manager::Release()
{
    if (m_pVBHPFill)
    {
        m_pVBHPFill->Unmap(0, nullptr);
        m_pVBHPFill->Release();
        m_pVBHPFill     = nullptr;
        m_pMappedHPFill = nullptr;
    }
    if (m_pVBHPOutline)  { m_pVBHPOutline->Release();  m_pVBHPOutline  = nullptr; }
    if (m_pVBCrosshair)  { m_pVBCrosshair->Release();  m_pVBCrosshair  = nullptr; }
    if (m_pVBMenuFill)   { m_pVBMenuFill->Release();   m_pVBMenuFill   = nullptr; }
    if (m_pVBMenuLine)   { m_pVBMenuLine->Release();   m_pVBMenuLine   = nullptr; }
    if (m_pVBClearFill)    { m_pVBClearFill->Release();    m_pVBClearFill    = nullptr; }
    if (m_pVBClearLine)    { m_pVBClearLine->Release();    m_pVBClearLine    = nullptr; }
    if (m_pVBGameOverFill) { m_pVBGameOverFill->Release(); m_pVBGameOverFill = nullptr; }
    if (m_pVBGameOverLine) { m_pVBGameOverLine->Release(); m_pVBGameOverLine = nullptr; }
    if (m_pCBDummy)      { m_pCBDummy->Release();      m_pCBDummy      = nullptr; }
    if (m_pPSOLine)      { m_pPSOLine->Release();      m_pPSOLine      = nullptr; }
    if (m_pPSOTriangle)  { m_pPSOTriangle->Release();  m_pPSOTriangle  = nullptr; }
}
