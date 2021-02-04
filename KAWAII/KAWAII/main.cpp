#include <iostream>
#include "FrameWork/WinApplication.h"
#include "FrameWork.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	Framework framework(1280, 720, L"DirectX 12 Irradiance Mapping");
	return Win32Application::Run(&framework, hInstance, nCmdShow);
}