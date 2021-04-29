#pragma once

struct EngineCreateInfo
{
	UINT Width;
	UINT Height;
};


class Engine
{
public:
	Engine(HINSTANCE hInstance);
	virtual ~Engine();
	static Engine* Get() { return m_Engine; }

	template<typename TSetup>
	int RunN(const EngineCreateInfo& engineCI, TSetup setup);

	bool IsInitialized() { return m_Initialized; }
	LRESULT MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
private:
	void Initialize(const EngineCreateInfo& engineCI);
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
private:
	static Engine* m_Engine;
	bool m_Initialized = false;

	HINSTANCE m_AppInst;
	HWND m_MainWnd;

	bool m_Paused = false;

	UINT m_Width;
	UINT m_Height;
	float m_Aspect;
	bool m_resizing = false;
	bool m_minimized = false;
	bool m_maximized = false;
};

template<typename TSetup>
int Engine::RunN(const EngineCreateInfo& engineCI, TSetup setup)
{
	Initialize(engineCI);
	setup();

	//m_Timer.Reset();
	MSG msg = {};
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//m_Timer.Tick();
		Tick();
	}

	Destroy();
	return (int)msg.wParam;
}