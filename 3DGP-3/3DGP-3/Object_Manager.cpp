#include "pch.h"
#include "Object_Manager.h"
#include "Bullet.h"
#include "Tank.h"
#include "Humvee.h"
#include "Player.h"

CObject_Manager* CObject_Manager::m_pInstance = nullptr;

CObject_Manager::CObject_Manager() {}
CObject_Manager::~CObject_Manager() { Release(); }

void CObject_Manager::Add_Object(OBJ_ID eID, CGameObject* pGameObject)
{
    if(eID < 0 || eID >= OBJ_END)
        return;
    m_ObjectList[eID].push_back(pGameObject);
}

void CObject_Manager::Update(float dt)
{
    for(int i = 0; i < OBJ_END; i++)
    {
        for(auto iter = m_ObjectList[i].begin(); iter != m_ObjectList[i].end();)
        {
            CGameObject* pObj = *iter;
            int iResult;
            if(pObj)
                iResult = pObj->Update(dt);
            else
                iResult = OBJ_DEAD;

            if(iResult == OBJ_DEAD)
            {
                if(pObj)
                    delete pObj;
                iter = m_ObjectList[i].erase(iter);
            }
            else
                ++iter;
        }
    }

    CheckCollisions();
}

void CObject_Manager::CheckCollisions()
{
    for(CGameObject* pBullet : m_ObjectList[OBJ_PLAYER_BULLET])
    {
        if(!pBullet || !pBullet->m_bActive)
            continue;

        bool bHit = false;
        for(CGameObject* pTank : m_ObjectList[OBJ_TANK])
        {
            if(!pTank || !pTank->m_bActive)
                continue;
            if(pBullet->m_xmOOBB.Intersects(pTank->m_xmOOBB))
            {
                static_cast<CTank*>(pTank)->OnHit(1);
                static_cast<CBullet*>(pBullet)->Kill();
                bHit = true;
                break;
            }
        }
        if(bHit)
            continue;

        for(CGameObject* pHumvee : m_ObjectList[OBJ_HUMVEE])
        {
            if(!pHumvee || !pHumvee->m_bActive)
                continue;
            if(pBullet->m_xmOOBB.Intersects(pHumvee->m_xmOOBB))
            {
                static_cast<CHumvee*>(pHumvee)->OnHit(1);
                static_cast<CBullet*>(pBullet)->Kill();
                break;
            }
        }
    }

    for(CGameObject* pBullet : m_ObjectList[OBJ_ENEMY_BULLET])
    {
        if(!pBullet || !pBullet->m_bActive)
            continue;
        for(CGameObject* pPlayer : m_ObjectList[OBJ_PLAYER])
        {
            if(!pPlayer || !pPlayer->m_bActive)
                continue;
            if(pBullet->m_xmOOBB.Intersects(pPlayer->m_xmOOBB))
            {
                static_cast<CPlayer*>(pPlayer)->OnHit(1);
                static_cast<CBullet*>(pBullet)->Kill();
                break;
            }
        }
    }
}

void CObject_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    for(int i = 0; i < OBJ_END; i++)
        for(CGameObject* pObj : m_ObjectList[i])
            if(pObj)
                pObj->Render(pd3dCommandList, pCamera);
}

void CObject_Manager::Release()
{
    for(int i = 0; i < OBJ_END; i++)
    {
        for(CGameObject* pObj : m_ObjectList[i])
            delete pObj;
        m_ObjectList[i].clear();
    }
}

void CObject_Manager::ReleaseUploadBuffers()
{
    for(int i = 0; i < OBJ_END; i++)
        for(CGameObject* pObj : m_ObjectList[i])
            if(pObj)
                pObj->ReleaseUploadBuffers();
}
