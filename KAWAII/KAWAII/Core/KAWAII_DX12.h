#pragma once
#include "stdafx.h"

#define H_RETURN(x, o, m, r)	{ const auto hr = x; if (FAILED(hr)) { o << m << std::endl; return r; } }

#define BARRIER_ALL_SUBRESOURCES	D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES

namespace KAWAII
{
#if _HAS_CXX17
	template <typename T>
	class com_ptr :
		public winrt::com_ptr<T>
	{
	public:
		using element_type = T;
		using winrt::com_ptr<T>::com_ptr;

		element_type** operator&() noexcept { return this->put(); }
	};
#else
	template <typename T>
	class com_ptr :
		public Microsoft::WRL::ComPtr<T>
	{
	public:
		using element_type = T;
		using Microsoft::WRL::ComPtr<T>::ComPtr;

		element_type* get() const throw() { return this->Get(); }
	};
#endif

	__forceinline uint8_t Log2(uint32_t value)
	{
		unsigned long mssb; // most significant set bit

		if (BitScanReverse(&mssb, value) > 0)
			return static_cast<uint8_t>(mssb);
		else return 0;
	}

	inline std::string HrToString(HRESULT hr)
	{
		char s_str[64] = {};
		sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<uint32_t>(hr));

		return std::string(s_str);
	}

	// Swapchain and blobs
	using Blob = com_ptr<ID3D10Blob>;
	using SwapChain = com_ptr<IDXGISwapChain3>;

	//Command lists relates
	using CommandAllocator = com_ptr<ID3D12CommandAllocator>;
	using Fence = com_ptr<ID3D12Fence>;
	
	MIDL_INTERFACE("0ec870a6-5d7e-4c22-8cfc-5baae07616ed")
	DLL_INTERFACE DX12CommandQueue : public ID3D12CommandQueue
	{

	};

	// Resources related
	using Resource = com_ptr<ID3D12Resource>;
	using VertexBufferView = D3D12_VERTEX_BUFFER_VIEW;
	using IndexBufferView = D3D12_INDEX_BUFFER_VIEW;
	using StreamOutBufferView = D3D12_STREAM_OUTPUT_BUFFER_VIEW;
	using Sampler = std::shared_ptr<D3D12_SAMPLER_DESC>;
	using ResourceBarrier = CD3DX12_RESOURCE_BARRIER;
	using TileCopyFlags = D3D12_TILE_COPY_FLAGS;

	// Descriptors related
	using DescriptorPool = com_ptr<ID3D12DescriptorHeap>;

	// Pipeline layouts related
	using PipelineLayout = com_ptr<ID3D12RootSignature>;
	using DescriptorRangeList = std::vector<CD3DX12_DESCRIPTOR_RANGE1>;

	struct RootParameter : public CD3DX12_ROOT_PARAMETER1
	{
		DescriptorRangeList ranges;
	};
	using DescriptorTableLayout = std::shared_ptr<RootParameter>;

	using Pipeline = com_ptr<ID3D12PipelineState>;
	// A command signature object enables apps to specify indirect drawing, including the buffer format etc.
	using CommandLayout = com_ptr<ID3D12CommandSignature>;

	//Devices
	MIDL_INTERFACE("189819f1-1db6-4b57-be54-1821339b85f7")
	DLL_INTERFACE DX12Device : public ID3D12Device
	{

	};
	using Device = com_ptr<DX12Device>;

	// Graphics pipelines related
	namespace Graphics
	{
		using Blend = std::shared_ptr<D3D12_BLEND_DESC>;
		using Rasterizer = std::shared_ptr<D3D12_RASTERIZER_DESC>;
		using DepthStencil = std::shared_ptr<D3D12_DEPTH_STENCIL_DESC>;
	}

}