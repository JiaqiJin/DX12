#include "pch.h"

#include "Engine.h"

using namespace DirectX;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	try
	{
		Engine engine(hInstance);

		EngineCreateInfo engineCI;
		engineCI.Width = 800;
		engineCI.Height = 800;

		auto setup = []() {};

		return engine.RunN(engineCI, setup);
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
};
