#include "pch.h"
#include "Level_GamePlay.h"
#include "Shader.h"
#include "Player.h"
#include "Camera.h"
#include "Object_Manager.h"
#include "Input_Manager.h"

void CLevel_GamePlay::Initialize(ID3D12Device* pd3dDevice,
                                  ID3D12GraphicsCommandList* pd3dCommandList,
                                  ID3D12RootSignature* pd3dRootSignature)
{
    m_pShader = new CObjectShader();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    CPlayer* pPlayer = new CPlayer();
    pPlayer->m_xmf4Color = XMFLOAT4(0.55f, 0.65f, 0.55f, 1.0f); // olive helicopter
    pPlayer->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/Apache.bin");
    pPlayer->SetPosition(XMFLOAT3(0.0f, 0.0f, 0.0f));
    m_pCamera = pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
    pPlayer->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, pPlayer);

    CInput_Manager::Get_Instance()->SetMouseLock(true);
}

int CLevel_GamePlay::Update(float dt)
{
    CObject_Manager::Get_Instance()->Update(dt);

    // HUD: position info in title bar
    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());
    if (pPlayer)
    {
        static int s_tick = 0;
        if (++s_tick % 20 == 0)
        {
            XMFLOAT3 p = pPlayer->GetPosition();
            wchar_t buf[256];
            swprintf_s(buf, 256, L"3DGP-3 Helicopter  Pos(%.1f, %.1f, %.1f)",
                       p.x, p.y, p.z);
            SetWindowTextW(g_hWnd, buf);
        }
    }
    return 0;
}

void CLevel_GamePlay::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());
    if (pPlayer) m_pCamera = pPlayer->GetCamera();

    if (m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);
}

void CLevel_GamePlay::Release()
{
    CObject_Manager::Destroy_Instance();
    if (m_pShader) { delete m_pShader; m_pShader = NULL; }
}

void CLevel_GamePlay::ReleaseUploadBuffers()
{
    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();
}
