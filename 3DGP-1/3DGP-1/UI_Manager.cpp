#include "stdafx.h"
#include "UI_Manager.h"

CUI_Manager* CUI_Manager::m_pInstance = nullptr;

void CUI_Manager::DrawHealthBar(HDC hDC, int x, int y, int nWidth, int nHeight,
    int nHP, int nMaxHP, COLORREF barColor, COLORREF bgColor, LPCTSTR szLabel)
{
    // 배경
    HBRUSH hBgBrush = CreateSolidBrush(bgColor);
    RECT rcBg = { x, y, x + nWidth, y + nHeight };
    FillRect(hDC, &rcBg, hBgBrush);
    DeleteObject(hBgBrush);

    // HP 비율 계산
    float fRatio = (nMaxHP > 0) ? (float)nHP / (float)nMaxHP : 0.f;
    if (fRatio < 0.f) fRatio = 0.f;
    if (fRatio > 1.f) fRatio = 1.f;
    int nFillWidth = (int)(nWidth * fRatio);

    // HP 바
    if (nFillWidth > 0)
    {
        HBRUSH hBarBrush = CreateSolidBrush(barColor);
        RECT rcBar = { x, y, x + nFillWidth, y + nHeight };
        FillRect(hDC, &rcBar, hBarBrush);
        DeleteObject(hBarBrush);
    }

    // 테두리
    HPEN hPen = CreatePen(PS_SOLID, 2, RGB(255, 255, 255));
    HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
    Rectangle(hDC, x, y, x + nWidth, y + nHeight);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    DeleteObject(hPen);

    // 텍스트
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(255, 255, 255));
    RECT rcText = { x, y - 20, x + nWidth, y };
    DrawText(hDC, szLabel, -1, &rcText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    // HP 숫자
    TCHAR szHP[32];
    _stprintf_s(szHP, _T("%d / %d"), nHP, nMaxHP);
    RECT rcNum = { x, y, x + nWidth, y + nHeight };
    DrawText(hDC, szHP, -1, &rcNum, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

void CUI_Manager::Render(HDC hDC, int nPlayerHP, int nPlayerMaxHP,
    int nBossHP, int nBossMaxHP)
{
    // 보스 체력바 - 화면 상단 가운데
    int nBossBarW = 400;
    int nBossBarH = 25;
    int nBossBarX = (WINCX - nBossBarW) / 2;
    int nBossBarY = 20;
    DrawHealthBar(hDC, nBossBarX, nBossBarY, nBossBarW, nBossBarH,
        nBossHP, nBossMaxHP,
        RGB(220, 30, 30),    // 빨강
        RGB(60, 10, 10),     // 어두운 빨강 배경
        _T("BOSS"));

    // 플레이어 체력바 - 화면 왼쪽 하단
    int nPlayerBarW = 250;
    int nPlayerBarH = 20;
    int nPlayerBarX = 20;
    int nPlayerBarY = WINCY - 60;
    DrawHealthBar(hDC, nPlayerBarX, nPlayerBarY, nPlayerBarW, nPlayerBarH,
        nPlayerHP, nPlayerMaxHP,
        RGB(30, 180, 30),    // 초록
        RGB(10, 50, 10),     // 어두운 초록 배경
        _T("PLAYER"));
}