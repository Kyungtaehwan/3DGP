#include "stdafx.h"
#include "Level_GamePlay.h"
#include "Object_Manager.h"
#include "GraphicsPipeline.h"
#include "Bmp_Manager.h"
#include "Player.h"
#include "Camera.h"
#include "Terrain.h"
#include "BossMonster.h"
#include "Bullet.h"
#include "Wall.h"

CLevel_GamePlay::CLevel_GamePlay() {}
CLevel_GamePlay::~CLevel_GamePlay() { Release(); }

void CLevel_GamePlay::Initialize()
{

    CBmp_Manager::Get_Instance()->Insert_Bmp(L"../Resource/BackGround.bmp", L"Back");
    
    
    CTerrain* m_pTerrain = new CTerrain();
    m_pTerrain->Initialize();
    CObject_Manager::Get_Instance()->Add_Object(OBJ_TERRAIN, m_pTerrain);
    
    // 플레이어 생성
    m_pPlayer = new CPlayer();
    m_pPlayer->Initialize();
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, m_pPlayer);

    // 카메라 생성 - 플레이어 뒤 10, 위 5 위치
    m_pCamera = new CCamera(m_pPlayer, XMFLOAT3(0.0f, 15.0f, 30.0f));
    m_pCamera->SetViewport(0, 0, WINCX, WINCY);
    m_pCamera->GeneratePerspectiveProjectionMatrix(0.1f, 600.f, 60.f);
    m_pCamera->GenerateViewMatrix(); // 초기 뷰 행렬
    CObject_Manager::Get_Instance()->SetCamera(m_pCamera);
    CBullet::PrepareExplosion();
    Set_GameWorld();
}

int CLevel_GamePlay::Update(float dt)
{
    CObject_Manager::Get_Instance()->Update(dt);
    return 0;
}

void CLevel_GamePlay::Late_Update(float dt)
{
    CObject_Manager::Get_Instance()->Late_Update(dt);
    m_pCamera->GenerateViewMatrix(); // 기존 GenerateViewMatrix() 대신
}

void CLevel_GamePlay::Render(HDC hDC)
{
    // 파이프라인에 카메라 행렬 세팅
    CGraphicsPipeline::SetViewPerspectiveProjectTransform(m_pCamera->GetViewProjectMatrix());
    CGraphicsPipeline::SetViewport(m_pCamera->GetViewport());

    HDC	hMemDC = CBmp_Manager::Get_Instance()->Find_Img(L"Back");
    BitBlt(hDC, 0, 0, WINCX, WINCY, hMemDC, 0, 0, SRCCOPY);
    CObject_Manager::Get_Instance()->Render(hDC);
}

void CLevel_GamePlay::Release()
{
    CBullet::ReleaseExplosion();
    if (m_pCamera) { delete m_pCamera; m_pCamera = nullptr; }
    CObject_Manager::Get_Instance()->Release();
}

void CLevel_GamePlay::Set_GameWorld()
{
    CBossMonster* pBoss = new CBossMonster();
    pBoss->Initialize();
    pBoss->SetPlayer(m_pPlayer);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_BOSS, pBoss);

    auto AddWall = [](float x, float z, float w, float h, float d)
        {
            CWall* pWall = new CWall();
            pWall->Initialize();
            pWall->SetupWall(x, z, w, h, d);
            CObject_Manager::Get_Instance()->Add_Object(OBJ_WALL, pWall);
        };

    // ── 보스 감금 구역 (14칸 = 140유닛, 높이 1) ──────────────
    // 북쪽 벽 (+Z)
    for (float x = -35.f; x <= 35.f; x += 10.f)
        AddWall(x, +45.f, 10.f, 5.f, 10.f);
    // 남쪽 벽 (-Z)
    for (float x = -35.f; x <= 35.f; x += 10.f)
        AddWall(x, -45.f, 10.f, 5.f, 10.f);
    // 서쪽 벽 (-X)
    for (float z = -35.f; z <= 35.f; z += 10.f)
        AddWall(-45.f, z, 10.f, 5.f, 10.f);
    // 동쪽 벽 (+X)
    for (float z = -35.f; z <= 35.f; z += 10.f)
        AddWall(+45.f, z, 10.f, 5.f, 10.f);

    // ── 맵 테두리 (높이 3) ────────────────────────────────────
    // 북쪽 (+Z = +245)
    //for (float x = -245.f; x <= 245.f; x += 10.f)
    //    AddWall(x, +245.f, 10.f, 30.f, 10.f);
    //// 남쪽 (-Z = -245)
    //for (float x = -245.f; x <= 245.f; x += 10.f)
    //    AddWall(x, -245.f, 10.f, 30.f, 10.f);
    //// 서쪽 (-X = -245)
    //for (float z = -235.f; z <= 235.f; z += 10.f)
    //    AddWall(-245.f, z, 10.f, 30.f, 10.f);
    //// 동쪽 (+X = +245)
    //for (float z = -235.f; z <= 235.f; z += 10.f)
    //    AddWall(+245.f, z, 10.f, 30.f, 10.f);

    // ── 내부 엄폐물 (보스 구역 밖, 높이 8) ───────────────────
    // 플레이어 시작(0,0,-150) 주변
// 남쪽 구역 (z: -235 ~ -85)
    AddWall(-105.f, -235.f, 10.f, 8.f, 10.f);
    AddWall(85.f, -235.f, 10.f, 8.f, 10.f);
    AddWall(-185.f, -215.f, 10.f, 8.f, 10.f);
    AddWall(165.f, -215.f, 10.f, 8.f, 10.f);
    AddWall(-25.f, -205.f, 10.f, 8.f, 10.f);
    AddWall(25.f, -205.f, 10.f, 8.f, 10.f);
    AddWall(-135.f, -185.f, 10.f, 8.f, 10.f);
    AddWall(115.f, -185.f, 10.f, 8.f, 10.f);
    AddWall(-55.f, -175.f, 10.f, 8.f, 10.f);
    AddWall(55.f, -175.f, 10.f, 8.f, 10.f);
    AddWall(-195.f, -165.f, 10.f, 8.f, 10.f);
    AddWall(175.f, -165.f, 10.f, 8.f, 10.f);
    AddWall(-15.f, -155.f, 10.f, 8.f, 10.f);
    AddWall(15.f, -155.f, 10.f, 8.f, 10.f);
    AddWall(-105.f, -145.f, 10.f, 8.f, 10.f);
    AddWall(85.f, -145.f, 10.f, 8.f, 10.f);
    AddWall(-75.f, -135.f, 10.f, 8.f, 10.f);
    AddWall(75.f, -135.f, 10.f, 8.f, 10.f);
    AddWall(-155.f, -125.f, 10.f, 8.f, 10.f);
    AddWall(145.f, -125.f, 10.f, 8.f, 10.f);
    AddWall(-35.f, -115.f, 10.f, 8.f, 10.f);
    AddWall(35.f, -115.f, 10.f, 8.f, 10.f);
    AddWall(-115.f, -105.f, 10.f, 8.f, 10.f);
    AddWall(105.f, -105.f, 10.f, 8.f, 10.f);
    AddWall(-55.f, -95.f, 10.f, 8.f, 10.f);
    AddWall(55.f, -95.f, 10.f, 8.f, 10.f);
    AddWall(-175.f, -95.f, 10.f, 8.f, 10.f);
    AddWall(165.f, -95.f, 10.f, 8.f, 10.f);

    // 북쪽 구역 (z: +85 ~ +235)
    AddWall(-105.f, 235.f, 10.f, 8.f, 10.f);
    AddWall(85.f, 235.f, 10.f, 8.f, 10.f);
    AddWall(-185.f, 215.f, 10.f, 8.f, 10.f);
    AddWall(165.f, 215.f, 10.f, 8.f, 10.f);
    AddWall(-25.f, 205.f, 10.f, 8.f, 10.f);
    AddWall(25.f, 205.f, 10.f, 8.f, 10.f);
    AddWall(-135.f, 185.f, 10.f, 8.f, 10.f);
    AddWall(115.f, 185.f, 10.f, 8.f, 10.f);
    AddWall(-55.f, 175.f, 10.f, 8.f, 10.f);
    AddWall(55.f, 175.f, 10.f, 8.f, 10.f);
    AddWall(-195.f, 165.f, 10.f, 8.f, 10.f);
    AddWall(175.f, 165.f, 10.f, 8.f, 10.f);
    AddWall(-15.f, 155.f, 10.f, 8.f, 10.f);
    AddWall(15.f, 155.f, 10.f, 8.f, 10.f);
    AddWall(-105.f, 145.f, 10.f, 8.f, 10.f);
    AddWall(85.f, 145.f, 10.f, 8.f, 10.f);
    AddWall(-75.f, 135.f, 10.f, 8.f, 10.f);
    AddWall(75.f, 135.f, 10.f, 8.f, 10.f);
    AddWall(-155.f, 125.f, 10.f, 8.f, 10.f);
    AddWall(145.f, 125.f, 10.f, 8.f, 10.f);
    AddWall(-35.f, 115.f, 10.f, 8.f, 10.f);
    AddWall(35.f, 115.f, 10.f, 8.f, 10.f);
    AddWall(-115.f, 105.f, 10.f, 8.f, 10.f);
    AddWall(105.f, 105.f, 10.f, 8.f, 10.f);
    AddWall(-55.f, 95.f, 10.f, 8.f, 10.f);
    AddWall(55.f, 95.f, 10.f, 8.f, 10.f);
    AddWall(-175.f, 95.f, 10.f, 8.f, 10.f);
    AddWall(165.f, 95.f, 10.f, 8.f, 10.f);

    // 서쪽 구역 (x: -235 ~ -85, z: -75 ~ +75)
    AddWall(-235.f, -55.f, 10.f, 8.f, 10.f);
    AddWall(-215.f, 25.f, 10.f, 8.f, 10.f);
    AddWall(-195.f, -35.f, 10.f, 8.f, 10.f);
    AddWall(-175.f, 55.f, 10.f, 8.f, 10.f);
    AddWall(-155.f, -15.f, 10.f, 8.f, 10.f);
    AddWall(-135.f, 35.f, 10.f, 8.f, 10.f);
    AddWall(-115.f, -45.f, 10.f, 8.f, 10.f);
    AddWall(-95.f, 15.f, 10.f, 8.f, 10.f);

    // 동쪽 구역 (x: +85 ~ +235, z: -75 ~ +75)
    AddWall(235.f, -55.f, 10.f, 8.f, 10.f);
    AddWall(215.f, 25.f, 10.f, 8.f, 10.f);
    AddWall(195.f, -35.f, 10.f, 8.f, 10.f);
    AddWall(175.f, 55.f, 10.f, 8.f, 10.f);
    AddWall(155.f, -15.f, 10.f, 8.f, 10.f);
    AddWall(135.f, 35.f, 10.f, 8.f, 10.f);
    AddWall(115.f, -45.f, 10.f, 8.f, 10.f);
    AddWall(95.f, 15.f, 10.f, 8.f, 10.f);

}

