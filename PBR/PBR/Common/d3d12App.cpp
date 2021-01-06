#include <algorithm>
#include <stdexcept>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <d3dx12.h>
#include <d3dcompiler.h>

#include "image.h"
#include "mesh.h"

#include "d3d12App.h"