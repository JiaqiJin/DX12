#include "../pch.h"
#include "CommandContext.h"
#include "CommandListManager.h"

namespace RHI
{
	CommandContext* CommandContextManager::AllocateCommandContext(D3D12_COMMAND_LIST_TYPE type)
	{
		CommandContext* context = nullptr;

		auto& avaliableCommandContexts = m_AvailableCommandContexts[type];

		if (avaliableCommandContexts.empty())
		{
			context = new CommandContext(type);
			m_CommandContextPool[type].emplace_back(context);
			context->Initialize();
		}
		else
		{
			context = avaliableCommandContexts.front();
			avaliableCommandContexts.pop();
			context->Reset();
		}

		assert(context != nullptr);
		assert(context->m_Type == type);

		return context;
	}

	void CommandContextManager::FreeCommandContext(CommandContext* usedContext)
	{
		assert(usedContext != nullptr);
		m_AvailableCommandContexts[usedContext->m_Type].push(usedContext);
	}

	// ------------------- CommandContext ------------------------------

	CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
		: m_Type(type),
		m_CommandList(nullptr),
		m_CurrentAllocator(nullptr)
	{
		// TODO
	}

	CommandContext::~CommandContext()
	{
		// TODO
	}

	void CommandContext::Initialize()
	{
		// Creating the new command list
		CommandListManager::GetSingleton().CreateNewCommandList(m_Type, m_CommandList.GetAddressOf(), &m_CurrentAllocator);
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);

		m_CurrentAllocator = CommandListManager::GetSingleton().GetQueue(m_Type).RequestAllocator();
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO
	}

	CommandContext& CommandContext::Begin(const std::wstring ID)
	{
		CommandContext* newContext = CommandContextManager::GetSingleton().AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		newContext->SetID(ID);

		return *newContext;
	}

	// Resource Initialization
	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const void* Data, size_t NumBytes, size_t DestOffset)
	{
		// TODO
	}

	void CommandContext::InitializeBuffer(GpuBuffer& Dest, const GpuUploadBuffer& Src, size_t SrcOffset, size_t NumBytes, size_t DestOffset)
	{
		// TODO
	}

	void CommandContext::InitializeTexture(GpuResource& Dest, UINT NumSubresources, D3D12_SUBRESOURCE_DATA SubData[])
	{
		// TODO
	}
}