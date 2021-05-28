#ifndef PCH_H
#define PCH_H

// 添加要在此处预编译的标头
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

// Util
#include "Common/d3dx12.h"
#include "Util/DxException.h"
#include "Util/d3dUtil.h"
#include "Util/MathHelper.h"
#include "Util/Singleton.h"
#include "Util/Debug.h"
#include "Util/HashUtils.hpp"

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

#include "Math/Common.h"
#include "Math/VectorMath.h"

#include "RHI/GraphicsEnums.h"
#include "Common/Align.h"

#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

// Dynamic Resource的Page大小，单位字节，1M
#define DYNAMIC_RESOURCE_PAGE_SIZE 1048576

#define NON_COPIABLE(ClassName) \
	ClassName(const ClassName&) = delete; \
	ClassName(ClassName&&) = delete; \
	ClassName& operator = (const ClassName&) = delete; \
	ClassName& operator = (ClassName&&) = delete;

#endif //PCH_H