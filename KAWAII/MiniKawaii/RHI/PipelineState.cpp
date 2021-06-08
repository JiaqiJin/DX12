#include "../pch.h"
#include "PipelineState.h"
#include "RenderDevice.h"

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDevice, const PipelineStateDesc& desc)
		: m_RenderDevice{ renderDevice },
		m_Desc{ desc },
		m_RootSignature{ renderDevice }
	{
		auto device = m_RenderDevice->GetD3D12Device();

		D3D12_GRAPHICS_PIPELINE_STATE_DESC PSODesc{};
		// TODO
		ThrowIfFailed(device->CreateGraphicsPipelineState(&PSODesc, IID_PPV_ARGS(&m_D3D12PSO)));
	}

	PipelineState::~PipelineState()
	{
		if (m_D3D12PSO)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_D3D12PSO));
	}

}