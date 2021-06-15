#pragma once

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

		GraphicsContext& GetGraphicsContext()
		{
			assert(m_Type != D3D12_COMMAND_LIST_TYPE_COMPUTE && "Cannot convert async compute context to graphics");
			return reinterpret_cast<GraphicsContext&>(*this);
		}

		ComputeContext& GetComputeContext()
		{
			return reinterpret_cast<ComputeContext&>(*this);
		}

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

		
	};

	class ComputeContext : public CommandContext
	{
	public:
		
	};
}