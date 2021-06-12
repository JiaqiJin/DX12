#include "pch.h"
#include "Application.h"
#include "Common/Input.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

Application* Application::m_Application = nullptr;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return Application::Get()->MsgProc(hWnd, message, wParam, lParam);
}

Application::Application(HINSTANCE hInstance)
{
    assert(m_Application == nullptr);
    m_AppInst = hInstance;
    m_Application = this;
}

Application::~Application()
{

}

void Application::Initialize(int width, int height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    InitialMainWindow();

    m_Initialized = true;

}

void Application::Tick()
{
    if (!m_Paused)
    {
        Update(m_Timer.DeltaTime());
        Render();
    }
}

void Application::Update(float deltaTime)
{
    CalculateFrameStats();

}

void Application::Render()
{

}

LRESULT Application::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (m_Initialized)
    {
        switch (msg)
        {
        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                Pause();
            }
            else
            {
                Resume();
            }
            return 0;

        case WM_SIZE:
            SetScreenSize(LOWORD(lParam), HIWORD(lParam));
            if (wParam == SIZE_MINIMIZED)
            {
                Pause();
                m_minimized = true;
                m_maximized = false;
            }
            else if (wParam == SIZE_MAXIMIZED)
            {
                Resume();
                m_minimized = false;
                m_maximized = true;
                OnResize();
            }
            else if (wParam == SIZE_RESTORED)
            {

                if (m_minimized)
                {
                    Resume();
                    m_minimized = false;
                    OnResize();
                }

                else if (m_maximized)
                {
                    Resume();
                    m_maximized = false;
                    OnResize();
                }
                else if (m_resizing)
                {
                }
                else
                {
                    OnResize();
                }
            }
            return 0;

        case WM_ENTERSIZEMOVE:
            Pause();
            m_resizing = true;
            return 0;

        case WM_EXITSIZEMOVE:
            Resume();
            m_resizing = false;
            OnResize();
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_MENUCHAR:
            return MAKELRESULT(0, MNC_CLOSE);

        case WM_GETMINMAXINFO:
            ((MINMAXINFO*)lParam)->ptMinTrackSize.x = 200;
            ((MINMAXINFO*)lParam)->ptMinTrackSize.y = 200;
            return 0;

        case WM_LBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyDown;
            return 0;

        case WM_MBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyDown;
            return 0;

        case WM_RBUTTONDOWN:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyDown;
            return 0;

        case WM_LBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse0] = KeyState::KeyUp;
            return 0;

        case WM_MBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse1] = KeyState::KeyUp;
            return 0;

        case WM_RBUTTONUP:
            Input::m_KeyStates[(int)KeyCode::Mouse2] = KeyState::KeyUp;
            return 0;

        case WM_MOUSEMOVE:
            Input::OnMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            return 0;

        case WM_KEYDOWN:
            Input::OnKeyDown(wParam);
            return 0;

        case WM_KEYUP:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            Input::OnKeyUp(wParam);
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Application::Destroy()
{
}

void Application::Pause()
{
    m_Paused = true;
    m_Timer.Stop();
}

void Application::Resume()
{
    m_Paused = false;
    m_Timer.Start();
}

void Application::InitialMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_AppInst;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
    }

    RECT R = { 0, 0, m_Width, m_Height };
    AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);
    int width = R.right - R.left;
    int height = R.bottom - R.top;

    wstring m_Title(L"KAWAII");
    m_MainWnd = CreateWindow(L"MainWnd", m_Title.c_str(),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_AppInst, 0);
    if (!m_MainWnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
    }

    ShowWindow(m_MainWnd, SW_SHOW);
    UpdateWindow(m_MainWnd);
}

void Application::SetScreenSize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
}

void Application::CalculateFrameStats()
{
    static int frameCnt = 0;
    static float timeElapsed = 0.0f;

    frameCnt++;

    // Compute averages over one second period.
    if ((m_Timer.TotalTime() - timeElapsed) >= 1.0f)
    {
        float fps = (float)frameCnt; // fps = frameCnt / 1
        float mspf = 1000.0f / fps;

        wstring fpsStr = to_wstring(fps);
        wstring mspfStr = to_wstring(mspf);

        wstring m_Title(L"KAWAII");
        wstring windowText = m_Title +
            L"    fps: " + fpsStr +
            L"   mspf: " + mspfStr;

        SetWindowText(m_MainWnd, windowText.c_str());

        // Reset for next average.
        frameCnt = 0;
        timeElapsed += 1.0f;
    }
}

void Application::OnResize()
{

}