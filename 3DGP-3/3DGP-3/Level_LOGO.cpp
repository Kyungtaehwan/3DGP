#include "pch.h"
#include "Level_LOGO.h"
#include "Level_Manager.h"
#include "Camera.h"
#include "Mesh.h"
#include "Shader.h"
#include "GameObject.h"
#include "Input_Manager.h"
#include "Text_Manager.h"
#include "Object_Manager.h"
#include "ExplosionEffect.h"

void CLevel_LOGO::Initialize(ID3D12Device* pd3dDevice,
                             ID3D12GraphicsCommandList* pd3dCommandList,
                             ID3D12RootSignature* pd3dRootSignature)
{
    CInput_Manager::Get_Instance()->SetMouseLock(false);
    m_pd3dDevice = pd3dDevice;

    m_pShader = new CColorShader();
    m_pShader->AddRef();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pCamera = new CCamera();
    m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    m_pCamera->SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));
    m_pCamera->GenerateViewMatrix();
    m_pCamera->GenerateProjectionMatrix(0.1f, 100.0f, ASPECT_RATIO, 60.0f);
    m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    const XMFLOAT4 cyan = { 0.4f, 1.0f, 1.0f, 1.0f };
    const XMFLOAT4 pink = { 1.0f, 0.5f, 0.7f, 1.0f };

    m_fNameY = ((NAME_NDC_Y_MIN + NAME_NDC_Y_MAX) * 0.5f) * kWorldPerNdcY;
    BuildTextCubes(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_TITLE,
                   TOP_Y, CELL_NAME, NAME_SPACING, cyan, m_TitleCubes);
    BuildTextCubes(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_NAME,
                   m_fNameY, CELL_NAME, NAME_SPACING, pink, m_NameCubes);

    CExplosionEffect::PrepareShared(pd3dDevice, pd3dCommandList);
}

void CLevel_LOGO::BuildTextCubes(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                                 int textId, float centerY, float cell, float spacing,
                                 XMFLOAT4 color, std::vector<LogoCube>& out)
{
    std::vector<XMFLOAT3> cells;
    CText_Manager::Get_Instance()->LayoutText(textId, 0.0f, centerY, cell, spacing, cells);
    if(cells.empty())
        return;

    CColorCubeMesh* pMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList, cell, cell, cell, color);

    for(const XMFLOAT3& c : cells)
    {
        CGameObject* pCube = new CGameObject();
        pCube->SetMesh(pMesh);
        pCube->SetShader(m_pShader);
        pCube->CreateShaderVariables(pd3dDevice, pd3dCommandList);

        LogoCube lc;
        lc.pObj = pCube;
        lc.localOffset = c;
        out.push_back(lc);
    }
}

int CLevel_LOGO::Update(float dt)
{

    m_fRotation += dt * 0.8f;
    XMMATRIX groupRot = XMMatrixRotationY(m_fRotation);
    for(LogoCube& lc : m_TitleCubes)
    {
        XMMATRIX world = XMMatrixTranslation(lc.localOffset.x, lc.localOffset.y, lc.localOffset.z) * groupRot;
        XMStoreFloat4x4(&lc.pObj->m_xmf4x4World, world);
    }
    if(!m_bExploding)
        for(LogoCube& lc : m_NameCubes)
        {
            XMMATRIX world = XMMatrixTranslation(lc.localOffset.x, lc.localOffset.y, lc.localOffset.z) * groupRot;
            XMStoreFloat4x4(&lc.pObj->m_xmf4x4World, world);
        }

    CObject_Manager::Get_Instance()->Update(dt);

    if(m_bExploding)
    {
        m_fExplodeTimer += dt;
        if(m_fExplodeTimer >= EXPLODE_DELAY)
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_MENU);
        return 0;
    }


    if(CInput_Manager::Get_Instance()->Key_Down(VK_LBUTTON))
    {
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(g_hWnd, &pt);
        float ndcX = ((float)pt.x / (float)FRAME_BUFFER_WIDTH) * 2.0f - 1.0f;
        float ndcY = -(((float)pt.y / (float)FRAME_BUFFER_HEIGHT) * 2.0f - 1.0f);

        if(ndcX >= NAME_NDC_X_MIN && ndcX <= NAME_NDC_X_MAX &&
           ndcY >= NAME_NDC_Y_MIN && ndcY <= NAME_NDC_Y_MAX)
        {
            CExplosionEffect* pFx = new CExplosionEffect();
            pFx->Initialize(m_pd3dDevice, m_pShader, XMFLOAT3(0.0f, m_fNameY, 0.0f));
            CObject_Manager::Get_Instance()->Add_Object(OBJ_EFFECT, pFx);
            m_bExploding = true;
        }
    }
    return 0;
}

void CLevel_LOGO::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    if(m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    for(LogoCube& lc : m_TitleCubes)
        if(lc.pObj)
            lc.pObj->Render(pd3dCommandList, m_pCamera);
    if(!m_bExploding)
        for(LogoCube& lc : m_NameCubes)
            if(lc.pObj)
                lc.pObj->Render(pd3dCommandList, m_pCamera);

    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);
}

void CLevel_LOGO::ReleaseUploadBuffers()
{
    for(LogoCube& lc : m_TitleCubes)
        if(lc.pObj)
            lc.pObj->ReleaseUploadBuffers();
    for(LogoCube& lc : m_NameCubes)
        if(lc.pObj)
            lc.pObj->ReleaseUploadBuffers();
    CExplosionEffect::ReleaseSharedUploadBuffers();
}

void CLevel_LOGO::Release()
{
    CObject_Manager::Get_Instance()->Release();

    for(LogoCube& lc : m_TitleCubes)
        if(lc.pObj)
            lc.pObj->Release();
    for(LogoCube& lc : m_NameCubes)
        if(lc.pObj)
            lc.pObj->Release();
    m_TitleCubes.clear();
    m_NameCubes.clear();

    if(m_pCamera)
    {
        m_pCamera->ReleaseShaderVariables();
        delete m_pCamera;
        m_pCamera = nullptr;
    }
    if(m_pShader)
    {
        m_pShader->Release();
        m_pShader = nullptr;
    }
}
