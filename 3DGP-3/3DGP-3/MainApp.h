#pragma once
#include "GameTimer.h"

class CMainApp
{
public:
    CMainApp();
    ~CMainApp();

    bool Initialize(HINSTANCE hInstance, HWND hWnd);
    void Update(float dt);
    void Render();
    void Release();

    LRESULT OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

    CGameTimer m_GameTimer;

private:
    void CreateDirect3DDevice();
    void CreateCommandQueueAndList();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    void CreateRenderTargetViews();
    void CreateDepthStencilView();
    void CreateRootSignature();
    void ExecuteLevelLoad();
    void ProcessPendingLevelChange();

    void WaitForGpuComplete();
    void MoveToNextFrame();

    void BeginRender();
    void EndRender();

    void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
    HINSTANCE m_hInstance = NULL;
    HWND m_hWnd = NULL;

    int m_nWndClientWidth = FRAME_BUFFER_WIDTH;
    int m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

    IDXGIFactory4* m_pdxgiFactory = NULL;
    IDXGISwapChain3* m_pdxgiSwapChain = NULL;
    ID3D12Device* m_pd3dDevice = NULL;

    static const UINT m_nSwapChainBuffers = 2;
    UINT m_nSwapChainBufferIndex = 0;

    ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers] = {};
    ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = NULL;
    UINT m_nRtvDescriptorIncrementSize = 0;

    ID3D12Resource* m_pd3dDepthStencilBuffer = NULL;
    ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = NULL;
    UINT m_nDsvDescriptorIncrementSize = 0;

    ID3D12CommandAllocator* m_pd3dCommandAllocator = NULL;
    ID3D12CommandQueue* m_pd3dCommandQueue = NULL;
    ID3D12GraphicsCommandList* m_pd3dCommandList = NULL;

    ID3D12Fence* m_pd3dFence = NULL;
    UINT64 m_nFenceValues[m_nSwapChainBuffers] = {};
    HANDLE m_hFenceEvent = NULL;

    ID3D12RootSignature* m_pd3dRootSignature = NULL;
};
