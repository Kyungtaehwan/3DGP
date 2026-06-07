#pragma once
#include "GameObject.h"
#include <vector>

class CShader;


class CExplosionEffect : public CGameObject
{
public:
    CExplosionEffect() = default;
    virtual ~CExplosionEffect();

    static void PrepareShared(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
    static void ReleaseShared();
    static void ReleaseSharedUploadBuffers();

    void Initialize(ID3D12Device* pd3dDevice, CShader* pShader, XMFLOAT3 center, float fWorldScale = 1.0f);

    virtual int Update(float dt) override;
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

private:
    static constexpr int DEBRIS_CNT = 28;
    static constexpr float BLOW_DUR = 0.8f; 
    static constexpr float DEBRIS_SPD = 1.3f;

    struct Debris
    {
        CGameObject* pObj = nullptr;
        XMFLOAT3 dir = {};
        XMFLOAT3 spin = {};
    };

    std::vector<Debris> m_Debris;
    XMFLOAT3 m_Center = { 0.0f, 0.0f, 0.0f };
    float m_fTimer = 0.0f;
    float m_fScale = 1.0f;

    static CMesh* s_pDebrisMesh;
};
