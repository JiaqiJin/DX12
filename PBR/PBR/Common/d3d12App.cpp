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

	void Renderer::shutdown()
	{
		waitForGPU();
		CloseHandle(m_fenceCompletionEvent);
	}

	void Renderer::setup()
	{
		CD3DX12_STATIC_SAMPLER_DESC defaultSamplerDesc{ 0, D3D12_FILTER_ANISOTROPIC };
		defaultSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		CD3DX12_STATIC_SAMPLER_DESC computeSamplerDesc{ 0, D3D12_FILTER_MIN_MAG_MIP_LINEAR };
		defaultSamplerDesc.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		//Create default command list
		if (FAILED(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[0].Get(), nullptr, IID_PPV_ARGS(&m_commandList)))) {
			throw std::runtime_error("Failed to create direct command list");
		}

		//Create root signature and pipeline for tone mapping
		{
			ComPtr<ID3DBlob> tonemapVS = compileShader("shaders/hlsl/tonemap.hlsl", "main_vs", "vs_5_0");
			ComPtr<ID3DBlob> tonemapPS = compileShader("shaders/hlsl/tonemap.hlsl", "main_ps", "ps_5_0");

			const CD3DX12_DESCRIPTOR_RANGE1 descriptorRanges[] = {
			{D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0, D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE},
			};

			CD3DX12_ROOT_PARAMETER1 rootParameters[1];
			rootParameters[0].InitAsDescriptorTable(1, &descriptorRanges[0], D3D12_SHADER_VISIBILITY_PIXEL);
			CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC signatureDesc = {};
			signatureDesc.Init_1_1(1, rootParameters, 1, &computeSamplerDesc, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS);
			m_tonemapRootSignature = createRootSignature(signatureDesc);

			D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
			psoDesc.pRootSignature = m_tonemapRootSignature.Get();
			psoDesc.VS = CD3DX12_SHADER_BYTECODE(tonemapVS.Get());
			psoDesc.PS = CD3DX12_SHADER_BYTECODE(tonemapPS.Get());
			psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC{ D3D12_DEFAULT };
			psoDesc.RasterizerState.FrontCounterClockwise = true;
			psoDesc.BlendState = CD3DX12_BLEND_DESC{ D3D12_DEFAULT };
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.NumRenderTargets = 1;
			psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleMask = UINT_MAX;

			if (FAILED(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_tonemapPipelineState)))) {
				throw std::runtime_error("Failed to create tonemap pipeline state");
			}
		}

		// Create root signature & pipeline configuration for rendering PBR model.
		{
			const std::vector<D3D12_INPUT_ELEMENT_DESC> meshInputLayout = {
			{ "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,    0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			ComPtr<ID3DBlob> pbrVS = compileShader("shaders/hlsl/pbr.hlsl", "main_vs", "vs_5_0");
			ComPtr<ID3DBlob> pbrPS = compileShader("shaders/hlsl/pbr.hlsl", "main_ps", "ps_5_0");
		}
	}

	void Renderer::render(GLFWwindow* window, const ViewSettings& view, const SceneSettings& scene)
	{
	
	}

	DescriptorHeap Renderer::createDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc) const
	{
		DescriptorHeap heap;
		if (FAILED(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&heap.heap)))) {
			throw std::runtime_error("Failed to create descriptor heap");
		}
		heap.numDescriptorsAllocated = 0;
		heap.numDescriptorsInHeap = desc.NumDescriptors;
		heap.descriptorSize = m_device->GetDescriptorHandleIncrementSize(desc.Type);
		return heap;
	}

	MeshBuffer Renderer::createMeshBuffer(const std::shared_ptr<Mesh>& mesh) const
	{
		MeshBuffer buffer;
		buffer.numElements = static_cast<UINT>(mesh->faces().size() * 3);

		const size_t vertexDataSize = mesh->vertices().size() * sizeof(Mesh::Vertex);
		const size_t indexDataSize = mesh->faces().size() * sizeof(Mesh::Face);

		// Create GPU resources & initialize view structures.

		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT },
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(vertexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&buffer.vertexBuffer))))
		{
			throw std::runtime_error("Failed to create vertex buffer");
		}
		buffer.vbv.BufferLocation = buffer.vertexBuffer->GetGPUVirtualAddress();
		buffer.vbv.SizeInBytes = static_cast<UINT>(vertexDataSize);
		buffer.vbv.StrideInBytes = sizeof(Mesh::Vertex);

		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT },
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(indexDataSize),
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr,
			IID_PPV_ARGS(&buffer.indexBuffer))))
		{
			throw std::runtime_error("Failed to create index buffer");
		}
		buffer.ibv.BufferLocation = buffer.indexBuffer->GetGPUVirtualAddress();
		buffer.ibv.SizeInBytes = static_cast<UINT>(indexDataSize);
		buffer.ibv.Format = DXGI_FORMAT_R32_UINT;

		// Create and upload to CPU accessible staging buffers.
		StagingBuffer vertexStagingBuffer;
		{
			const D3D12_SUBRESOURCE_DATA data = { mesh->vertices().data() };
			vertexStagingBuffer = createStagingBuffer(buffer.vertexBuffer, 0, 1, &data);
		}
		StagingBuffer indexStagingBuffer;
		{
			const D3D12_SUBRESOURCE_DATA data = { mesh->faces().data() };
			indexStagingBuffer = createStagingBuffer(buffer.indexBuffer, 0, 1, &data);
		}

		// Enqueue upload to the GPU.
		m_commandList->CopyResource(buffer.vertexBuffer.Get(), vertexStagingBuffer.buffer.Get());
		m_commandList->CopyResource(buffer.indexBuffer.Get(), indexStagingBuffer.buffer.Get());

		const D3D12_RESOURCE_BARRIER barriers[] =
		{
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER),
			CD3DX12_RESOURCE_BARRIER::Transition(buffer.indexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER)
		};
	
		m_commandList->ResourceBarrier(2, barriers);

		// Execute graphics command list and wait for GPU to finish.
		executeCommandList();
		waitForGPU();

		return buffer;
	}

	UploadBuffer Renderer::createUploadBuffer(UINT capacity) const
	{
		UploadBuffer buffer;
		buffer.cursor = 0;
		buffer.capacity = capacity;

		if (FAILED(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(capacity),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&buffer.buffer))))
		{
			throw std::runtime_error("Failed to create GPU upload buffer");
		}
		if (FAILED(buffer.buffer->Map(0, &CD3DX12_RANGE{ 0, 0 }, reinterpret_cast<void**>(&buffer.cpuAddress)))) {
			throw std::runtime_error("Failed to map GPU upload buffer to host address space");
		}
		buffer.gpuAddress = buffer.buffer->GetGPUVirtualAddress();
		return buffer;
	}

	ComPtr<ID3DBlob> Renderer::compileShader(const std::string& filename, const std::string& entryPoint, const std::string& profile)
	{
		UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

		ComPtr<ID3DBlob> shader;
		ComPtr<ID3DBlob> errorBlob;

		std::printf("Compiling HLSL shader: %s [%s]\n", filename.c_str(), entryPoint.c_str());

		if (FAILED(D3DCompileFromFile(Utility::convertToUTF16(filename).c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), profile.c_str(), flags, 0, &shader, &errorBlob))) {
			std::string errorMsg = "Shader compilation failed: " + filename;
			if (errorBlob) {
				errorMsg += std::string("\n") + static_cast<const char*>(errorBlob->GetBufferPointer());
			}
			throw std::runtime_error(errorMsg);
		}
		return shader;
	}
} // end namespace
