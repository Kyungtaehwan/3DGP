#include "stdafx.h"
#include "MainApp.h"
#include "Level_Manager.h"
#include "Bmp_Manager.h"
#include "Input_Manager.h"

CMainApp::CMainApp()
{
}

CMainApp::~CMainApp()
{
}

void CMainApp::Initialize(void)
{
	m_hDC = GetDC(g_hWnd);
	m_Timer.Reset(); 

	CBmp_Manager::Get_Instance()->Insert_Bmp(L"../Resource/Back_Buffer.bmp", L"BackBuffer");
	CLevel_Manager::Get_Instance()->Level_Change(LEVEL_GAMEPLAY);
}

void CMainApp::Update(float dt)
{
	CInput_Manager::Get_Instance()->Update_Mouse(g_hWnd);
	CLevel_Manager::Get_Instance()->Update(dt);
}

void CMainApp::Late_Update(float dt)
{
	CLevel_Manager::Get_Instance()->Late_Update(dt);
}

void CMainApp::Render(void)
{
	HDC	hBackDC = CBmp_Manager::Get_Instance()->Find_Img(L"BackBuffer");
	CLevel_Manager::Get_Instance()->Render(hBackDC);
	BitBlt(m_hDC, 0, 0, WINCX, WINCY, hBackDC, 0, 0, SRCCOPY);
}

void CMainApp::Release(void)
{
}
