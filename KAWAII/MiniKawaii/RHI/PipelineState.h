#pragma once

namespace RHI
{
	class RenderDevice;

	/*
	* Hardware setting that determine how the input data is interpreted and rendered (Setting : rasterizer state, blend state etc)
	*/
	class PipelineState
	{
	public:
		PipelineState(RenderDevice* renderDivice);
		~PipelineState();

		// GETTERS
		ID3D12PipelineState* GetD3D12PipelineState() const { return m_D3D12PSO.Get(); }
		RenderDevice* GetRenderDevice() const { return m_RenderDevice; }
	private:
		RenderDevice* m_RenderDevice;

		Microsoft::WRL::ComPtr<ID3D12PipelineState> m_D3D12PSO;
	};
}