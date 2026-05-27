#pragma once
#include "GameObject.h"
#include "Map_Manager.h"

enum class EnemyState { IDLE, CHASE };

class CEnemy : public CGameObject
{
public:
    CEnemy()  = default;
    ~CEnemy() { Release(); }

    void Init(ID3D12Device* pd3dDevice,
              ID3D12GraphicsCommandList* pd3dCommandList,
              CShader* pShader,
              XMFLOAT3 spawnPos);

    virtual int  Update(float dt) override;
    virtual void OnHit(int nDamage) override;
    virtual void Release() override;
    virtual void ReleaseUploadBuffers() override;

private:
    EnemyState m_eState          = EnemyState::IDLE;
    float      m_fSpeed          = 3.0f;
    float      m_fDetectRange    = 12.0f;
    float      m_fAttackRange    = 6.0f;
    float      m_fAttackCooldown = 1.5f;   
    float      m_fAttackTimer    = 0.0f;   


    ID3D12Device* m_pd3dDevice   = nullptr;

    static constexpr float RADIUS         = 0.6f;
    static constexpr float BULLET_SPEED   = 8.0f;   
    static constexpr float BULLET_SPAWN_Y = 0.7f;   
};
