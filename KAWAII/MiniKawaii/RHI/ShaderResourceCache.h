#pragma once

#include "DescriptorHeap.h"
#include "GpuBuffer.h"
#include "GpuResourceDescriptor.h"

namespace RHI
{
	class RootSignature;
	class RenderDevice;
	class CommandContext;

	// Shader resource cache provides the storage to keep references to object bound by application to shader resource.
	// Only provide a storage space (GPUDescriptorHeap)
	// To store resource bound to the Shader, ShaderResourceCache will allocate space on GPU-Visible Descriptor heap to store the 
	// Descriptor of the resource
	// Root View does not allocate space in GPUDesciptorHeap. it is directly bound to RootSignature.
	// Static and Mutable Root Table will allocate space in GPUDescriptorHeap, and Dynamic will allocate space dynamically in each Draw Call
	class ShaderResourceCache
	{
		friend class CommandContext;
	public:
		ShaderResourceCache() = default;

		ShaderResourceCache(const ShaderResourceCache&) = delete;
		ShaderResourceCache(ShaderResourceCache&&) = delete;
		ShaderResourceCache& operator = (const ShaderResourceCache&) = delete;
		ShaderResourceCache& operator = (ShaderResourceCache&&) = delete;

		~ShaderResourceCache() = default;

	private:

	};
}