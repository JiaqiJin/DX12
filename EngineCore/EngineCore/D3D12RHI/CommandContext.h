#pragma once
#include "GpuBuffer.h"
#include "GpuTexture.h"
#include "DescriptorHeap.h"

namespace RHI
{
	class CommandContext;
	class GraphicsContext;
	class ComputeContext;

	// Compute command only support those transition states
#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
		| D3D12_RESOURCE_STATE_COPY_DEST \
		| D3D12_RESOURCE_STATE_COPY_SOURCE )

	class CommandContextManager : public Singleton<CommandContextManager>
	{
	public:
		CommandContext* AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type);
		void FreeCommandContext(CommandContext* usedContext);

	private:
		std::vector<std::unique_ptr<CommandContext>> m_CommandContextPool[4];
		std::queue<CommandContext*> m_AvailableCommandContexts[4];
	};

	/*
	* Collection of "CommadList" and CommandListAllocator.
	* Calling Begin function will start recording the commands, 
	* and call End to push the commands into the CommandQueue.
	* A new CommandAllocator will be requested at Begin and this allocator will
	* recycled at the End.
	*/
	class CommandContext
	{
		friend class CommandContextManager;
	public:
		~CommandContext();

		CommandContext() = default;
		CommandContext(const CommandContext&) = delete;
		CommandContext& operator=(const CommandContext&) = delete;

		// Begin function : creating a command.
		static CommandContext& Begin(const std::wstring ID = L"");
		uint64_t Finish(bool WaitForCompletion = false, bool releaseDynamic = false);

		GraphicsContext& GetGraphicsContext()
		{
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE && "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext()
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}

		// Resources Initialization
		static void InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset = 0);
		static void InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes = -1, size_t DestOffset = 0);
		static void InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[]);

		// 
		void TransitionResource(GpuResource& Resource, D3D12_RESOURCE_STATES NewState, bool FlushImmediate = false);

	private:
		CommandContext(D3D12_COMMAND_LIST_TYPE type);

		// Call when the CommandContext is created. This function will create a new commandList and request an Allocator
		void Initialize();

		// Called when the CommandContext is reused to reset the rendering state
		void Reset();

	protected:
		void SetID(const std::wstring& ID) { m_ID = ID; }

		// Command list type
		D3D12_COMMAND_LIST_TYPE m_Type;
		// D3D12 Command List
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		std::wstring m_ID;
	};

	// Encapsule the list of commands for rendering (command list for exuction, for setting and clearing the pipeline state). 
	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nn-d3d12-id3d12graphicscommandlist
	class GraphicsContext : public CommandContext
	{
	public:
		static GraphicsContext& Begin(const std::wstring& ID = L"")
		{
			return CommandContext::Begin(ID).GetGraphicsContext();
		}

		//void ClearColor(D3D12_RECT* Rect = nullptr);
		//void ClearColor(D3D12_RECT* Rect = nullptr);
		//void ClearDepth();
		//void ClearStencil();
		//void ClearDepthAndStencil();

		//void SetViewport(const D3D12_VIEWPORT& vp);
		//void SetScissor(const D3D12_RECT& rect);
		//void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		//void SetRenderTargets(UINT NumRTVs);

		//// Vertex Buffer、Index Buffer
		//void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView);
		//void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView);

		//// Constant Buffer
		//void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferAddress);

		//// Descriptor
		//void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable);

		//void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
		//void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
		//void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
		//	UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
		//void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
		//	INT BaseVertexLocation, UINT StartInstanceLocation);
	};

	class ComputeContext : public CommandContext
	{
	public:
		
	};
}