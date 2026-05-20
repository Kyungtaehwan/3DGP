#include "pch.h"
#include "Object_Manager.h"
#include "Camera.h"

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
            if (pObj->m_bActive)
                pObj->Render(pd3dCommandList, pCamera);
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
    // Basic sphere collision: player bullets vs enemies
    auto& bulletList = m_ObjectList[OBJ_PLAYER_BULLET];
    auto& enemyList  = m_ObjectList[OBJ_ENEMY];

    const float fCollisionRadius = 1.5f; // Simple radius for cube-sized objects

    for (auto itBullet = bulletList.begin(); itBullet != bulletList.end(); )
    {
        CGameObject* pBullet = *itBullet;
        bool bHit = false;

        for (auto itEnemy = enemyList.begin(); itEnemy != enemyList.end(); )
        {
            CGameObject* pEnemy = *itEnemy;

            float fDist = Vector3::Distance(pBullet->GetPosition(), pEnemy->GetPosition());
            if (fDist < fCollisionRadius * 2.0f)
            {
                pEnemy->OnHit(25);
                bHit = true;

                if (pEnemy->IsDead())
                {
                    delete pEnemy;
                    itEnemy = enemyList.erase(itEnemy);
                }
                else
                {
                    ++itEnemy;
                }
                break; // Bullet hits one enemy per frame
            }
            else
            {
                ++itEnemy;
            }
        }

        if (bHit)
        {
            delete pBullet;
            itBullet = bulletList.erase(itBullet);
        }
        else
        {
            ++itBullet;
        }
    }
}
