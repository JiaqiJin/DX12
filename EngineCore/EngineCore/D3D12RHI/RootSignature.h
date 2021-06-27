#pragma once

namespace RHI
{
	class RenderDevice;

	class RootSignature
	{
	public:
		RootSignature(RenderDevice* renderDevice);
		~RootSignature();

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_pd3d12RootSignature.Get(); }
	private:
		// Root Signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;
		RenderDevice* m_RenderDevice;
	};
}