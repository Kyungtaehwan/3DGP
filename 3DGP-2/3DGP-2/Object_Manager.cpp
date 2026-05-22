#include "pch.h"
#include "Object_Manager.h"
#include "Camera.h"
#include "Bullet.h"

// Static member definition
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
    if (eID < 0 || eID >= OBJ_END) return;
    m_ObjectList[eID].push_back(pGameObject);
}

int CObject_Manager::Update(float dt)
{
    for (int i = 0; i < OBJ_END; i++)
    {
        auto& list = m_ObjectList[i];
        for (auto it = list.begin(); it != list.end(); )
        {
            CGameObject* pObj = *it;
            int nResult = pObj->Update(dt);

            if (nResult == OBJ_DEAD || pObj->IsDead())
            {
                delete pObj;
                it = list.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
    return OBJ_NOEVENT;
}

void CObject_Manager::Late_Update(float dt)
{
    for (int i = 0; i < OBJ_END; i++)
    {
        for (CGameObject* pObj : m_ObjectList[i])
        {
            pObj->Late_Update(dt);
        }
    }
}

void CObject_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    for (int i = 0; i < OBJ_END; i++)
    {
        for (CGameObject* pObj : m_ObjectList[i])
        {
            // Each object's Render() decides its own visibility (e.g. bullets render explosion even when inactive)
            if (pObj) pObj->Render(pd3dCommandList, pCamera);
        }
    }
}

void CObject_Manager::Release()
{
    for (int i = 0; i < OBJ_END; i++)
    {
        for (CGameObject* pObj : m_ObjectList[i])
        {
            delete pObj;
        }
        m_ObjectList[i].clear();
    }
}

void CObject_Manager::ReleaseUploadBuffers()
{
    for (int i = 0; i < OBJ_END; i++)
    {
        for (CGameObject* pObj : m_ObjectList[i])
        {
            pObj->ReleaseUploadBuffers();
        }
    }
}

void CObject_Manager::DeleteID(OBJ_ID eID)
{
    if (eID < 0 || eID >= OBJ_END) return;
    for (CGameObject* pObj : m_ObjectList[eID])
    {
        delete pObj;
    }
    m_ObjectList[eID].clear();
}

void CObject_Manager::CheckCollisions()
{
    auto& playerList      = m_ObjectList[OBJ_PLAYER];
    auto& enemyList       = m_ObjectList[OBJ_ENEMY];
    auto& playerBullets   = m_ObjectList[OBJ_PLAYER_BULLET];
    auto& enemyBullets    = m_ObjectList[OBJ_ENEMY_BULLET];

    for (auto itB = playerBullets.begin(); itB != playerBullets.end(); )
    {
        CGameObject* pBullet = *itB;
        if (!pBullet || !pBullet->m_bActive) { ++itB; continue; }

        bool bHit = false;
        for (auto itE = enemyList.begin(); itE != enemyList.end(); )
        {
            CGameObject* pEnemy = *itE;
            if (!pEnemy) { ++itE; continue; }

            if (pBullet->m_xmOOBB.Intersects(pEnemy->m_xmOOBB))
            {
                pEnemy->OnHit(25);
                bHit = true;
                if (pEnemy->IsDead())
                {
                    delete pEnemy;
                    itE = enemyList.erase(itE);
                }
                else ++itE;
                break;
            }
            else ++itE;
        }

        if (bHit) { static_cast<CBullet*>(pBullet)->SetBlowingUp(); ++itB; }
        else ++itB;
    }

    for (auto itB = enemyBullets.begin(); itB != enemyBullets.end(); )
    {
        CGameObject* pBullet = *itB;
        if (!pBullet || !pBullet->m_bActive) { ++itB; continue; }

        bool bHit = false;
        for (auto& pPlayer : playerList)
        {
            if (!pPlayer) continue;
            if (pBullet->m_xmOOBB.Intersects(pPlayer->m_xmOOBB))
            {
                pPlayer->OnHit(10);
                bHit = true;
                break;
            }
        }

        if (bHit) { delete pBullet; itB = enemyBullets.erase(itB); }
        else ++itB;
    }
}
