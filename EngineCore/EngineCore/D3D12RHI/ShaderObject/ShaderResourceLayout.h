#pragma once
#include "ShaderResource.h"
#include "../GpuBuffer.h"
#include "../GpuResourceDescriptor.h"

// http://diligentgraphics.com/diligent-engine/architecture/d3d12/shader-resource-layout/
// https://github.com/DiligentGraphics/DiligentCore/blob/6605548066bbff052ea1c1f7e4fec14446fa7727/Graphics/GraphicsEngineD3D12/include/ShaderResourceLayoutD3D12.hpp

// All resources are stored in a single continuous chunk of memory using the following layout:
// 
//   m_ResourceBuffer
//      |                         |                         |
//      | D3D12Resource[0]  ...   | D3D12Resource[s]  ...   | D3D12Resource[s+m]  ...  | D3D12Resource[smd]  ...  | D3D12Resource[smd+s']  ...  | D3D12Resource[smd+s'+m']  ...  D3D12Resource[s+m+d+s'+m'+d'-1] ||
//      |                         |                         |                          |                          |                             |                                                                ||
//      |  SRV/CBV/UAV - STATIC   |  SRV/CBV/UAV - MUTABLE  |   SRV/CBV/UAV - DYNAMIC  |   Samplers - STATIC      |  Samplers - MUTABLE         |   Samplers - DYNAMIC                                           ||
//      |                         |                         |                          |             
//
//      s == NumCbvSrvUav[SHADER_RESOURCE_VARIABLE_TYPE_STATIC]
//      m == NumCbvSrvUav[SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE]
//      d == NumCbvSrvUav[SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC]
//      smd = s+m+d
//
//      s' == NumSamplers[SHADER_RESOURCE_VARIABLE_TYPE_STATIC]
//      m' == NumSamplers[SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE]
//      d' == NumSamplers[SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC]
//
//   Every D3D12Resource structure holds a reference to D3DShaderResourceAttribs structure from ShaderResourcesD3D12.
//   ShaderResourceLayoutD3D12 holds shared pointer to ShaderResourcesD3D12 instance. Note that ShaderResourcesD3D12::SamplerId 
//   references a sampler in ShaderResourcesD3D12, while D3D12Resource::SamplerId references a sampler in ShaderResourceLayoutD3D12, 
//   and the two are not necessarily the same
//
//                                                       
//                                                          ________________SamplerId____________________
//                                                         |                                             |
//    _____________________                  ______________|_____________________________________________V________
//   |                     |  unique_ptr    |        |           |           |           |           |            |
//   |ShaderResourcesD3D12 |--------------->|   CBs  |  TexSRVs  |  TexUAVs  |  BufSRVs  |  BufUAVs  |  Samplers  |
//   |_____________________|                |________|___________|___________|___________|___________|____________|
//            A                                         A                              A                   A  
//            |                                          \                            /                     \
//            |shared_ptr                                Ref                        Ref                     Ref
//    ________|__________________                  ________\________________________/_________________________\________________________________________________
//   |                           |   unique_ptr   |                  |                  |               |                    |                      |          |
//   | ShaderResourceLayoutD3D12 |--------------->| D3D12Resource[0] | D3D12Resource[1] |       ...     | D3D12Resource[smd] | D3D12Resource[smd+1] |   ...    |
//   |___________________________|                |__________________|__________________|_______________|____________________|______________________|__________|
//                A                                    A    |             A                                      A                   
//                |                                     \   |______________\____SamplerId________________________|          
//                |                                      \                  \                                              
//                |                                      Ref                Ref                   
//                |                                        \                  \_____              
//                |                                         \                       \             
//    ____________|_______________                   ________\_______________________\__________________________________________
//   |                            |                 |                            |                            |                 |
//   | ShaderVariableManagerD3D12 |---------------->| ShaderVariableD3D12Impl[0] | ShaderVariableD3D12Impl[1] |     ...         |
//   |____________________________|                 |____________________________|____________________________|_________________|
// 
//    ShaderResourceLayoutD3D12 is used as follows:
//    * Every pipeline state object (PipelineStateD3D12Impl) maintains shader resource layout for every active shader stage
//      ** These resource layouts are used as reference layouts for shader resource binding objects
//      ** All variable types are preserved
//      ** Root indices and descriptor table offsets are assigned during the initialization
//    * Every pipeline state object also contains shader resource layout that facilitates management of static shader resources
//      ** The resource layout defines artificial layout where root index matches the resource type (CBV/SRV/UAV/SAM)
//      ** Only static variables are referenced
//
//    * Every shader resource binding object (ShaderResourceBindingD3D12Impl) encompasses shader variable 
//      manager (ShaderVariableManagerD3D12) for every active shader stage in the parent pipeline state that
//      handles mutable and dynamic resources

namespace RHI
{
	struct ShaderVariableConfig;
	class RootSignature;

	// Define mappings between shader resource and descriptor in descriptor table.
	// HLSL shader registers are first mapped to descriptor in descriptor table as defined by Root Signature.
	// Descriptor then reference actual resource in GPU memory
	class ShaderResourceLayout
	{
	public:

	private:
		ID3D12Device* m_D3D12Device;
	};
}
