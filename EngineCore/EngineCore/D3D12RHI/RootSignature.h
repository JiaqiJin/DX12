#pragma once

// https://logins.github.io/graphics/2020/06/26/DX12RootSignatureObject.html
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/root-signatures-overview
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/creating-a-root-signature
// https://docs.microsoft.com/en-us/windows/win32/direct3d12/example-root-signatures
/*The root signature is an object intended to manage all resource bindings in a DirectX 12 application.
* Root signature defines an array of root parameters.
* Every parameter is assigned a unique root index and can be one of the following: Root constant, Root table or Root view.
* Every root table consists of one or more descriptor ranges.
* Descriptor range is a continuous range of one or more descriptors of the same type.
* Exampler DR : DescRange[0].Init(D3D12_DESCRIPTOR_RANGE_SRV,6,2); DescRange[1].Init(D3D12_DESCRIPTOR_RANGE_UAV,4,0);
*/

namespace RHI
{
	class RenderDevice;

	// A root parameter is one entry in the root signature.
	// A root parameter can be a root constant, root descriptor, or descriptor table.
	class RootParameter
	{
	public:
		// Three constructors, corresponding to the three types of RootParamater in DX12: "Root Constant", "Root Descriptor", "Root Table"
		// It is best not to store the Root Constant, the total size of the Root Parameter is limited, and the Root Constant will take up a lot of space
		// Root Table is a little more complicated. A Root Table can have multiple Descriptor Ranges.
		// A Descriptor Range is actually a Descriptor array of the same type (CBV, SRV, UAV)

		// Root Descriptor
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
            UINT32                        RootIndex,
            UINT                          Register, // Shader register arguments are bound to(cbuffer cbA : register(b1) {…}; 2...).
            UINT                          RegisterSpace, //Texture2D gDiffuseMap:register(t0, space0);Texture2D gNormalMap:register(t0, space1);
            D3D12_SHADER_VISIBILITY       Visibility,
            SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType }
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_CBV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_SRV || ParameterType == D3D12_ROOT_PARAMETER_TYPE_UAV && "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Descriptor.ShaderRegister = Register;
            m_RootParam.Descriptor.RegisterSpace = RegisterSpace;
        }

        // Root Constant
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
            UINT32                        RootIndex,
            UINT                          Register,
            UINT                          RegisterSpace,
            UINT                          NumDwords,
            D3D12_SHADER_VISIBILITY       Visibility,
            SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType }
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS && "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.Constants.Num32BitValues = NumDwords;
            m_RootParam.Constants.ShaderRegister = Register;
            m_RootParam.Constants.RegisterSpace = RegisterSpace;
        }

        // Root Table
        RootParameter(D3D12_ROOT_PARAMETER_TYPE     ParameterType,
            UINT32                        RootIndex,
            UINT                          NumRanges,
            D3D12_SHADER_VISIBILITY       Visibility,
            SHADER_RESOURCE_VARIABLE_TYPE VarType) noexcept :
            m_RootIndex{ RootIndex },
            m_ShaderVarType{ VarType },
            m_DescriptorRanges{ NumRanges }
        {
            assert(ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Unexpected parameter type - verify argument list");
            m_RootParam.ParameterType = ParameterType;
            m_RootParam.ShaderVisibility = Visibility;
            m_RootParam.DescriptorTable.NumDescriptorRanges = NumRanges;
            m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];
        }

        RootParameter(const RootParameter& RP) noexcept :
            m_RootParam{ RP.m_RootParam },
            m_DescriptorTableSize{ RP.m_DescriptorTableSize },
            m_ShaderVarType{ RP.m_ShaderVarType },
            m_RootIndex{ RP.m_RootIndex },
            m_DescriptorRanges(RP.m_DescriptorRanges)
        {
            // After copying, you need to reset the pointer to m_descriptorRanges
            if (m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && m_DescriptorRanges.size() > 0)
                m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];
            //assert(m_RootParam.ParameterType != D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Use another constructor to copy descriptor table");
        }

        // Adding Descriptor Range in existing Descriptor Table
        void AddDescriptorRanges(UINT32 addRangesNum)
        {
            m_DescriptorRanges.insert(m_DescriptorRanges.end(), addRangesNum, D3D12_DESCRIPTOR_RANGE{});

            // After adding a new Descriptor Range, you need to re-assign the pointer, 
            // because when the vector is expanded, the storage location will change!
            m_RootParam.DescriptorTable.pDescriptorRanges = &m_DescriptorRanges[0];
            m_RootParam.DescriptorTable.NumDescriptorRanges += addRangesNum;
        }

        void SetDescriptorRange(UINT    RangeIndex,
            D3D12_DESCRIPTOR_RANGE_TYPE Type,
            UINT                        Register,
            UINT                        Count,
            UINT                        Space = 0,
            UINT                        OffsetFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND)
        {
            assert(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Incorrect parameter table: descriptor table is expected");
            auto& table = m_RootParam.DescriptorTable;
            assert(RangeIndex < table.NumDescriptorRanges && "Invalid descriptor range index");
            D3D12_DESCRIPTOR_RANGE& range = const_cast<D3D12_DESCRIPTOR_RANGE&>(table.pDescriptorRanges[RangeIndex]);
            //assert(range.RangeType == static_cast<D3D12_DESCRIPTOR_RANGE_TYPE>(-1) && "Descriptor range has already been initialized. m_DescriptorTableSize may be updated incorrectly");
            range.RangeType = Type;
            range.NumDescriptors = Count;
            range.BaseShaderRegister = Register;
            range.RegisterSpace = Space;
            range.OffsetInDescriptorsFromTableStart = OffsetFromTableStart;
            m_DescriptorTableSize = std::max(m_DescriptorTableSize, OffsetFromTableStart + Count);
        }

        SHADER_RESOURCE_VARIABLE_TYPE GetShaderVariableType() const { return m_ShaderVarType; }

        UINT32 GetDescriptorTableSize() const
        {
            assert(m_RootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE && "Incorrect parameter table: descriptor table is expected");
            return m_DescriptorTableSize;
        }

        D3D12_SHADER_VISIBILITY   GetShaderVisibility() const { return m_RootParam.ShaderVisibility; }
        D3D12_ROOT_PARAMETER_TYPE GetParameterType() const { return m_RootParam.ParameterType; }

        RootParameter& operator=(const RootParameter&) = delete;
        RootParameter& operator=(RootParameter&&) = delete;

        UINT32 GetRootIndex() const { return m_RootIndex; }

        // Define implicit conversion to D3D12_ROOT_PARAMETER
        operator const D3D12_ROOT_PARAMETER& () const { return m_RootParam; }

        bool operator==(const RootParameter& rhs) const
        {
            if (m_ShaderVarType != rhs.m_ShaderVarType ||
                m_DescriptorTableSize != rhs.m_DescriptorTableSize ||
                m_RootIndex != rhs.m_RootIndex)
                return false;

            if (m_RootParam.ParameterType != rhs.m_RootParam.ParameterType ||
                m_RootParam.ShaderVisibility != rhs.m_RootParam.ShaderVisibility)
                return false;

            switch (m_RootParam.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                const auto& tbl0 = m_RootParam.DescriptorTable;
                const auto& tbl1 = rhs.m_RootParam.DescriptorTable;
                if (tbl0.NumDescriptorRanges != tbl1.NumDescriptorRanges)
                    return false;
                for (UINT r = 0; r < tbl0.NumDescriptorRanges; ++r)
                {
                    const auto& rng0 = tbl0.pDescriptorRanges[r];
                    const auto& rng1 = tbl1.pDescriptorRanges[r];
                    if (memcmp(&rng0, &rng1, sizeof(rng0)) != 0)
                        return false;
                }
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                const auto& cnst0 = m_RootParam.Constants;
                const auto& cnst1 = rhs.m_RootParam.Constants;
                if (memcmp(&cnst0, &cnst1, sizeof(cnst0)) != 0)
                    return false;
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            {
                const auto& dscr0 = m_RootParam.Descriptor;
                const auto& dscr1 = rhs.m_RootParam.Descriptor;
                if (memcmp(&dscr0, &dscr1, sizeof(dscr0)) != 0)
                    return false;
            }
            break;

            default: LOG_ERROR("Unexpected root parameter type");
            }

            return true;
        }

        bool operator!=(const RootParameter& rhs) const
        {
            return !(*this == rhs);
        }

        size_t GetHash() const
        {
            size_t hash = ComputeHash(m_ShaderVarType, m_DescriptorTableSize, m_RootIndex);
            HashCombine(hash, m_RootParam.ParameterType, m_RootParam.ShaderVisibility);

            switch (m_RootParam.ParameterType)
            {
            case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
            {
                const auto& tbl = m_RootParam.DescriptorTable;
                HashCombine(hash, tbl.NumDescriptorRanges);
                for (UINT r = 0; r < tbl.NumDescriptorRanges; ++r)
                {
                    const auto& rng = tbl.pDescriptorRanges[r];
                    HashCombine(hash, rng.BaseShaderRegister, rng.NumDescriptors, rng.OffsetInDescriptorsFromTableStart, rng.RangeType, rng.RegisterSpace);
                }
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
            {
                const auto& cnst = m_RootParam.Constants;
                HashCombine(hash, cnst.Num32BitValues, cnst.RegisterSpace, cnst.ShaderRegister);
            }
            break;

            case D3D12_ROOT_PARAMETER_TYPE_CBV:
            case D3D12_ROOT_PARAMETER_TYPE_SRV:
            case D3D12_ROOT_PARAMETER_TYPE_UAV:
            {
                const auto& dscr = m_RootParam.Descriptor;
                HashCombine(hash, dscr.RegisterSpace, dscr.ShaderRegister);
            }
            break;

            default: LOG_ERROR("Unexpected root parameter type");
            }

            return hash;
        }

	private:
		SHADER_RESOURCE_VARIABLE_TYPE m_ShaderVarType = static_cast<SHADER_RESOURCE_VARIABLE_TYPE>(-1);
		D3D12_ROOT_PARAMETER m_RootParam = {};
		UINT32 m_DescriptorTableSize = 0;
		UINT32 m_RootIndex = static_cast<UINT32>(-1);
        // A slot parameter initialized as a descriptor table takes an array of D3D12_DESCRIPTOR_RANGE
        // CD3DX12_DESCRIPTOR_RANGE descRange[3]; [0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2//dec_count, 0//shader_register) ...
		std::vector<D3D12_DESCRIPTOR_RANGE> m_DescriptorRanges;
	};

	// A roor Signature define the data that the shader in current PSO
	class RootSignature
	{
	public:
		RootSignature(RenderDevice* renderDevice);
		~RootSignature();

		ID3D12RootSignature* GetD3D12RootSignature() const { return m_pd3d12RootSignature.Get(); }
	private:
		// Root Signature
		Microsoft::WRL::ComPtr<ID3D12RootSignature> m_pd3d12RootSignature;
		RenderDevice* m_RenderDevice;
	};
}