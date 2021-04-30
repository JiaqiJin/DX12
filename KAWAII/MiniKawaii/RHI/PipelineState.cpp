#include "../pch.h"
#include "PipelineState.h"
#include "RenderDevice.h"

namespace RHI
{
	PipelineState::PipelineState(RenderDevice* renderDivice)
		: m_RenderDevice(renderDivice)
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