#pragma once

#include "ShaderResourceBindingUtility.h"

// ShaderResources class uses continuous chunk of memory to store all resources, as follows:
//
//
//       m_MemoryBuffer            m_TexSRVOffset                      m_TexUAVOffset                      m_BufSRVOffset                      m_BufUAVOffset                      m_SamplersOffset                            m_MemorySize
//        |                         |                                   |                                   |                                   |                                   |                           |                  |
//        |  CB[0]  ...  CB[Ncb-1]  |  TexSRV[0]  ...  TexSRV[Ntsrv-1]  |  TexUAV[0]  ...  TexUAV[Ntuav-1]  |  BufSRV[0]  ...  BufSRV[Nbsrv-1]  |  BufUAV[0]  ...  BufUAV[Nbuav-1]  |  Sam[0]  ...  Sam[Nsam-1] |  Resource Names  |
//

namespace RHI
{
	struct ShaderDesc;
	struct ShaderVariableConfig;

	// Indicate the resource used in Shaders
	struct ShaderResourceAttribs
	{
		// Texture2D g_Tex, and Texture2D g_Tex[4],the name will be deduced as"g_Tex". cbuffer cbBuff0 : register(b5), the name "cbBuffer0"
		const std::string Name; // Name is the name of the resource.
		const UINT16 BindPoint; // The register number of variable such as CBuffer. cbBuffer0 : register(b5), BindPoint = 5
		const UINT16 BindCount; // Number of the binding slots taken by resource (array res= array size, non-array res = 1)

		//            4               4                 24           
		// bit | 0  1  2  3   |  4  5  6  7  |  8   9  10   ...   31  |   
		//     |              |              |                        |
		//     |  InputType   |   SRV Dim    | SamplerOrTexSRVIdBits  |
		static constexpr const UINT32 ShaderInputTypeBits = 4; // Identifes resource type and takes values from D3D_SHADER_INPUT_TYPE enumeration.
		static constexpr const UINT32 SRVDimBits = 4; // IS the shader resource view dimension (D3D_SRV_DIMENSION).
		static constexpr const UINT32 SamplerOrTexSRVIdBits = 24; // Texture or Sampler 

	private:
		// We need to use Uint32 instead of the actual type for reliability and correctness.
		// There originally was a problem when the type of InputType was D3D_SHADER_INPUT_TYPE:
		// the value of D3D_SIT_UAV_RWBYTEADDRESS (8) was interpreted as -8 (as the underlying enum type is signed) causing errors
		const UINT32 InputType : ShaderInputTypeBits;  // Max value: D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER == 11
		const UINT32 SRVDimension : SRVDimBits; // Max value: D3D_SRV_DIMENSION_BUFFEREX == 11

		static_assert(ShaderInputTypeBits + SRVDimBits + SamplerOrTexSRVIdBits == 32, "Attributes are better be packed into 32 bits");

		static_assert(D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER < (1 << ShaderInputTypeBits), "Not enough bits to represent D3D_SHADER_INPUT_TYPE");;
		static_assert(D3D_SRV_DIMENSION_BUFFEREX < (1 << SRVDimBits), "Not enough bits to represent D3D_SRV_DIMENSION");

	public:
		static constexpr const UINT32 InvalidTexSRVId = (1 << SamplerOrTexSRVIdBits) - 1;
		static constexpr const UINT16 InvalidBindPoint = std::numeric_limits<UINT16>::max();
		static constexpr const UINT16 MaxBindPoint = InvalidBindPoint - 1;
		static constexpr const UINT16 MaxBindCount = std::numeric_limits<UINT16>::max();

		ShaderResourceAttribs(const std::string& name,
			UINT bindPoint,
			UINT bindCount,
			D3D_SHADER_INPUT_TYPE inputType,
			D3D_SRV_DIMENSION srvDimension) noexcept :
			Name{ name },
			BindPoint{ static_cast<decltype(BindPoint)>(bindPoint) },
			BindCount{ static_cast<decltype(BindCount)>(bindCount) },
			InputType{ static_cast<decltype(InputType)>(inputType) },
			SRVDimension{ static_cast<decltype(SRVDimension)>(srvDimension) }
		{

		}

		ShaderResourceAttribs(const ShaderResourceAttribs& rhs) = delete;
		ShaderResourceAttribs& operator = (const ShaderResourceAttribs& rhs) = delete;

		D3D_SHADER_INPUT_TYPE GetInputType() const
		{
			return static_cast<D3D_SHADER_INPUT_TYPE>(InputType);
		}

		D3D_SRV_DIMENSION GetSRVDimension() const
		{
			return static_cast<D3D_SRV_DIMENSION>(SRVDimension);
		}

		bool IsValidBindPoint() const
		{
			return BindPoint != InvalidBindPoint;
		}

		bool IsCompatibleWith(const ShaderResourceAttribs& Attribs) const
		{
			return BindPoint == Attribs.BindPoint &&
				BindCount == Attribs.BindCount &&
				InputType == Attribs.InputType &&
				SRVDimension == Attribs.SRVDimension;
		}

		size_t GetHash() const
		{
			return ComputeHash(BindPoint, BindCount, InputType, SRVDimension);
		}
	};

	// Describe all resources used by a Shader, implemented using DX Shader Reflection system (ID3D12ShaderReflection).
	// The class allocates continueos chunck of memory to store all resource in follwing order : 
	// 1 - CBV, 2 - Texture SRVs, 3 - Texture UAVs, 4 - Buffer SRVs, 5 - Buffer UAVs, 6 - Sampler
	// Regarding the processing of arrays : An array will be represented by a ShadeResourceAttribs, but the RootSignature will allocate -
	// the corresponding Descriptor in Root Table according the size of the array.
	// Similary the array is represented by ShaderVariables, and the index of the array needs to be passed when binding resource to array.
	class ShaderResource
	{
	public:
		ShaderResource(ID3DBlob* pShaderBytecode, const ShaderDesc& shaderDesc);

		ShaderResource(const ShaderResource&) = delete;
		ShaderResource(ShaderResource&&) = delete;
		ShaderResource& operator = (const ShaderResource&) = delete;
		ShaderResource& operator = (ShaderResource&&) = delete;

		UINT32 GetCBNum() const noexcept { return (UINT32)m_CBs.size(); }
		UINT32 GetTexSRVNum() const noexcept { return (UINT32)m_TextureSRVs.size(); }
		UINT32 GetTexUAVNum() const noexcept { return (UINT32)m_TextureUAVs.size(); }
		UINT32 GetBufSRVNum() const noexcept { return (UINT32)m_BufferSRVs.size(); }
		UINT32 GetBufUAVNum() const noexcept { return (UINT32)m_BufferUAVs.size(); }

		const ShaderResourceAttribs& GetCB(UINT32 n) const noexcept { assert(n >= 0 && n < m_CBs.size()); return *m_CBs[n].get(); }
		const ShaderResourceAttribs& GetTexSRV(UINT32 n) const noexcept { assert(n >= 0 && n < m_TextureSRVs.size()); return *m_TextureSRVs[n].get(); }
		const ShaderResourceAttribs& GetTexUAV(UINT32 n) const noexcept { assert(n >= 0 && n < m_TextureUAVs.size()); return *m_TextureUAVs[n].get(); }
		const ShaderResourceAttribs& GetBufSRV(UINT32 n) const noexcept { assert(n >= 0 && n < m_BufferSRVs.size()); return *m_BufferSRVs[n].get(); }
		const ShaderResourceAttribs& GetBufUAV(UINT32 n) const noexcept { assert(n >= 0 && n < m_BufferUAVs.size()); return *m_BufferUAVs[n].get(); }

		SHADER_TYPE GetShaderType() const noexcept { return m_ShaderType; }

		template <typename THandleCB,
			typename THandleTexSRV,
			typename THandleTexUAV,
			typename THandleBufSRV,
			typename THandleBufUAV>
			void ProcessResources(THandleCB HandleCB,
				THandleTexSRV HandleTexSRV,
				THandleTexUAV HandleTexUAV,
				THandleBufSRV HandleBufSRV,
				THandleBufUAV HandleBufUAV) const
		{
			for (UINT32 i = 0; i < m_CBs.size(); ++i)
				HandleCB(GetCB(i), i);

			for (UINT32 i = 0; i < m_TextureSRVs.size(); ++i)
				HandleTexSRV(GetTexSRV(i), i);

			for (UINT32 i = 0; i < m_TextureUAVs.size(); ++i)
				HandleTexUAV(GetTexUAV(i), i);

			for (UINT32 i = 0; i < m_BufferSRVs.size(); ++i)
				HandleBufSRV(GetBufSRV(i), i);

			for (UINT32 i = 0; i < m_BufferUAVs.size(); ++i)
				HandleBufUAV(GetBufUAV(i), i);
		}

		bool IsCompatibleWith(const ShaderResource& shaderResource) const;
		const std::string& GetShaderName() const { return m_ShaderName; }

		// Find the Variable Type (Static, Mutable, Dynamic) corresponding to ShaderResource 
		// from the ShaderVariableConfig of the PSO configured in the upper layer
		SHADER_RESOURCE_VARIABLE_TYPE FindVariableType(const ShaderResourceAttribs& ResourceAttribs,
			const ShaderVariableConfig& shaderVariableConfig) const;

		size_t GetHash() const;

		void GetShaderModel(UINT32& Major, UINT32& Minor) const
		{
			Major = (m_ShaderVersion & 0x000000F0) >> 4;
			Minor = (m_ShaderVersion & 0x0000000F);
		}

	private:
		// The purpose of grouping here is to convert the different types in D3D into these types: CBV, TexSRV, TexUAV, BufferSRV, BufferUAV
		std::vector<std::unique_ptr<ShaderResourceAttribs>> m_CBs;
		std::vector<std::unique_ptr<ShaderResourceAttribs>> m_TextureSRVs;
		std::vector<std::unique_ptr<ShaderResourceAttribs>> m_TextureUAVs;
		std::vector<std::unique_ptr<ShaderResourceAttribs>> m_BufferSRVs;
		std::vector<std::unique_ptr<ShaderResourceAttribs>> m_BufferUAVs;

		// Shader Type (Vertex, Pixel...)
		const SHADER_TYPE m_ShaderType;
		// Shader Version
		UINT32 m_ShaderVersion = 0;
		// Shader name
		std::string m_ShaderName;
	};
}

namespace std
{
	template<>
	struct hash<RHI::ShaderResourceAttribs>
	{
		size_t operator()(const RHI::ShaderResourceAttribs& attribs) const
		{
			return attribs.GetHash();
		}
	};

	template<>
	struct hash<RHI::ShaderResource>
	{
		size_t operator()(const RHI::ShaderResource& shaderResource) const
		{
			return shaderResource.GetHash();
		}
	};
}