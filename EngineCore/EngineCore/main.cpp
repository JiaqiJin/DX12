#include "pch.h"
#include "Application.h"

using namespace DirectX;

#pragma comment(lib, "runtimeobject.lib")

int main(int argc, const char* argv[])
{
	Microsoft::WRL::Wrappers::RoInitializeWrapper InitializeWinRT(RO_INIT_MULTITHREADED);

	HINSTANCE hInst = GetModuleHandle(0);

	try
	{
		Application app(hInst);

		auto setup = []()
		{

		};

		return app.Run(1024, 768, setup);
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}

}
