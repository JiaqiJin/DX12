#include "Graphics.h"
#include "CommandListManager.h"

using namespace Rendering;

CommandListManager Graphics::s_CommandManager;
ID3D12Device* Graphics::s_Device = nullptr;