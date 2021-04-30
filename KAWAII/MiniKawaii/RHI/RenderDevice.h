#pragma once

namespace RHI
{
	class RenderDevice : public Singleton<RenderDevice>
	{
	public:
		RenderDevice(ID3D12Device* device);
		~RenderDevice();

		ID3D12Device* GetD3D12Device() { return m_device.Get(); }

	private:
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;
	};
}