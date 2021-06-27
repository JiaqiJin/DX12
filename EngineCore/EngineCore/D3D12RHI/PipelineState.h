#pragma once

#include "RootSignature.h"

namespace RHI
{
	class RenderDevice;

	// Graphic Pipeline
	struct GraphicsPipelineDesc
	{
		// TODO
		// Shaders (Vertex, Pixels, Geometry ...)
		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicPipelineState;
	};

	struct PipelineStateDesc
	{
		std::wstring Name = L"Default PSO";

		PIPELINE_TYPE PipelineType;

		GraphicsPipelineDesc GraphicsPipeline;
	};

	class PipelineState
	{
	public:
		PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc);
		~PipelineState();

		// Getters
		ID3D12PipelineState* GetD3D12PipelineState() const	{ return m_D3D12PSO.Get(); }
		ID3D12RootSignature* GetD3D12RootSignature() const { return m_RootSignature.GetD3D12RootSignature(); }
		const RootSignature* GetRootSignature() const { return &m_RootSignature; }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }
	private:
		// PSO
		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
		// Root Signature
		RootSignature m_RootSignature;
		RenderDevice* m_RenderDevice;
		PipelineStateDesc m_Desc;
	};
}