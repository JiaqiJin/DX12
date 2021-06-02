﻿#include "../pch.h"
#include "RootSignature.h"
#include "RenderDevice.h"
#include "ShaderResourceBindingUtility.h"
#include "ShaderResource.h"

namespace RHI
{
	void RootSignature::RootParamsManager::AddRootDescriptor(D3D12_ROOT_PARAMETER_TYPE ParameterType,
		UINT32 RootIndex,
		UINT Register,
		D3D12_SHADER_VISIBILITY Visibility,
		SHADER_RESOURCE_VARIABLE_TYPE VarType)
	{
		m_RootDescriptors.emplace_back(ParameterType, RootIndex, Register, 0u/*Register Space*/, Visibility, VarType);
	}

	void RootSignature::RootParamsManager::AddRootTable(UINT32 RootIndex,
		D3D12_SHADER_VISIBILITY Visibility,
		SHADER_RESOURCE_VARIABLE_TYPE VarType,
		UINT32 NumRangesInNewTable)
	{
		m_RootTables.emplace_back(D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, RootIndex, NumRangesInNewTable, Visibility, VarType);
	}

	void RootSignature::RootParamsManager::AddDescriptorRanges(UINT32 RootTableInd, UINT32 NumExtraRanges)
	{
		assert(RootTableInd < m_RootTables.size());
		m_RootTables[RootTableInd].AddDescriptorRanges(NumExtraRanges);
	}

	bool RootSignature::RootParamsManager::operator==(const RootParamsManager& RootParams) const
	{
		if (m_RootTables.size() != RootParams.m_RootTables.size() ||
			m_RootDescriptors.size() != RootParams.m_RootDescriptors.size())
			return false;

		for (UINT32 i = 0; i < m_RootDescriptors.size(); ++i)
		{
			const auto& rootView0 = GetRootDescriptor(i);
			const auto& rootView1 = RootParams.GetRootDescriptor(i);
			if (rootView0 != rootView1)
				return false;
		}

		for (UINT32 i = 0; i < m_RootTables.size(); ++i)
		{
			const auto& rootTable0 = GetRootTable(i);
			const auto& rootTable1 = RootParams.GetRootTable(i);
			if (rootTable0 != rootTable1)
				return false;
		}

		return true;
	}

	size_t RootSignature::RootParamsManager::GetHash() const
	{
		size_t hash = ComputeHash(m_RootTables.size(), m_RootDescriptors.size());
		for (UINT32 i = 0; i < m_RootDescriptors.size(); ++i)
			HashCombine(hash, GetRootDescriptor(i).GetHash());

		for (UINT32 i = 0; i < m_RootTables.size(); ++i)
			HashCombine(hash, GetRootTable(i).GetHash());

		return hash;
	}

	// ------------------------------- RootSignature-------------------------------------

	RootSignature::RootSignature(RenderDevice* renderDevice)
		:m_RenderDevice(renderDevice)
	{
		m_SrvCbvUavRootTablesMap.fill(InvalidRootTableIndex);
	}

	RootSignature::~RootSignature()
	{
		if (m_pd3d12RootSignature)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_pd3d12RootSignature));
	}

	void RootSignature::Finalize(ID3D12Device* pd3d12Device)
	{
		// Count the number of Descriptors in the Root Table
		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			const auto& RootTbl = m_RootParams.GetRootTable(i);
			const auto& d3d12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(RootTbl);
			assert(d3d12RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE);

			auto TableSize = RootTbl.GetDescriptorTableSize();
			assert(d3d12RootParam.DescriptorTable.NumDescriptorRanges > 0 && TableSize > 0 && "Unexpected empty descriptor table");
			auto VarType = RootTbl.GetShaderVariableType();
			m_NumDescriptorInRootTable[VarType] += TableSize;
		}

		// Count the number of Root Views
		for (UINT32 i = 0; i < m_RootParams.GetRootDescriptorNum(); ++i)
		{
			const auto& RootView = m_RootParams.GetRootDescriptor(i);
			++m_NumRootDescriptor[RootView.GetShaderVariableType()];
		}

		D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

		// Total Number of RootParameter
		auto TotalParams = m_RootParams.GetRootTableNum() + m_RootParams.GetRootDescriptorNum();

		std::vector<D3D12_ROOT_PARAMETER> D3D12Parameters(TotalParams, D3D12_ROOT_PARAMETER{});

		// Set each Root Parameter
		for (UINT32 i = 0; i < m_RootParams.GetRootTableNum(); ++i)
		{
			const auto& RootTable = m_RootParams.GetRootTable(i);
			const D3D12_ROOT_PARAMETER& SrcParam = RootTable;
			assert(SrcParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && SrcParam.DescriptorTable.NumDescriptorRanges > 0 && "Non-empty descriptor table is expected");
			D3D12Parameters[RootTable.GetRootIndex()] = SrcParam;
		}
		for (UINT32 i = 0; i < m_RootParams.GetRootDescriptorNum(); ++i)
		{
			const auto& RootView = m_RootParams.GetRootDescriptor(i);
			const D3D12_ROOT_PARAMETER& SrcParam = RootView;
			assert(SrcParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV && "Root CBV is expected");
			D3D12Parameters[RootView.GetRootIndex()] = SrcParam;
		}

		rootSignatureDesc.NumParameters = static_cast<UINT>(D3D12Parameters.size());
		rootSignatureDesc.pParameters = D3D12Parameters.size() ? D3D12Parameters.data() : nullptr;

		std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> staticSamplers = GetStaticSamplers();
		rootSignatureDesc.NumStaticSamplers = staticSamplers.size();
		rootSignatureDesc.pStaticSamplers = staticSamplers.data();

		Microsoft::WRL::ComPtr<ID3DBlob> signature = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;

		HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, signature.GetAddressOf(), error.GetAddressOf());

		if (error != nullptr)
		{
			LOG_ERROR((char*)error->GetBufferPointer());
		}
		ThrowIfFailed(hr);

		ThrowIfFailed(pd3d12Device->CreateRootSignature(0, signature->GetBufferPointer(), 
			signature->GetBufferSize(), IID_PPV_ARGS(m_pd3d12RootSignature.GetAddressOf())));
	}

	void RootSignature::AllocateResourceSlot(SHADER_TYPE ShaderType,
		PIPELINE_TYPE PipelineType,
		const ShaderResourceAttribs& ShaderResAttribs,
		SHADER_RESOURCE_VARIABLE_TYPE VariableType,
		D3D12_DESCRIPTOR_RANGE_TYPE RangeType,
		UINT32& RootIndex,				
		UINT32& OffsetFromTableStart)
	{
		const D3D12_SHADER_VISIBILITY shaderVisibility = GetShaderVisibility(ShaderType);

		// Assign a CBV as a Root Descriptor
		if (RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_CBV && ShaderResAttribs.BindCount == 1)
		{
			RootIndex = m_RootParams.GetRootTableNum() + m_RootParams.GetRootDescriptorNum();
			OffsetFromTableStart = 0;

			m_RootParams.AddRootDescriptor(D3D12_ROOT_PARAMETER_TYPE_CBV, RootIndex, ShaderResAttribs.BindPoint, 
				shaderVisibility, VariableType);
		}
		// Assign a new Root Table, or add Descriptor to the existing Root Table
		else
		{
			const INT32 ShaderInd = GetShaderTypePipelineIndex(ShaderType, PipelineType);
			assert(ShaderInd != -1);
			// Grouped according to the update frequency of the resources, Static, Mutable, and Dynamic 
			// resources are placed in three Tables respectively
			const INT32 TableIndKey = ShaderInd * SHADER_RESOURCE_VARIABLE_TYPE_NUM_TYPES + VariableType;
			UINT8& RootTableArrayInd = m_SrvCbvUavRootTablesMap[TableIndKey];
			if (RootTableArrayInd == InvalidRootTableIndex)
			{
				// The Table has not been created yet, create a new Root Table
				RootIndex = m_RootParams.GetRootTableNum() + m_RootParams.GetRootDescriptorNum();
				RootTableArrayInd = static_cast<UINT8>(m_RootParams.GetRootTableNum());
				m_RootParams.AddRootTable(RootIndex, shaderVisibility, VariableType, 1);
			}
			else
			{
				m_RootParams.AddDescriptorRanges(RootTableArrayInd, 1);
			}
			// Reference to either existing or just added table
			auto& RootTable = m_RootParams.GetRootTable(RootTableArrayInd);
			RootIndex = RootTable.GetRootIndex();

			const auto& d3d12RootParam = static_cast<const D3D12_ROOT_PARAMETER&>(RootTable);

			assert(d3d12RootParam.ShaderVisibility == shaderVisibility && "Shader visibility is not correct");

			// Descriptors are closely arranged, so the Offset of the Descriptor to be added is the number of Descriptors before adding
			// When AddDescriptorRanges is called, GetDescriptorTableSize has not increased
			OffsetFromTableStart = RootTable.GetDescriptorTableSize();
			UINT32 NewDescriptorRangeIndex = d3d12RootParam.DescriptorTable.NumDescriptorRanges - 1;

			RootTable.SetDescriptorRange(NewDescriptorRangeIndex,
				RangeType,
				ShaderResAttribs.BindPoint,
				ShaderResAttribs.BindCount,
				0,
				OffsetFromTableStart);
		}
	}

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 7> RootSignature::GetStaticSamplers()
	{
		const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
			0, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
			1, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
			2, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
			3, // shaderRegister
			D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			4, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
			0.0f,                             // mipLODBias
			8);                               // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
			5, // shaderRegister
			D3D12_FILTER_ANISOTROPIC, // filter
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
			0.0f,                              // mipLODBias
			8);                                // maxAnisotropy

		const CD3DX12_STATIC_SAMPLER_DESC shadow(
			6, // shaderRegister
			D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, // filter
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressU
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressV
			D3D12_TEXTURE_ADDRESS_MODE_BORDER,  // addressW
			0.0f,                               // mipLODBias
			16,                                 // maxAnisotropy
			D3D12_COMPARISON_FUNC_LESS_EQUAL,
			D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE);

		return {
			pointWrap, pointClamp,
			linearWrap, linearClamp,
			anisotropicWrap, anisotropicClamp, shadow };
	}
}