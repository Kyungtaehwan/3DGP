#include "stdafx.h"
#include "Button.h"
#include "Input_Manager.h"

void CButton::Setup(int x, int y, int nWidth, int nHeight,
    LPCTSTR szText, DWORD dwColor)
{
    m_rc = { x, y, x + nWidth, y + nHeight };
    m_dwColor = dwColor;
    m_dwHoverColor = RGB(
        min(255, GetRValue(dwColor) + 50),
        min(255, GetGValue(dwColor) + 50),
        min(255, GetBValue(dwColor) + 50));
    _tcscpy_s(m_szText, szText);
}

bool CButton::Update()
{
    auto* pInput = CInput_Manager::Get_Instance();
    POINT pt = pInput->GetMousePos();

    m_bHover = PtInRect(&m_rc, pt);

    if (m_bHover && pInput->Key_Down(VK_LBUTTON))
        return true;

    return false;
}

void CButton::Render(HDC hDC)
{
    // 배경
    DWORD col = m_bHover ? m_dwHoverColor : m_dwColor;
    HBRUSH hBrush = CreateSolidBrush(col);
    FillRect(hDC, &m_rc, hBrush);
    DeleteObject(hBrush);

    // 테두리
    HPEN hPen = CreatePen(PS_SOLID, 2,
        m_bHover ? RGB(255, 255, 255) : RGB(150, 150, 150));
    HPEN hOldPen = (HPEN)SelectObject(hDC, hPen);
    HBRUSH hOldBrush = (HBRUSH)SelectObject(hDC, GetStockObject(NULL_BRUSH));
    Rectangle(hDC, m_rc.left, m_rc.top, m_rc.right, m_rc.bottom);
    SelectObject(hDC, hOldPen);
    SelectObject(hDC, hOldBrush);
    DeleteObject(hPen);

    // 텍스트
    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, m_bHover ? RGB(255, 255, 0) : RGB(255, 255, 255));

    HFONT hFont = CreateFont(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH, _T("Arial"));
    HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);

    DrawText(hDC, m_szText, -1, &m_rc,
        DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hDC, hOldFont);
    DeleteObject(hFont);
}