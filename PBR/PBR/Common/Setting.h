#pragma once
/* https://software.intel.com/content/www/us/en/develop/articles/tutorial-migrating-your-apps-to-directx-12-part-3.html */
#include <memory>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

struct ViewSettings
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	float distance;
	float fov;
};

struct SceneSettings
{
	float pitch = 0.0f;
	float yaw = 0.0f;
	
	static const int NumLights = 3;
	struct Light {
		glm::vec3 direction;
		glm::vec3 radiance;
		bool enabled = false;
	} lights[NumLights];
};

namespace D3D12
{
	using Microsoft::WRL::ComPtr;
	using namespace std;

	struct Descriptor
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

	struct DescriptorHeap
	{
		ComPtr<ID3D12DescriptorHeap> heap;
		UINT descriptorSize;
		UINT numDescriptorsInHeap;
		UINT numDescriptorsAllocated;

		Descriptor alloc()
		{
			return (*this)[numDescriptorsAllocated++];
		}
		Descriptor operator[](UINT index) const
		{
			assert(index < numDescriptorsInHeap);
			return {
				D3D12_CPU_DESCRIPTOR_HANDLE{heap->GetCPUDescriptorHandleForHeapStart().ptr + index * descriptorSize},
				D3D12_GPU_DESCRIPTOR_HANDLE{heap->GetGPUDescriptorHandleForHeapStart().ptr + index * descriptorSize}
			};
		}
	};

	struct DescriptorHeapMark
	{
		DescriptorHeapMark(DescriptorHeap& heap)
			: heap(heap)
			, mark(heap.numDescriptorsAllocated)
		{}
		~DescriptorHeapMark()
		{
			heap.numDescriptorsAllocated = mark;
		}
		DescriptorHeap& heap;
		const UINT mark;
	};

	struct StagingBuffer
	{
		std::vector<D3D12_PLACED_SUBRESOURCE_FOOTPRINT> layouts;
		ComPtr<ID3D12Resource> buffer;
		UINT firstSubresource;
		UINT numSubresources;
	};

	struct UploadBuffer
	{
		ComPtr<ID3D12Resource> buffer;
		UINT capacity;
		UINT cursor;
		uint8_t* cpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
	};

	struct UploadBufferRegion
	{
		void* cpuAddress;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress;
		UINT size;
	};

	struct MeshBuffer
	{
		ComPtr<ID3D12Resource> vertexBuffer;
		ComPtr<ID3D12Resource> indexBuffer;
		D3D12_VERTEX_BUFFER_VIEW vbv;
		D3D12_INDEX_BUFFER_VIEW ibv;
		UINT numElements;
	};

	struct FrameBuffer
	{
		ComPtr<ID3D12Resource> colorTexture;
		ComPtr<ID3D12Resource> depthStencilTexture;
		Descriptor rtv;
		Descriptor dsv;
		Descriptor srv;
		UINT width, height;
		UINT samples;
	};

	struct SwapChainBuffer
	{
		ComPtr<ID3D12Resource> buffer;
		Descriptor rtv;
	};

	struct ConstantBufferView
	{
		UploadBufferRegion data;
		Descriptor cbv;

		template<typename T> T* as() const
		{
			return reinterpret_cast<T*>(data.cpuAddress);
		}
	};

	struct Texture
	{
		ComPtr<ID3D12Resource> texture;
		Descriptor srv;
		Descriptor uav;
		UINT width, height;
		UINT levels;
	};
}