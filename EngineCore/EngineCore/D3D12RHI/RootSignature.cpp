#include "../pch.h"
#include "RootSignature.h"
#include "RenderDevice.h"

namespace RHI
{

	// ------------------- Root Parameters Manager ----------------------

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
		// Compare Root Table and Root View 
		if (m_RootTables.size() != RootParams.m_RootTables.size() ||
			m_RootDescriptors.size() != RootParams.m_RootDescriptors.size())
			return false;

		// Compare Root View
		for (UINT32 i = 0; i < m_RootDescriptors.size(); ++i)
		{
			const auto& rootView0 = GetRootDescriptor(i);
			const auto& rootView1 = RootParams.GetRootDescriptor(i);
			if (rootView0 != rootView1)
				return false;
		}

		// Compare Root Table
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

	// ------------------- Root Signature -------------------------------
	RootSignature::RootSignature(RenderDevice* renderDevice)
		:m_RenderDevice(renderDevice)
	{
		// TODO
	}

	RootSignature::~RootSignature()
	{
		if (m_pd3d12RootSignature)
			m_RenderDevice->SafeReleaseDeviceObject(std::move(m_pd3d12RootSignature));
	}

}