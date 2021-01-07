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

namespace D3D12
{
	GLFWwindow* Renderer::initialize(int width, int height, int maxSamples)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		GLFWwindow* window = glfwCreateWindow(width, height, "Physically Based Rendering (Direct3D 12)", nullptr, nullptr);
		if (!window) {
			throw std::runtime_error("Failed to create window");
		}

		UINT dxgiFactoryFlags = 0;

#ifdef _DEBUG
		{
			ComPtr<ID3D12Debug> debugController;
			if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
				debugController->EnableDebugLayer();
				dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
			}
		}
#endif

		ComPtr<IDXGIFactory4> dxgiFactory;
		if (FAILED(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)))) {
			throw std::runtime_error("Failed to create DXGI factory");
		}

		// Find D3D12 compatible adapter and create the device object.
		std::string dxgiAdapterDescription;
		{
			ComPtr<IDXGIAdapter1> adapter = getAdapter(dxgiFactory);
			if (adapter) {
				DXGI_ADAPTER_DESC adapterDesc;
				adapter->GetDesc(&adapterDesc);
				dxgiAdapterDescription = Utility::convertToUTF8(adapterDesc.Description);
			}
			else {
				throw std::runtime_error("No suitable video adapter supporting D3D12 found");
			}

			if (FAILED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)))) {
				throw std::runtime_error("Failed to create D3D12 device");
			}
		}

		// Create default direct command queue.
		D3D12_COMMAND_QUEUE_DESC queueDesc = {};
		queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		if (FAILED(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)))) {
			throw std::runtime_error("Failed to create command queue");
		}

		// Create window swap chain.
		{
			DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
			swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDesc.SampleDesc.Count = 1;
			swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDesc.BufferCount = NumFrames;
			swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

			ComPtr<IDXGISwapChain1> swapChain;
			if (FAILED(dxgiFactory->CreateSwapChainForHwnd(m_commandQueue.Get(), glfwGetWin32Window(window), &swapChainDesc, nullptr, nullptr, &swapChain))) {
				throw std::runtime_error("Failed to create swap chain");
			}
			swapChain.As(&m_swapChain);
		}

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
		dxgiFactory->MakeWindowAssociation(glfwGetWin32Window(window), DXGI_MWA_NO_ALT_ENTER);

		//Determine max support MSAA level
		UINT samples;
		for (samples = maxSamples; samples > 1; samples /= 2) {
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS mqlColor = { DXGI_FORMAT_R16G16B16A16_FLOAT, samples };
			D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS mqlDepthStencil = { DXGI_FORMAT_D24_UNORM_S8_UINT, samples };
			m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &mqlColor, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
			m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &mqlDepthStencil, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
			if (mqlColor.NumQualityLevels > 0 && mqlDepthStencil.NumQualityLevels > 0) {
				break;
			}
		}

		// Determine supported root signature version.
		{
			D3D12_FEATURE_DATA_ROOT_SIGNATURE rootSignatureFeature = { D3D_ROOT_SIGNATURE_VERSION_1_1 };
			m_device->CheckFeatureSupport(D3D12_FEATURE_ROOT_SIGNATURE, &rootSignatureFeature, sizeof(D3D12_FEATURE_DATA_ROOT_SIGNATURE));
			m_rootSignatureVersion = rootSignatureFeature.HighestVersion;
		}

		//Create descripto heaps
		m_descHeapRTV = createDescriptorHeap({ D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
		m_descHeapDSV = createDescriptorHeap({ D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 16, D3D12_DESCRIPTOR_HEAP_FLAG_NONE });
		m_descHeapCBV_SRV_UAV = createDescriptorHeap({ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE });

		//Create per frame resource
		for (UINT frameIndex = 0; frameIndex < NumFrames; ++frameIndex)
		{
			if (FAILED(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[frameIndex]))))
			{
				throw std::runtime_error("Failed to create command allocator");
			}

			if (FAILED(m_swapChain->GetBuffer(frameIndex, IID_PPV_ARGS(&m_backbuffers[frameIndex].buffer))))
			{
				throw std::runtime_error("Failed to retrieve swap chain back buffer");
			}

			m_backbuffers[frameIndex].rtv = m_descHeapRTV.alloc();
			m_device->CreateRenderTargetView(m_backbuffers[frameIndex].buffer.Get(), nullptr, m_backbuffers[frameIndex].rtv.cpuHandle);

			m_framebuffers[frameIndex] = createFrameBuffer(width, height, samples, DXGI_FORMAT_R16G16B16A16_FLOAT, DXGI_FORMAT_D24_UNORM_S8_UINT);
			if (samples > 1) {
				m_resolveFramebuffers[frameIndex] = createFrameBuffer(width, height, 1, DXGI_FORMAT_R16G16B16A16_FLOAT, (DXGI_FORMAT)0);
			}
			else {
				m_resolveFramebuffers[frameIndex] = m_framebuffers[frameIndex];
			}
		}

		// Create frame synchronization objects.
		{
			if (FAILED(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
			{
				throw std::runtime_error("Failed to create fence object");
			}
			m_fenceCompletionEvent = CreateEvent(nullptr, false, false, nullptr);
		}

		std::printf("Direct3D 12 Renderer [%s]\n", dxgiAdapterDescription.c_str());
		return window;
	}

} // end namespace
