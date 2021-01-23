#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
//export data, functions, classes, or class member functions
#define DLL_INTERFACE __declspec(dllexport)

#include <windows.h>

#include <dxgi1_4.h>
#include <D3Dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include "d3dx12.h"

// C RunTime Header Files
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#if _HAS_CXX17
#include <winrt/base.h>
#else
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#endif
#include <functional>
#include <wrl.h>
#include <shellapi.h>

#if defined(DEBUG) | defined(_DEBUG)
#ifndef DBG_NEW
#define DBG_NEW new (_NORMAL_BLOCK, __FILE__, __LINE__)
#define new DBG_NEW
#endif
#endif  // _DEBUG