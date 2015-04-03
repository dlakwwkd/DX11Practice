#pragma once
#include <windows.h>
#include <string>
#include "GameTimer.h"

class Application
{
public:
    Application(HINSTANCE hInstance);
    ~Application();

    bool    Init();
    int     Run();
    void    Resize();
    LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    bool    InitMainWindow();
    void    CalculateFrameStats();

private:
    HINSTANCE       m_AppInst;
    HWND            m_MainWnd;
    std::wstring    m_MainWndCaption;

    GameTimer       m_Timer;

    int             m_ClientWidth;
    int             m_ClientHeight;

    bool            m_AppPaused;
    bool            m_Minimized;
    bool            m_Maximized;
    bool            m_Resizing;
};

