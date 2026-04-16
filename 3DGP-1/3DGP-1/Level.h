#pragma once

class CLevel
{
public:
	CLevel();
	virtual ~CLevel();

public:
	virtual void Initialize() = 0;
	virtual int	 Update(float dt) = 0;
	virtual void Late_Update(float dt) = 0;
	virtual void Render(HDC hDC) = 0;
	virtual void Release(void) = 0;
};
