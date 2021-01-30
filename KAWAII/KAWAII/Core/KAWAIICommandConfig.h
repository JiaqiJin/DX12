#pragma once

#include "KAWAII_DX12.h"

namespace KAWAII
{
	enum class API
	{
		DIRECTX_12
	};

	enum class Format : uint32_t
	{
		UNKNOWN,
		R32G32B32A32_TYPELESS,
		R32G32B32A32_FLOAT,
		R32G32B32A32_UINT,
		R32G32B32A32_SINT,
		R32G32B32_TYPELESS,
		R32G32B32_FLOAT,
		R32G32B32_UINT,
		R32G32B32_SINT,
		R16G16B16A16_TYPELESS,
		R16G16B16A16_FLOAT,
		R16G16B16A16_UNORM,
		R16G16B16A16_UINT,
		R16G16B16A16_SNORM,
		R16G16B16A16_SINT,
		R32G32_TYPELESS,
		R32G32_FLOAT,
		R32G32_UINT,
		R32G32_SINT,
		R32G8X24_TYPELESS,
		D32_FLOAT_S8X24_UINT,
		R32_FLOAT_X8X24_TYPELESS,
		X32_TYPELESS_G8X24_UINT,
		R10G10B10A2_TYPELESS,
		R10G10B10A2_UNORM,
		R10G10B10A2_UINT,
		R11G11B10_FLOAT,
		R8G8B8A8_TYPELESS,
		R8G8B8A8_UNORM,
		R8G8B8A8_UNORM_SRGB,
		R8G8B8A8_UINT,
		R8G8B8A8_SNORM,
		R8G8B8A8_SINT,
		R16G16_TYPELESS,
		R16G16_FLOAT,
		R16G16_UNORM,
		R16G16_UINT,
		R16G16_SNORM,
		R16G16_SINT,
		R32_TYPELESS,
		D32_FLOAT,
		R32_FLOAT,
		R32_UINT,
		R32_SINT,
		R24G8_TYPELESS,
		D24_UNORM_S8_UINT,
		R24_UNORM_X8_TYPELESS,
		X24_TYPELESS_G8_UINT,
		R8G8_TYPELESS,
		R8G8_UNORM,
		R8G8_UINT,
		R8G8_SNORM,
		R8G8_SINT,
		R16_TYPELESS,
		R16_FLOAT,
		D16_UNORM,
		R16_UNORM,
		R16_UINT,
		R16_SNORM,
		R16_SINT,
		R8_TYPELESS,
		R8_UNORM,
		R8_UINT,
		R8_SNORM,
		R8_SINT,
		A8_UNORM,
		R1_UNORM,
		R9G9B9E5_SHAREDEXP,
		R8G8_B8G8_UNORM,
		G8R8_G8B8_UNORM,
		BC1_TYPELESS,
		BC1_UNORM,
		BC1_UNORM_SRGB,
		BC2_TYPELESS,
		BC2_UNORM,
		BC2_UNORM_SRGB,
		BC3_TYPELESS,
		BC3_UNORM,
		BC3_UNORM_SRGB,
		BC4_TYPELESS,
		BC4_UNORM,
		BC4_SNORM,
		BC5_TYPELESS,
		BC5_UNORM,
		BC5_SNORM,
		B5G6R5_UNORM,
		B5G5R5A1_UNORM,
		B8G8R8A8_UNORM,
		B8G8R8X8_UNORM,
		RGB10_XR_BIAS_A2_UNORM,
		B8G8R8A8_TYPELESS,
		B8G8R8A8_UNORM_SRGB,
		B8G8R8X8_TYPELESS,
		B8G8R8X8_UNORM_SRGB,
		BC6H_TYPELESS,
		BC6H_UF16,
		BC6H_SF16,
		BC7_TYPELESS,
		BC7_UNORM,
		BC7_UNORM_SRGB,
		AYUV,
		Y410,
		Y416,
		NV12,
		P010,
		P016,
		OPAQUE_420,
		YUY2,
		Y210,
		Y216,
		NV11,
		AI44,
		IA44,
		P8,
		A8P8,
		B4G4R4A4_UNORM,

		P208,
		V208,
		V408,

		FORCE_UINT
	};

	// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ne-d3d12-d3d12_resource_states
	enum class ResourceState : uint8_t
	{
		COMMON = 0,
		VERTEX_AND_CONSTANT_BUFFER = (1 << 0),
		INDEX_BUFFER = (1 << 1),
		RENDER_TARGET = (1 << 2),
		UNORDERED_ACCESS = (1 << 3),
		DEPTH_WRITE = (1 << 4),
		DEPTH_READ = (1 << 5),
		NON_PIXEL_SHADER_RESOURCE = (1 << 6),
		PIXEL_SHADER_RESOURCE = (1 << 7),
		STREAM_OUT = (1 << 8),
		INDIRECT_ARGUMENT = (1 << 9),
		COPY_DEST = (1 << 10),
		COPY_SOURCE = (1 << 11),
		RESOLVE_DEST = (1 << 12),
		RESOLVE_SOURCE = (1 << 13),
		PREDICATION = (1 << 14),
		RAYTRACING_ACCELERATION_STRUCTURE = (1 << 15),
		SHADING_RATE_SOURCE = (1 << 16),
		GENERAL_READ = (VERTEX_AND_CONSTANT_BUFFER | INDEX_BUFFER | NON_PIXEL_SHADER_RESOURCE | PIXEL_SHADER_RESOURCE | INDIRECT_ARGUMENT | COPY_SOURCE | PREDICATION),
		PRESENT = 0,

		VIDEO_DECODE_READ = (1 << 17),
		VIDEO_DECODE_WRITE = (1 << 18),
		VIDEO_PROCESS_READ = (1 << 19),
		VIDEO_PROCESS_WRITE = (1 << 20),
		VIDEO_ENCODE_READ = (1 << 21),
		VIDEO_ENCODE_WRITE = (1 << 22)
	};

	DEFINE_ENUM_FLAG_OPERATORS(ResourceState);

	enum class BarrierFlag : uint8_t
	{
		NONE = 0,
		BEGIN_ONLY = (1 << 0),
		END_ONLY = (1 << 1)
	};

	DEFINE_ENUM_FLAG_OPERATORS(BarrierFlag);

	enum class CommandListType : uint8_t
	{
		DIRECT,
		BUNDLE,
		COMPUTE,
		COPY,
		VIDEO_DECODE,
		VIDEO_PROCESS,
		VIDEO_ENCODE
	};

	enum class CommandQueueFlag : uint8_t
	{
		NONE = 0,
		DISABLE_GPU_TIMEOUT = (1 << 0)
	};

	DEFINE_ENUM_FLAG_OPERATORS(CommandQueueFlag);

	enum class FenceFlag : uint8_t
	{
		NONE = 0,
		SHARED = (1 << 0),
		SHARED_CROSS_ADAPTER = (1 << 1),
		NON_MONITORED = (1 << 2)
	};

	DEFINE_ENUM_FLAG_OPERATORS(FenceFlag);

	enum class InputClassification : uint8_t
	{
		PER_VERTEX_DATA,
		PER_INSTANCE_DATA
	};

	enum class MemoryType : uint8_t
	{
		DEFAULT,
		UPLOAD,
		READBACK,
		CUSTOM
	};

	enum class PrimitiveTopologyType : uint8_t
	{
		UNDEFINED,
		POINT,
		LINE,
		TRIANGLE,
		PATCH
	};

	enum class PrimitiveTopology : uint8_t
	{
		UNDEFINED,
		POINTLIST,
		LINELIST,
		LINESTRIP,
		TRIANGLELIST,
		TRIANGLESTRIP,
		LINELIST_ADJ,
		LINESTRIP_ADJ,
		TRIANGLELIST_ADJ,
		TRIANGLESTRIP_ADJ,
		CONTROL_POINT1_PATCHLIST,
		CONTROL_POINT2_PATCHLIST,
		CONTROL_POINT3_PATCHLIST,
		CONTROL_POINT4_PATCHLIST,
		CONTROL_POINT5_PATCHLIST,
		CONTROL_POINT6_PATCHLIST,
		CONTROL_POINT7_PATCHLIST,
		CONTROL_POINT8_PATCHLIST,
		CONTROL_POINT9_PATCHLIST,
		CONTROL_POINT10_PATCHLIST,
		CONTROL_POINT11_PATCHLIST,
		CONTROL_POINT12_PATCHLIST,
		CONTROL_POINT13_PATCHLIST,
		CONTROL_POINT14_PATCHLIST,
		CONTROL_POINT15_PATCHLIST,
		CONTROL_POINT16_PATCHLIST,
		CONTROL_POINT17_PATCHLIST,
		CONTROL_POINT18_PATCHLIST,
		CONTROL_POINT19_PATCHLIST,
		CONTROL_POINT20_PATCHLIST,
		CONTROL_POINT21_PATCHLIST,
		CONTROL_POINT22_PATCHLIST,
		CONTROL_POINT23_PATCHLIST,
		CONTROL_POINT24_PATCHLIST,
		CONTROL_POINT25_PATCHLIST,
		CONTROL_POINT26_PATCHLIST,
		CONTROL_POINT27_PATCHLIST,
		CONTROL_POINT28_PATCHLIST,
		CONTROL_POINT29_PATCHLIST,
		CONTROL_POINT30_PATCHLIST,
		CONTROL_POINT31_PATCHLIST,
		CONTROL_POINT32_PATCHLIST
	};

	enum class ClearFlag : uint8_t
	{
		NONE = 0,
		DEPTH = (1 << 0),
		STENCIL = (1 << 1)
	};

	DEFINE_ENUM_FLAG_OPERATORS(ClearFlag);

	enum class ResourceFlag : uint32_t
	{
		NONE = 0,
		ALLOW_RENDER_TARGET = (1 << 0),
		ALLOW_DEPTH_STENCIL = (1 << 1),
		ALLOW_UNORDERED_ACCESS = (1 << 2),
		DENY_SHADER_RESOURCE = (1 << 3),
		ALLOW_CROSS_ADAPTER = (1 << 4),
		ALLOW_SIMULTANEOUS_ACCESS = (1 << 5),
		VIDEO_DECODE_REFERENCE_ONLY = (1 << 6),
		NEED_PACKED_UAV = ALLOW_UNORDERED_ACCESS | 0x8000,
		ACCELERATION_STRUCTURE = ALLOW_UNORDERED_ACCESS | 0x400000
	};

	DEFINE_ENUM_FLAG_OPERATORS(ResourceFlag);

	// Structure defines the coordinates of the upper-left and lower-right corners of a rectangle.
	struct RectRange
	{
		RectRange() = default;
		RectRange(long left, long top, long right, long bottom)
		{
			Left = left;
			Top = top;
			Right = right;
			Bottom = bottom;
		}
		long Left;
		long Top;
		long Right;
		long Bottom;
	};

	// Describes a 3D box.
	struct BoxRange
	{
		BoxRange() = default;
		BoxRange(long left, long right)
		{
			Left = static_cast<uint32_t>(left);
			Top = 0;
			Front = 0;
			Right = static_cast<uint32_t>(right);
			Bottom = 1;
			Back = 1;
		}
		BoxRange(long left, long top, long right, long bottom)
		{
			Left = static_cast<uint32_t>(left);
			Top = static_cast<uint32_t>(top);
			Front = 0;
			Right = static_cast<uint32_t>(right);
			Bottom = static_cast<uint32_t>(bottom);
			Back = 1;
		}
		BoxRange(long left, long top, long front, long right, long bottom, long back)
		{
			Left = static_cast<uint32_t>(left);
			Top = static_cast<uint32_t>(top);
			Front = static_cast<uint32_t>(front);
			Right = static_cast<uint32_t>(right);
			Bottom = static_cast<uint32_t>(bottom);
			Back = static_cast<uint32_t>(back);
		}

		uint32_t Left;
		uint32_t Top;
		uint32_t Front;
		uint32_t Right;
		uint32_t Bottom;
		uint32_t Back;
	};

	// Describes a portion of a texture for the purpose of texture copies.
	struct TextureCopyLocation
	{
		TextureCopyLocation() = default;
		TextureCopyLocation(const Resource& res, uint32_t sub)
		{
			TextureResource = res;
			SubresourceIndex = sub;
		}
		Resource TextureResource;
		uint32_t SubresourceIndex;
	};

	// Describes the coordinates of a tiled resource.
	struct TiledResourceCoord
	{
		TiledResourceCoord() = default;
		TiledResourceCoord(uint32_t x, uint32_t y, uint32_t z, uint32_t subresource)
		{
			X = x;
			Y = y;
			Z = z;
			Subresource = subresource;
		}

		uint32_t X;
		uint32_t Y;
		uint32_t Z;
		UINT Subresource;
	};

	// Describes the size of a tiled region.
	struct TileRegionSize
	{
		uint32_t NumTiles;
		bool UseBox;
		uint32_t Width;
		uint16_t Height;
		uint16_t Depth;
	};

	// Describes the dimensions of a viewport.
	struct Viewport
	{
		Viewport() = default;
		Viewport(float topLeftX, float topLeftY, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f)
		{
			TopLeftX = topLeftX;
			TopLeftY = topLeftY;
			Width = width;
			Height = height;
			MinDepth = minDepth;
			MaxDepth = maxDepth;
		}

		float TopLeftX;
		float TopLeftY;
		float Width;
		float Height;
		float MinDepth;
		float MaxDepth;
	};
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_subresource_data
	struct SubresourceData
	{
		const void* pData;
		intptr_t RowPitch;
		intptr_t SlicePitch;
	};
	//https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_range
	struct Range
	{
		Range() = default;
		Range(uintptr_t begin, uintptr_t end)
		{
			Begin = begin;
			End = end;
		}

		uintptr_t Begin;
		uintptr_t End;
	};

	//Descriptor Related
	using Descriptor = uintptr_t; //storing a data pointer for the address of the descriptor.
	using DescriptorTable = std::shared_ptr<uint64_t>;
	struct Framebuffer
	{
		std::shared_ptr<Descriptor> RenderTargetViews;
	};

	/*...*/
}