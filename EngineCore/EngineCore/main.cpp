#include "pch.h"
#include "Application.h"

using namespace DirectX;

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
	try
	{
		Application app(hInstance);

		EngineCreateInfo engineCI;
		engineCI.Width = 1920;
		engineCI.Height = 1080;

		auto setup = []()
		{
			
		};

		return app.RunN(engineCI, setup);
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}
// https://github.com/Jiaqidesune/DX12/tree/768b5dc7d5492cc8c70c09bf824838dc8d50ad2f