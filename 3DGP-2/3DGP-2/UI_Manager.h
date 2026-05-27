#pragma once

class CCamera;

class CUI_Manager
{
public:
    static CUI_Manager* Get_Instance();
    static void         Destroy_Instance();

    void Init(ID3D12Device* pd3dDevice,
              ID3D12GraphicsCommandList* pd3dCommandList,
              ID3D12RootSignature* pd3dRootSignature);

    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void RenderMenu(ID3D12GraphicsCommandList* pd3dCommandList);
    void RenderClearOverlay(ID3D12GraphicsCommandList* pd3dCommandList);
    void RenderGameOverOverlay(ID3D12GraphicsCommandList* pd3dCommandList);
    void RenderHPBar(ID3D12GraphicsCommandList* pd3dCommandList,
                     int currentHP, int maxHP);

    void Release();

private:
    CUI_Manager()  = default;
    ~CUI_Manager() { Release(); }
    CUI_Manager(const CUI_Manager&)            = delete;
    CUI_Manager& operator=(const CUI_Manager&) = delete;

    struct UIVertex
    {
        XMFLOAT2 pos;   // NDC
        XMFLOAT4 col;
    };

    static CUI_Manager* s_pInstance;

    //PSO
    ID3D12PipelineState*     m_pPSOLine        = nullptr;
    ID3D12PipelineState*     m_pPSOTriangle    = nullptr;

    // Crosshair
    ID3D12Resource*          m_pVBCrosshair    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewCrosshair = {};
    UINT                     m_nCrosshairVerts = 0;

    ID3D12Resource*          m_pVBMenuFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewMenuFill = {};
    UINT                     m_nMenuFillVerts = 0;

    ID3D12Resource*          m_pVBMenuLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewMenuLine = {};
    UINT                     m_nMenuLineVerts = 0;

    ID3D12Resource*          m_pVBClearFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewClearFill = {};
    UINT                     m_nClearFillVerts = 0;

    ID3D12Resource*          m_pVBClearLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewClearLine = {};
    UINT                     m_nClearLineVerts = 0;

    ID3D12Resource*          m_pVBGameOverFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewGameOverFill = {};
    UINT                     m_nGameOverFillVerts = 0;

    ID3D12Resource*          m_pVBGameOverLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewGameOverLine = {};
    UINT                     m_nGameOverLineVerts = 0;


    ID3D12Resource*          m_pCBDummy    = nullptr;

    ID3D12Resource*          m_pVBHPOutline    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewHPOutline = {};
    UINT                     m_nHPOutlineVerts = 0;

    ID3D12Resource*          m_pVBHPFill       = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewHPFill    = {};
    void*                    m_pMappedHPFill   = nullptr; 
};
