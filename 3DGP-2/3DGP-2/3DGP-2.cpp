#include "pch.h"
#include "3DGP-2.h"
#include "MainApp.h"

HWND      g_hWnd      = NULL;
CMainApp gMainApp;

ATOM             MyRegisterClass(HINSTANCE hInstance);
BOOL             InitInstance(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_     HINSTANCE hInstance,
                      _In_opt_ HINSTANCE hPrevInstance,
                      _In_     LPWSTR    lpCmdLine,
                      _In_     int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow))
        return FALSE;

    MSG msg = {};

    while (true)
    {
        if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) break;
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
        }
        else
        {
            gMainApp.m_GameTimer.Tick(144.f);
            float dt = gMainApp.m_GameTimer.GetTimeElapsed();
            
            TCHAR szFPS[32];
            gMainApp.m_GameTimer.GetFrameRate(szFPS, 32);

            gMainApp.Update(dt);
            gMainApp.LateUpdate(dt);
            gMainApp.Render();
        }
    }

    gMainApp.Release();
    return (int)msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex = {};
    wcex.cbSize        = sizeof(WNDCLASSEXW);
    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = WndProc;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY3DGP2));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = L"3DGP2WindowClass";
    wcex.hIconSm       = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return ::RegisterClassExW(&wcex);
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    RECT rcWnd = { 0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT };
    DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_BORDER;
    ::AdjustWindowRect(&rcWnd, dwStyle, FALSE);
    g_hWnd = ::CreateWindowW(
        L"3DGP2WindowClass",
        L"3DGP-2  DX12 Shooter",
        dwStyle,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rcWnd.right - rcWnd.left,
        rcWnd.bottom - rcWnd.top,
        NULL, NULL, hInstance, NULL);

    if (!g_hWnd) return FALSE;
    gMainApp.Initialize(hInstance, g_hWnd);

    ::ShowWindow(g_hWnd, nCmdShow);
    ::UpdateWindow(g_hWnd);

    return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_SIZE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_KEYDOWN:
    case WM_KEYUP:
        gMainApp.OnProcessingWindowMessage(hWnd, message, wParam, lParam);
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        break;
    default:
        return(::DefWindowProc(hWnd, message, wParam, lParam));
    }
    return 0;
}
