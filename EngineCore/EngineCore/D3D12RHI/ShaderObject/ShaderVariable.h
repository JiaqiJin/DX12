#pragma once
#include "ShaderResourceLayout.h"
#include "ShaderResourceCache.h"
#include "../GpuBuffer.h"
#include "../GpuResourceDescriptor.h"
#include "ShaderResourceBindingUtility.h"

namespace RHI
{
	// Represents a variable in Shader, external resources can be bound through this variable (a constant or a map)
	class ShaderVariable
	{
		friend class ShaderVariableCollection;
	public:
		ShaderVariable(ShaderResourceCache* cache,
			const ShaderResourceLayout::Resource& resource) :
			m_ResourceCache(cache),
			m_Resource(resource)
		{
		}

		ShaderVariable(const ShaderVariable&) = delete;
		ShaderVariable(ShaderVariable&&) = delete;
		ShaderVariable& operator=(const ShaderVariable&) = delete;
		ShaderVariable& operator=(ShaderVariable&&) = delete;

		/* Only CBV and other Buffer SRV/UAV can be bound as Root Descriptor, because when calling 
		 * ID3D12GraphicsCommandList::SetGraphicsRootConstantBufferView() function,
		 * The size of Buffer is not specified. The size of Buffer is determined by Shader.
		 * The Texture needs a lot of information to describe, it is not enough to provide only one address.
		 * There is no array when Buffer is directly bound to Root Descriptor
		 * Currently only CBV is bound as a Root Descriptor
		 * An array is also represented by a ShaderVariable, and the index in the array needs to be passed when binding resources to the array
		 */
		void Set(std::shared_ptr<GpuBuffer> buffer, UINT32 arrayIndex = 0);
		void Set(std::shared_ptr<GpuResourceDescriptor> view, UINT32 arrayIndex = 0);
		bool IsBound(UINT32 arrayIndex) const;

	private:
		ShaderResourceCache* m_ResourceCache;
		const ShaderResourceLayout::Resource& m_Resource;
	};

	/*
	 * ShaderVariableCollection holds a specific type of ShaderVariable list.
	 * PipelineState uses Manager to manage Static resources, and ShaderResourceBinding uses Manager to manage Mutable and Dynamic resources
	 * Link ShaderResourceLayout and ShaderResourceCache
	 */
	class ShaderVariableCollection
	{
	public:
		ShaderVariableCollection(ShaderResourceCache* resourceCache,
			const ShaderResourceLayout& srcLayout,
			const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
			UINT32 allowedTypeNum);

		ShaderVariableCollection(const ShaderVariableCollection&) = delete;
		ShaderVariableCollection(ShaderVariableCollection&&) = delete;
		ShaderVariableCollection& operator=(const ShaderVariableCollection&) = delete;
		ShaderVariableCollection& operator=(ShaderVariableCollection&&) = delete;

		// Getters
		ShaderVariable* GetVariable(const std::string& name);
		ShaderVariable* GetVariable(UINT32 index);
		UINT32 GetVariableCount() const { return m_Variables.size(); }
	private:
		ShaderResourceCache* m_ResourceCache;
		std::vector<std::unique_ptr<ShaderVariable>> m_Variables;
	};
}