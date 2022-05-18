#include "D3D12Viewport.h"
#include "Runtime/D3D12RHI/D3D12PlatformRHI.h"

void XD3D12Viewport::Resize(
    uint32 size_x_in, 
    uint32 size_y_in)
{
    size_x = size_x_in;
    size_y = size_y_in;

    DXGI_FORMAT DxFormat = (DXGI_FORMAT)GPixelFormats[(int)Format].PlatformFormat;

    XD3D12PhysicDevice* PhysicalDevice = device->GetPhysicalDevice();
    XD3D12CommandQueue* direct_cmd_queue = device->GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE_DIRECT);
    XD3DDirectContex* directx_ctx = device->GetDirectContex(0);
    XD3D12DirectCommandList* cmd_list = directx_ctx->GetCmdList();
    XD3D12CommandAllocator* cmd_alloc = directx_ctx->GetCmdAlloc();

    direct_cmd_queue->CommandQueueWaitFlush();
    directx_ctx->OpenCmdList();

    for (int i = 0; i < BACK_BUFFER_COUNT_DX12; ++i)
    {
        if(back_buffer_resources[i].GetResource())
            back_buffer_resources[i].GetResource()->Release();
    }

    ThrowIfFailed(mSwapChain->ResizeBuffers(BACK_BUFFER_COUNT_DX12, size_x, size_y,
        DxFormat, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
    
    current_back_buffer = 0;

    D3D12_RENDER_TARGET_VIEW_DESC rt_desc;
    rt_desc.Format = DxFormat;
    rt_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    rt_desc.Texture2D.MipSlice = 0;
    rt_desc.Texture2D.PlaneSlice = 0;
    
    for (UINT i = 0; i < BACK_BUFFER_COUNT_DX12; i++)
    {
        uint32 index_of_desc_in_heap_rt;
        uint32 index_of_heap_rt;
        device->GetRenderTargetDescArrayManager()->AllocateDesc(index_of_desc_in_heap_rt, index_of_heap_rt);

        ThrowIfFailed(mSwapChain->GetBuffer(i, IID_PPV_ARGS(back_buffer_resources[i].GetPtrToResourceAdress())));
        back_buffer_resources[i].SetResourceState(D3D12_RESOURCE_STATE_COMMON);
        back_buffer_resources[i].GetResource()->SetName(L"BackBuffer");
        back_rt_views[i].Create(PhysicalDevice,&back_buffer_resources[i], rt_desc,
            device->GetRenderTargetDescArrayManager()->compute_cpu_ptr(index_of_desc_in_heap_rt, index_of_heap_rt),
            device->GetRenderTargetDescArrayManager()->compute_gpu_ptr(index_of_desc_in_heap_rt, index_of_heap_rt)
        );

        std::shared_ptr<XD3D12Texture2D>BackBufferTexture = std::make_shared<XD3D12Texture2D>(Format);
        BackBufferTexture->SetRenderTargetView(back_rt_views[i]);
        BackBufferTextures.push_back(BackBufferTexture);
    }

    directx_ctx->CloseCmdList();

    ID3D12CommandList* cmdsLists[] = { directx_ctx->GetCmdList()->GetDXCmdList()};
    direct_cmd_queue->GetDXCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    direct_cmd_queue->CommandQueueWaitFlush();
}

void XD3D12Viewport::Present()
{
    XD3DDirectContex* DirectCtx = device->GetDirectContex(0);
    XD3D12CommandQueue* DirectCmdQueue = device->GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE_DIRECT);

    XD3D12PlatformRHI::TransitionResource(*DirectCtx->GetCmdList(), this->GetCurrentBackTexture()->GetRenderTargetView(), D3D12_RESOURCE_STATE_PRESENT);
    DirectCtx->GetCmdList()->CmdListFlushBarrier();
    DirectCtx->CloseCmdList();

    ID3D12CommandList* cmdsLists[] = { DirectCtx->GetCmdList()->GetDXCmdList() };
    DirectCmdQueue->GetDXCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    DirectCmdQueue->CommandQueueWaitFlush();

    ThrowIfFailed(mSwapChain->Present(0, 0));
    current_back_buffer = (current_back_buffer + 1) % BACK_BUFFER_COUNT_DX12;
}
