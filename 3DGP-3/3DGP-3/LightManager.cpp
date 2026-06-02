#include "pch.h"
#include "LightManager.h"

CLightManager::CLightManager() {}
CLightManager::~CLightManager()
{
    ReleaseShaderVariables();
    if (m_pLights) delete[] m_pLights;
}

void CLightManager::BuildDefaultLights()
{
    m_nLights = 4;
    m_pLights = new LIGHT[m_nLights];
    ::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

    m_xmf4GlobalAmbient = XMFLOAT4(0.30f, 0.30f, 0.30f, 1.0f);

    // 0: red point light — decorative leftover from the heli demo; sits at the
    //    terrain's start corner and casts a red/purple blob, so keep it off.
    m_pLights[0].m_bEnable        = false;
    m_pLights[0].m_nType          = POINT_LIGHT;
    m_pLights[0].m_fRange         = 1000.0f;
    m_pLights[0].m_xmf4Ambient    = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
    m_pLights[0].m_xmf4Diffuse    = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
    m_pLights[0].m_xmf4Specular   = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
    m_pLights[0].m_xmf3Position   = XMFLOAT3(30.0f, 30.0f, 30.0f);
    m_pLights[0].m_xmf3Direction  = XMFLOAT3(0.0f, 0.0f, 0.0f);
    m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);

    // 1: white spot light (driven by player position/look in Level_GamePlay::Update)
    m_pLights[1].m_bEnable        = true;
    m_pLights[1].m_nType          = SPOT_LIGHT;
    m_pLights[1].m_fRange         = 500.0f;
    m_pLights[1].m_xmf4Ambient    = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
    m_pLights[1].m_xmf4Diffuse    = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    m_pLights[1].m_xmf4Specular   = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
    m_pLights[1].m_xmf3Position   = XMFLOAT3(-50.0f, 20.0f, -5.0f);
    m_pLights[1].m_xmf3Direction  = XMFLOAT3(0.0f, 0.0f, 1.0f);
    m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
    m_pLights[1].m_fFalloff       = 8.0f;
    m_pLights[1].m_fPhi           = (float)cos(XMConvertToRadians(40.0f));
    m_pLights[1].m_fTheta         = (float)cos(XMConvertToRadians(20.0f));

    // 2: directional sun — comes from above at an angle so it actually lights
    //    the upward-facing terrain (a horizontal direction would leave it dark).
    m_pLights[2].m_bEnable        = true;
    m_pLights[2].m_nType          = DIRECTIONAL_LIGHT;
    m_pLights[2].m_xmf4Ambient    = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_pLights[2].m_xmf4Diffuse    = XMFLOAT4(0.9f, 0.9f, 0.85f, 1.0f);
    m_pLights[2].m_xmf4Specular   = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
    m_pLights[2].m_xmf3Direction  = XMFLOAT3(-0.505f, -0.808f, -0.303f); // ~down, angled


    // 3: green spot light — also a decorative leftover at the start corner; off.
    m_pLights[3].m_bEnable        = false;
    m_pLights[3].m_nType          = SPOT_LIGHT;
    m_pLights[3].m_fRange         = 600.0f;
    m_pLights[3].m_xmf4Ambient    = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_pLights[3].m_xmf4Diffuse    = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
    m_pLights[3].m_xmf4Specular   = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
    m_pLights[3].m_xmf3Position   = XMFLOAT3(50.0f, 30.0f, 30.0f);
    m_pLights[3].m_xmf3Direction  = XMFLOAT3(0.0f, 1.0f, 1.0f);
    m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
    m_pLights[3].m_fFalloff       = 8.0f;
    m_pLights[3].m_fPhi           = (float)cos(XMConvertToRadians(90.0f));
    m_pLights[3].m_fTheta         = (float)cos(XMConvertToRadians(30.0f));
}

void CLightManager::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                          ID3D12GraphicsCommandList* pd3dCommandList)
{
    UINT cbSize = (sizeof(LIGHTS) + 255) & ~255;

    m_pd3dcbLights = ::CreateBufferResource(
        pd3dDevice, pd3dCommandList,
        NULL, cbSize,
        D3D12_HEAP_TYPE_UPLOAD,
        D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        NULL);

    m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

void CLightManager::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (!m_pcbMappedLights) return;

    ::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
    m_pcbMappedLights->m_xmf4GlobalAmbient = m_xmf4GlobalAmbient;
    m_pcbMappedLights->m_nLights           = m_nLights;

    // Bind at root[2] = b4.
    pd3dCommandList->SetGraphicsRootConstantBufferView(
        ROOT_SLOT_LIGHTS, m_pd3dcbLights->GetGPUVirtualAddress());
}

void CLightManager::ReleaseShaderVariables()
{
    if (m_pd3dcbLights)
    {
        m_pd3dcbLights->Unmap(0, NULL);
        m_pd3dcbLights->Release();
        m_pd3dcbLights    = NULL;
        m_pcbMappedLights = NULL;
    }
}

void CLightManager::SetSpotLight(int nIndex, const XMFLOAT3& pos, const XMFLOAT3& dir)
{
    if (nIndex < 0 || nIndex >= m_nLights) return;
    m_pLights[nIndex].m_xmf3Position  = pos;
    m_pLights[nIndex].m_xmf3Direction = dir;
}
