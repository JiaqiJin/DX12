#pragma once

#include <memory>
#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl/client.h>

#include <glm/mat4x4.hpp>
#include <glm/glm.hpp>

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

	struct Descriptor_Handle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	};

	struct DescriptorHeap//tells shaders where to find the resource, and how to interpret the data in the resource.
	{
		ComPtr<ID3D12DescriptorHeap> heap;
		UINT descriptorSize;
		UINT numDescriptorsInHeap;
		UINT numDescriptorsAllocated;

		Descriptor_Handle alloc()
		{
			return (*this)[numDescriptorsAllocated++];
		}

		Descriptor_Handle operator[](UINT index) const
		{
			assert(index < numDescriptorsInHeap);
			return
			{
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
		Descriptor_Handle dsv;
		Descriptor_Handle rtv;
		Descriptor_Handle srv;
		UINT width, height;
		UINT samples;
	};

	struct SwapChainBuffer
	{
		ComPtr<ID3D12Resource> buffer;
		Descriptor_Handle rtv;
	};

	struct ConstantBufferView
	{
		UploadBufferRegion data;
		Descriptor_Handle cbv;
		template<typename T> T* as() const
		{
			return reinterpret_cast<T*>(data.cpuAddress);
		}
	};

	struct Texture
	{
		ComPtr<ID3D12Resource> texture;
		Descriptor_Handle srv;
		//view of an unordered access resource (which can include buffers, textures, and texture arrays, though without multi-sampling)
		Descriptor_Handle uav;
		UINT width, height;
		UINT levels;
	};
}