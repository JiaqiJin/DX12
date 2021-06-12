#include "pch.h"

#include "Engine.h"

using namespace DirectX;
// https://www.3dgep.com/learning-directx-12-1/
// https://microsoft.github.io/DirectX-Specs/d3d/ResourceBinding.html
// http://diligentgraphics.com/diligent-engine/architecture/d3d12/ 
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

// https://www.poe-vault.com/guides/cyclone-cast-on-critical-ice-nova-assassin-build-guide
// https://www.3dgep.com/learning-directx-12-1/#GPU_Synchronization
// https://milty.nl/grad_guide/basic_implementation/d3d12/command_list.html

