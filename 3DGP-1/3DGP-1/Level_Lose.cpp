#include "stdafx.h"
#include "Level_Lose.h"
#include "Bmp_Manager.h"
#include "Level_Manager.h"
#include "Input_Manager.h"

void CLevel_Lose::Initialize()
{
	CInput_Manager::Get_Instance()->SetMouseLock(false);
	if (!CBmp_Manager::Get_Instance()->Insert_Bmp(L"../Resource/BackGround.bmp", L"Back"))
		CBmp_Manager::Get_Instance()->Insert_Bmp(L"Resource/BackGround.bmp", L"Back");
	int nBtnW = 220, nBtnH = 60;
	m_btnMenu.Setup((WINCX - nBtnW) / 2, WINCY / 2 + 60,
		nBtnW, nBtnH, _T("BACK TO MENU"), RGB(30, 60, 80));
}

int CLevel_Lose::Update(float dt)
{
	CInput_Manager::Get_Instance()->Update_Mouse(g_hWnd);

	if (m_btnMenu.Update())
		CLevel_Manager::Get_Instance()->Level_Change(LEVEL_MENU);
	return 0;

}

void CLevel_Lose::Late_Update(float dt)
{
}

void CLevel_Lose::Render(HDC hDC)
{
	HDC	hMemDC = CBmp_Manager::Get_Instance()->Find_Img(L"Back");
	BitBlt(hDC, 0, 0, WINCX, WINCY, hMemDC, 0, 0, SRCCOPY);
	SetBkMode(hDC, TRANSPARENT);
	SetTextColor(hDC, RGB(255, 215, 0));
	HFONT hFont = CreateFont(70, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));
	HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

	RECT rcTitle = { 0, WINCY / 4, WINCX, WINCY / 2 };
	DrawText(hDC, _T("LOSE"), -1, &rcTitle,
		DT_CENTER | DT_VCENTER | DT_SINGLELINE);

	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
	m_btnMenu.Render(hDC);

}

void CLevel_Lose::Release()
{
}
