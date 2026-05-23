#pragma once
#include "GameObject.h"

class CCamera;
class CPlayer;

class CObject_Manager
{
private:
    CObject_Manager();
    ~CObject_Manager();

public:
    void Add_Object(OBJ_ID eID, CGameObject* pGameObject);
    void Update(float dt);
    void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
    void Release();
    void ReleaseUploadBuffers();

    CGameObject* Get_Player()
    {
        if (!m_ObjectList[OBJ_PLAYER].empty())
            return m_ObjectList[OBJ_PLAYER].front();
        return nullptr;
    }

    std::list<CGameObject*>* Get_List(OBJ_ID eID) { return &m_ObjectList[eID]; }

    static CObject_Manager* Get_Instance()
    {
        if (!m_pInstance) m_pInstance = new CObject_Manager;
        return m_pInstance;
    }

    static void Destroy_Instance()
    {
        if (m_pInstance) { delete m_pInstance; m_pInstance = nullptr; }
    }

private:
    static CObject_Manager* m_pInstance;
    std::list<CGameObject*> m_ObjectList[OBJ_END];
};
