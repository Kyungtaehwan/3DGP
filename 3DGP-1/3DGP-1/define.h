#pragma once

#define WINCX   1280
#define WINCY   720

#define OBJ_NOEVENT 0
#define OBJ_DEAD    1
#define EXPLOSION_DEBRISES 30

enum LEVEL_ID { LEVEL_MENU, LEVEL_GAMEPLAY, LEVEL_WIN,LEVEL_LOSE ,LEVEL_END };
enum OBJ_ID {OBJ_TERRAIN,OBJ_WALL, OBJ_PLAYER_BULLET, OBJ_BOSS_BULLET, OBJ_BOSS,OBJ_PLAYER,OBJ_END};

template<typename T>
void Safe_Delete(T& Temp)
{
	if (Temp)
	{
		delete Temp;
		Temp = nullptr;
	}
}

class CDeleteMap
{
public:
	template<typename T>
	void operator()(T& MyPair)
	{
		if (MyPair.second)
		{
			delete MyPair.second;
			MyPair.second = nullptr;
		}
	}
};



class CTagFinder
{
public:
	CTagFinder(const TCHAR* pKey) : m_pKey(pKey) {}

public:
	template<typename T>
	bool	operator()(T& Pair)
	{
		if (!lstrcmp(m_pKey, Pair.first))
			return true;

		return false;
	}

private:
	const TCHAR* m_pKey;
};
