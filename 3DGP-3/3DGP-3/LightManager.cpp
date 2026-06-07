#include "pch.h"
#include "LightManager.h"

CLightManager::CLightManager() {}
CLightManager::~CLightManager()
{
    ReleaseShaderVariables();
    if(m_pLights)
        delete[] m_pLights;
}

void CLightManager::BuildDefaultLights()
{
    m_nLights = 1;
    m_pLights = new LIGHT[m_nLights];
    ::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

    m_xmf4GlobalAmbient = XMFLOAT4(0.30f, 0.30f, 0.30f, 1.0f);

    // sun
    m_pLights[0].m_bEnable = true;
    m_pLights[0].m_nType = DIRECTIONAL_LIGHT;
    m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
    m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.9f, 0.9f, 0.85f, 1.0f);
    m_pLights[0].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
    m_pLights[0].m_xmf3Direction = XMFLOAT3(-0.505f, -0.808f, -0.303f);
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
    if(!m_pcbMappedLights)
        return;

    ::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
    m_pcbMappedLights->m_xmf4GlobalAmbient = m_xmf4GlobalAmbient;
    m_pcbMappedLights->m_nLights = m_nLights;

    //root[2] = b4.
    pd3dCommandList->SetGraphicsRootConstantBufferView(
        ROOT_SLOT_LIGHTS, m_pd3dcbLights->GetGPUVirtualAddress());
}

void CLightManager::ReleaseShaderVariables()
{
    if(m_pd3dcbLights)
    {
        m_pd3dcbLights->Unmap(0, NULL);
        m_pd3dcbLights->Release();
        m_pd3dcbLights = NULL;
        m_pcbMappedLights = NULL;
    }
}
