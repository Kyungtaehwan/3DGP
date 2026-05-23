#pragma once
#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
public:
    CPlayer();
    virtual ~CPlayer();

    bool LoadModel(ID3D12Device* pd3dDevice,
                   ID3D12GraphicsCommandList* pd3dCommandList,
                   CShader* pShader,
                   const char* pstrFileName);

    XMFLOAT3 GetPosition()    { return m_xmf3Position; }
    XMFLOAT3 GetLookVector()  { return m_xmf3Look; }
    XMFLOAT3 GetUpVector()    { return m_xmf3Up; }
    XMFLOAT3 GetRightVector() { return m_xmf3Right; }

    CCamera* GetCamera()                   { return m_pCamera; }
    void     SetCamera(CCamera* pCamera)   { m_pCamera = pCamera; }

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
    XMFLOAT3 m_xmf3Right    = { 1.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Up       = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 m_xmf3Look     = { 0.0f, 0.0f, 1.0f };

    float m_fPitch = 0.0f;
    float m_fYaw   = 0.0f;
    float m_fRoll  = 0.0f;

    XMFLOAT3 m_xmf3LookAt = { 0.0f, 0.0f, 10.0f };

    float m_fMoveSpeed = 30.0f;
    float m_fModelScale = 0.2f;

    CCamera* m_pCamera = NULL;

    // Apache hierarchy root, owned by us
    CGameObject* m_pModel = NULL;
    // Rotor frames inside m_pModel; not owned (just cached pointers).
    CGameObject* m_pMainRotorFrame = NULL;
    CGameObject* m_pTailRotorFrame = NULL;
    ID3D12Device* m_pd3dDevice = NULL;
};
