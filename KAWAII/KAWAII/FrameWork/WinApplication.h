#pragma once

#include "DXFramework.h"

class DXFramework;

class Win32Application
{
public:
	static int Run(DXFramework* pFramework, HINSTANCE hInstance, int nCmdShow);
	static HWND GetHwnd() { return m_hwnd; }

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
	static HWND m_hwnd;
};