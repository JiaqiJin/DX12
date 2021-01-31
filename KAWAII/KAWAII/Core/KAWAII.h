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
	class DLL_INTERFACE CommandList
	{
	public:
		//CommandList();
		virtual ~CommandList() {};
		// Indicates that recording to the command list has finished.
		virtual bool Close() const = 0;
		//Resets a command list back to its initial state as if a new command list was just created.
		virtual bool Reset(const CommandAllocator& allocator, const Pipeline& initialState) const = 0;
		//Reusing the same geometry to draw multiple obj in the scene.
		virtual void Draw(uint32_t  VertexCountPerInstance,
			uint32_t  InstanceCount,
			uint32_t  StartVertexLocation,
			uint32_t  StartInstanceLocation) const = 0;
		virtual void DrawIndexed(
			uint32_t indexCountPerInstance,
			uint32_t instanceCount,
			uint32_t startIndexLocation,
			int32_t baseVertexLocation,
			uint32_t startInstanceLocation) const = 0;
		// Executes a command list from a thread group(execute commands in a compute shader).
		virtual void Dispatch(
			uint32_t ThreadGroupCountX,
			uint32_t ThreadGroupCountY,
			uint32_t ThreadGroupCountZ) const = 0;
		// This is a support method used internally by the PIX event runtime. 
		virtual void BeginEvent(uint32_t metaData, const void* pData, uint32_t size) const = 0;
		// Clears the depth-stencil resource.
		virtual void ClearDepthStencilView(const Framebuffer& framebuffer, ClearFlag clearFlags,
			float depth, uint8_t stencil = 0, uint32_t numRects = 0, const RectRange* pRects = nullptr) const = 0;
		virtual void ClearDepthStencilView(const Descriptor& depthStencilView, ClearFlag clearFlags,
			float depth, uint8_t stencil = 0, uint32_t numRects = 0, const RectRange* pRects = nullptr) const = 0;
		// Sets all the elements in a render target to one value.
		virtual void ClearRenderTargetView(const Descriptor& renderTargetView, const float colorRGBA[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const = 0;
		// Resets the state of a direct command list back to the state it was in when the command list was created.
		virtual void ClearState(const Pipeline& initialState) const = 0;
		// Sets all of the elements in an unordered-access view (UAV) to the specified float values.
		virtual void ClearUnorderedAccessViewFloat(const DescriptorTable& descriptorTable,
			const Descriptor& descriptor, const Resource& resource, const float values[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const = 0;
		// Sets all the elements in a unordered-access view (UAV) to the specified integer values.
		virtual void ClearUnorderedAccessViewUint(const DescriptorTable& descriptorTable,
			const Descriptor& descriptor, const Resource& resource, const uint32_t values[4],
			uint32_t numRects = 0, const RectRange* pRects = nullptr) const = 0;
		// Copies a region of a buffer from one resource to another.
		virtual void CopyBufferRegion(const Resource& dstBuffer, uint64_t dstOffset,
			const Resource& srcBuffer, uint64_t srcOffset, uint64_t numBytes) const = 0;
		// Copies the entire contents of the source resource to the destination resource
		virtual void CopyResource(const Resource& dstResource, const Resource& srcResource) const = 0;
		// GPU to copy texture data between two locations. 
		virtual void CopyTextureRegion(const TextureCopyLocation& dst,
			uint32_t dstX, uint32_t dstY, uint32_t dstZ,
			const TextureCopyLocation& src, const BoxRange* pSrcBox = nullptr) const = 0;
		// Copies tiles from buffer to tiled resource or vice versa.
		virtual void CopyTiles(const Resource& tiledResource, const TiledResourceCoord* pTileRegionStartCoord,
			const TileRegionSize* pTileRegionSize, const Resource& buffer, uint64_t bufferStartOffsetInBytes,
			TileCopyFlags flags) const = 0;
		virtual void EndEvent() = 0;
		// Executes a bundle(Bundles inherit all state from the parent command list on which ExecuteBundle is called).
		virtual void ExecuteBundle(CommandList& commandList) const = 0;
		// Apps perform indirect draws/dispatches using the ExecuteIndirect method.
		virtual void ExecuteIndirect(CommandLayout commandlayout, uint32_t maxCommandCount,
			const Resource& argumentBuffer, uint64_t argumentBufferOffset = 0,
			const Resource& countBuffer = nullptr, uint64_t countBufferOffset = 0) = 0;
		// Set view for the index buffer
		virtual void IASetIndexBuffer(const IndexBufferView& view) const = 0;
		// Bind information about the primitive type, and data order that describes input data for the input assembler stage.
		virtual void IASetPrimitiveTopology(PrimitiveTopology primitiveTopology) const = 0;
		// Sets a CPU descriptor handle for the vertex buffers.
		virtual void IASetVertexBuffers(uint32_t startSlot, uint32_t numViews, const VertexBufferView* pViews) const = 0;
		// Sets the blend factor that modulate values for a pixel shader, render target, or both.
		virtual void OMSetBlendFactor(const float blendFactor[4]) const = 0;
		// Sets CPU descriptor handles for the render targets and depth stencil.
		virtual void OMSetRenderTargets(
			uint32_t numRenderTargetDescriptors,
			const Descriptor* pRenderTargetViews,
			const Descriptor* pDepthStencilView = nullptr,
			bool rtsSingleHandleToDescriptorRange = false) const = 0;
		// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12graphicscommandlist-omsetrendertargets
		virtual void OMSetFramebuffer(const Framebuffer& framebuffer) const = 0;
		// Sets the reference value for depth stencil tests.
		virtual void OMSetStencilRef(uint32_t stencilRef) const = 0;
		// Copy a multi-sampled resource into a non-multi-sampled resource.
		virtual void ResolveSubresource(const Resource& dstResource, uint32_t dstSubresource,
			const Resource& srcResource, uint32_t srcSubresource, Format format) const = 0;
		// Notifies the driver that it needs to synchronize multiple accesses to resources.
		virtual void Barrier(uint32_t numBarriers, const ResourceBarrier* pBarriers) const = 0;
		// Binds an array of scissor rectangles to the rasterizer stage.
		virtual void RSSetScissorRects(uint32_t numRects, const RectRange* pRects) const = 0;
		// Bind an array of viewports to the rasterizer stage of the pipeline.
		virtual void RSSetViewports(uint32_t numViewports, const Viewport* pViewports) const = 0;
		virtual void SetDescriptorPools(uint32_t numDescriptorPools, const DescriptorPool* pDescriptorPools) const = 0;
		virtual void SetComputePipelineLayout(const PipelineLayout& pipelineLayout) const = 0;
		virtual void SetGraphicsPipelineLayout(const PipelineLayout& pipelineLayout) const = 0;
		virtual void SetComputeDescriptorTable(uint32_t index, const DescriptorTable& descriptorTable) const = 0;
		virtual void SetGraphicsDescriptorTable(uint32_t index, const DescriptorTable& descriptorTable) const = 0;
		virtual void SetCompute32BitConstant(uint32_t index, uint32_t srcData, uint32_t destOffsetIn32BitValues = 0) const = 0;
		virtual void SetGraphics32BitConstant(uint32_t index, uint32_t srcData, uint32_t destOffsetIn32BitValues = 0) const = 0;
		virtual void SetCompute32BitConstants(uint32_t index, uint32_t num32BitValuesToSet,
			const void* pSrcData, uint32_t destOffsetIn32BitValues = 0) const = 0;
		virtual void SetGraphics32BitConstants(uint32_t index, uint32_t num32BitValuesToSet,
			const void* pSrcData, uint32_t destOffsetIn32BitValues = 0) const = 0;
		virtual void SetComputeRootConstantBufferView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		virtual void SetGraphicsRootConstantBufferView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		virtual void SetComputeRootShaderResourceView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		virtual void SetGraphicsRootShaderResourceView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		virtual void SetComputeRootUnorderedAccessView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		virtual void SetGraphicsRootUnorderedAccessView(uint32_t index, const Resource& resource, int offset = 0) const = 0;
		// Not intended to be called directly.  Use the PIX event runtime to insert events into a command list.
		virtual void SetMarker(uint32_t metaData, const void* pData, uint32_t size) const = 0;
		// Sets all shaders and programs most of the fixed-function state of the graphics processing unit (GPU) pipeline.
		virtual void SetPipelineState(const Pipeline& pipelineState) const = 0;
		virtual void SOSetTargets(uint32_t startSlot, uint32_t numViews, const StreamOutBufferView* pViews) const = 0;
		
		using uptr = std::unique_ptr<CommandList>;
		using sptr = std::shared_ptr<CommandList>;

	};

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