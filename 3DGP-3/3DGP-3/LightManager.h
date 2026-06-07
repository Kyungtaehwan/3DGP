#pragma once

#define MAX_LIGHTS 16

#define POINT_LIGHT 1
#define SPOT_LIGHT 2
#define DIRECTIONAL_LIGHT 3

struct LIGHT
{
    XMFLOAT4 m_xmf4Ambient;
    XMFLOAT4 m_xmf4Diffuse;
    XMFLOAT4 m_xmf4Specular;
    XMFLOAT3 m_xmf3Position;
    float m_fFalloff;
    XMFLOAT3 m_xmf3Direction;
    float m_fTheta; // cos(theta)
    XMFLOAT3 m_xmf3Attenuation;
    float m_fPhi; // cos(phi)
    bool m_bEnable;
    int m_nType;
    float m_fRange;
    float padding;
};

struct LIGHTS
{
    LIGHT m_pLights[MAX_LIGHTS];
    XMFLOAT4 m_xmf4GlobalAmbient;
    int m_nLights;
};

class CLightManager
{
public:
    CLightManager();
    ~CLightManager();

    void CreateShaderVariables(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList);
    void ReleaseShaderVariables();

    void BuildDefaultLights();
    void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    int m_nLights = 0;

    LIGHT* m_pLights = NULL;
    XMFLOAT4 m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

private:
    ID3D12Resource* m_pd3dcbLights = NULL;
    LIGHTS* m_pcbMappedLights = NULL;
};
