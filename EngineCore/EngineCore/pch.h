#ifndef PCH_H
#define PCH_H


#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif

// Windows
#include <windows.h>
#include <WindowsX.h>
#include <wrl.h>


// Direct3d12
#include <d3d12.h>
#include <D3d12SDKLayers.h>
#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXPackedVector.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib,"d3dcompiler.lib")
#pragma comment(lib,"dxguid.lib")

// Utility
#include "Common/d3dx12.h"
#include "Utility/DxException.h"
#include "Utility/PathUtil.h"
#include "Utility/d3dUtil.h"
#include "Utility/MathHelper.h"
#include "Utility/Singleton.h"
#include "Utility/Debug.h"
#include "Utility/HashUtils.hpp"

// C++
#include <array>
#include <vector>
#include <queue>
#include <deque>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <memory>
#include <string>
#include <exception>
#include <shellapi.h>
#include <cassert>
#include <iostream>
#include <limits>

// ENGINE
#include "Common/Align.h"
#include "Math/Common.h"
#include "Math/VectorMath.h"
#include "Common/GraphicsEnums.h"
#include "Common/ConstantObject.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

// Dynamic resource page
#define DYNAMIC_RESOURCE_PAGE_SIZE 1048576

#endif //PCH_H
