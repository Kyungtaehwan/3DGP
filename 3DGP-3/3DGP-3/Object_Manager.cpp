#include "pch.h"
#include "Object_Manager.h"

CObject_Manager* CObject_Manager::m_pInstance = nullptr;

CObject_Manager::CObject_Manager() {}
CObject_Manager::~CObject_Manager() { Release(); }

void CObject_Manager::Add_Object(OBJ_ID eID, CGameObject* pGameObject)
{
    if (eID < 0 || eID >= OBJ_END) return;
    m_ObjectList[eID].push_back(pGameObject);
}

void CObject_Manager::Update(float dt)
{
    for (int i = 0; i < OBJ_END; i++)
        for (CGameObject* pObj : m_ObjectList[i])
            if (pObj) pObj->Update(dt);
}

void CObject_Manager::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
    for (int i = 0; i < OBJ_END; i++)
        for (CGameObject* pObj : m_ObjectList[i])
            if (pObj) pObj->Render(pd3dCommandList, pCamera);
}

void CObject_Manager::Release()
{
    for (int i = 0; i < OBJ_END; i++)
    {
        for (CGameObject* pObj : m_ObjectList[i])
            delete pObj;
        m_ObjectList[i].clear();
    }
}

void CObject_Manager::ReleaseUploadBuffers()
{
    for (int i = 0; i < OBJ_END; i++)
        for (CGameObject* pObj : m_ObjectList[i])
            if (pObj) pObj->ReleaseUploadBuffers();
}
