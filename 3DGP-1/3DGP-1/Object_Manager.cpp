#include "stdafx.h"
#include "Object_Manager.h"
#include "Camera.h"
#include "Bullet.h"
#include "Player.h"
#include "BossMonster.h"


CObject_Manager* CObject_Manager::m_pInstance = nullptr;

CObject_Manager::CObject_Manager()
{
}

CObject_Manager::~CObject_Manager()
{
	Release();
}

void CObject_Manager::Add_Object(OBJ_ID eID, CGameObject* pGameObject)
{
	if (OBJ_END <= eID || nullptr == pGameObject)
		return;

	m_ObjectList[eID].push_back(pGameObject);
}

int CObject_Manager::Update(float dt)
{
	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for (auto iter = m_ObjectList[i].begin();
			iter != m_ObjectList[i].end(); )
		{
			int iResult = (*iter)->Update(dt);

			if (OBJ_DEAD == iResult)
			{
				Safe_Delete<CGameObject*>(*iter);
				iter = m_ObjectList[i].erase(iter);
			}
			else
				++iter;
		}
	}

	return 0;
}

void CObject_Manager::Late_Update(float dt)
{
	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for (auto& iter : m_ObjectList[i])
		{
			iter->Late_Update(dt);

			if (m_ObjectList[i].empty())
				break;

		}
	}

    CheckCollisions();
}

void CObject_Manager::Render(HDC hDC)
{
    for (auto& pObj : m_ObjectList[OBJ_TERRAIN])
        if (pObj) pObj->Render(hDC);

    std::vector<std::pair<float, CGameObject*>> sortedObjects;

    XMFLOAT3 camPos = { 0.f, 0.f, 0.f };
    XMFLOAT3 camLook = { 0.f, 0.f, 1.f };
    if (m_pCamera)
    {
        camPos = m_pCamera->GetPosition();
        camLook = m_pCamera->GetLook();
    }

    float fPlayerDepth = -9999.f;
    float fBossDepth = -9999.f;

    for (size_t i = 0; i < OBJ_END; ++i)
    {
        if (i == OBJ_TERRAIN) continue;

        for (auto& pObj : m_ObjectList[i])
        {
            if (!pObj) continue;

            XMFLOAT3 objPos = {
                pObj->m_xmf4x4World._41,
                pObj->m_xmf4x4World._42,
                pObj->m_xmf4x4World._43
            };

            float dx = objPos.x - camPos.x;
            float dy = objPos.y - camPos.y;
            float dz = objPos.z - camPos.z;
            float fDepth = dx * camLook.x + dy * camLook.y + dz * camLook.z;

            if (i == OBJ_PLAYER) fPlayerDepth = fDepth;
            if (i == OBJ_BOSS)   fBossDepth = fDepth;

            sortedObjects.push_back({ fDepth, pObj });
        }
    }

    sort(sortedObjects.begin(), sortedObjects.end(),
        [](const std::pair<float, CGameObject*>& a, const std::pair<float, CGameObject*>& b)
        { return a.first > b.first; });

    for (auto& p : sortedObjects)
        p.second->Render(hDC);

}
void CObject_Manager::Release(void)
{

	for (size_t i = 0; i < OBJ_END; ++i)
	{
		for_each(m_ObjectList[i].begin(), m_ObjectList[i].end(), Safe_Delete<CGameObject*>);
		m_ObjectList[i].clear();
	}
}

void CObject_Manager::DeleteID(OBJ_ID eID)
{
	for (auto& iter : m_ObjectList[eID])
		Safe_Delete(iter);

	m_ObjectList[eID].clear();
}

void CObject_Manager::CheckCollisions()
{
    // 플레이어 총알  보스
    for (auto& pBullet : m_ObjectList[OBJ_PLAYER_BULLET])
    {
        if (!pBullet || !pBullet->m_bActive) continue;
        for (auto& pBoss : m_ObjectList[OBJ_BOSS])
        {
            if (!pBoss) continue;
            if (pBullet->m_xmOOBB.Intersects(pBoss->m_xmOOBB))
            {
                pBoss->OnHit(10);
                ((CBullet*)pBullet)->SetBlowingUp();
            }
        }
    }

    // 보스 총알  플레이어
    for (auto& pBullet : m_ObjectList[OBJ_BOSS_BULLET])
    {
        if (!pBullet || !pBullet->m_bActive) continue;
        for (auto& pPlayer : m_ObjectList[OBJ_PLAYER])
        {
            if (!pPlayer) continue;
            if (pBullet->m_xmOOBB.Intersects(pPlayer->m_xmOOBB))
            {
                pPlayer->OnHit(10);
                ((CBullet*)pBullet)->SetBlowingUp();
            }
        }
    }

    // 플레이어 총알 벽
    for (auto& pBullet : m_ObjectList[OBJ_PLAYER_BULLET])
    {
        if (!pBullet || !pBullet->m_bActive) continue;
        for (auto& pWall : m_ObjectList[OBJ_WALL])
        {
            if (!pWall) continue;
            if (pBullet->m_xmOOBB.Intersects(pWall->m_xmOOBB))
            {
                ((CBullet*)pBullet)->SetBlowingUp();
                break;
            }
        }
    }

    // 보스 총알 벽
    for (auto& pBullet : m_ObjectList[OBJ_BOSS_BULLET])
    {
        if (!pBullet || !pBullet->m_bActive) continue;
        for (auto& pWall : m_ObjectList[OBJ_WALL])
        {
            if (!pWall) continue;
            if (pBullet->m_xmOOBB.Intersects(pWall->m_xmOOBB))
            {
                ((CBullet*)pBullet)->SetBlowingUp();
                break;
            }
        }
    }

    // 플레이어  벽
    for (auto& pPlayer : m_ObjectList[OBJ_PLAYER])
    {
        if (!pPlayer) continue;
        CPlayer* pP = (CPlayer*)pPlayer;

        for (auto& pWall : m_ObjectList[OBJ_WALL])
        {
            if (!pWall) continue;
            if (pPlayer->m_xmOOBB.Intersects(pWall->m_xmOOBB))
            {
                // 벽 중심 → 플레이어 방향을 법선으로 사용
                XMFLOAT3 wallPos = {
                    pWall->m_xmf4x4World._41,
                    pWall->m_xmf4x4World._42,
                    pWall->m_xmf4x4World._43
                };
                XMFLOAT3 playerPos = pP->GetPosition();
                XMFLOAT3 normal = {
                    playerPos.x - wallPos.x,
                    0.f,
                    playerPos.z - wallPos.z
                };
                float len = sqrtf(normal.x * normal.x + normal.z * normal.z);
                if (len > 0.f)
                {
                    normal.x /= len;
                    normal.z /= len;
                }
                pP->ReflectVelocity(normal);
            }
        }
    }
}