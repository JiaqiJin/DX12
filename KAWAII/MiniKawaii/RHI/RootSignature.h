#pragma once

namespace RHI
{
	class RenderDevice;
	/*
	* Root signature define the data that shader can access. RS like parameter list for function, where function is a shader
	* and RS are parameter list of type of the data the shader access.	
	* RS(root constant, root descriptor(descriptor access often by shader) and Descriptor Table(offset and lenght into a descriptor)).
	*/
	class RootSignature
	{
	public:
		RootSignature(RenderDevice* renderDevice);
		~RootSignature();

	private:
		RenderDevice* m_RenderDevice;
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;
	};
}