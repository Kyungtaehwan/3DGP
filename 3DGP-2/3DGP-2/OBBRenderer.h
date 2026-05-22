#pragma once
#include "Mesh.h"

class COBBRenderer
{
public:
    static COBBRenderer* Get_Instance();
    static void          Destroy_Instance();

    void Init(ID3D12Device* pd3dDevice,
              ID3D12GraphicsCommandList* pd3dCommandList,
              ID3D12RootSignature* pd3dRootSignature);

    void BeginFrame();
    void AddOBB(const BoundingOrientedBox& obb,
                XMFLOAT4 color = { 0.0f, 1.0f, 0.0f, 1.0f });
    void Render(ID3D12GraphicsCommandList* pd3dCommandList);
    void Release();

    static bool s_bShow;

private:
    COBBRenderer()  = default;
    ~COBBRenderer() { Release(); }
    COBBRenderer(const COBBRenderer&)            = delete;
    COBBRenderer& operator=(const COBBRenderer&) = delete;

    static COBBRenderer* s_pInstance;

    ID3D12PipelineState* m_pPSO = nullptr;

    static constexpr UINT MAX_VERTICES = 24 * 256; // 256 OBBs max
    ID3D12Resource*          m_pVB       = nullptr;
    CVertex*                 m_pMappedVB = nullptr;
    D3D12_VERTEX_BUFFER_VIEW m_vbView    = {};
    UINT                     m_nVerts    = 0;

    // Identity world matrix constant buffer bound to b1
    ID3D12Resource* m_pCBWorld       = nullptr;
    void*           m_pMappedCBWorld = nullptr;
};
