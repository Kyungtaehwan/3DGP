#pragma once
#include "GameObject.h"
#include "Camera.h"
#include "Input_Manager.h"

struct BodyPart
{
    CGameObject* pObject = NULL;
    XMFLOAT3     vOffset = { 0.0f, 0.0f, 0.0f };
};

class CPlayer : public CGameObject
{
protected:
    XMFLOAT3 m_xmf3Position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Right    = { 1.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Up       = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3 m_xmf3Look     = { 0.0f, 0.0f, 1.0f };

    float m_fPitch = 0.0f;
    float m_fYaw   = 0.0f;
    float m_fRoll  = 0.0f;

    XMFLOAT3 m_xmf3LookAt   = { 0.0f, 0.0f, 10.0f };
    XMFLOAT3 m_xmf3Velocity = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_xmf3Gravity  = { 0.0f, 0.0f, 0.0f };

    float m_fMaxVelocityXZ = 20.0f;
    float m_fMaxVelocityY  = 20.0f;
    float m_fFriction      = 2.0f;

    CCamera* m_pCamera = NULL;

    ID3D12Device* m_pd3dDevice = NULL;

    std::vector<BodyPart> m_BodyParts;

    // Walk
    float m_fAnimTime = 0.0f;
    bool  m_bIsMoving = false;

    // Shoot
    float m_fRecoilTimer    = -1.0f;
    static constexpr float RECOIL_DURATION = 0.22f;

    enum { BP_HEAD=0, BP_TORSO, BP_ARM_L, BP_ARM_R, BP_LEG_L, BP_LEG_R, BP_GUN, BP_COUNT };

public:
    CPlayer();
    virtual ~CPlayer();

    XMFLOAT3 GetPosition()    { return m_xmf3Position; }
    XMFLOAT3 GetLookVector()  { return m_xmf3Look; }
    XMFLOAT3 GetUpVector()    { return m_xmf3Up; }
    XMFLOAT3 GetRightVector() { return m_xmf3Right; }

    void SetFriction(float f)            { m_fFriction = f; }
    void SetGravity(const XMFLOAT3& g)  { m_xmf3Gravity = g; }
    void SetMaxVelocityXZ(float v)       { m_fMaxVelocityXZ = v; }
    void SetMaxVelocityY(float v)        { m_fMaxVelocityY  = v; }
    void SetPosition(const XMFLOAT3& pos);

    CCamera* GetCamera()                  { return m_pCamera; }
    void     SetCamera(CCamera* pCamera)  { m_pCamera = pCamera; }

    void CreateBodyParts(ID3D12Device* pd3dDevice,
                         ID3D12GraphicsCommandList* pd3dCommandList,
                         CShader* pShader);

    void Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity = false);
    void Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity = false);
    void Rotate(float fPitch, float fYaw, float fRoll);

    virtual int Update(float fTimeElapsed) override;

    CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
    virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

    virtual void OnPrepareRender();
    virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) override;

    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList) override;
    virtual void ReleaseShaderVariables() override;
    virtual void ReleaseUploadBuffers() override;

private:
    void UpdateBodyPartsWorld();
    void RenderFPSArm(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};
