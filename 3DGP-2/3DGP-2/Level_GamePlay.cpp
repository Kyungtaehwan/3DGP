#include "pch.h"
#include "Level_GamePlay.h"
#include "Object_Manager.h"
#include "Player.h"
#include "Mesh.h"
#include "Camera.h"



void CLevel_GamePlay::Initialize(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  ID3D12RootSignature* pd3dRootSignature)
{
    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

}

int CLevel_GamePlay::Update(float dt)
{
    CObject_Manager::Get_Instance()->Update(dt);
    CObject_Manager::Get_Instance()->CheckCollisions();
    return 0;
}

void CLevel_GamePlay::Late_Update(float dt)
{
    CObject_Manager::Get_Instance()->Late_Update(dt);
}

void CLevel_GamePlay::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(
            pd3dCommandList);

        m_pCamera->UpdateShaderVariables(
            pd3dCommandList);
    }

    CObject_Manager::Get_Instance()->Render(
        pd3dCommandList,
        m_pCamera);
}

void CLevel_GamePlay::Release()
{
    CObject_Manager::Destroy_Instance();

    if (m_pShader)
    {
        delete m_pShader;
        m_pShader = NULL;
    }
}

void CLevel_GamePlay::ReleaseUploadBuffers()
{
    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();
}
