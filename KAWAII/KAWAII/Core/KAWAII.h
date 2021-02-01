#pragma once

#include "stdafx.h"

#pragma warning(disable:4250)

#define APPEND_ALIGNED_ELEMENT 0xffffffff

namespace KAWAII
{
	enum class Format : uint32_t;
	enum class CommandListType : uint8_t;
	enum class IndirectArgumentType : uint32_t;
	enum class CommandQueueFlag : uint8_t;
	enum class FenceFlag : uint8_t;

	enum Requirement : uint32_t
	{
		REQ_MIP_LEVELS,
		REQ_TEXTURECUBE_DIMENSION,
		REQ_TEXTURE1D_U_DIMENSION,
		REQ_TEXTURE2D_U_OR_V_DIMENSION,
		REQ_TEXTURE3D_U_V_OR_W_DIMENSION,
		REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION,
		REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION
	};

	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_indirect_argument_desc
	struct IndirectArgument
	{
		IndirectArgumentType Type;
		union
		{
			struct
			{
				uint32_t Slot;
			} VertexBuffer;
			struct
			{
				uint32_t Index;
				uint32_t DestOffsetIn32BitValues;
				uint32_t Num32BitValuesToSet;
			} Constant;
			struct
			{
				uint32_t Index;
			} ConstantBufferView;
			struct
			{
				uint32_t Index;
			} ShaderResourceView;
			struct
			{
				uint32_t Index;
			} UnorderedAccessView;
		};
	};

	class CommandList;
}

#include "KAWAIICommandConfig.h"

namespace KAWAII
{
	//--------------------------------------------------------------------------------------
	// Command list 
	//--------------------------------------------------------------------------------------
	class CommandList
	{
	public:
		CommandList() {};
		~CommandList() {};
		// Indicates that recording to the command list has finished.
		 bool Close() const;
		//Resets a command list back to its initial state as if a new command list was just created.
		 bool Reset(const CommandAllocator& allocator, const Pipeline& initialState) const ;
		//Reusing the same geometry to draw multiple obj in the scene.
		 void Draw(uint32_t  VertexCountPerInstance,
			uint32_t  InstanceCount,
			uint32_t  StartVertexLocation,
			uint32_t  StartInstanceLocation) const ;
		 void DrawIndexed(
			uint32_t indexCountPerInstance,
			uint32_t instanceCount,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation,
			uint32_t startInstanceLocation) const ;
		// Executes a command list from a thread group(execute commands in a compute shader).
		 void Dispatch(
			uint32_t ThreadGroupCountX,
			uint32_t ThreadGroupCountY,
			uint32_t ThreadGroupCountZ) const;
		// This is a support method used internally by the PIX event runtime. 
		 void BeginEvent(uint32_t metaData, const void* pData, uint32_t size) const ;
		// Clears the depth-stencil resource.
		 void ClearDepthStencilView(const Framebuffer& framebuffer, ClearFlag clearFlags,
			float depth, uint8_t stencil = 0, uint32_t numRects = 0, const RectRange* pRects = nullptr) const ;
		 void ClearDepthStencilView(const Descriptor& depthStencilView, ClearFlag clearFlags,
			float depth, uint8_t stencil = 0, uint32_t numRects = 0, const RectRange* pRects = nullptr) const ;
		// Sets all the elements in a render target to one value.
		 void ClearRenderTargetView(const Descriptor& renderTargetView, const float colorRGBA[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const;
		// Resets the state of a direct command list back to the state it was in when the command list was created.
		 void ClearState(const Pipeline& initialState) const;
		// Sets all of the elements in an unordered-access view (UAV) to the specified float values.
		 void ClearUnorderedAccessViewFloat(const DescriptorTable& descriptorTable,
			const Descriptor& descriptor, const Resource& resource, const float values[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const;
		// Sets all the elements in a unordered-access view (UAV) to the specified integer values.
		 void ClearUnorderedAccessViewUint(const DescriptorTable& descriptorTable,
			const Descriptor& descriptor, const Resource& resource, const uint32_t values[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const ;
		// Copies a region of a buffer from one resource to another.
		 void CopyBufferRegion(const Resource& dstBuffer, uint64_t dstOffset,
			const Resource& srcBuffer, uint64_t srcOffset, uint64_t numBytes) const;
		// Copies the entire contents of the source resource to the destination resource
		 void CopyResource(const Resource& dstResource, const Resource& srcResource) const;
		// GPU to copy texture data between two locations. 
		 void CopyTextureRegion(const TextureCopyLocation& dst,
			uint32_t dstX, uint32_t dstY, uint32_t dstZ,
			const TextureCopyLocation& src, const BoxRange* pSrcBox = nullptr) const;
		// Copies tiles from buffer to tiled resource or vice versa.
		 void CopyTiles(const Resource& tiledResource, const TiledResourceCoord* pTileRegionStartCoord,
			const TileRegionSize* pTileRegionSize, const Resource& buffer, uint64_t bufferStartOffsetInBytes,
			TileCopyFlags flags) const ;
		 void EndEvent() ;
		// Executes a bundle(Bundles inherit all state from the parent command list on which ExecuteBundle is called).
		 void ExecuteBundle(CommandList& commandList) const;
		// Apps perform indirect draws/dispatches using the ExecuteIndirect method.
		 void ExecuteIndirect(CommandLayout commandlayout, uint32_t maxCommandCount,
			const Resource& argumentBuffer, uint64_t argumentBufferOffset = 0,
			const Resource& countBuffer = nullptr, uint64_t countBufferOffset = 0) ;
		// Set view for the index buffer
		 void IASetIndexBuffer(const IndexBufferView& view) const;
		// Bind information about the primitive type, and data order that describes input data for the input assembler stage.
		 void IASetPrimitiveTopology(PrimitiveTopology primitiveTopology) const ;
		// Sets a CPU descriptor handle for the vertex buffers.
		 void IASetVertexBuffers(uint32_t startSlot, uint32_t numViews, const VertexBufferView* pViews) const ;
		// Sets the blend factor that modulate values for a pixel shader, render target, or both.
		 void OMSetBlendFactor(const float blendFactor[4]) const ;
		// Sets CPU descriptor handles for the render targets and depth stencil.
		 void OMSetRenderTargets(
			uint32_t numRenderTargetDescriptors,
			const Descriptor* pRenderTargetViews,
			const Descriptor* pDepthStencilView = nullptr,
			bool rtsSingleHandleToDescriptorRange = false) const;
		// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-omsetrendertargets
		 void OMSetFramebuffer(const Framebuffer& framebuffer) const;
		// Sets the reference value for depth stencil tests.
		 void OMSetStencilRef(uint32_t stencilRef) const ;
		// Copy a multi-sampled resource into a non-multi-sampled resource.
		 void ResolveSubresource(const Resource& dstResource, uint32_t dstSubresource,
			const Resource& srcResource, uint32_t srcSubresource, Format format) const ;
		// Notifies the driver that it needs to synchronize multiple accesses to resources.
		 void Barrier(uint32_t numBarriers, const ResourceBarrier* pBarriers) const;
		// Binds an array of scissor rectangles to the rasterizer stage.
		 void RSSetScissorRects(uint32_t numRects, const RectRange* pRects) const;
		// Bind an array of viewports to the rasterizer stage of the pipeline.
		 void RSSetViewports(uint32_t numViewports, const Viewport* pViewports) const;
		 void SetDescriptorPools(uint32_t numDescriptorPools, const DescriptorPool* pDescriptorPools) const;
		 void SetComputePipelineLayout(const PipelineLayout& pipelineLayout) const;
		 void SetGraphicsPipelineLayout(const PipelineLayout& pipelineLayout) const;
		 void SetComputeDescriptorTable(uint32_t index, const DescriptorTable& descriptorTable) const;
		 void SetGraphicsDescriptorTable(uint32_t index, const DescriptorTable& descriptorTable) const;
		 void SetCompute32BitConstant(uint32_t index, uint32_t srcData, uint32_t destOffsetIn32BitValues = 0) const ;
		 void SetGraphics32BitConstant(uint32_t index, uint32_t srcData, uint32_t destOffsetIn32BitValues = 0) const;
		 void SetCompute32BitConstants(uint32_t index, uint32_t num32BitValuesToSet,
			const void* pSrcData, uint32_t destOffsetIn32BitValues = 0) const;
		 void SetGraphics32BitConstants(uint32_t index, uint32_t num32BitValuesToSet,
			const void* pSrcData, uint32_t destOffsetIn32BitValues = 0) const;
		 void SetComputeRootConstantBufferView(uint32_t index, const Resource& resource, int offset = 0) const;
		 void SetGraphicsRootConstantBufferView(uint32_t index, const Resource& resource, int offset = 0) const;
		 void SetComputeRootShaderResourceView(uint32_t index, const Resource& resource, int offset = 0) const;
		 void SetGraphicsRootShaderResourceView(uint32_t index, const Resource& resource, int offset = 0) const;
		 void SetComputeRootUnorderedAccessView(uint32_t index, const Resource& resource, int offset = 0) const;
		 void SetGraphicsRootUnorderedAccessView(uint32_t index, const Resource& resource, int offset = 0) const;
		// Not intended to be called directly.  Use the PIX event runtime to insert events into a command list.
		 void SetMarker(uint32_t metaData, const void* pData, uint32_t size) const;
		// Sets all shaders and programs most of the fixed-function state of the graphics processing unit (GPU) pipeline.
		 void SetPipelineState(const Pipeline& pipelineState) const;
		 void SOSetTargets(uint32_t startSlot, uint32_t numViews, const StreamOutBufferView* pViews) const;

		com_ptr<ID3D12GraphicsCommandList>& GetGraphicsCommandList();

		using uptr = std::unique_ptr<CommandList>;
		using sptr = std::shared_ptr<CommandList>;

	protected:
		com_ptr<ID3D12GraphicsCommandList> m_commandList;
	};

	//--------------------------------------------------------------------------------------
	// Constant buffer
	//--------------------------------------------------------------------------------------
	//--------------------------------------------------------------------------------------
	// Constant buffer
	//--------------------------------------------------------------------------------------
	class DLL_INTERFACE ConstantBuffer
	{
	public:
		//ConstantBuffer();
		virtual ~ConstantBuffer() {};

		// Creating the Constant Buffer View
		virtual bool Create(const Device& device, size_t byteWidth, uint32_t numCBVs = 1,
			const size_t* offsets = nullptr, MemoryType memoryType = MemoryType::UPLOAD,
			const wchar_t* name = nullptr) = 0;

		virtual bool Upload(CommandList* pCommandList, Resource& uploader, const void* pData,
			size_t size, uint32_t cbvIndex = 0, ResourceState srcState = ResourceState::COMMON,
			ResourceState dstState = ResourceState::COMMON) = 0;

		virtual const Resource& GetResource() const = 0;
		virtual const Descriptor& GetCBV(uint32_t index = 0) const = 0;

		// Mapping the Constant Buffer
		virtual void* Map(uint32_t cbvIndex = 0) = 0;
		// Clear the Constant Buffer Data
		virtual void Unmap() = 0;


		using uptr = std::unique_ptr<ConstantBuffer>;
		using sptr = std::shared_ptr<ConstantBuffer>;
	};


}