#include "../pch.h"
#include "ShaderResourceCache.h"
#include "../pch.h"
#include "RootSignature.h"
#include "RenderDevice.h"
#include "CommandContext.h"
#include "ShaderResourceBindingUtility.h"

using namespace std;
namespace RHI
{
	void ShaderResourceCache::Initialize(RenderDevice* device,
		const RootSignature* rootSignature,
		const SHADER_RESOURCE_VARIABLE_TYPE* allowedVarTypes,
		UINT32 allowedTypeNum)
	{
		assert(rootSignature != nullptr);

		const UINT32 allowedTypeBits = GetAllowedTypeBits(allowedVarTypes, allowedTypeNum);

		rootSignature->ProcessRootDescriptors([&](const RootParameter& rootDescriptor)
			{
				SHADER_RESOURCE_VARIABLE_TYPE variableType = rootDescriptor.GetShaderVariableType();
				UINT32 rootIndex = rootDescriptor.GetRootIndex();

				if (IsAllowedType(variableType, allowedTypeBits))
					m_RootDescriptors.insert(std::make_pair(rootIndex, RootDescriptor(variableType)));
			});

		UINT32 descriptorNum = 0;

		rootSignature->ProcessRootTables([&](const RootParameter& rootTable)
			{
				SHADER_RESOURCE_VARIABLE_TYPE variableType = rootTable.GetShaderVariableType();
				UINT32 rootIndex = rootTable.GetRootIndex();
				UINT32 rootTableSize = rootTable.GetDescriptorTableSize();
				const auto& D3D12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(rootTable); 

				assert(D3D12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);
				assert(rootTableSize > 0 && "Unexpected empty descriptor table");


				if (IsAllowedType(variableType, allowedTypeBits))
				{
					m_RootTables.insert({ rootIndex, RootTable(variableType, rootTableSize) });

					if (variableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
					{
						// Set the starting position of each Root Table in Heap, 
						// because the Root Table is closely arranged, so the starting position is the current total
						m_RootTables[rootIndex].TableStartOffset = descriptorNum;
						descriptorNum += rootTableSize;
					}

					if (variableType == SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
						m_NumDynamicDescriptor += rootTableSize;
				}
			});


		if (descriptorNum)
		{
			m_CbvSrvUavGPUHeapSpace = device->AllocateGPUDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, descriptorNum);
			assert(!m_CbvSrvUavGPUHeapSpace.IsNull() && "Failed to allocate  GPU-visible CBV/SRV/UAV descriptor");
		}

		m_D3D12Device = device->GetD3D12Device();
	}

	void ShaderResourceCache::CommitResource(CommandContext& cmdContext)
	{
		// Submit Root View (CBV), only need to bind the address of Buffer
		for (const auto& [rootIndex, rootDescriptor] : m_RootDescriptors)
		{
			// Both Dynamic Buffer and Dynamic Variable are submitted in CommitDynamic before Draw
			if (rootDescriptor.VariableType != SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC)
			{
				if (rootDescriptor.ConstantBuffer == nullptr)
					LOG_ERROR("No Resource Binding");

				GpuDynamicBuffer* dynamicBuffer = dynamic_cast<GpuDynamicBuffer*>(rootDescriptor.ConstantBuffer.get());

				//TODO
			}
		}
	}

}