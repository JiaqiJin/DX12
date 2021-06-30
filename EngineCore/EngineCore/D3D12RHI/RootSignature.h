#pragma once

// https://logins.github.io/graphics/2020/06/26/DX12RootSignatureObject.html
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures-overview
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-root-signature
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/example-root-signatures
/*The root signature is an object intended to manage all resource bindings in a DirectX 12 application.
* Root signature defines an array of root parameters.
* Every parameter is assigned a unique root index and can be one of the following: Root constant, Root table or Root view.
* Every root table consists of one or more descriptor ranges.
* Descriptor range is a continuous range of one or more descriptors of the same type.
* Exampler DR : DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_SRV,6,2); DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_UAV,4,0);
*/

namespace RHI
{
	class RenderDevice;

	// A root parameter is one entry in the root signature.
	// A root parameter can be a root constant, root descriptor, or descriptor table.
	class RootParameter
	{
	public:
		// Three constructors, corresponding to the three types of RootParamater in DX12: "Root Constant", "Root Descriptor", "Root Table"
		// It is best not to store the Root Constant, the total size of the Root Parameter is limited, and the Root Constant will take up a lot of space
		// Root Table is a little more complicated. A Root Table can have multiple Descriptor Ranges.
		// A Descriptor Range is actually a Descriptor array of the same type (CBV, SRV, UAV)

		// Root Descriptor
		SHADER_RESOURCE_VARIABLE_TYPE
	};

	// A roor Signature define the data that the shader in current PSO
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