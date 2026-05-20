#include "pch.h"
#include "Level_GamePlay.h"
#include "Object_Manager.h"
#include "Player.h"
#include "Mesh.h"



void CLevel_GamePlay::Initialize(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  ID3D12RootSignature* pd3dRootSignature)
{
    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    CPlayer* pPlayer = new CPlayer();
    pPlayer->CreateBodyParts(pd3dDevice, pd3dCommandList, m_pShader);

    // 3인칭 카메라 생성 (플레이어가 화면에 보임)
    m_pCamera = pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

    pPlayer->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, pPlayer);

    CInput_Manager::Get_Instance()->SetMouseLock(true);

    // 이동이 눈에 보이도록 바닥과 기준 기둥 추가
    auto addStatic = [&](float x, float y, float z,
                         float w, float h, float d, XMFLOAT4 color)
    {
        CGameObject* pObj = new CGameObject();
        pObj->SetShader(m_pShader);
        pObj->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList, w, h, d, color));
        pObj->CreateShaderVariables(pd3dDevice, pd3dCommandList);
        pObj->SetPosition(x, y, z);
        m_StaticObjects.push_back(pObj);
    };

    // 바닥 (40x40 회색 평면)
    addStatic(0.0f, -0.05f, 0.0f, 40.0f, 0.1f, 40.0f, { 0.35f, 0.35f, 0.35f, 1.0f });

    // 앞쪽 방향 마커 (빨간 기둥) - 플레이어 초기 Look 방향
    addStatic(0.0f, 1.5f, 15.0f, 0.5f, 3.0f, 0.5f, { 1.0f, 0.2f, 0.2f, 1.0f });

    // 사방 모서리 기둥 (주황색)
    XMFLOAT4 pillar = { 0.9f, 0.5f, 0.1f, 1.0f };
    addStatic(-10.0f, 1.5f,  10.0f, 0.5f, 3.0f, 0.5f, pillar);
    addStatic( 10.0f, 1.5f,  10.0f, 0.5f, 3.0f, 0.5f, pillar);
    addStatic(-10.0f, 1.5f, -10.0f, 0.5f, 3.0f, 0.5f, pillar);
    addStatic( 10.0f, 1.5f, -10.0f, 0.5f, 3.0f, 0.5f, pillar);
}

int CLevel_GamePlay::Update(float dt)
{
    // 입력과 카메라 갱신은 CPlayer::Update 내부에서 처리
    CObject_Manager::Get_Instance()->Update(dt);
    CObject_Manager::Get_Instance()->CheckCollisions();
    return 0;
}

void CLevel_GamePlay::Late_Update(float dt)
{
    CObject_Manager::Get_Instance()->Late_Update(dt);
}

void CLevel_GamePlay::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    // 카메라 전환이 일어나도 항상 현재 플레이어 카메라를 사용
    CGameObject* pPlayerObj = CObject_Manager::Get_Instance()->Get_Player();
    if (pPlayerObj)
        m_pCamera = static_cast<CPlayer*>(pPlayerObj)->GetCamera();

    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);

    for (CGameObject* pObj : m_StaticObjects)
        pObj->Render(pd3dCommandList, m_pCamera);
}

void CLevel_GamePlay::Release()
{
    for (CGameObject* pObj : m_StaticObjects)
        if (pObj) delete pObj;
    m_StaticObjects.clear();

    CObject_Manager::Destroy_Instance();

    if (m_pShader)
    {
        delete m_pShader;
        m_pShader = NULL;
    }
}

void CLevel_GamePlay::ReleaseUploadBuffers()
{
    for (CGameObject* pObj : m_StaticObjects)
        pObj->ReleaseUploadBuffers();

    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();
}
