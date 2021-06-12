#pragma once

#include "Common/GameTimer.h"

class Application
{
public:
	Application(HINSTANCE hInstance);
	virtual ~Application();
	static Application* Get() { return m_Application; }

	template<typename TSetup>
	int Run(int width, int height, TSetup setup);

	bool IsInitialized() { return m_Initialized; }
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	void Initialize(int width, int height);
	void Tick();
	void Update(float deltaTime);
	void Render();
	void Destroy();

	void Pause();
	void Resume();
	void OnResize();
	void SetScreenSize(UINT width, UINT height);
	void CalculateFrameStats();

	void InitialMainWindow();

	static Application* m_Application;

	bool m_Initialized = false;

	// Window
	HINSTANCE m_AppInst;
	HWND m_MainWnd;

	// Timer
	GameTimer m_Timer;
	bool m_Paused = false;

	// Window Parameters
	UINT m_Width;
	UINT m_Height;
	float m_Aspect;
	bool m_resizing = false;
	bool m_minimized = false;
	bool m_maximized = false;


	Microsoft::WRL::ComPtr< IDXGIFactory4> m_DXGIFactory;

};

template<typename TSetup>
int Application::Run(int width, int height, TSetup setup)
{
	Initialize(width, height);
	setup();


	m_Timer.Reset();
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		m_Timer.Tick();
		Tick();
	}

	Destroy();
	return (int)msg.wParam;
}