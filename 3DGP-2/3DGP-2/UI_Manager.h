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

    // Crosshair is only drawn when pCamera is in FIRST_PERSON_CAMERA mode.
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

    // Menu screen overlay: two stage panels (Stage 1 left = green, Stage 2 right = blue).
    void RenderMenu(ID3D12GraphicsCommandList* pd3dCommandList);

    // Big centered box flashed on stage clear.
    void RenderClearOverlay(ID3D12GraphicsCommandList* pd3dCommandList);

    // Red overlay shown when the player dies (face = X_X).
    void RenderGameOverOverlay(ID3D12GraphicsCommandList* pd3dCommandList);

    // HP bar in the lower-left corner. Updates the fill quad every frame.
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
        XMFLOAT2 pos;   // NDC space (-1..+1)
        XMFLOAT4 col;
    };

    static CUI_Manager* s_pInstance;

    // Two PSOs sharing the same VS/PS/root-sig; only PrimitiveTopologyType differs.
    ID3D12PipelineState*     m_pPSOLine        = nullptr;
    ID3D12PipelineState*     m_pPSOTriangle    = nullptr;

    // Crosshair
    ID3D12Resource*          m_pVBCrosshair    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewCrosshair = {};
    UINT                     m_nCrosshairVerts = 0;

    // Menu — filled panels (TRIANGLE) + outlines and digit glyphs (LINE)
    ID3D12Resource*          m_pVBMenuFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewMenuFill = {};
    UINT                     m_nMenuFillVerts = 0;

    ID3D12Resource*          m_pVBMenuLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewMenuLine = {};
    UINT                     m_nMenuLineVerts = 0;

    // Clear overlay — filled box (TRIANGLE) + outline and check mark (LINE)
    ID3D12Resource*          m_pVBClearFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewClearFill = {};
    UINT                     m_nClearFillVerts = 0;

    ID3D12Resource*          m_pVBClearLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewClearLine = {};
    UINT                     m_nClearLineVerts = 0;

    // Game-over overlay — filled red box + outline and X_X glyph
    ID3D12Resource*          m_pVBGameOverFill    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewGameOverFill = {};
    UINT                     m_nGameOverFillVerts = 0;

    ID3D12Resource*          m_pVBGameOverLine    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewGameOverLine = {};
    UINT                     m_nGameOverLineVerts = 0;

    // Dummy constant buffer for b0/b1. UI shader doesn't read these but the
    // root signature requires both slots to be bound to a valid resource —
    // needed when drawing from levels without a camera (e.g. the menu).
    ID3D12Resource*          m_pCBDummy    = nullptr;

    // HP bar: static outline (LINE) + dynamic fill quad (TRIANGLE)
    ID3D12Resource*          m_pVBHPOutline    = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewHPOutline = {};
    UINT                     m_nHPOutlineVerts = 0;

    ID3D12Resource*          m_pVBHPFill       = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbViewHPFill    = {};
    void*                    m_pMappedHPFill   = nullptr;  // persistently mapped
};
