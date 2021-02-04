#pragma once
#include "FrameWork/DXFramework.h"
#include "FrameWork/Timer.h"
#include "Renderer/Renderer.h"

class Framework : public DXFramework
{
public:
	Framework(uint32_t width, uint32_t height, std::wstring name);
	virtual ~Framework();

	virtual void OnInit();
	virtual void OnUpdate();
	virtual void OnRender();
	virtual void OnDestroy();

	virtual void OnKeyUp(uint8_t /*key*/);
	virtual void OnLButtonDown(float posX, float posY);
	virtual void OnLButtonUp(float posX, float posY);
	virtual void OnMouseMove(float posX, float posY);
	virtual void OnMouseWheel(float deltaZ, float posX, float posY);
	virtual void OnMouseLeave();

	virtual void ParseCommandLineArgs(wchar_t* argv[], int argc);
};
