#include "KAWAII_Resources.h"
#include "KAWAIICommand_DX12.h"
#include "KAWAIIEnum_DX12.h"

namespace KAWAII
{
	Format MapToPackedFormat(Format& format)
	{
		auto formatUAV = Format::R32_UINT;
		
		switch (format)
		{
		case Format::R10G10B10A2_UNORM:
		case Format::R10G10B10A2_UINT:
			format = Format::R10G10B10A2_TYPELESS;
			break;
		case Format::R8G8B8A8_UNORM:
		case Format::R8G8B8A8_UNORM_SRGB:
		case Format::R8G8B8A8_UINT:
		case Format::R8G8B8A8_SNORM:
		case Format::R8G8B8A8_SINT:
			format = Format::R8G8B8A8_TYPELESS;
			break;
		case Format::B8G8R8A8_UNORM:
		case Format::B8G8R8A8_UNORM_SRGB:
			format = Format::B8G8R8A8_TYPELESS;
			break;
		case Format::B8G8R8X8_UNORM:
		case Format::B8G8R8X8_UNORM_SRGB:
			format = Format::B8G8R8X8_TYPELESS;
			break;
		case Format::R16G16_FLOAT:
		case Format::R16G16_UNORM:
		case Format::R16G16_UINT:
		case Format::R16G16_SNORM:
		case Format::R16G16_SINT:
			format = Format::R16G16_TYPELESS;
			break;
		default:
			formatUAV = format;
		}

		return formatUAV;
	}

	//--------------------------------------------------------------------------------------
	// Resource base
	//--------------------------------------------------------------------------------------
	ResourceBase::ResourceBase() 
		: m_device(nullptr),
		m_resource(nullptr),
		m_format(Format::UNKNOWN),
		m_srvUavPools(0),
		m_srvs(0),
		m_states(){
	}

	ResourceBase::~ResourceBase()
	{
	}

	uint32_t ResourceBase::SetBarrier(ResourceBarrier* pBarriers, ResourceState dstState,
		uint32_t numBarriers, uint32_t subresource, BarrierFlag flags)
	{
		
	}

	const Resource& ResourceBase::GetResource() const
	{
		return m_resource;
	}

	const Descriptor& ResourceBase::GetSRV(uint32_t index) const
	{
		
	}

	ResourceBarrier ResourceBase::Transition(ResourceState dstState,
		uint32_t subresource, BarrierFlag flags)
	{
		ResourceState srcState;
		if (subresource == BARRIER_ALL_SUBRESOURCES)
		{
			srcState = m_states[0];
			if (flags != BarrierFlag::BEGIN_ONLY)
				for (auto& state : m_states) state = dstState;
		}
		else
		{
			srcState = m_states[subresource];
			m_states[subresource] = flags == BarrierFlag::BEGIN_ONLY ? srcState : dstState;
		}

		return srcState == dstState && dstState == ResourceState::UNORDERED_ACCESS ?
			ResourceBarrier::UAV(m_resource.get()) :
			ResourceBarrier::Transition(m_resource.get(), GetDX12ResourceStates(srcState),
				GetDX12ResourceStates(dstState), subresource, GetDX12BarrierFlags(flags));
	}

	ResourceState ResourceBase::GetResourceState(uint32_t subresource) const
	{
		return m_states[subresource];
	}

	Format ResourceBase::GetFormat() const
	{
		return m_format;
	}

	uint32_t ResourceBase::GetWidth() const
	{
		return static_cast<uint32_t>(m_resource->GetDesc().Width);
	}

	void ResourceBase::setDevice(const Device& device)
	{
		m_device = device;
		m_srvUavPools.clear();
	}

	Descriptor ResourceBase::allocateSrvUavPool()
	{
		m_srvUavPools.emplace_back();
		auto& srvUavPool = m_srvUavPools.back();

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1;
		V_RETURN(m_device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&srvUavPool)), std::cerr, 0);

		if (!m_name.empty()) srvUavPool->SetName((m_name + L".SrvUavPool").c_str());

		return srvUavPool->GetCPUDescriptorHandleForHeapStart().ptr;
	}

	//--------------------------------------------------------------------------------------
	// 2D Texture
	//--------------------------------------------------------------------------------------
	Texture2D::Texture2D() 
		: ResourceBase(),
		m_uavs(0),
		m_packedUavs(0),
		m_srvLevels(0)
	{
	}

	Texture2D::~Texture2D()
	{
	}

	bool Texture2D::create(const Device& device, uint32_t width, uint32_t height,
		uint32_t arraySize, Format format, uint8_t numMips, uint8_t sampleCount,
		ResourceFlag resourceFlags, const float* pClearColor, bool isCubeMap,
		const wchar_t* name)
	{
		M_RETURN(!device, cerr, "The device is NULL.", false);
		setDevice(device);
		m_rtvPools.clear();

		if (name) m_name = name;

		const auto needPackedUAV = (resourceFlags & ResourceFlag::NEED_PACKED_UAV) == ResourceFlag::NEED_PACKED_UAV;
		const auto hasSRV = (resourceFlags & ResourceFlag::DENY_SHADER_RESOURCE) == ResourceFlag::NONE;
		const bool hasUAV = (resourceFlags & ResourceFlag::ALLOW_UNORDERED_ACCESS) == ResourceFlag::ALLOW_UNORDERED_ACCESS;

		// Map formats
		m_format = format;
		const auto formatUAV = needPackedUAV ? MapToPackedFormat(format) : format;

		// Setup the texture description.
		const CD3DX12_HEAP_PROPERTIES heapProperties(D3D12_HEAP_TYPE_DEFAULT);
		const auto desc = CD3DX12_RESOURCE_DESC::Tex2D(GetDXGIFormat(format),
			width, height, arraySize, numMips, sampleCount, 0,
			GetDX12ResourceFlags(ResourceFlag::ALLOW_RENDER_TARGET | resourceFlags));

		// Determine initial state
		m_states.resize(arraySize * numMips);
		for (auto& state : m_states) state = ResourceState::COMMON;

		// Optimized clear value
		D3D12_CLEAR_VALUE clearValue = { GetDXGIFormat(m_format) };
		if (pClearColor) memcpy(clearValue.Color, pClearColor, sizeof(clearValue.Color));

		// Create the render target texture.
		V_RETURN(m_device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
			GetDX12ResourceStates(m_states[0]), &clearValue, IID_PPV_ARGS(&m_resource)), std::clog, false);
		if (!m_name.empty()) m_resource->SetName((m_name + L".Resource").c_str());

		// Create SRV
		if (hasSRV)
		{
			N_RETURN(CreateSRVs(arraySize, m_format, numMips, sampleCount, isCubeMap), false);
			N_RETURN(CreateSRVLevels(arraySize, numMips, m_format, sampleCount, isCubeMap), false);
		}

		// Create UAV
		if (hasUAV)
		{
			N_RETURN(CreateUAVs(arraySize, m_format, numMips), false);
			if (needPackedUAV) N_RETURN(CreateUAVs(arraySize, formatUAV, numMips, &m_packedUavs), false);
		}

		return true;
	}

}