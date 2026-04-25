#pragma once
#include "define.h"

class CUI_Manager
{
public:
    static CUI_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new CUI_Manager();
        return m_pInstance;
    }
    static void Destroy_Instance()
    {
        if (m_pInstance) { delete m_pInstance; m_pInstance = nullptr; }
    }

    void Render(HDC hDC, int nPlayerHP, int nPlayerMaxHP, int nBossHP, int nBossMaxHP);

private:
    CUI_Manager() {}
    ~CUI_Manager() {}

    void DrawHealthBar(HDC hDC, int x, int y, int nWidth, int nHeight,
        int nHP, int nMaxHP, COLORREF barColor, COLORREF bgColor, LPCTSTR szLabel);

    static CUI_Manager* m_pInstance;
};