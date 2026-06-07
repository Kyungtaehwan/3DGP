#pragma once

class CUI_Manager
{
public:
    static CUI_Manager* Get_Instance();
    static void Destroy_Instance();

    void Initialize(ID3D12Device* pd3dDevice,
                    ID3D12GraphicsCommandList* pd3dCommandList,
                    ID3D12RootSignature* pd3dRootSignature);

    void RenderCrosshair(ID3D12GraphicsCommandList* pd3dCommandList);
    void RenderHPBar(ID3D12GraphicsCommandList* pd3dCommandList, int currentHP, int maxHP);
    void RenderEnemyHPBar(ID3D12GraphicsCommandList* pd3dCommandList, int currentHP, int maxHP);

    void Release();

private:
    CUI_Manager() = default;
    ~CUI_Manager() { Release(); }
    CUI_Manager(const CUI_Manager&) = delete;
    CUI_Manager& operator=(const CUI_Manager&) = delete;

    struct UIVertex
    {
        XMFLOAT2 pos;
        XMFLOAT4 col;
    };

    static CUI_Manager* s_pInstance;

    ID3D12PipelineState* m_pPSOLine = nullptr; // LINELIST topology
    ID3D12PipelineState* m_pPSOTriangle = nullptr; // TRIANGLELIST topology

    // Crosshair
    ID3D12Resource* m_pVBCrosshair = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbCrosshair = {};
    UINT m_nCrosshairVerts = 0;

    // HP bar
    ID3D12Resource* m_pVBHPOutline = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbHPOutline = {};
    UINT m_nHPOutlineVerts = 0;

    ID3D12Resource* m_pVBHPFill = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbHPFill = {};
    void* m_pMappedHPFill = nullptr;

    // Enemy HP bar
    ID3D12Resource* m_pVBEnemyHPOutline = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbEnemyHPOutline = {};
    UINT m_nEnemyHPOutlineVerts = 0;

    ID3D12Resource* m_pVBEnemyHPFill = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbEnemyHPFill = {};
    void* m_pMappedEnemyHPFill = nullptr;
};
