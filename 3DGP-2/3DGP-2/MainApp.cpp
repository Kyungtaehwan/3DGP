#include "pch.h"
#include "MainApp.h"
#include "Input_Manager.h"
#include "Level_Manager.h"
#include "Object_Manager.h"
#include "UI_Manager.h"

ID3D12Resource* CreateBufferResource(
    ID3D12Device*              pd3dDevice,
    ID3D12GraphicsCommandList* pd3dCommandList,
    void*                      pData,
    UINT                       nBytes,
    D3D12_HEAP_TYPE            d3dHeapType,
    D3D12_RESOURCE_STATES      d3dResourceStates,
    ID3D12Resource**           ppd3dUploadBuffer)
{
    ID3D12Resource* pd3dBuffer = NULL;

    D3D12_HEAP_PROPERTIES d3dHeapPropertiesDesc;
    ::ZeroMemory(&d3dHeapPropertiesDesc, sizeof(D3D12_HEAP_PROPERTIES));
    d3dHeapPropertiesDesc.Type                 = d3dHeapType;
    d3dHeapPropertiesDesc.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3dHeapPropertiesDesc.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3dHeapPropertiesDesc.CreationNodeMask     = 1;
    d3dHeapPropertiesDesc.VisibleNodeMask      = 1;

    D3D12_RESOURCE_DESC d3dResourceDesc;
    ::ZeroMemory(&d3dResourceDesc, sizeof(D3D12_RESOURCE_DESC));
    d3dResourceDesc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
    d3dResourceDesc.Width              = nBytes;
    d3dResourceDesc.Height             = 1;
    d3dResourceDesc.DepthOrArraySize   = 1;
    d3dResourceDesc.MipLevels          = 1;
    d3dResourceDesc.Format             = DXGI_FORMAT_UNKNOWN;
    d3dResourceDesc.SampleDesc.Count   = 1;
    d3dResourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    d3dResourceDesc.Flags              = D3D12_RESOURCE_FLAG_NONE;

    D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;
    if (d3dHeapType == D3D12_HEAP_TYPE_UPLOAD)
        initState = D3D12_RESOURCE_STATE_GENERIC_READ;
    else if (d3dHeapType == D3D12_HEAP_TYPE_READBACK)
        initState = D3D12_RESOURCE_STATE_COPY_DEST;

    pd3dDevice->CreateCommittedResource(
        &d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, initState, NULL,
        __uuidof(ID3D12Resource), (void**)&pd3dBuffer);

    if (pData)
    {
        if (d3dHeapType == D3D12_HEAP_TYPE_DEFAULT && ppd3dUploadBuffer)
        {
            d3dHeapPropertiesDesc.Type = D3D12_HEAP_TYPE_UPLOAD;
            pd3dDevice->CreateCommittedResource(
                &d3dHeapPropertiesDesc, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
                D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
                __uuidof(ID3D12Resource), (void**)ppd3dUploadBuffer);

            UINT8* pMapped = NULL;
            (*ppd3dUploadBuffer)->Map(0, NULL, (void**)&pMapped);
            ::memcpy(pMapped, pData, nBytes);
            (*ppd3dUploadBuffer)->Unmap(0, NULL);

            D3D12_RESOURCE_BARRIER barrier = {};
            barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
            barrier.Transition.pResource   = pd3dBuffer;
            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
            barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
            pd3dCommandList->ResourceBarrier(1, &barrier);

            pd3dCommandList->CopyBufferRegion(pd3dBuffer, 0, *ppd3dUploadBuffer, 0, nBytes);

            barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
            barrier.Transition.StateAfter  = d3dResourceStates;
            pd3dCommandList->ResourceBarrier(1, &barrier);
        }
        else
        {
            UINT8* pMapped = NULL;
            pd3dBuffer->Map(0, NULL, (void**)&pMapped);
            ::memcpy(pMapped, pData, nBytes);
            pd3dBuffer->Unmap(0, NULL);
        }
    }

    return pd3dBuffer;
}



CMainApp::CMainApp()
{
    ::ZeroMemory(m_ppd3dSwapChainBackBuffers, sizeof(m_ppd3dSwapChainBackBuffers));
    ::ZeroMemory(m_nFenceValues, sizeof(m_nFenceValues));
    ::ZeroMemory(m_pszFrameRate, sizeof(m_pszFrameRate));
}

CMainApp::~CMainApp()
{
}

bool CMainApp::Initialize(HINSTANCE hInstance, HWND hWnd)
{
    m_hInstance = hInstance;
    m_hWnd      = hWnd;
    m_GameTimer.Reset();

    CreateDirect3DDevice();
    CreateCommandQueueAndList();
    CreateRtvAndDsvDescriptorHeaps();
    CreateSwapChain();
    CreateRenderTargetViews();
    CreateDepthStencilView();
    CreateRootSignature();
    ExecuteLevelLoad();
    return true;
}

void CMainApp::CreateDirect3DDevice()
{
    HRESULT hResult;
    UINT nDXGIFactoryFlags = 0;


#ifdef _DEBUG
    ID3D12Debug* pd3dDebugController = NULL;
    hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void
        **)&pd3dDebugController);
    if (pd3dDebugController)
    {
        pd3dDebugController->EnableDebugLayer();
        pd3dDebugController->Release();
    }
    nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

    hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void
        **)&m_pdxgiFactory);

    IDXGIAdapter1* pd3dAdapter = NULL;
    for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i,
        &pd3dAdapter); i++)
    {
        DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
        pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
        if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
        if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,
            _uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;
    }

    if (!pd3dAdapter)
    {
        m_pdxgiFactory->EnumWarpAdapter(_uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
        D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), (void**)&m_pd3dDevice);
    }


    if (!m_pd3dDevice)
    {
        IDXGIAdapter* pWarp = NULL;
        m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter), (void**)&pWarp);
        ::D3D12CreateDevice(pWarp, D3D_FEATURE_LEVEL_11_0,
                            __uuidof(ID3D12Device), (void**)&m_pd3dDevice);
        if (pWarp) pWarp->Release();
    }

    if (pd3dAdapter) pd3dAdapter->Release();

    D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
    d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    d3dMsaaQualityLevels.SampleCount = 4;
    d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
    d3dMsaaQualityLevels.NumQualityLevels = 0;
    m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
        &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
    m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
    m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

    hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
        (void**)&m_pd3dFence);

    m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

    m_nRtvDescriptorIncrementSize =
        m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    m_nDsvDescriptorIncrementSize =
        m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}
void CMainApp::CreateCommandQueueAndList()
{
    D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
    ::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));

    d3dCommandQueueDesc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
    d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

    HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc,
        _uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);


    hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
        __uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);


    m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                     m_pd3dCommandAllocator, NULL,
                                     __uuidof(ID3D12GraphicsCommandList),
                                     (void**)&m_pd3dCommandList);


    hResult = m_pd3dCommandList->Close();

}
void CMainApp::CreateSwapChain()
{
    RECT rcClient;
    ::GetClientRect(m_hWnd, &rcClient);
    m_nWndClientWidth  = rcClient.right  - rcClient.left;
    m_nWndClientHeight = rcClient.bottom - rcClient.top;
    DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
    ::ZeroMemory(&dxgiSwapChainDesc, sizeof(dxgiSwapChainDesc));

    dxgiSwapChainDesc.BufferCount                        = m_nSwapChainBuffers;
    dxgiSwapChainDesc.BufferDesc.Width                   = m_nWndClientWidth;
    dxgiSwapChainDesc.BufferDesc.Height                  = m_nWndClientHeight;
    dxgiSwapChainDesc.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator   = 60;
    dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    dxgiSwapChainDesc.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    dxgiSwapChainDesc.SwapEffect                         = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    dxgiSwapChainDesc.OutputWindow                       = m_hWnd;
    dxgiSwapChainDesc.SampleDesc.Count                   = (m_bMsaa4xEnable) ? 4 : 1;
    dxgiSwapChainDesc.SampleDesc.Quality                 = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels -1) : 0;
    dxgiSwapChainDesc.Windowed                           = TRUE;
    dxgiSwapChainDesc.Flags                              = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    IDXGISwapChain* pSwap = NULL;
    HRESULT hResult = m_pdxgiFactory->CreateSwapChain(
        m_pd3dCommandQueue,
        &dxgiSwapChainDesc,
        &pSwap);

    pSwap->QueryInterface(
        __uuidof(IDXGISwapChain3),
        (void**)&m_pdxgiSwapChain);

    pSwap->Release();

    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
    m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
}
void CMainApp::CreateRtvAndDsvDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
    ::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

    d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
    d3dDescriptorHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    d3dDescriptorHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    d3dDescriptorHeapDesc.NodeMask       = 0;
    HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap),
                                        (void**)&m_pd3dRtvDescriptorHeap);

    d3dDescriptorHeapDesc.NumDescriptors = 1;
    d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
    hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
        __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
}
void CMainApp::CreateRenderTargetViews()
{
    D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
        m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    for (UINT i = 0; i < m_nSwapChainBuffers; i++)
    {
        m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource),
                                    (void**)&m_ppd3dSwapChainBackBuffers[i]);
        m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
        d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
    }
}
void CMainApp::CreateDepthStencilView()
{
    D3D12_RESOURCE_DESC d3dResourceDesc;
    d3dResourceDesc.Dimension           = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    d3dResourceDesc.Alignment           = 0;
    d3dResourceDesc.Width               = m_nWndClientWidth;
    d3dResourceDesc.Height              = m_nWndClientHeight;
    d3dResourceDesc.DepthOrArraySize    = 1;
    d3dResourceDesc.MipLevels           = 1;
    d3dResourceDesc.Format              = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dResourceDesc.SampleDesc.Count    = (m_bMsaa4xEnable) ? 4 : 1;
    d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1): 0;
    d3dResourceDesc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    d3dResourceDesc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;



    D3D12_HEAP_PROPERTIES d3dHeapProperties;
    ::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
    d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
    d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    d3dHeapProperties.CreationNodeMask = 1;
    d3dHeapProperties.VisibleNodeMask = 1;

    D3D12_CLEAR_VALUE d3dClearValue;
    d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    d3dClearValue.DepthStencil.Depth = 1.0f;
    d3dClearValue.DepthStencil.Stencil = 0;

    m_pd3dDevice->CreateCommittedResource(
        &d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
        __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

    D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
        m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL,
        d3dDsvCPUDescriptorHandle);
}

void CMainApp::CreateRootSignature()
{
    D3D12_ROOT_PARAMETER params[2] = {};

    params[0].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[0].Descriptor.ShaderRegister = 0;
    params[0].Descriptor.RegisterSpace  = 0;
    params[0].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

    params[1].ParameterType             = D3D12_ROOT_PARAMETER_TYPE_CBV;
    params[1].Descriptor.ShaderRegister = 1;
    params[1].Descriptor.RegisterSpace  = 0;
    params[1].ShaderVisibility           = D3D12_SHADER_VISIBILITY_ALL;

    D3D12_ROOT_SIGNATURE_DESC sigDesc = {};
    sigDesc.NumParameters = 2;
    sigDesc.pParameters   = params;
    sigDesc.Flags         = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    ID3DBlob* pd3dSignatureBlob = NULL;
    ID3DBlob* pd3dErrorBlob = NULL;
    ::D3D12SerializeRootSignature(&sigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
                                   &pd3dSignatureBlob, &pd3dErrorBlob);
    if (pd3dErrorBlob) { OutputDebugStringA((char*)pd3dErrorBlob->GetBufferPointer()); pd3dErrorBlob->Release(); }

    m_pd3dDevice->CreateRootSignature(
        0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(),
        __uuidof(ID3D12RootSignature), (void**)&m_pd3dRootSignature);

    pd3dSignatureBlob->Release();
}

void CMainApp::ExecuteLevelLoad()
{
    m_pd3dCommandAllocator->Reset();
    m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

    CUI_Manager::Get_Instance()->Init(
        m_pd3dDevice, m_pd3dCommandList, m_pd3dRootSignature);

    CLevel_Manager::Get_Instance()->Level_Change(
        LEVEL_LOGO,
        m_pd3dDevice,
        m_pd3dCommandList,
        m_pd3dRootSignature);

    m_pd3dCommandList->Close();

    ID3D12CommandList* ppLists[] = { m_pd3dCommandList };

    m_pd3dCommandQueue->ExecuteCommandLists(1, ppLists);

    WaitForGpuComplete();

    CLevel_Manager::Get_Instance()->ReleaseUploadBuffers();

    m_GameTimer.Reset();
}

void CMainApp::WaitForGpuComplete()
{

    const UINT64 nFence = ++m_nFenceValues[0];
    m_pd3dCommandQueue->Signal(m_pd3dFence, nFence);
    if (m_pd3dFence->GetCompletedValue() < nFence)
    {
        m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
        ::WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

void CMainApp::MoveToNextFrame()
{
    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

    const UINT64 nFence = ++m_nFenceValues[0];
    m_pd3dCommandQueue->Signal(m_pd3dFence, nFence);
    if (m_pd3dFence->GetCompletedValue() < nFence)
    {
        m_pd3dFence->SetEventOnCompletion(nFence, m_hFenceEvent);
        ::WaitForSingleObject(m_hFenceEvent, INFINITE);
    }
}

void CMainApp::ChangeSwapChainState()
{
    WaitForGpuComplete();

    BOOL bFull = FALSE;
    m_pdxgiSwapChain->GetFullscreenState(&bFull, NULL);
    m_pdxgiSwapChain->SetFullscreenState(!bFull, NULL);

    DXGI_MODE_DESC modeDesc = {};
    modeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    modeDesc.Width  = m_nWndClientWidth;
    modeDesc.Height = m_nWndClientHeight;
    modeDesc.RefreshRate.Numerator   = 60;
    modeDesc.RefreshRate.Denominator = 1;
    m_pdxgiSwapChain->ResizeTarget(&modeDesc);

    for (int i = 0; i < (int)m_nSwapChainBuffers; i++)
    {
        if (m_ppd3dSwapChainBackBuffers[i])
        {
            m_ppd3dSwapChainBackBuffers[i]->Release();
            m_ppd3dSwapChainBackBuffers[i] = NULL;
        }
    }

    DXGI_SWAP_CHAIN_DESC scDesc = {};
    m_pdxgiSwapChain->GetDesc(&scDesc);
    m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers,
                                     m_nWndClientWidth, m_nWndClientHeight,
                                     scDesc.BufferDesc.Format, scDesc.Flags);

    m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
    CreateRenderTargetViews();
}


void CMainApp::Update(float dt)
{
    CLevel_Manager::Get_Instance()->Update(dt);
}

void CMainApp::LateUpdate(float dt)
{
    CLevel_Manager::Get_Instance()->Late_Update(dt);
}


void CMainApp::Render()
{
    BeginRender();

    CLevel_Manager::Get_Instance()->Render(m_pd3dCommandList);

    EndRender();

    if (CLevel_Manager::Get_Instance()->HasPendingChange())
        ProcessPendingLevelChange();
}

void CMainApp::ProcessPendingLevelChange()
{
    m_pd3dCommandAllocator->Reset();
    m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);

    CLevel_Manager::Get_Instance()->Apply_Pending_Change(
        m_pd3dDevice, m_pd3dCommandList, m_pd3dRootSignature);

    m_pd3dCommandList->Close();

    ID3D12CommandList* ppLists[] = { m_pd3dCommandList };
    m_pd3dCommandQueue->ExecuteCommandLists(1, ppLists);
    WaitForGpuComplete();

    CLevel_Manager::Get_Instance()->ReleaseUploadBuffers();

    m_GameTimer.Reset();
}


void CMainApp::BeginRender()
{
    m_pd3dCommandAllocator->Reset();

    m_pd3dCommandList->Reset(
        m_pd3dCommandAllocator,
        NULL);

    D3D12_RESOURCE_BARRIER barrier = {};

    barrier.Type =
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

    barrier.Transition.pResource =
        m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];

    barrier.Transition.StateBefore =
        D3D12_RESOURCE_STATE_PRESENT;

    barrier.Transition.StateAfter =
        D3D12_RESOURCE_STATE_RENDER_TARGET;

    barrier.Transition.Subresource =
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_pd3dCommandList->ResourceBarrier(1, &barrier);

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle =
        m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    rtvHandle.ptr +=
        m_nSwapChainBufferIndex *
        m_nRtvDescriptorIncrementSize;

    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle =
        m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

    float clearColor[4] =
    {
        0.05f, 0.05f, 0.08f, 1.f
    };

    m_pd3dCommandList->ClearRenderTargetView(
        rtvHandle,
        clearColor,
        0,
        NULL);

    m_pd3dCommandList->ClearDepthStencilView(
        dsvHandle,
        D3D12_CLEAR_FLAG_DEPTH |
        D3D12_CLEAR_FLAG_STENCIL,
        1.f,
        0,
        0,
        NULL);

    m_pd3dCommandList->OMSetRenderTargets(
        1,
        &rtvHandle,
        TRUE,
        &dsvHandle);

    D3D12_VIEWPORT vp = {};
    vp.TopLeftX = 0.0f;
    vp.TopLeftY = 0.0f;
    vp.Width    = (float)m_nWndClientWidth;
    vp.Height   = (float)m_nWndClientHeight;
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_pd3dCommandList->RSSetViewports(1, &vp);

    D3D12_RECT sc = { 0, 0, (LONG)m_nWndClientWidth, (LONG)m_nWndClientHeight };
    m_pd3dCommandList->RSSetScissorRects(1, &sc);

    m_pd3dCommandList->SetGraphicsRootSignature(
        m_pd3dRootSignature);
}

void CMainApp::EndRender()
{
    D3D12_RESOURCE_BARRIER barrier = {};

    barrier.Type =
        D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;

    barrier.Transition.pResource =
        m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex];

    barrier.Transition.StateBefore =
        D3D12_RESOURCE_STATE_RENDER_TARGET;

    barrier.Transition.StateAfter =
        D3D12_RESOURCE_STATE_PRESENT;

    barrier.Transition.Subresource =
        D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

    m_pd3dCommandList->ResourceBarrier(1, &barrier);

    m_pd3dCommandList->Close();

    ID3D12CommandList* ppLists[] =
    {
        m_pd3dCommandList
    };

    m_pd3dCommandQueue->ExecuteCommandLists(
        1,
        ppLists);

    m_pdxgiSwapChain->Present(0, 0);

    MoveToNextFrame();
}
void CMainApp::Release()
{
    WaitForGpuComplete();

    CLevel_Manager::Destroy_Instance();
    CUI_Manager::Destroy_Instance();

    if (m_pd3dRootSignature) { m_pd3dRootSignature->Release(); m_pd3dRootSignature = NULL; }

    ::CloseHandle(m_hFenceEvent);
    if (m_pd3dFence) m_pd3dFence->Release();

    for (int i = 0; i < (int)m_nSwapChainBuffers; i++)
        if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i]->Release();

    if (m_pd3dRtvDescriptorHeap)  m_pd3dRtvDescriptorHeap->Release();
    if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
    if (m_pd3dDsvDescriptorHeap)  m_pd3dDsvDescriptorHeap->Release();
    if (m_pd3dCommandList)        m_pd3dCommandList->Release();
    if (m_pd3dCommandAllocator)   m_pd3dCommandAllocator->Release();
    if (m_pd3dCommandQueue)       m_pd3dCommandQueue->Release();

    m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
    if (m_pdxgiSwapChain) m_pdxgiSwapChain->Release();
    if (m_pd3dDevice)     m_pd3dDevice->Release();
    if (m_pdxgiFactory)   m_pdxgiFactory->Release();

#ifdef _DEBUG
    IDXGIDebug1* pDbg = NULL;
    if (SUCCEEDED(DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pDbg)))
    {
        pDbg->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
        pDbg->Release();
    }
#endif
}


void CMainApp::OnProcessingKeyboardMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{

    switch (wParam)
    {
    case VK_ESCAPE:
        ::PostQuitMessage(0);
        break;
    case VK_F1:

        break;
    case VK_F2:

        break;

    case VK_F9:
        ChangeSwapChainState();
        break;
    }
}

LRESULT CMainApp::OnProcessingWindowMessage(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    switch (nMsg)
    {
    case WM_SIZE:
        m_nWndClientWidth  = LOWORD(lParam);
        m_nWndClientHeight = HIWORD(lParam);
        break;
    case WM_KEYUP:
        OnProcessingKeyboardMessage(hWnd, nMsg, wParam, lParam);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    }
    return ::DefWindowProc(hWnd, nMsg, wParam, lParam);
}
