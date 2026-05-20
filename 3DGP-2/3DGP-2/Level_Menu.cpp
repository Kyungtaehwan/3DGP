#include "pch.h"
#include "Level_Menu.h"

void CLevel_Menu::Initialize(ID3D12Device* pd3dDevice,
                              ID3D12GraphicsCommandList* pd3dCommandList,
                              ID3D12RootSignature* pd3dRootSignature)
{
    // Menu level has no objects to initialize.
    // The background is simply the cleared render target.
    m_fTimeAcc = 0.0f;
}

int CLevel_Menu::Update(float dt)
{
    // Stay in menu; level switch happens via keyboard (Enter) in GameFramework.
    // Return 0 = no automatic level change.
    m_fTimeAcc += dt;
    return 0;
}
