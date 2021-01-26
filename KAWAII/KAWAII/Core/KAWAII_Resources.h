#pragma once

#include "KAWAII.h"

namespace KAWAII
{
	//--------------------------------------------------------------------------------------
	// Resource base 
	// https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
	//--------------------------------------------------------------------------------------
	class DLL_INTERFACE ResourceBase
	{
	public:
		//ResourceBase();
		virtual ~ResourceBase() {};
		//(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, 
		//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		virtual uint32_t SetBarrier(ResourceBarrier* barrier, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t subResource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flags = BarrierFlag::NONE) = 0;

		virtual const Resource& GetResource() const = 0;
		virtual const Descriptor& GetSRV(uint32_t index = 0) const = 0;

		virtual ResourceBarrier	Transition(ResourceState dstState, uint32_t subresource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flag = BarrierFlag::NONE) = 0;
		virtual ResourceState GetResourceState(uint32_t subresource = 0) const = 0;

		virtual Format GetFormat() const = 0;

		using uptr = std::unique_ptr<ResourceBase>;
		using sptr = std::shared_ptr<ResourceBase>;
	};

	//--------------------------------------------------------------------------------------
	// 2D Texture
	//--------------------------------------------------------------------------------------
	class DLL_INTERFACE Texture2D : public virtual ResourceBase
	{
	public:
		//Texture2D();
		virtual ~Texture2D() {};

		virtual bool Create(const Device& device, uint32_t width, uint32_t height, Format format,
			uint32_t arraySize = 1, ResourceFlag resourceFlags = ResourceFlag::NONE,
			uint8_t numMips = 1, uint8_t sampleCount = 1, MemoryType memoryType = MemoryType::DEFAULT,
			bool isCubeMap = false, const wchar_t* name = nullptr) = 0;
		virtual bool Upload(CommandList* pCommandList, Resource& uploader,
			const SubresourceData* pSubresourceData, uint32_t numSubresources = 1,
			ResourceState dstState = ResourceState::COMMON, uint32_t firstSubresource = 0) = 0;
		virtual bool Upload(CommandList* pCommandList, Resource& uploader, const void* pData,
			uint8_t stride = sizeof(float), ResourceState dstState = ResourceState::COMMON) = 0;
		virtual bool CreateSRVs(uint32_t arraySize, Format format = Format::UNKNOWN, uint8_t numMips = 1,
			uint8_t sampleCount = 1, bool isCubeMap = false) = 0;
		virtual bool CreateSRVLevels(uint32_t arraySize, uint8_t numMips, Format format = Format::UNKNOWN,
			uint8_t sampleCount = 1, bool isCubeMap = false) = 0;
		virtual bool CreateUAVs(uint32_t arraySize, Format format = Format::UNKNOWN, uint8_t numMips = 1,
			std::vector<Descriptor>* pUavs = nullptr) = 0;

		virtual uint32_t SetBarrier(ResourceBarrier* pBarriers, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t subresource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flags = BarrierFlag::NONE) = 0;
		virtual uint32_t SetBarrier(ResourceBarrier* pBarriers, uint8_t mipLevel, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t slice = 0, BarrierFlag flags = BarrierFlag::NONE) = 0;

		virtual void Blit() = 0;
		virtual void GenerateMips() = 0;

		virtual const Descriptor& GetUAV(uint8_t index = 0) const = 0;
		virtual const Descriptor& GetPackedUAV(uint8_t index = 0) const = 0;
		virtual const Descriptor& GetSRVLevel(uint8_t level) const = 0;

		virtual uint32_t	GetHeight() const = 0;
		virtual uint32_t	GetArraySize() const = 0;
		virtual uint8_t		GetNumMips() const = 0;

		Texture2D* AsTexture2D();

		using uptr = std::unique_ptr<Texture2D>;
		using sptr = std::shared_ptr<Texture2D>;
	};

}