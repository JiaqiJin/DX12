#include "pch.h"
#include "Engine.h"

using namespace std;
using namespace DirectX;
using Microsoft::WRL::ComPtr;

Engine* Engine::m_Engine = nullptr;
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    return Engine::Get()->MsgProc(hWnd, message, wParam, lParam);
}

Engine::Engine(HINSTANCE hInstance)
{
	assert(m_Engine == nullptr);
	m_AppInst = hInstance;
	m_Engine = this;
}

Engine::~Engine()
{
	
}

void Engine::Initialize(const EngineCreateInfo& engineCI)
{
    m_Width = engineCI.Width;
    m_Height = engineCI.Height;
    m_Aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
    InitialMainWindow();
}

void Engine::Tick()
{

}

void Engine::Update(float deltaTime)
{

}

void Engine::Render()
{

}

LRESULT Engine::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
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
            return 0;

        case WM_MBUTTONDOWN:
            return 0;

        case WM_RBUTTONDOWN:
            return 0;

        case WM_LBUTTONUP:
            return 0;

        case WM_MBUTTONUP:
            return 0;

        case WM_RBUTTONUP:
            return 0;

        case WM_MOUSEMOVE:
            return 0;

        case WM_KEYDOWN:
            return 0;

        case WM_KEYUP:
            if (wParam == VK_ESCAPE)
            {
                PostQuitMessage(0);
            }
            return 0;
        }
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void Engine::Destroy()
{
}

void Engine::Pause()
{
    m_Paused = true;
}

void Engine::Resume()
{
    m_Paused = false;
}

void Engine::InitialMainWindow()
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

void Engine::SetScreenSize(UINT width, UINT height)
{
    m_Width = width;
    m_Height = height;
    m_Aspect = static_cast<float>(width) / static_cast<float>(height);
}

void Engine::CalculateFrameStats()
{
   
}

void Engine::OnResize()
{
   
}