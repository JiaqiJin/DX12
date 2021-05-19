#include "../pch.h"
#include "CommandContext.h"
#include "RenderDevice.h"
#include "CommandListManager.h"

namespace RHI
{
	CommandContext* ContextManager::AllocateContext(D3D12_COMMAND_LIST_TYPE type)
	{
		auto& availableContexts = m_AvailableContexts[type];
		CommandContext* context = nullptr;

		if (availableContexts.empty())
		{
			context = new CommandContext(type);
			m_ContextPool[type].emplace_back(context);
			context->Initialize();
		}
		else
		{
			context = availableContexts.front();
			availableContexts.pop();
			context->Reset();
		}

		assert(context != nullptr);

		assert(context->m_type == type);

		return context;
	}

	void ContextManager::FreeContext(CommandContext* usedContext)
	{
		assert(usedContext != nullptr);
		m_AvailableContexts[usedContext->m_type].push(usedContext);
	}

	CommandContext::CommandContext(D3D12_COMMAND_LIST_TYPE type)
		: m_type(type),
		m_CommandList(nullptr),
		m_CurrentAllocator(nullptr),
		m_numBarriersToFlush(0),
		m_DynamicGPUDescriptorAllocator(RenderDevice::GetSingleton().GetGPUDescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV), 128, "DynamicDescriptorMgr"),
		m_DynamicResourceHeap(RenderDevice::GetSingleton().GetDynamicResourceAllocator(), DYNAMIC_RESOURCE_PAGE_SIZE)
	{

	}

	CommandContext::~CommandContext()
	{
		m_DynamicGPUDescriptorAllocator.ReleaseAllocations();
		m_DynamicResourceHeap.ReleaseAllocatedPages();
	}

	void CommandContext::Initialize()
	{
		CommandListManager::GetSingleton().CreateNewCommandList(m_type, m_CommandList.GetAddressOf(), &m_CurrentAllocator);
	}

	void CommandContext::Reset()
	{
		assert(m_CommandList != nullptr && m_CurrentAllocator == nullptr);

		m_CurrentAllocator = CommandListManager::GetSingleton().GetQueue(m_type).RequestAllocator();
		m_CommandList->Reset(m_CurrentAllocator, nullptr);

		// TODO

	}
}