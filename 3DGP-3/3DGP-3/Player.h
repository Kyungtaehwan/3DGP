#pragma once
#include "GameObject.h"
#include "Camera.h"

class CHeightMapTerrain;

class CPlayer : public CGameObject
{
public:
    CPlayer();
    virtual ~CPlayer();

    bool LoadModel(ID3D12Device* pd3dDevice,
                   ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader,
                   const char* pstrFileName);

    XMFLOAT3 GetPosition() { return m_xmf3Position; }
    XMFLOAT3 GetLookVector() { return m_xmf3Look; }
    XMFLOAT3 GetUpVector() { return m_xmf3Up; }
    XMFLOAT3 GetRightVector() { return m_xmf3Right; }

    CCamera* GetCamera() { return m_pCamera; }
    void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

    void SetCombatResources(CMesh* pBulletMesh, CShader* pColorShader)
    {
        m_pBulletMesh = pBulletMesh;
        m_pColorShader = pColorShader;
    }

    void SetTerrain(CHeightMapTerrain* pTerrain) { m_pTerrain = pTerrain; }

    void OnHit(int nDamage)
    {
        if((m_nHP -= nDamage) < 0)
            m_nHP = 0;
    }
    bool IsDestroyed() const { return m_nHP <= 0; }
    int GetHP() const { return m_nHP; }
    int GetMaxHP() const { return m_nMaxHP; }

    void SetPosition(const XMFLOAT3& pos);

    void Move(DWORD dwDirection, float fDistance);
    void Move(const XMFLOAT3& xmf3Shift);
    void Rotate(float fPitch, float fYaw, float fRoll);

    virtual int Update(float fTimeElapsed);

    CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void ReleaseShaderVariables() override;
    virtual void ReleaseUploadBuffers() override;

protected:
    void RebuildLocalAxis();

    XMFLOAT3 m_xmf3Position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Right = { 1.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Up = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 m_xmf3Look = { 0.0f, 0.0f, 1.0f };

    float m_fPitch = 0.0f;
    float m_fYaw = 0.0f;
    float m_fRoll = 0.0f;

    float m_fBankPitch = 0.0f; 
    float m_fBankRoll = 0.0f;

    XMFLOAT3 m_xmf3LookAt = { 0.0f, 0.0f, 10.0f };

    float m_fMoveSpeed = 60.0f;
    float m_fModelScale = 0.2f;

    CCamera* m_pCamera = NULL;

    CGameObject* m_pModel = NULL;
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;
    ID3D12Device* m_pd3dDevice = NULL;

    CMesh* m_pBulletMesh = NULL;
    CShader* m_pColorShader = NULL;
    CHeightMapTerrain* m_pTerrain = NULL;
    float m_fFireTimer = 0.0f;

    int m_nMaxHP = 10;
    int m_nHP = 10;
    bool m_bDeathFxShown = false;
};
