#pragma once

#include "../stdafx.h"
#include "KAWAII.h"
//https://software.intel.com/content/www/us/en/develop/articles/introduction-to-resource-binding-in-microsoft-directx-12.html
namespace KAWAII
{
	//--------------------------------------------------------------------------------------
	// Resource base 
	// https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
	//--------------------------------------------------------------------------------------
	class ResourceBase
	{
	public:
		ResourceBase();
		~ResourceBase() {};
		//(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_vertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, 
		//D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
		uint32_t SetBarrier(ResourceBarrier* barrier, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t subResource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flags = BarrierFlag::NONE);

		const Resource& GetResource() const;
		const Descriptor& GetSRV(uint32_t index = 0) const;

		ResourceBarrier	Transition(ResourceState dstState, uint32_t subresource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flag = BarrierFlag::NONE);
		ResourceState GetResourceState(uint32_t subresource = 0) const;

		Format GetFormat() const;
		uint32_t GetWidth() const;

		using uptr = std::unique_ptr<ResourceBase>;
		using sptr = std::shared_ptr<ResourceBase>;

	protected:
		void setDevice(const Device& device);
		Descriptor allocateSrvUavPool();

		Device m_device;
		Resource m_resource;
		Format m_format;
		std::vector<DescriptorPool>	m_srvUavPools;
		std::vector<Descriptor> m_srvs;
		std::vector<ResourceState> m_states;
		std::wstring m_name;
	};

	//--------------------------------------------------------------------------------------
	// 2D Texture
	//--------------------------------------------------------------------------------------
	class Texture2D : public ResourceBase
	{
	public:
		Texture2D();
		~Texture2D() {};

		bool Create(const Device& device, uint32_t width, uint32_t height, Format format,
			uint32_t arraySize = 1, ResourceFlag resourceFlags = ResourceFlag::NONE,
			uint8_t numMips = 1, uint8_t sampleCount = 1, MemoryType memoryType = MemoryType::DEFAULT,
			bool isCubeMap = false, const wchar_t* name = nullptr);
		bool Upload(CommandList* pCommandList, Resource& uploader,
			const SubresourceData* pSubresourceData, uint32_t numSubresources = 1,
			ResourceState dstState = ResourceState::COMMON, uint32_t firstSubresource = 0);
		bool Upload(CommandList* pCommandList, Resource& uploader, const void* pData,
			uint8_t stride = sizeof(float), ResourceState dstState = ResourceState::COMMON);
		bool CreateSRVs(uint32_t arraySize, Format format = Format::UNKNOWN, uint8_t numMips = 1,
			uint8_t sampleCount = 1, bool isCubeMap = false);
		bool CreateSRVLevels(uint32_t arraySize, uint8_t numMips, Format format = Format::UNKNOWN,
			uint8_t sampleCount = 1, bool isCubeMap = false);
		bool CreateUAVs(uint32_t arraySize, Format format = Format::UNKNOWN, uint8_t numMips = 1,
			std::vector<Descriptor>* pUavs = nullptr);

		uint32_t SetBarrier(ResourceBarrier* pBarriers, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t subresource = BARRIER_ALL_SUBRESOURCES,
			BarrierFlag flags = BarrierFlag::NONE);
		uint32_t SetBarrier(ResourceBarrier* pBarriers, uint8_t mipLevel, ResourceState dstState,
			uint32_t numBarriers = 0, uint32_t slice = 0, BarrierFlag flags = BarrierFlag::NONE);

		void Blit();
		uint32_t Blit(uint32_t numBarriers = 0);
		uint32_t GenerateMips();

		const Descriptor& GetUAV(uint8_t index = 0) const;
		const Descriptor& GetPackedUAV(uint8_t index = 0) const;
		const Descriptor& GetSRVLevel(uint8_t level) const;

		uint32_t	GetHeight() const;
		uint32_t	GetArraySize() const;
		uint8_t		GetNumMips() const;

		Texture2D* AsTexture2D();

		using uptr = std::unique_ptr<Texture2D>;
		using sptr = std::shared_ptr<Texture2D>;

	protected:
		bool create(const Device& device, uint32_t width, uint32_t height,
			uint32_t arraySize, Format format, uint8_t numMips, uint8_t sampleCount,
			ResourceFlag resourceFlags, const float* pClearColor, bool isCubeMap,
			const wchar_t* name);
		std::vector<Descriptor>	m_uavs;
		std::vector<Descriptor>	m_packedUavs;
		std::vector<Descriptor>	m_srvLevels;
	};

	//--------------------------------------------------------------------------------------
	// 3D Texture
	//--------------------------------------------------------------------------------------
	class Texture3D : public Texture2D
	{
	public:
		Texture3D();
		~Texture3D() {};

		bool Create(const Device& device, uint32_t width, uint32_t height, uint32_t depth,
			Format format, ResourceFlag resourceFlags = ResourceFlag::NONE, uint8_t numMips = 1,
			MemoryType memoryType = MemoryType::DEFAULT, const wchar_t* name = nullptr);
		bool CreateSRVs(Format format = Format::UNKNOWN, uint8_t numMips = 1);
		bool CreateSRVLevels(uint8_t numMips, Format format = Format::UNKNOWN);
		bool CreateUAVs(Format format = Format::UNKNOWN, uint8_t numMips = 1,
			std::vector<Descriptor>* pUavs = nullptr);

		uint32_t GetDepth() const ;

		using uptr = std::unique_ptr<Texture3D>;
		using sptr = std::shared_ptr<Texture3D>;
	};

	//--------------------------------------------------------------------------------------
	// Raw buffer
	//--------------------------------------------------------------------------------------
	class RawBuffer : public ResourceBase
	{
	public:
		//RawBuffer();
		virtual ~RawBuffer() {};

		virtual bool Create(const Device& device, size_t byteWidth, ResourceFlag resourceFlags = ResourceFlag::NONE,
			MemoryType memoryType = MemoryType::DEFAULT, uint32_t numSRVs = 1) = 0; //TO-DO

		virtual bool Upload(CommandList* pCommandList, Resource& uploader, const void* pData, size_t size,
			uint32_t descriptorIndex = 0, ResourceState dstState = ResourceState::COMMON) = 0;
		virtual bool CreateSRVs() = 0;
		virtual bool CreateUAVs() = 0;

		virtual const Descriptor& GetUAV(uint32_t index = 0) const = 0;

		virtual void* Map(uint32_t descriptorIndex = 0, uintptr_t readBegin = 0, uintptr_t readEnd = 0) = 0;
		virtual void* Map(const Range* pReadRange, uint32_t descriptorIndex = 0) = 0;
		virtual void Unmap() = 0;

		using uptr = std::unique_ptr<RawBuffer>;
		using sptr = std::shared_ptr<RawBuffer>;
	};

}