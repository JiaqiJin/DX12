#include "FrameWork.h"

using namespace std;
using namespace KAWAII;

Framework::Framework(uint32_t width, uint32_t height, wstring name)
	:DXFramework(width,height,name)
{
#if defined (_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	AllocConsole();
	FILE* stream;
	freopen_s(&stream, "CONOUT$", "w+t", stdout);
	freopen_s(&stream, "CONIN$", "r+t", stdin);
#endif
}

Framework::~Framework()
{
#if defined (_DEBUG)
	FreeConsole();
#endif
}

void Framework::OnInit()
{
	
}
void Framework::OnUpdate()
{

}
void Framework::OnRender()
{

}
void Framework::OnDestroy()
{

}

void Framework::OnKeyUp(uint8_t /*key*/)
{

}
void Framework::OnLButtonDown(float posX, float posY)
{

}
void Framework::OnLButtonUp(float posX, float posY)
{

}
void Framework::OnMouseMove(float posX, float posY)
{

}
void Framework::OnMouseWheel(float deltaZ, float posX, float posY)
{

}
void Framework::OnMouseLeave()
{

}
void Framework::ParseCommandLineArgs(wchar_t* argv[], int argc)
{

}