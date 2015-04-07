#include "InputManager.h"


InputManager::InputManager()
{
    SetKeyState();
    m_MouseState[MK_LBUTTON] = false;
    m_MouseState[MK_MBUTTON] = false;
    m_MouseState[MK_RBUTTON] = false;
    m_PrevMousePos = { 0, 0 };
    m_LastMousePos = { 0, 0 };
}


InputManager::~InputManager()
{
}

void InputManager::SetKeyState()
{
    BYTE byKey[256];
    if (GetKeyboardState(byKey))
    {
        for (int i = 0; i < 256; ++i)
        {
            if (byKey[i] & 0x80)
                m_KeyState[i] = true;
            else
                m_KeyState[i] = false;
        }
    }
}
