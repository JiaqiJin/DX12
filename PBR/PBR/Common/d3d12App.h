#pragma once

#include "Setting.h"
#include "utils.h"

namespace D3D12 
{
	using Microsoft::WRL::ComPtr;

	class Renderer
	{
	public:
		GLFWwindow* initialize(int width, int height, int maxSamples) ;
		void shutdown() ;
		void setup() ;
		void render(GLFWwindow* window, const ViewSettings& view, const SceneSettings& scene);

	private:
		DescriptorHeap createDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc) const;

		MeshBuffer createMeshBuffer(const std::shared_ptr<class Mesh>& mesh) const;
		UploadBuffer createUploadBuffer(UINT capacity) const;
		UploadBufferRegion allocFromUploadBuffer(UploadBuffer& buffer, UINT size, int align) const;
		StagingBuffer createStagingBuffer(const ComPtr<ID3D12Resource>& resource, UINT firstSubresource, UINT numSubresources, const D3D12_SUBRESOURCE_DATA* data) const;

		Texture createTexture(UINT width, UINT height, UINT depth, DXGI_FORMAT format, UINT levels = 0);
		Texture createTexture(const std::shared_ptr<class Image>& image, DXGI_FORMAT format, UINT levels = 0);
		void generateMipmap(const Texture& texture);

		void createTextureSRV(Texture& texture, D3D12_SRV_DIMENSION dimension, UINT mostDetailedMip = 0, UINT mipLevels = 0);
		void createTextureUAV(Texture& texture, UINT mipSlice);

		FrameBuffer createFrameBuffer(UINT width, UINT height, UINT samples, DXGI_FORMAT colorFormat, DXGI_FORMAT depthstencilFormat);
		void resolveFrameBuffer(const FrameBuffer& srcfb, const FrameBuffer& dstfb, DXGI_FORMAT format) const;

		ComPtr<ID3D12RootSignature> createRootSignature(D3D12_VERSIONED_ROOT_SIGNATURE_DESC& desc) const;

		ConstantBufferView createConstantBufferView(const void* data, UINT size);
		template<typename T> ConstantBufferView createConstantBufferView(const T* data = nullptr)
		{
			return createConstantBufferView(data, sizeof(T));
		}

		void executeCommandList(bool reset = true) const;
		void waitForGPU() const;
		void presentFrame();

		static ComPtr<IDXGIAdapter1> getAdapter(const ComPtr<IDXGIFactory4>& factory);
		static ComPtr<ID3DBlob> compileShader(const std::string& filename, const std::string& entryPoint, const std::string& profile);

	private:
		ComPtr<ID3D12Device> m_device;
		ComPtr<ID3D12CommandQueue> m_commandQueue;
		ComPtr<ID3D12GraphicsCommandList> m_commandList;
		ComPtr<IDXGISwapChain3> m_swapChain;

		DescriptorHeap m_descHeapRTV;
		DescriptorHeap m_descHeapDSV;
		DescriptorHeap m_descHeapCBV_SRV_UAV;

		static const UINT NumFrames = 2;
		ComPtr<ID3D12CommandAllocator> m_commandAllocators[NumFrames];
		SwapChainBuffer m_backbuffers[NumFrames];
		FrameBuffer m_framebuffers[NumFrames];
		FrameBuffer m_resolveFramebuffers[NumFrames];
		ConstantBufferView m_transformCBVs[NumFrames];
		ConstantBufferView m_shadingCBVs[NumFrames];

		struct 
		{
			ComPtr<ID3D12RootSignature> rootSignature;
			ComPtr<ID3D12PipelineState> linearTexturePipelineState;
			ComPtr<ID3D12PipelineState> gammaTexturePipelineState;
			ComPtr<ID3D12PipelineState> arrayTexturePipelineState;
		}m_mipmapGeneration;

		//Vertex index
		MeshBuffer m_pbrModel;
		MeshBuffer m_skybox;

		//Texture resource, srv, uav
		Texture m_albedoTexture;
		Texture m_normalTexture;
		Texture m_metalnessTexture;
		Texture m_roughnessTexture;

		Texture m_envTexture;
		Texture m_irmapTexture;
		Texture m_spBRDF_LUT;

		ComPtr<ID3D12RootSignature> m_tonemapRootSignature;
		ComPtr<ID3D12PipelineState> m_tonemapPipelineState;
		ComPtr<ID3D12RootSignature> m_pbrRootSignature;
		ComPtr<ID3D12PipelineState> m_pbrPipelineState;
		ComPtr<ID3D12RootSignature> m_skyboxRootSignature;
		ComPtr<ID3D12PipelineState> m_skyboxPipelineState;

		//Syncronization
		UINT m_frameIndex;
		ComPtr<ID3D12Fence> m_fence;
		HANDLE m_fenceCompletionEvent;
		mutable UINT64 m_fenceValues[NumFrames] = {};

		D3D_ROOT_SIGNATURE_VERSION m_rootSignatureVersion;
	};
}