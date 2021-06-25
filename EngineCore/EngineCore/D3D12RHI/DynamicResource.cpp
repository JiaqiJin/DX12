#include "../pch.h"
#include "DynamicResource.h"
#include "RenderDevice.h"

namespace RHI
{
	// Allocate a Buffer on the Upload heap and Map it
	D3D12DynamicPage::D3D12DynamicPage(UINT64 Size)
	{

        D3D12_HEAP_PROPERTIES HeapProps;
        HeapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
        HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        HeapProps.CreationNodeMask = 1;
        HeapProps.VisibleNodeMask = 1;

        D3D12_RESOURCE_DESC ResourceDesc;
        ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        ResourceDesc.Width = Size;
        ResourceDesc.Alignment = 0;
        ResourceDesc.Height = 1;
        ResourceDesc.DepthOrArraySize = 1;
        ResourceDesc.MipLevels = 1;
        ResourceDesc.Format = DXGI_FORMAT_UNKNOWN;
        ResourceDesc.SampleDesc.Count = 1;
        ResourceDesc.SampleDesc.Quality = 0;
        ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

        ID3D12Device* d3dDevice = RenderDevice::GetSingleton().GetD3D12Device();
        ThrowIfFailed(d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_pd3d12Buffer)));

        m_pd3d12Buffer->SetName(L"Dynamic memory page");

        m_GPUVirtualAddress = m_pd3d12Buffer->GetGPUVirtualAddress();

        m_pd3d12Buffer->Map(0, nullptr, &m_CPUVirtualAddress);
	}

    DynamicResourceAllocator::DynamicResourceAllocator(UINT32 NumPagesToReserve, UINT64 PageSize)
    {
        for (UINT32 i = 0; i < NumPagesToReserve; i++)
        {
            D3D12DynamicPage Page(PageSize);
            auto Size = Page.GetSize();
            m_AvailablePages.emplace(Size, std::move(Page));
        }
    }

    D3D12DynamicPage DynamicResourceAllocator::AllocatePage(UINT64 SizeInBytes)
    {
        // Returns the iterator of the first Page that is larger than SizeInBytes
        auto PageIt = m_AvailablePages.lower_bound(SizeInBytes);
        if (PageIt != m_AvailablePages.end())
        {
            assert(PageIt->first >= SizeInBytes);
            D3D12DynamicPage Page(std::move(PageIt->second));
            m_AvailablePages.erase(PageIt);
            return Page;
        }
        else
        {
            return D3D12DynamicPage{ SizeInBytes };
        }
    }

    void DynamicResourceAllocator::ReleasePages(std::vector<D3D12DynamicPage>& Pages)
    {
        struct StalePage
        {
            D3D12DynamicPage           Page;
            DynamicResourceAllocator* Mgr;

            StalePage(D3D12DynamicPage&& _Page, DynamicResourceAllocator& _Mgr) noexcept :
                Page{ std::move(_Page) },
                Mgr{ &_Mgr }
            {
            }

            StalePage(const StalePage&) = delete;
            StalePage& operator= (const StalePage&) = delete;
            StalePage& operator= (StalePage&&) = delete;

            StalePage(StalePage&& rhs)noexcept :
                Page{ std::move(rhs.Page) },
                Mgr{ rhs.Mgr }
            {
                rhs.Mgr = nullptr;
            }

            ~StalePage()
            {
                if (Mgr != nullptr)
                {
                    auto PageSize = Page.GetSize();
                    Mgr->m_AvailablePages.emplace(PageSize, std::move(Page));
                }
            }
        };
        for (auto& Page : Pages)
        {
            RenderDevice::GetSingleton().SafeReleaseDeviceObject(StalePage{ std::move(Page), *this });
        }
    }

    void DynamicResourceAllocator::Destroy()
    {
        m_AvailablePages.clear();
    }

    DynamicResourceAllocator::~DynamicResourceAllocator()
    {
        assert(m_AvailablePages.empty() && "Not all pages are destroyed. Dynamic memory manager must be explicitly destroyed with Destroy() method");
    }
}