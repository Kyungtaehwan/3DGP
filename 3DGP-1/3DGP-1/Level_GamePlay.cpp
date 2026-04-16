#include "stdafx.h"
#include "Level_GamePlay.h"
#include "Object_Manager.h"
#include "GraphicsPipeline.h"
#include "Bmp_Manager.h"

CLevel_GamePlay::CLevel_GamePlay() {}
CLevel_GamePlay::~CLevel_GamePlay() { Release(); }

void CLevel_GamePlay::Initialize()
{

    CBmp_Manager::Get_Instance()->Insert_Bmp(L"../Resource/BackGround.bmp", L"Back");
    // 플레이어 생성
    m_pPlayer = new CPlayer();
    m_pPlayer->Initialize();
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, m_pPlayer);

    // 카메라 생성 - 플레이어 뒤 10, 위 5 위치
    m_pCamera = new CCamera(m_pPlayer, XMFLOAT3(0.0f, 5.0f, -15.0f));
    m_pCamera->SetViewport(0, 0, WINCX, WINCY);
    m_pCamera->GeneratePerspectiveProjectionMatrix(1.f, 500.f, 60.f);
    m_pCamera->GenerateViewMatrix(); // 초기 뷰 행렬
}

int CLevel_GamePlay::Update(float dt)
{
    CObject_Manager::Get_Instance()->Update(dt);
    return 0;
}

void CLevel_GamePlay::Late_Update(float dt)
{
    CObject_Manager::Get_Instance()->Late_Update(dt);
    m_pCamera->GenerateViewMatrix_Fixed(); // 기존 GenerateViewMatrix() 대신
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
    if (m_pCamera) { delete m_pCamera; m_pCamera = nullptr; }
    CObject_Manager::Get_Instance()->Release();
}