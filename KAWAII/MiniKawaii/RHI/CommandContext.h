#pragma once

#include "DescriptorHeap.h"
#include "PipelineState.h"
#include "GpuBuffer.h"
#include "GpuTexture.h"
#include "DescriptorHeap.h"
#include "GpuRenderTextureColor.h"
#include "GpuRenderTextureDepth.h"
#include "DynamicResource.h"

#define ContextPoolSize 4
#define AvailableContextSize 4

namespace RHI
{
	class CommandContext;
	class GraphicsContext;

	// Compute Command List only supports the transition of these states
#define VALID_COMPUTE_QUEUE_RESOURCE_STATES \
	    ( D3D12_RESOURCE_STATE_UNORDERED_ACCESS \
	    | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE \
		| D3D12_RESOURCE_STATE_COPY_DEST \
		| D3D12_RESOURCE_STATE_COPY_SOURCE )

	class ContextManager : public Singleton<ContextManager>
	{
	public:
		CommandContext* AllocateContext(D3D12_COMMAND_LIST_TYPE type);
		void FreeContext(CommandContext* usedContext);

	private:
		std::vector<std::unique_ptr<CommandContext>> m_ContextPool[ContextPoolSize];
		std::queue<CommandContext*> m_AvailableContexts[AvailableContextSize];
	};

	struct NonCopyable
	{
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	/*
	* Collection of command list and command allocator
	* Calling the function "Begin" will request a Command Context and then you can start recording the command,
	* and call function "End" you will push the command into a Command Queue.
	* A new Command Allocator will be requeted at begin and this allocator will recycled at End
	* Each thread will use his own CommandContext
	*/
	class CommandContext : public NonCopyable
	{
		friend class ContextManager;
	private:
		CommandContext(D3D12_COMMAND_LIST_TYPE type);

		// Called when the CommandContext is created, this function will create the CommandList and request and Allocator
		void Initialize();
		// Called when the CommandContext is reused to reset the rendering state, this function will request an Allocator and call CL::Reset
		void Reset();

	public:
		~CommandContext();

		// Start recording commands
		static CommandContext& Begin(const std::wstring ID = L"");
		// Flush existing commands to the GPU but keep the context alive
		uint64_t Flush(bool WaitForCompletion = false);
		// Finish the command record
		uint64_t Finish(bool waitForCompletion = false, bool releaseDynamic = false);

		GraphicsContext& GetGraphicsContext()
		{
			assert(m_type != D3D12_COMMAND_LIST_TYPE_COMPUTE && "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}


		static void InitializeBuffer(GpuBuffer& dest, const void* data, size_t numBytes, size_t destOffset = 0);
		static void InitializeBuffer(GpuBuffer& dest, const GpuUploadBuffer Src, size_t srcOffset, size_t numBytes = -1, size_t destOffset = 0);
		static void InitializeTexture(GpuResource& dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA subData[]);

		// Dynamic Descriptor is allocated on GPUDescriptorHeap and released in Finish
		DescriptorHeapAllocation AllocateDynamicGPUVisibleDescriptor(UINT Count = 1);

		// Allocate memory from Dynamic Resource
		D3D12DynamicAllocation AllocateDynamicSpace(size_t numByte, size_t alignment);
		// https://docs.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12
		// GpuResource has two members : 
		// m_UsageState : Represent the current state of the resource. When TransitionResource is called, the current state of the resource
		// will be check with m_UsageState. If it not equal, submit the resource barrier.
		// m_TransitionState : Indicates the state of the resource in transition, used the split barrier to mark that resource has begin the Barrier
		// So try to cache to 16 and then execute them together (FlushResourceBarriers) ?
		void TransitionResource(GpuResource& resource, D3D12_RESOURCE_STATES newState, bool FlushImmediate = true);
		void FlushResourceBarriers(void);

		/*Rendering state and resource binding
		* When switching PSO, Static SRB will be submitted automatically, Static SRB only binds Static Shader Variable
		* When switching SRB, Mutable SRB resources will be automatically submitted
		* Before Draw Call, the Dynamic Shader Variable and Dynamic Buffer in the current SRB will be 
		* automatically submitted, and it will be checked whether the resource is updated before submission
		*/
		void SetPipelineState(PipelineState* PSO);
		void SetShaderResourceBinding(ShaderResourceBinding* SRB);
		void CommitDynamic();

		void SetDescriptorHeap(ID3D12DescriptorHeap* cbvsrvuavHeap, ID3D12DescriptorHeap* samplerHeap);
	protected:
		void SetID(const std::wstring& ID) { m_ID = ID; }
		// Command list type
		D3D12_COMMAND_LIST_TYPE m_type;
		// Command List (Command list is held by Command Contex, Command Allocator is manage by object pool)
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_CommandList;
		ID3D12CommandAllocator* m_CurrentAllocator;

		// Resource Barrier(Flush at 16)
		D3D12_RESOURCE_BARRIER m_ResourceBarrierBuffer[16];
		UINT m_numBarriersToFlush;

		// Dynamic Descriptor
		DynamicSuballocationsManager m_DynamicGPUDescriptorAllocator;

		// Dynamic Resource
		DynamicResourceHeap m_DynamicResourceHeap;

		// Resource Binding
		PipelineState* m_CurPSO = nullptr;
		ShaderResourceBinding* m_CurSRB = nullptr;

		std::wstring  m_ID;
	};

	class GraphicsContext : public CommandContext
	{
	public:
		static GraphicsContext& Begin(const std::wstring& ID = L"")
		{
			return CommandContext::Begin(ID).GetGraphicsContext();
		}

		// Clear
		void ClearColor(GpuResourceDescriptor& RTV, D3D12_RECT* Rect = nullptr);
		void ClearColor(GpuResourceDescriptor& RTV, Color Colour, D3D12_RECT* Rect = nullptr);
		void ClearDepth(GpuResourceDescriptor& DSV);
		void ClearStencil(GpuResourceDescriptor& DSV);
		void ClearDepthAndStencil(GpuResourceDescriptor& DSV);

		void SetViewport(const D3D12_VIEWPORT& vp);
		void SetScissor(const D3D12_RECT& rect);
		void SetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY Topology);

		void SetRenderTargets(UINT NumRTVs, GpuResourceDescriptor* RTVs[], GpuResourceDescriptor* DSV = nullptr);

		// Vertex Buffer、Index Buffer
		void SetVertexBuffer(UINT Slot, const D3D12_VERTEX_BUFFER_VIEW& VBView);
		void SetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW& IBView);

		// Constant Buffer
		void SetConstantBuffer(UINT RootIndex, D3D12_GPU_VIRTUAL_ADDRESS BufferAddress);

		// Descriptor
		void SetDescriptorTable(UINT RootIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable);

		void Draw(UINT VertexCount, UINT VertexStartOffset = 0);
		void DrawIndexed(UINT IndexCount, UINT StartIndexLocation = 0, INT BaseVertexLocation = 0);
		void DrawInstanced(UINT VertexCountPerInstance, UINT InstanceCount,
			UINT StartVertexLocation = 0, UINT StartInstanceLocation = 0);
		void DrawIndexedInstanced(UINT IndexCountPerInstance, UINT InstanceCount, UINT StartIndexLocation,
			INT BaseVertexLocation, UINT StartInstanceLocation);
	};
}