#include "pch.h"
#include "Player.h"
#include "Camera.h"
#include "Shader.h"
#include "Mesh.h"
#include "Bullet.h"
#include "Object_Manager.h"
#include "Map_Manager.h"

CPlayer::CPlayer()
{
    m_xmf4x4World = Matrix4x4::Identity();
    m_xmLocalOBB = BoundingOrientedBox(
        XMFLOAT3(0.0f, 1.0f, 0.0f),      
        XMFLOAT3(0.28f, 1.0f, 0.28f),   
        XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f) 
    );
}

CPlayer::~CPlayer()
{
    ReleaseShaderVariables();

    if (m_pCamera) { delete m_pCamera; m_pCamera = NULL; }

    for (auto& part : m_BodyParts)
        if (part.pObject) { delete part.pObject; part.pObject = NULL; }
    m_BodyParts.clear();
}


void CPlayer::CreateBodyParts(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               CShader* pShader)
{
    XMFLOAT4 skin    = { 0.9f,  0.7f,  0.5f,  1.0f };
    XMFLOAT4 clothes = { 0.3f,  0.3f,  0.7f,  1.0f };
    XMFLOAT4 pants   = { 0.15f, 0.2f,  0.45f, 1.0f };

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

    // BP_HEAD
    addPart(0.40f, 0.40f, 0.40f, skin,    {  0.00f, 1.80f, 0.00f });
    // BP_TORSO
    addPart(0.50f, 0.70f, 0.25f, clothes, {  0.00f, 1.25f, 0.00f });
    // BP_ARM_L
    addPart(0.18f, 0.60f, 0.18f, clothes, { -0.34f, 1.30f, 0.00f });
    // BP_ARM_R
    addPart(0.18f, 0.60f, 0.18f, clothes, {  0.34f, 1.60f, 0.30f });
    // BP_LEG_L
    addPart(0.22f, 1.00f, 0.22f, pants,   { -0.14f, 0.50f, 0.00f });

    addPart(0.22f, 1.00f, 0.22f, pants,   {  0.14f, 0.50f, 0.00f });

    {
        BodyPart bp;
        bp.pObject = new CGameObject();
        bp.pObject->SetShader(pShader);
        bp.pObject->SetMesh(new CGlockMesh(pd3dDevice, pd3dCommandList));
        bp.pObject->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        bp.vOffset = { 0.34f, 1.60f, 0.60f };//오른팔 끝
        m_BodyParts.push_back(bp);
    }
}

void CPlayer::UpdateBodyPartsWorld()
{
    XMMATRIX mRot = XMMatrixRotationY(DegreeToRadian(m_fYaw));
    XMMATRIX mPos = XMMatrixTranslation(m_xmf3Position.x, m_xmf3Position.y, m_xmf3Position.z);

    float fSwing = sinf(m_fAnimTime * 8.0f) * (m_bIsMoving ? 35.0f : 0.0f);

    auto setWorld = [](CGameObject* obj, XMMATRIX m) {
        XMStoreFloat4x4(&obj->m_xmf4x4World, m);
    };


    setWorld(m_BodyParts[BP_HEAD].pObject,
        XMMatrixTranslation(0, 1.80f, 0) * mRot * mPos);


    setWorld(m_BodyParts[BP_TORSO].pObject,
        XMMatrixTranslation(0, 1.25f, 0) * mRot * mPos);


    {
        XMMATRIX mSwing = XMMatrixRotationX(DegreeToRadian(-fSwing));
        setWorld(m_BodyParts[BP_ARM_L].pObject,
            XMMatrixTranslation(0, -0.30f, 0) * mSwing
            * XMMatrixTranslation(-0.34f, 1.60f, 0)
            * mRot * mPos);
    }


    {
        setWorld(m_BodyParts[BP_ARM_R].pObject,
            XMMatrixTranslation(0, -0.30f, 0)
            * XMMatrixRotationX(DegreeToRadian(-90.0f))
            * XMMatrixTranslation(0.34f, 1.60f, 0)
            * mRot * mPos);
    }


    {
        XMMATRIX mSwing = XMMatrixRotationX(DegreeToRadian(fSwing));
        setWorld(m_BodyParts[BP_LEG_L].pObject,
            XMMatrixTranslation(0, -0.50f, 0) * mSwing
            * XMMatrixTranslation(-0.14f, 1.00f, 0)
            * mRot * mPos);
    }


    {
        XMMATRIX mSwing = XMMatrixRotationX(DegreeToRadian(-fSwing));
        setWorld(m_BodyParts[BP_LEG_R].pObject,
            XMMatrixTranslation(0, -0.50f, 0) * mSwing
            * XMMatrixTranslation(0.14f, 1.00f, 0)
            * mRot * mPos);
    }

    // GUN 오른팔 끝
    {
        XMMATRIX mGunLocal =
            XMMatrixTranslation(0.0f, -0.12f, 0.0f)  
            * XMMatrixTranslation(0.34f, 1.60f, 0.60f) // 오른팔 끝
            * mRot * mPos;
        setWorld(m_BodyParts[BP_GUN].pObject, mGunLocal);
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

    // 1인칭에서 앞/뒤 이동은 수평 평면으로 투영
    DWORD nCamMode = m_pCamera ? m_pCamera->GetMode() : 0;
    XMFLOAT3 moveLook = m_xmf3Look;
    if (nCamMode == FIRST_PERSON_CAMERA)
    {
        moveLook.y = 0.0f;
        float fLen = sqrtf(moveLook.x * moveLook.x + moveLook.z * moveLook.z);
        if (fLen > 0.001f) { moveLook.x /= fLen; moveLook.z /= fLen; }
        else               { moveLook = { 0.0f, 0.0f, 1.0f }; }
    }

    if (dwDirection & DIR_FORWARD)  shift = Vector3::Add(shift, moveLook,    fDistance);
    if (dwDirection & DIR_BACKWARD) shift = Vector3::Add(shift, moveLook,   -fDistance);
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
    CInput_Manager* pInput = CInput_Manager::Get_Instance();
    pInput->Update_Mouse(g_hWnd);

    // 1인칭 / 3인칭 전환
    if (pInput->Key_Down('1'))
        ChangeCamera(FIRST_PERSON_CAMERA, fTimeElapsed);
    else if (pInput->Key_Down('3'))
        ChangeCamera(THIRD_PERSON_CAMERA, fTimeElapsed);

    // 이동 입력
    const float fSpeed = 7.0f;
    DWORD dwDir = 0;
    bool bW = pInput->Key_Pressing('W');
    bool bS = pInput->Key_Pressing('S');
    bool bA = pInput->Key_Pressing('A');
    bool bD = pInput->Key_Pressing('D');
    if (bW) dwDir |= DIR_FORWARD;
    if (bS) dwDir |= DIR_BACKWARD;
    if (bA) dwDir |= DIR_LEFT;
    if (bD) dwDir |= DIR_RIGHT;
    if (dwDir) Move(dwDir, fSpeed * fTimeElapsed);

    m_bIsMoving  = (dwDir != 0);
    m_fAnimTime += fTimeElapsed;

    if (pInput->Key_Down(VK_LBUTTON))
    {
        m_fRecoilTimer = 0.0f;

        if (m_pCamera && m_pd3dDevice)
        {
            XMFLOAT3 shootOrigin, shootDir;
            if (m_pCamera->GetMode() == FIRST_PERSON_CAMERA)
            {
                shootOrigin = m_pCamera->GetPosition();
                shootDir    = m_pCamera->GetLookVector();
            }
            else
            {
                // 3인칭: 카메라가 뒤에 있으므로 플레이어 눈 위치에서 플레이어 정면 방향으로
                shootOrigin = { m_xmf3Position.x, m_xmf3Position.y + 1.7f, m_xmf3Position.z };
                shootDir    = m_xmf3Look;
            }

            XMVECTOR vOrigin = XMLoadFloat3(&shootOrigin);
            XMVECTOR vDir    = XMVector3Normalize(XMLoadFloat3(&shootDir));

            constexpr float RAY_MAX = 150.0f;
            float hitDist        = RAY_MAX;
            CGameObject* pHitObj = nullptr;

            // Ray -> 몬스터
            for (CGameObject* pEnemy : *CObject_Manager::Get_Instance()->Get_List(OBJ_ENEMY))
            {
                if (!pEnemy) continue;
                float d;
                if (pEnemy->m_xmOOBB.Intersects(vOrigin, vDir, d) && d < hitDist)
                {
                    hitDist = d;
                    pHitObj = pEnemy;
                }
            }

            // Ray -> 맵
            XMFLOAT3 dirN;
            XMStoreFloat3(&dirN, vDir);
            for (float t = 0.05f; t < hitDist; t += 0.05f)
            {
                int r = (int)floorf((shootOrigin.z + dirN.z * t) / TILE_SCALE);
                int c = (int)floorf((shootOrigin.x + dirN.x * t) / TILE_SCALE);
                if (!CMap_Manager::Get_Instance()->IsPassable(r, c))
                {
                    hitDist = t;
                    pHitObj = nullptr;
                    break;
                }
            }

            if (pHitObj)
                pHitObj->OnHit(25);

            // 충돌 지점 파티클
            XMFLOAT3 hitPos;
            XMStoreFloat3(&hitPos, XMVectorAdd(vOrigin, XMVectorScale(vDir, hitDist)));

            CBullet* pExplosion = new CBullet();
            pExplosion->Fire(m_pd3dDevice, hitPos, shootDir, CBullet::GetSharedShader());
            pExplosion->SetBlowingUp();
            CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER_BULLET, pExplosion);
        }
    }

    if (m_fRecoilTimer >= 0.0f)
    {
        m_fRecoilTimer += fTimeElapsed;
        if (m_fRecoilTimer >= RECOIL_DURATION)
            m_fRecoilTimer = -1.0f;
    }


    // 마우스 회전
    Rotate(pInput->GetMouseDY() * 0.15f,
           pInput->GetMouseDX() * 0.15f,
           0.0f);

    // 물리 (속도, 중력, 마찰)
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

    // 카메라 업데이트
    if (m_pCamera)
        m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);

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
    if (!m_pCamera) return;

    DWORD mode = m_pCamera->GetMode();

    if (mode == THIRD_PERSON_CAMERA)
    {
        UpdateBodyPartsWorld();
        for (auto& part : m_BodyParts)
            if (part.pObject) part.pObject->Render(pd3dCommandList, pCamera);
    }
    else if (mode == FIRST_PERSON_CAMERA)
    {
        // 오른팔 + 총 모두 FPS 뷰모델로 렌더링
        RenderFPSArm(pd3dCommandList, pCamera);
    }
}

// FPS 뷰
void CPlayer::RenderFPSArm(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    if ((int)m_BodyParts.size() <= BP_ARM_R) return;
    CGameObject* pArm = m_BodyParts[BP_ARM_R].pObject;
    if (!pArm) return;

    XMFLOAT3 camPos   = m_pCamera->GetPosition();
    XMFLOAT3 camRight = m_pCamera->GetRightVector();
    XMFLOAT3 camUp    = m_pCamera->GetUpVector();
    XMFLOAT3 camLook  = m_pCamera->GetLookVector();

    // 반동 크기
    float fRecoil = 0.0f;
    if (m_fRecoilTimer >= 0.0f)
        fRecoil = sinf((m_fRecoilTimer / RECOIL_DURATION) * XM_PI);

    const float fKickBack = fRecoil * 0.07f;
    const float fKickUp   = fRecoil * 0.04f;

    XMFLOAT3 armPos = {
        camPos.x + camRight.x * 0.38f - camUp.x * 0.22f + camLook.x * 0.1f,
        camPos.y + camRight.y * 0.38f - camUp.y * 0.22f + camLook.y * 0.1f,
        camPos.z + camRight.z * 0.38f - camUp.z * 0.22f + camLook.z * 0.1f
    };

    // 반동
    armPos.x += -camLook.x * fKickBack + camUp.x * fKickUp;
    armPos.y += -camLook.y * fKickBack + camUp.y * fKickUp;
    armPos.z += -camLook.z * fKickBack + camUp.z * fKickUp;

    // 팔 Y축(긴 축)

    XMFLOAT3 armZ = {
        camRight.y * camLook.z - camRight.z * camLook.y,
        camRight.z * camLook.x - camRight.x * camLook.z,
        camRight.x * camLook.y - camRight.y * camLook.x
    };

    XMFLOAT4X4 mWorld;
    mWorld._11 = camRight.x; mWorld._12 = camRight.y; mWorld._13 = camRight.z; mWorld._14 = 0.0f;
    mWorld._21 = camLook.x;  mWorld._22 = camLook.y;  mWorld._23 = camLook.z;  mWorld._24 = 0.0f;
    mWorld._31 = armZ.x;     mWorld._32 = armZ.y;     mWorld._33 = armZ.z;     mWorld._34 = 0.0f;
    mWorld._41 = armPos.x;   mWorld._42 = armPos.y;   mWorld._43 = armPos.z;   mWorld._44 = 1.0f;

    pArm->m_xmf4x4World = mWorld;
    pArm->Render(pd3dCommandList, pCamera);

    // 총 배치
    if ((int)m_BodyParts.size() > BP_GUN && m_BodyParts[BP_GUN].pObject)
    {
\
        XMFLOAT3 gunPos = {
            armPos.x + camLook.x * 0.50f,
            armPos.y + camLook.y * 0.50f,
            armPos.z + camLook.z * 0.50f
        };

        XMFLOAT4X4 mGun;
        mGun._11 = camRight.x; mGun._12 = camRight.y; mGun._13 = camRight.z; mGun._14 = 0.0f;
        mGun._21 = camUp.x;    mGun._22 = camUp.y;    mGun._23 = camUp.z;    mGun._24 = 0.0f;
        mGun._31 = camLook.x;  mGun._32 = camLook.y;  mGun._33 = camLook.z;  mGun._34 = 0.0f;
        mGun._41 = gunPos.x;   mGun._42 = gunPos.y;   mGun._43 = gunPos.z;   mGun._44 = 1.0f;

        m_BodyParts[BP_GUN].pObject->m_xmf4x4World = mGun;
        m_BodyParts[BP_GUN].pObject->Render(pd3dCommandList, pCamera);
    }
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
        m_pCamera->GenerateProjectionMatrix(0.1f, 200.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    case SPACESHIP_CAMERA:
        SetFriction(1.0f);
        SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
        SetMaxVelocityXZ(10.0f);
        SetMaxVelocityY(10.0f);
        m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentMode);
        m_pCamera->SetTimeLag(0.0f);
        m_pCamera->SetOffset(XMFLOAT3(0.0f, 2.0f, -5.0f));
        m_pCamera->GenerateProjectionMatrix(0.1f, 200.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    case THIRD_PERSON_CAMERA:
        m_fPitch   = 0.0f;
        m_xmf3Up   = XMFLOAT3(0.0f, 1.0f, 0.0f);
        m_xmf3Look.y = 0.0f;
        {
            float fLen = sqrtf(m_xmf3Look.x * m_xmf3Look.x + m_xmf3Look.z * m_xmf3Look.z);
            if (fLen > 0.001f) { m_xmf3Look.x /= fLen; m_xmf3Look.z /= fLen; }
            else               { m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f); }
        }
        m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));

        SetFriction(10.0f);
        SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
        SetMaxVelocityXZ(10.0f);
        SetMaxVelocityY(10.0f);
        m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentMode);
        m_pCamera->SetTimeLag(0.0f);
        m_pCamera->SetOffset(XMFLOAT3(0.0f, 4.0f, -8.0f));
        m_pCamera->GenerateProjectionMatrix(0.1f, 200.0f, ASPECT_RATIO, 60.0f);
        m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
        m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
        break;

    default:
        break;
    }

    if (m_pd3dDevice)
        m_pCamera->CreateShaderVariables(m_pd3dDevice, nullptr);

    OnPrepareRender();
    m_pCamera->Update(m_xmf3LookAt, fTimeElapsed);
    m_pCamera->SetLookAt(m_xmf3LookAt);
    m_pCamera->RegenerateViewMatrix();

    return m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice,
                                     ID3D12GraphicsCommandList* pd3dCommandList)
{
    m_pd3dDevice = pd3dDevice;

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
