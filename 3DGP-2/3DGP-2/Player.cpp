#include "pch.h"
#include "Player.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"

CPlayer::CPlayer()
{
    m_xmf4x4World = Matrix4x4::Identity();
}

CPlayer::~CPlayer()
{
    ReleaseShaderVariables();

    if (m_pCamera) { delete m_pCamera; m_pCamera = NULL; }

    for (auto& part : m_BodyParts)
        if (part.pObject) { delete part.pObject; part.pObject = NULL; }
    m_BodyParts.clear();
}

// ---------------------------------------------------------------
// CreateBodyParts  (y=0 at player feet)
// ---------------------------------------------------------------
void CPlayer::CreateBodyParts(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader)
{
    XMFLOAT4 skin    = { 0.9f,  0.7f,  0.5f,  1.0f };
    XMFLOAT4 clothes = { 0.3f,  0.3f,  0.7f,  1.0f };
    XMFLOAT4 pants   = { 0.15f, 0.2f,  0.45f, 1.0f };
    XMFLOAT4 shoes   = { 0.2f,  0.1f,  0.05f, 1.0f };

    auto addPart = [&](float w, float h, float d, XMFLOAT4 color, XMFLOAT3 offset)
    {
        BodyPart bp;
        bp.pObject = new CGameObject();
        bp.pObject->SetShader(pShader);
        bp.pObject->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList, w, h, d, color));
        bp.pObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        bp.vOffset = offset;
        m_BodyParts.push_back(bp);
    };

    addPart(0.40f, 0.40f, 0.40f, skin,    { 0.0f,   1.90f,  0.0f });   // head
    addPart(0.18f, 0.15f, 0.18f, skin,    { 0.0f,   1.625f, 0.0f });   // neck
    addPart(0.50f, 0.65f, 0.30f, clothes, { 0.0f,   1.325f, 0.0f });   // torso
    addPart(0.22f, 0.45f, 0.22f, clothes, {-0.36f,  1.35f,  0.0f });   // upper arm L
    addPart(0.22f, 0.45f, 0.22f, clothes, { 0.36f,  1.35f,  0.0f });   // upper arm R
    addPart(0.20f, 0.40f, 0.20f, skin,    {-0.38f,  0.90f,  0.0f });   // lower arm L
    addPart(0.20f, 0.40f, 0.20f, skin,    { 0.38f,  0.90f,  0.0f });   // lower arm R
    addPart(0.22f, 0.50f, 0.22f, pants,   {-0.14f,  0.80f,  0.0f });   // upper leg L
    addPart(0.22f, 0.50f, 0.22f, pants,   { 0.14f,  0.80f,  0.0f });   // upper leg R
    addPart(0.20f, 0.45f, 0.20f, pants,   {-0.14f,  0.275f, 0.0f });   // lower leg L
    addPart(0.20f, 0.45f, 0.20f, pants,   { 0.14f,  0.275f, 0.0f });   // lower leg R
    addPart(0.24f, 0.10f, 0.30f, shoes,   {-0.14f,  0.05f,  0.04f });  // foot L
    addPart(0.24f, 0.10f, 0.30f, shoes,   { 0.14f,  0.05f,  0.04f });  // foot R
}

// body-part world = T(localOffset) * Ry(yaw) * T(playerPos)
void CPlayer::UpdateBodyPartsWorld()
{
    XMMATRIX mRot = XMMatrixRotationY(DegreeToRadian(m_fYaw));
    XMMATRIX mPos = XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);

    for (auto& part : m_BodyParts)
    {
        if (!part.pObject) continue;
        XMMATRIX mOffset = XMMatrixTranslation(part.vOffset.x, part.vOffset.y, part.vOffset.z);
        XMMATRIX mFinal  = mOffset * mRot * mPos;
        XMStoreFloat4x4(&part.pObject->m_xmf4x4World, mFinal);
    }
}

void CPlayer::SetPosition(const XMFLOAT3& xmf3Position)
{
    m_xmf3Position = xmf3Position;
    OnPrepareRender();
}

void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
    if (dwDirection == 0) return;

    XMFLOAT3 shift = { 0.0f, 0.0f, 0.0f };

    if (dwDirection & DIR_FORWARD)  shift = Vector3::Add(shift, m_xmf3Look,   fDistance);
    if (dwDirection & DIR_BACKWARD) shift = Vector3::Add(shift, m_xmf3Look,  -fDistance);
    if (dwDirection & DIR_RIGHT)    shift = Vector3::Add(shift, m_xmf3Right,  fDistance);
    if (dwDirection & DIR_LEFT)     shift = Vector3::Add(shift, m_xmf3Right, -fDistance);
    if (dwDirection & DIR_UP)       shift = Vector3::Add(shift, m_xmf3Up,     fDistance);
    if (dwDirection & DIR_DOWN)     shift = Vector3::Add(shift, m_xmf3Up,    -fDistance);

    Move(shift, bUpdateVelocity);
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
    if (bUpdateVelocity)
    {
        m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
    }
    else
    {
        m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
        OnPrepareRender();
        if (m_pCamera) m_pCamera->Move(const_cast<XMFLOAT3&>(xmf3Shift));
    }
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
    DWORD nMode = m_pCamera ? m_pCamera->GetMode() : 0;

    if (nMode == FIRST_PERSON_CAMERA)
    {
        if (!IsZero(fPitch))
        {
            m_fPitch += fPitch;
            if (m_fPitch >  89.0f) { fPitch -= (m_fPitch -  89.0f); m_fPitch =  89.0f; }
            if (m_fPitch < -89.0f) { fPitch -= (m_fPitch + 89.0f);  m_fPitch = -89.0f; }

            XMMATRIX mtxRot = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
                                                    DegreeToRadian(fPitch));
            m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRot);
            m_xmf3Up   = Vector3::TransformNormal(m_xmf3Up,   mtxRot);
        }
        if (!IsZero(fYaw))
        {
            m_fYaw += fYaw;
            if (m_fYaw >  360.0f) m_fYaw -= 360.0f;
            if (m_fYaw <    0.0f) m_fYaw += 360.0f;

            XMFLOAT3 worldUp = { 0.0f, 1.0f, 0.0f };
            XMMATRIX mtxRot  = XMMatrixRotationAxis(XMLoadFloat3(&worldUp),
                                                     DegreeToRadian(fYaw));
            m_xmf3Look  = Vector3::TransformNormal(m_xmf3Look,  mtxRot);
            m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRot);
            m_xmf3Up    = Vector3::TransformNormal(m_xmf3Up,    mtxRot);
        }
        if (m_pCamera) m_pCamera->Rotate(fPitch, fYaw, fRoll);
    }
    else if (nMode == THIRD_PERSON_CAMERA || nMode == SPACESHIP_CAMERA)
    {
        if (!IsZero(fYaw))
        {
            m_fYaw += fYaw;
            if (m_fYaw >  360.0f) m_fYaw -= 360.0f;
            if (m_fYaw <    0.0f) m_fYaw += 360.0f;

            XMMATRIX mtxRot = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
                                                    DegreeToRadian(fYaw));
            m_xmf3Look  = Vector3::TransformNormal(m_xmf3Look,  mtxRot);
            m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRot);
        }
        if (m_pCamera) m_pCamera->Rotate(fPitch, fYaw, fRoll);
    }

    m_xmf3Look  = Vector3::Normalize(m_xmf3Look);
    m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
    m_xmf3Up    = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));

    OnPrepareRender();
}

int CPlayer::Update(float fTimeElapsed)
{
    m_xmf3Velocity = Vector3::Add(m_xmf3Velocity,
                                   Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed));

    float fVelXZ = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x +
                         m_xmf3Velocity.z * m_xmf3Velocity.z);
    if (fVelXZ > m_fMaxVelocityXZ)
    {
        float s = m_fMaxVelocityXZ / fVelXZ;
        m_xmf3Velocity.x *= s;
        m_xmf3Velocity.z *= s;
    }
    if (fabsf(m_xmf3Velocity.y) > m_fMaxVelocityY)
        m_xmf3Velocity.y = (m_xmf3Velocity.y < 0.0f) ? -m_fMaxVelocityY : m_fMaxVelocityY;

    Move(Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed), false);

    float fVel = Vector3::Length(m_xmf3Velocity);
    if (fVel > 0.0f)
    {
        float fDecel = m_fFriction * fTimeElapsed;
        if (fDecel > fVel) fDecel = fVel;
        XMFLOAT3 dir = Vector3::Normalize(m_xmf3Velocity);
        m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(dir, -fDecel));
    }

    if (m_pCamera)
    {
        m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);
        m_pCamera->SetLookAt(m_xmf3LookAt);
        m_pCamera->RegenerateViewMatrix();
    }

    return OBJ_NOEVENT;
}

void CPlayer::OnPrepareRender()
{
    m_xmf4x4World._11 = m_xmf3Right.x;    m_xmf4x4World._12 = m_xmf3Right.y;    m_xmf4x4World._13 = m_xmf3Right.z;    m_xmf4x4World._14 = 0.0f;
    m_xmf4x4World._21 = m_xmf3Up.x;       m_xmf4x4World._22 = m_xmf3Up.y;       m_xmf4x4World._23 = m_xmf3Up.z;       m_xmf4x4World._24 = 0.0f;
    m_xmf4x4World._31 = m_xmf3Look.x;     m_xmf4x4World._32 = m_xmf3Look.y;     m_xmf4x4World._33 = m_xmf3Look.z;     m_xmf4x4World._34 = 0.0f;
    m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z; m_xmf4x4World._44 = 1.0f;

    m_xmf3LookAt = Vector3::Add(m_xmf3Position, m_xmf3Look, 10.0f);
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if (!m_pCamera || m_pCamera->GetMode() != THIRD_PERSON_CAMERA) return;

    UpdateBodyPartsWorld();
    for (auto& part : m_BodyParts)
        if (part.pObject) part.pObject->Render(pd3dCommandList, pCamera);
}

CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
    CCamera* pNewCamera = NULL;
    switch (nNewCameraMode)
    {
    case FIRST_PERSON_CAMERA:  pNewCamera = new CFirstPersonCamera(m_pCamera);  break;
    case SPACESHIP_CAMERA:     pNewCamera = new CSpaceShipCamera(m_pCamera);    break;
    case THIRD_PERSON_CAMERA:  pNewCamera = new CThirdPersonCamera(m_pCamera);  break;
    default:                   pNewCamera = new CFirstPersonCamera(m_pCamera);  break;
    }

    if (pNewCamera)
    {
        pNewCamera->SetPlayer(this);
        pNewCamera->SetMode(nNewCameraMode);
        pNewCamera->SetPosition(Vector3::Add(m_xmf3Position, pNewCamera->GetOffset()));
    }

    if (m_pCamera) delete m_pCamera;
    return pNewCamera;
}

CCamera* CPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
    DWORD nCurrentMode = m_pCamera ? m_pCamera->GetMode() : 0;
    if (nCurrentMode == nNewCameraMode) return m_pCamera;

    switch (nNewCameraMode)
    {
    case FIRST_PERSON_CAMERA:
        SetFriction(2.0f);
        SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
        SetMaxVelocityXZ(10.0f);
        SetMaxVelocityY(10.0f);
        m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentMode);
        m_pCamera->SetTimeLag(0.0f);
        m_pCamera->SetOffset(XMFLOAT3(0.0f, 1.7f, 0.0f));
        m_pCamera->GenerateProjectionMatrix(0.1f, 500.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    case SPACESHIP_CAMERA:
        SetFriction(1.0f);
        SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
        SetMaxVelocityXZ(10.0f);
        SetMaxVelocityY(10.0f);
        m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentMode);
        m_pCamera->SetTimeLag(0.1f);
        m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -8.0f));
        m_pCamera->GenerateProjectionMatrix(0.1f, 500.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    case THIRD_PERSON_CAMERA:
        SetFriction(10.0f);
        SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
        SetMaxVelocityXZ(10.0f);
        SetMaxVelocityY(10.0f);
        m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentMode);
        m_pCamera->SetTimeLag(0.25f);
        m_pCamera->SetOffset(XMFLOAT3(0.0f, 3.0f, -8.0f));
        m_pCamera->GenerateProjectionMatrix(0.1f, 500.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    default:
        break;
    }

    OnPrepareRender();
    m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);
    m_pCamera->SetLookAt(m_xmf3LookAt);
    m_pCamera->RegenerateViewMatrix();

    return m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                     ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_pCamera)
        m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
    if (m_pCamera) m_pCamera->ReleaseShaderVariables();
    CGameObject::ReleaseShaderVariables();

    for (auto& part : m_BodyParts)
        if (part.pObject) part.pObject->ReleaseShaderVariables();
}

void CPlayer::ReleaseUploadBuffers()
{
    CGameObject::ReleaseUploadBuffers();

    for (auto& part : m_BodyParts)
        if (part.pObject) part.pObject->ReleaseUploadBuffers();
}
