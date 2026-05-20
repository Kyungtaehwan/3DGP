#pragma once

// Camera type identifiers
#define FIRST_PERSON_CAMERA   0x01
#define SPACESHIP_CAMERA      0x02
#define THIRD_PERSON_CAMERA   0x03

class CPlayer;

// Constant buffer data for b0 (camera info)
struct VS_CB_CAMERA_INFO
{
    XMFLOAT4X4 m_xmf4x4View;
    XMFLOAT4X4 m_xmf4x4Projection;
    XMFLOAT3   m_xmf3Position;
    float      padding;
};

class CCamera
{
public:
    CCamera();
    CCamera(CCamera* pCamera);
    virtual ~CCamera();

    // Shader resource management
    virtual void CreateShaderVariables(ID3D12Device* pd3dDevice,
                                       ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
    virtual void ReleaseShaderVariables();

    // Upload buffer cleanup
    virtual void ReleaseUploadBuffers() {}

    // Camera update — called every frame
    virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) {}
    virtual void SetLookAt(XMFLOAT3& xmf3LookAt) {}

    // Viewport / scissor
    void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight,
                     float fMinZ = 0.0f, float fMaxZ = 1.0f);
    void SetScissorRect(LONG xLeft, LONG yTop, LONG xRight, LONG yBottom);
    void GenerateViewMatrix();
    void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance,
                                  float fAspectRatio, float fFOVAngle);
    void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList);
    void RegenerateViewMatrix();

    // Accessors
    void SetCamera(CCamera* pCamera) {}
    void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
    CPlayer* GetPlayer() { return m_pPlayer; }

    void SetMode(DWORD dwMode) { m_nMode = dwMode; }
    DWORD GetMode() { return m_nMode; }

    void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
    XMFLOAT3& GetPosition() { return m_xmf3Position; }

    void SetLookAtMatrix(XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up = XMFLOAT3(0, 1, 0));
    XMFLOAT3& GetLookAtPosition() { return m_xmf3LookAt; }

    XMFLOAT3& GetRightVector() { return m_xmf3Right; }
    XMFLOAT3& GetUpVector()    { return m_xmf3Up; }
    XMFLOAT3& GetLookVector()  { return m_xmf3Look; }

    float& GetPitch() { return m_fPitch; }
    float& GetRoll()  { return m_fRoll; }
    float& GetYaw()   { return m_fYaw; }

    XMFLOAT4X4 GetViewMatrix()       { return m_xmf4x4View; }
    XMFLOAT4X4 GetProjectionMatrix() { return m_xmf4x4Projection; }
    D3D12_VIEWPORT GetViewport()     { return m_d3dViewport; }
    D3D12_RECT     GetScissorRect()  { return m_d3dScissorRect; }

    void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
    XMFLOAT3 GetOffset() { return m_xmf3Offset; }
    void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
    float GetTimeLag() { return m_fTimeLag; }

    void SetFOVAngle(float fFOVAngle) { m_fFOVAngle = fFOVAngle; }
    float GetFOVAngle() { return m_fFOVAngle; }

    void Move(XMFLOAT3& xmf3Shift) {
        m_xmf3Position.x += xmf3Shift.x;
        m_xmf3Position.y += xmf3Shift.y;
        m_xmf3Position.z += xmf3Shift.z;
        RegenerateViewMatrix();
    }
    void Rotate(float fPitch, float fYaw, float fRoll);

protected:
    XMFLOAT3   m_xmf3Position = { 0.0f, 0.0f, 0.0f };
    XMFLOAT3   m_xmf3Right    = { 1.0f, 0.0f, 0.0f };
    XMFLOAT3   m_xmf3Up       = { 0.0f, 1.0f, 0.0f };
    XMFLOAT3   m_xmf3Look     = { 0.0f, 0.0f, 1.0f };
    XMFLOAT3   m_xmf3LookAt   = { 0.0f, 0.0f, 0.0f };

    float m_fPitch = 0.0f;
    float m_fRoll  = 0.0f;
    float m_fYaw   = 0.0f;

    DWORD m_nMode = 0;

    XMFLOAT3 m_xmf3Offset  = { 0.0f, 0.0f, 0.0f };
    float    m_fTimeLag    = 0.0f;
    XMFLOAT3 m_xmf3Target  = { 0.0f, 0.0f, 0.0f };

    float m_fFOVAngle        = 60.0f;
    float m_fNearPlane       = 0.1f;
    float m_fFarPlane        = 1000.0f;
    float m_fAspectRatio     = ASPECT_RATIO;

    XMFLOAT4X4 m_xmf4x4View;
    XMFLOAT4X4 m_xmf4x4Projection;

    D3D12_VIEWPORT m_d3dViewport;
    D3D12_RECT     m_d3dScissorRect;

    CPlayer* m_pPlayer = NULL;

    // GPU constant buffer (b0)
    VS_CB_CAMERA_INFO*  m_pcbMappedCamera = NULL;
    ID3D12Resource*     m_pd3dcbCamera    = NULL;
};

// ============================================================
// Derived camera classes
// ============================================================

class CSpaceShipCamera : public CCamera
{
public:
    CSpaceShipCamera(CCamera* pCamera);
    virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) override;
    virtual void SetLookAt(XMFLOAT3& xmf3LookAt) override;
};

class CFirstPersonCamera : public CCamera
{
public:
    CFirstPersonCamera(CCamera* pCamera);
    virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) override;
    virtual void SetLookAt(XMFLOAT3& xmf3LookAt) override;
};

class CThirdPersonCamera : public CCamera
{
public:
    CThirdPersonCamera(CCamera* pCamera);
    virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) override;
    virtual void SetLookAt(XMFLOAT3& xmf3LookAt) override;
};
