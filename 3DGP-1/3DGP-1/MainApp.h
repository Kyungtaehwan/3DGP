#pragma once
#include "define.h"
#include "Timer.h"

class CMainApp
{
public:
	CMainApp();
	~CMainApp();

public:
	void Initialize(void);
	void Update(float dt);
	void Late_Update(float dt);
	void Render(void);
	void Release(void);

	CGameTimer		m_Timer;

private:

	HDC				m_hDC;

	TCHAR			m_szFPS[32];

	float m_fTimeAcc = 0.f;
	int   m_iFrameCount = 0;
};