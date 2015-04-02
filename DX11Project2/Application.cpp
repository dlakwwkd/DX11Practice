#include "Application.h"
#include "D3DManager.h"
#include <WindowsX.h>
#include <sstream>
#include <fstream>

namespace
{
    Application* g_App = nullptr;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    return g_App->MsgProc(hwnd, msg, wParam, lParam);
}

Application::Application(HINSTANCE hInstance)
:   m_AppInst(hInstance),
    m_MainWnd(nullptr),
    m_MainWndCaption(L"D3D11 Application"),
    m_ClientWidth(800),
    m_ClientHeight(600),
    m_AppPaused(false),
    m_Minimized(false),
    m_Maximized(false),
    m_Resizing(false)
{
    g_App = this;
}


Application::~Application()
{
}

bool Application::Init()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_AppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"D3DWndClassName";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    RECT R = { 0, 0, m_ClientWidth, m_ClientHeight };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    m_MainWnd = CreateWindow(L"D3DWndClassName", m_MainWndCaption.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        width, height, 0, 0, m_AppInst, 0);
    if (!m_MainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_MainWnd, SW_SHOW);
    UpdateWindow(m_MainWnd);

    if (!D3DManager::getInstance()->InitDevice(m_MainWnd))
    {
        return false;
    }
    return true;
}

int Application::Run()
{
    MSG msg = { 0 };
    m_Timer.Reset();
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_Timer.Tick();
            if (!m_AppPaused)
            {
                CalculateFrameStats();
                D3DManager::getInstance()->UpdateScene(m_Timer.DeltaTime());
                D3DManager::getInstance()->DrawScene();
            }
            else
            {
                Sleep(100);
            }
        }
    }
    D3DManager::getInstance()->CleanupDevice();
    return (int)msg.wParam;
}

void Application::Resize()
{
    D3DManager::getInstance()->Resize();
}

LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) == WA_INACTIVE)
        {
            m_AppPaused = true;
            m_Timer.Stop();
        }
        else
        {
            m_AppPaused = false;
            m_Timer.Start();
        }
        return 0;

    case WM_SIZE:
        m_ClientWidth = LOWORD(lParam);
        m_ClientHeight = HIWORD(lParam);

        if (D3DManager::getInstance()->GetDevice())
        {
            if (wParam == SIZE_MINIMIZED)
            {
                m_AppPaused = true;
                m_Minimized = true;
                m_Maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                m_AppPaused = false;
                m_Minimized = false;
                m_Maximized = true;
                Resize();
            }
            else if (wParam == SIZE_RESTORED)
            {
                if (m_Minimized || m_Maximized)
                {
                    m_AppPaused = false;
                    m_Minimized = false;
                    Resize();
                }
                else if (m_Resizing)
                {
                }
                else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
                {
                    Resize();
                }
            }
        }
        return 0;

    case WM_ENTERSIZEMOVE:
        m_AppPaused = true;
        m_Resizing = true;
        m_Timer.Stop();
        return 0;

    case WM_EXITSIZEMOVE:
        m_AppPaused = false;
        m_Resizing = false;
        m_Timer.Start();
        Resize();
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        //OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        //OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    case WM_MOUSEMOVE:
        //OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Application::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        std::wostringstream outs;
        outs.precision(6);
        outs << m_MainWndCaption << L"    "
            << L"FPS: " << fps << L"    "
            << L"Frame Time: " << mspf << L" (ms)";
        SetWindowText(m_MainWnd, outs.str().c_str());

        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}
