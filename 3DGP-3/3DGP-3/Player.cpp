#include "pch.h"
#include "Player.h"
#include "Camera.h"
#include "Shader.h"
#include "Input_Manager.h"

CPlayer::CPlayer()
{
    m_xmf4x4World = Matrix4x4::Identity();
}

CPlayer::~CPlayer()
{
    ReleaseShaderVariables();
    if (m_pCamera) { delete m_pCamera; m_pCamera = NULL; }
    if (m_pModel)  { m_pModel->Release(); m_pModel = NULL; }
}

bool CPlayer::LoadModel(ID3D12Device* pd3dDevice,
                        ID3D12GraphicsCommandList* pd3dCommandList,
                        CShader* pShader,
                        const char* pstrFileName)
{
    m_pModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList,
                                                  pstrFileName, pShader);
    if (!m_pModel) return false;
    m_pModel->AddRef();
    m_pModel->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    // Apache rotor frames (matches CApacheObject::OnInitialize in Sample).
    m_pMainRotorFrame = m_pModel->FindFrame((char*)"rotor");
    m_pTailRotorFrame = m_pModel->FindFrame((char*)"black_m_7");
    return true;
}

void CPlayer::SetPosition(const XMFLOAT3& pos)
{
    m_xmf3Position = pos;
}

void CPlayer::Move(DWORD dwDirection, float fDistance)
{
    if (dwDirection == 0) return;

    XMFLOAT3 shift = { 0.0f, 0.0f, 0.0f };
    if (dwDirection & DIR_FORWARD)  shift = Vector3::Add(shift, m_xmf3Look,    fDistance);
    if (dwDirection & DIR_BACKWARD) shift = Vector3::Add(shift, m_xmf3Look,   -fDistance);
    if (dwDirection & DIR_RIGHT)    shift = Vector3::Add(shift, m_xmf3Right,   fDistance);
    if (dwDirection & DIR_LEFT)     shift = Vector3::Add(shift, m_xmf3Right,  -fDistance);
    if (dwDirection & DIR_UP)       shift = Vector3::Add(shift, m_xmf3Up,      fDistance);
    if (dwDirection & DIR_DOWN)     shift = Vector3::Add(shift, m_xmf3Up,     -fDistance);
    Move(shift);
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift)
{
    m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
    if (m_pCamera) m_pCamera->Move(const_cast<XMFLOAT3&>(xmf3Shift));
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
    if (!IsZero(fYaw))
    {
        m_fYaw += fYaw;
        XMMATRIX mtxRot = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), DegreeToRadian(fYaw));
        m_xmf3Look  = Vector3::TransformNormal(m_xmf3Look,  mtxRot);
        m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRot);
    }
    if (!IsZero(fPitch))
    {
        m_fPitch += fPitch;
        if (m_fPitch >  60.0f) { fPitch -= (m_fPitch -  60.0f); m_fPitch =  60.0f; }
        if (m_fPitch < -60.0f) { fPitch -= (m_fPitch + 60.0f);  m_fPitch = -60.0f; }
        XMMATRIX mtxRot = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), DegreeToRadian(fPitch));
        m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRot);
        m_xmf3Up   = Vector3::TransformNormal(m_xmf3Up,   mtxRot);
    }
    RebuildLocalAxis();

    if (m_pCamera) m_pCamera->Rotate(fPitch, fYaw, fRoll);
}

void CPlayer::RebuildLocalAxis()
{
    m_xmf3Look  = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
    m_xmf3Up    = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}

int CPlayer::Update(float fTimeElapsed)
{
    CInput_Manager* pInput = CInput_Manager::Get_Instance();
    pInput->Update_Mouse(g_hWnd);

    // 1) Mouse rotates the orbit camera around the player.
    CThirdPersonCamera* pOrbit = (m_pCamera && m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
                                  ? static_cast<CThirdPersonCamera*>(m_pCamera) : nullptr;
    if (pOrbit)
    {
        pOrbit->OrbitInput((float)pInput->GetMouseDX() * 0.2f,
                           (float)pInput->GetMouseDY() * 0.2f);
    }

    // 2) Read movement input. WASD operates in the chopper's LOCAL frame --
    //    the chopper's heading is fixed; mouse only orbits the camera.
    float fForwardIn = 0.0f;   // +1 W, -1 S
    float fStrafeIn  = 0.0f;   // +1 D, -1 A
    if (pInput->Key_Pressing('W')) fForwardIn += 1.0f;
    if (pInput->Key_Pressing('S')) fForwardIn -= 1.0f;
    if (pInput->Key_Pressing('D')) fStrafeIn  += 1.0f;
    if (pInput->Key_Pressing('A')) fStrafeIn  -= 1.0f;

    // Chopper's local axes from its (unchanged) yaw.
    float yawR = DegreeToRadian(m_fYaw);
    XMFLOAT3 helFwd = XMFLOAT3(sinf(yawR), 0.0f,  cosf(yawR));
    XMFLOAT3 helRgt = XMFLOAT3(cosf(yawR), 0.0f, -sinf(yawR));

    float step = m_fMoveSpeed * fTimeElapsed;
    XMFLOAT3 shift = { 0.0f, 0.0f, 0.0f };
    shift = Vector3::Add(shift, helFwd, fForwardIn * step);
    shift = Vector3::Add(shift, helRgt, fStrafeIn  * step);
    if (pInput->Key_Pressing(VK_SPACE))    shift.y += step;
    if (pInput->Key_Pressing(VK_CONTROL))  shift.y -= step;
    if (!IsZero(shift.x) || !IsZero(shift.y) || !IsZero(shift.z))
        m_xmf3Position = Vector3::Add(m_xmf3Position, shift);

    // 3) Bank straight from input (input is already in the chopper's local frame).
    const float kMaxBankPitch = 20.0f;
    const float kMaxBankRoll  = 25.0f;
    const float kBankTau      = 0.15f;
    float fTargetBankPitch =  fForwardIn * kMaxBankPitch;
    float fTargetBankRoll  = -fStrafeIn  * kMaxBankRoll;   // D -> right wing dips down
    float fBankT = fTimeElapsed / kBankTau;
    if (fBankT > 1.0f) fBankT = 1.0f;
    m_fBankPitch += (fTargetBankPitch - m_fBankPitch) * fBankT;
    m_fBankRoll  += (fTargetBankRoll  - m_fBankRoll)  * fBankT;

    // 4) Cache local axes (yaw is unchanged here).
    m_xmf3Look  = helFwd;
    m_xmf3Right = helRgt;
    m_xmf3Up    = XMFLOAT3(0.0f, 1.0f, 0.0f);

    // 6) Camera updates after player position/heading are set.
    if (m_pCamera) m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);

    // Spin the rotors (CApacheObject::Animate behavior).
    if (m_pMainRotorFrame)
    {
        XMMATRIX r = XMMatrixRotationY(XMConvertToRadians(360.0f * 2.0f) * fTimeElapsed);
        m_pMainRotorFrame->m_xmf4x4Transform =
            Matrix4x4::Multiply(r, m_pMainRotorFrame->m_xmf4x4Transform);
    }
    if (m_pTailRotorFrame)
    {
        XMMATRIX r = XMMatrixRotationY(XMConvertToRadians(360.0f * 4.0f) * fTimeElapsed);
        m_pTailRotorFrame->m_xmf4x4Transform =
            Matrix4x4::Multiply(r, m_pTailRotorFrame->m_xmf4x4Transform);
    }

    // 7) Build model transform: scale -> bank(roll Z, pitch X) -> yaw Y -> translate.
    //    Bank is applied in the chopper's local frame BEFORE yaw so the lean
    //    direction follows where the chopper is currently pointing.
    if (m_pModel)
    {
        XMMATRIX mScale = XMMatrixScaling(m_fModelScale, m_fModelScale, m_fModelScale);
        XMMATRIX mRoll  = XMMatrixRotationZ(DegreeToRadian(m_fBankRoll));
        XMMATRIX mPitch = XMMatrixRotationX(DegreeToRadian(m_fBankPitch));
        XMMATRIX mYaw   = XMMatrixRotationY(yawR);
        XMMATRIX mTrans = XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);
        XMMATRIX mWorld = mScale * mRoll * mPitch * mYaw * mTrans;
        XMStoreFloat4x4(&m_pModel->m_xmf4x4Transform, mWorld);
        m_pModel->UpdateTransform(NULL);
    }

    m_xmf3LookAt = Vector3::Add(m_xmf3Position, m_xmf3Look, 10.0f);

    return OBJ_NOEVENT;
}

CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
    CCamera* pOld = m_pCamera;
    CCamera* pNew = NULL;
    switch (nNewCameraMode)
    {
    case THIRD_PERSON_CAMERA: pNew = new CThirdPersonCamera(pOld); break;
    default:                  pNew = new CThirdPersonCamera(pOld); break;
    }

    pNew->SetPlayer(this);
    pNew->SetMode(nNewCameraMode);
    pNew->SetTimeLag(0.0f);
    pNew->SetOffset(XMFLOAT3(0.0f, 8.0f, -25.0f));
    pNew->GenerateProjectionMatrix(0.1f, 5000.0f, ASPECT_RATIO, 60.0f);
    pNew->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    pNew->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
    pNew->SetPosition(Vector3::Add(m_xmf3Position, pNew->GetOffset()));

    if (pOld) delete pOld;
    m_pCamera = pNew;

    if (m_pd3dDevice)
        m_pCamera->CreateShaderVariables(m_pd3dDevice, nullptr);

    m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);
    m_pCamera->SetLookAt(m_xmf3LookAt);
    m_pCamera->RegenerateViewMatrix();

    return m_pCamera;
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (m_pModel) m_pModel->Render(pd3dCommandList, pCamera);
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                    ID3D12GraphicsCommandList* pd3dCommandList)
{
    m_pd3dDevice = pd3dDevice;
    if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
    if (m_pCamera) m_pCamera->ReleaseShaderVariables();
    if (m_pModel)  m_pModel->ReleaseShaderVariables();
}

void CPlayer::ReleaseUploadBuffers()
{
    if (m_pModel) m_pModel->ReleaseUploadBuffers();
}
