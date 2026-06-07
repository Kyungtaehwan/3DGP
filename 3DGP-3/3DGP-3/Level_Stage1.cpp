#include "pch.h"
#include "Level_Stage1.h"
#include "Shader.h"
#include "Mesh.h"
#include "Player.h"
#include "Camera.h"
#include "Object_Manager.h"
#include "Input_Manager.h"
#include "LightManager.h"
#include "Terrain.h"
#include "GameObject.h"
#include "Humvee.h"
#include "Tree.h"
#include "Tank.h"
#include "Bullet.h"
#include "ExplosionEffect.h"
#include "UI_Manager.h"
#include "Level_Manager.h"
#include "Text_Manager.h"


struct CB_TERRAIN_LINE
{
    XMFLOAT4 p0p1; 
    XMFLOAT4 color; 
    float halfWidth;
    float pad[3];
};

void CLevel_Stage1::Initialize(ID3D12Device* pd3dDevice,
                               ID3D12GraphicsCommandList* pd3dCommandList,
                               ID3D12RootSignature* pd3dRootSignature)
{
    m_pShader = new CIlluminatedShader();
    m_pShader->AddRef();
    m_pShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pTerrainShader = new CTerrainShader();
    m_pTerrainShader->AddRef();
    m_pTerrainShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pColorShader = new CColorShader();
    m_pColorShader->AddRef();
    m_pColorShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dRootSignature);

    m_pPlayerBulletMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList,
                                             2.5f, 2.5f, 2.5f, XMFLOAT4(1.0f, 0.9f, 0.2f, 1.0f)); // yellow
    m_pPlayerBulletMesh->AddRef();
    m_pEnemyBulletMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList,
                                            2.5f, 2.5f, 2.5f, XMFLOAT4(1.0f, 0.25f, 0.15f, 1.0f)); // red
    m_pEnemyBulletMesh->AddRef();

    CUI_Manager::Get_Instance()->Initialize(pd3dDevice, pd3dCommandList, pd3dRootSignature);
    CExplosionEffect::PrepareShared(pd3dDevice, pd3dCommandList);

    m_pLightManager = new CLightManager();
    m_pLightManager->BuildDefaultLights();
    m_pLightManager->CreateShaderVariables(pd3dDevice, pd3dCommandList);

    const int TERRAIN_W = 1024;
    const int TERRAIN_L = 1024;
    const int BLOCK_W = 257;
    const int BLOCK_L = 257;
    const XMFLOAT3 TERRAIN_SCALE = XMFLOAT3(4.0f, 2.0f, 4.0f);
    const XMFLOAT4 TERRAIN_COLOR = XMFLOAT4(0.7f, 0.45f, 0.35f, 1.0f);

    m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList,
                                       "Model/HeightMap.raw", TERRAIN_W, TERRAIN_L, BLOCK_W, BLOCK_L,
                                       TERRAIN_SCALE, TERRAIN_COLOR, m_pTerrainShader);

    const float fMargin = 40.0f;
    const float fW = m_pTerrain->GetWidth();
    const float fL = m_pTerrain->GetLength();

    XMFLOAT3 startPos = XMFLOAT3(fMargin, 0.0f, fMargin);
    XMFLOAT3 endPos = XMFLOAT3(fW - fMargin, 0.0f, fL - fMargin);

    CPlayer* pPlayer = new CPlayer();
    pPlayer->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/Apache.bin");

    float fGround = m_pTerrain->GetHeight(startPos.x, startPos.z);
    pPlayer->SetPosition(XMFLOAT3(startPos.x, fGround + 200.0f, startPos.z));

    m_pCamera = pPlayer->ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
    pPlayer->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    pPlayer->SetCombatResources(m_pPlayerBulletMesh, m_pColorShader);
    pPlayer->SetTerrain(m_pTerrain);
    CObject_Manager::Get_Instance()->Add_Object(OBJ_PLAYER, pPlayer);

    CHumvee* pHumvee = new CHumvee();
    if(pHumvee->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/Hummer.bin"))
    {
        pHumvee->SetTerrain(m_pTerrain);
        pHumvee->SetPath(startPos, endPos, 40.0f /*speed*/, 4.0f /*scale*/);
        pHumvee->SetBodyColor(XMFLOAT4(0.58f, 0.52f, 0.33f, 1.0f));
        pHumvee->SetCombatResources(pd3dDevice, m_pColorShader);
        CObject_Manager::Get_Instance()->Add_Object(OBJ_HUMVEE, pHumvee);
    }
    else
    {
        delete pHumvee;
        pHumvee = nullptr;
    }

    {
        const int nTanks = 20;
        XMFLOAT3 dir = Vector3::Normalize(Vector3::Subtract(endPos, startPos));
        XMFLOAT3 perp = XMFLOAT3(-dir.z, 0.0f, dir.x);
        const XMFLOAT4 olive = XMFLOAT4(0.28f, 0.32f, 0.24f, 1.0f);

        for(int i = 0; i < nTanks; ++i)
        {
            float t = (i + 1) / (float)(nTanks + 1);
            XMFLOAT3 base = Vector3::Add(startPos, Vector3::Subtract(endPos, startPos), t);
            float side;
            if(i % 2 == 0)
                side = 1.0f;
            else
                side = -1.0f;
            float offset = 130.0f + (i % 3) * 70.0f;
            XMFLOAT3 spawn = Vector3::Add(base, perp, side * offset);

            if(spawn.x < fMargin)
                spawn.x = fMargin;
            if(spawn.x > fW - fMargin)
                spawn.x = fW - fMargin;
            if(spawn.z < fMargin)
                spawn.z = fMargin;
            if(spawn.z > fL - fMargin)
                spawn.z = fL - fMargin;

            float bodyYaw = XMConvertToDegrees(atan2f(base.x - spawn.x, base.z - spawn.z));

            CTank* pTank = new CTank();
            if(pTank->LoadModel(pd3dDevice, pd3dCommandList, m_pShader, "Model/M26.bin"))
            {
                pTank->SetTerrain(m_pTerrain);
                pTank->SetTarget(pPlayer);
                pTank->SetCombatResources(pd3dDevice, m_pEnemyBulletMesh, m_pColorShader);
                pTank->SetSpawn(spawn, bodyYaw, 5.0f /*scale*/);
                pTank->SetBodyColor(olive);
                CObject_Manager::Get_Instance()->Add_Object(OBJ_TANK, pTank);
            }
            else
            {
                delete pTank;
            }
        }
    }

    // Route line constant buffer(b5)
    UINT cbSize = (sizeof(CB_TERRAIN_LINE) + 255) & ~255;
    m_pcbLine = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, cbSize,
                                       D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
    m_pcbLine->Map(0, NULL, &m_pcbLineMapped);
    {
        CB_TERRAIN_LINE* p = (CB_TERRAIN_LINE*)m_pcbLineMapped;
        p->p0p1 = XMFLOAT4(startPos.x, startPos.z, endPos.x, endPos.z);
        p->color = XMFLOAT4(0.0f, 0.0f, 0.0f, 0.8f);
        p->halfWidth = 30.0f;
    }

    // Trees
    CGameObject* pTreeProto = CGameObject::LoadGeometryFromFile(
        pd3dDevice, pd3dCommandList, "Model/Tree.bin", m_pShader);
    if(pTreeProto)
    {
        pTreeProto->AddRef();

        srand(2);
        const int nTrees = 60;
        for(int i = 0; i < nTrees; ++i)
        {
            float rx = fMargin + (fW - 2 * fMargin) * (rand() / (float)RAND_MAX);
            float rz = fMargin + (fL - 2 * fMargin) * (rand() / (float)RAND_MAX);
            float ry = m_pTerrain->GetHeight(rx, rz);

            CTree* pTree = new CTree();
            pTree->InitInstance(pd3dDevice, pd3dCommandList, pTreeProto,
                                5.0f /*scale*/, XMFLOAT3(rx, ry, rz));
            CObject_Manager::Get_Instance()->Add_Object(OBJ_TREE, pTree);
        }

        pTreeProto->Release();
    }

    //Result
    m_pResultCamera = new CCamera();
    m_pResultCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
    m_pResultCamera->SetPosition(XMFLOAT3(0.0f, 0.0f, -4.0f));
    m_pResultCamera->GenerateViewMatrix();
    m_pResultCamera->GenerateProjectionMatrix(0.1f, 100.0f, ASPECT_RATIO, 60.0f);
    m_pResultCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
    m_pResultCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);

    BuildResultText(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_WIN,
                    XMFLOAT4(0.2f, 1.0f, 0.35f, 1.0f), m_WinCubes);
    BuildResultText(pd3dDevice, pd3dCommandList, CText_Manager::TEXT_LOSE,
                    XMFLOAT4(1.0f, 0.25f, 0.25f, 1.0f), m_LoseCubes);

    CInput_Manager::Get_Instance()->SetMouseLock(true);
}

void CLevel_Stage1::BuildResultText(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
                                    int textId, XMFLOAT4 color, std::vector<ResultCube>& cubesOut)
{
    const float cell = 0.18f; //cube size
    const float spacing = cell * 0.6f; //gap

    std::vector<XMFLOAT3> cells;
    CText_Manager::Get_Instance()->LayoutText(textId, 0.0f, 0.0f, cell, spacing, cells);
    if(cells.empty())
        return;

    CColorCubeMesh* pMesh = new CColorCubeMesh(pd3dDevice, pd3dCommandList, cell, cell, cell, color);

    for(const XMFLOAT3& c : cells)
    {
        CGameObject* pCube = new CGameObject();
        pCube->SetMesh(pMesh);
        pCube->SetShader(m_pColorShader);
        pCube->CreateShaderVariables(pd3dDevice, pd3dCommandList);

        ResultCube rc;
        rc.pObj = pCube;
        rc.localOffset = c;
        cubesOut.push_back(rc);
    }
}

int CLevel_Stage1::Update(float dt)
{
    CObject_Manager* pOM = CObject_Manager::Get_Instance();
    CPlayer* pPlayer = static_cast<CPlayer*>(pOM->Get_Player());

    if(m_eState == PLAYING || m_eState == WINNING || m_eState == LOSING)
        pOM->Update(dt);

    if(m_eState == PLAYING)
    {
        std::list<CGameObject*>* pHumvees = pOM->Get_List(OBJ_HUMVEE);

        bool bHumveeEscaped = false;
        if(!pHumvees->empty())
        {
            CHumvee* pHumvee = static_cast<CHumvee*>(pHumvees->front());
            bHumveeEscaped = pHumvee->IsArrived();
        }

        if(pPlayer && pPlayer->IsDestroyed())
        {
            m_eState = LOSING;
            m_fEventTimer = 0.0f;
        }
        else if(pHumvees->empty())
        {
            m_eState = WINNING;
            m_fEventTimer = 0.0f;
        }
        else if(bHumveeEscaped)
        {
            m_eState = LOSING;
            m_fEventTimer = 0.0f;
        }
    }
    else if(m_eState == WINNING || m_eState == LOSING)
    {
        m_fEventTimer += dt;
        if(m_fEventTimer >= m_fEventDelay)
        {
            if(m_eState == WINNING)
                m_eState = WIN;
            else
                m_eState = LOSE;
            m_fResultTimer = 0.0f;
        }
    }
    else
    {
        m_fResultTimer += dt;
        if(m_fResultTimer >= m_fResultHold)
            CLevel_Manager::Get_Instance()->Request_Level_Change(LEVEL_MENU);
    }

    return 0;
}

void CLevel_Stage1::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
    CPlayer* pPlayer = static_cast<CPlayer*>(CObject_Manager::Get_Instance()->Get_Player());

    if(m_eState == WIN || m_eState == LOSE)
    {
        if(m_pResultCamera)
        {
            m_pResultCamera->SetViewportsAndScissorRects(pd3dCommandList);
            m_pResultCamera->UpdateShaderVariables(pd3dCommandList);
        }

        XMMATRIX groupRot = XMMatrixRotationY(m_fResultTimer * 1.2f);
        std::vector<ResultCube>& cubes = (m_eState == WIN) ? m_WinCubes : m_LoseCubes;
        for(ResultCube& rc : cubes)
        {
            if(!rc.pObj)
                continue;
            XMMATRIX world = XMMatrixTranslation(rc.localOffset.x, rc.localOffset.y, rc.localOffset.z) * groupRot;
            XMStoreFloat4x4(&rc.pObj->m_xmf4x4World, world);
            rc.pObj->Render(pd3dCommandList, m_pResultCamera);
        }
        return;
    }

    if(pPlayer)
        m_pCamera = pPlayer->GetCamera();

    if(m_pCamera)
    {
        m_pCamera->SetViewportsAndScissorRects(pd3dCommandList);
        m_pCamera->UpdateShaderVariables(pd3dCommandList);
    }

    if(m_pLightManager)
        m_pLightManager->UpdateShaderVariables(pd3dCommandList);

    if(m_pcbLine)
        pd3dCommandList->SetGraphicsRootConstantBufferView(
            ROOT_SLOT_TERRAINLINE, m_pcbLine->GetGPUVirtualAddress());

    if(m_pTerrain)
        m_pTerrain->Render(pd3dCommandList, m_pCamera);

    CObject_Manager::Get_Instance()->Render(pd3dCommandList, m_pCamera);

    CUI_Manager* pUI = CUI_Manager::Get_Instance();
    pUI->RenderCrosshair(pd3dCommandList);
    if(pPlayer)
        pUI->RenderHPBar(pd3dCommandList, pPlayer->GetHP(), pPlayer->GetMaxHP());

    // Target HP bar
    std::list<CGameObject*>* pHumvees = CObject_Manager::Get_Instance()->Get_List(OBJ_HUMVEE);
    if(!pHumvees->empty())
    {
        CHumvee* pHumvee = static_cast<CHumvee*>(pHumvees->front());
        pUI->RenderEnemyHPBar(pd3dCommandList, pHumvee->GetHP(), pHumvee->GetMaxHP());
    }
}

void CLevel_Stage1::Release()
{
    CObject_Manager::Destroy_Instance();

    CUI_Manager::Get_Instance()->Release();

    for(ResultCube& rc : m_WinCubes)
        if(rc.pObj)
            rc.pObj->Release();
    for(ResultCube& rc : m_LoseCubes)
        if(rc.pObj)
            rc.pObj->Release();
    m_WinCubes.clear();
    m_LoseCubes.clear();
    if(m_pResultCamera)
    {
        m_pResultCamera->ReleaseShaderVariables();
        delete m_pResultCamera;
        m_pResultCamera = NULL;
    }

    if(m_pcbLine)
    {
        m_pcbLine->Unmap(0, NULL);
        m_pcbLine->Release();
        m_pcbLine = NULL;
        m_pcbLineMapped = NULL;
    }

    if(m_pPlayerBulletMesh)
    {
        m_pPlayerBulletMesh->Release();
        m_pPlayerBulletMesh = NULL;
    }
    if(m_pEnemyBulletMesh)
    {
        m_pEnemyBulletMesh->Release();
        m_pEnemyBulletMesh = NULL;
    }

    if(m_pTerrain)
    {
        m_pTerrain->Release();
        m_pTerrain = NULL;
    }
    if(m_pLightManager)
    {
        delete m_pLightManager;
        m_pLightManager = NULL;
    }
    if(m_pColorShader)
    {
        m_pColorShader->Release();
        m_pColorShader = NULL;
    }
    if(m_pTerrainShader)
    {
        m_pTerrainShader->Release();
        m_pTerrainShader = NULL;
    }
    if(m_pShader)
    {
        m_pShader->Release();
        m_pShader = NULL;
    }
}

void CLevel_Stage1::ReleaseUploadBuffers()
{
    CObject_Manager::Get_Instance()->ReleaseUploadBuffers();
    if(m_pTerrain)
        m_pTerrain->ReleaseUploadBuffers();
    if(m_pPlayerBulletMesh)
        m_pPlayerBulletMesh->ReleaseUploadBuffers();
    if(m_pEnemyBulletMesh)
        m_pEnemyBulletMesh->ReleaseUploadBuffers();

    for(ResultCube& rc : m_WinCubes)
        if(rc.pObj)
            rc.pObj->ReleaseUploadBuffers();
    for(ResultCube& rc : m_LoseCubes)
        if(rc.pObj)
            rc.pObj->ReleaseUploadBuffers();
}
