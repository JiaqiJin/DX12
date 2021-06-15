#include "../pch.h"
#include "CommandContext.h"

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
		m_CommandList(nullptr)
	{

	}

	CommandContext::~CommandContext()
	{
		// TODO
	}

	void CommandContext::Initialize()
	{
		// TODO
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);
		//m_CommandList->Reset()
		// TODO
	}

	CommandContext& CommandContext::Begin(const std::wstring ID = L"")
	{
		CommandContext* newContext = CommandContextManager::GetSingleton().AllocateCommandContext(D3D12_COMMAND_LIST_TYPE_DIRECT);
		newContext->SetID(ID);

		return *newContext;
	}
}