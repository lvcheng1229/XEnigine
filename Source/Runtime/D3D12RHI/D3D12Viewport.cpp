#include "D3D12Viewport.h"

void XD3D12Viewport::Create(
    XD3D12AbstractDevice* device_in, 
    uint32 size_x_in, 
    uint32 size_y_in,
    DXGI_FORMAT format_back_buffer_in, 
    HWND WindowHandle_in)
{
    device = device_in;
    size_x = size_x_in;
    size_y = size_y_in;
    format = format_back_buffer_in;

    DXGI_SWAP_CHAIN_DESC sd;
    sd.BufferDesc.Width = size_x;
    sd.BufferDesc.Height = size_y;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferDesc.Format = format;
    sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
    sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
    sd.BufferCount = BACK_BUFFER_COUNT_DX12;
    sd.OutputWindow = WindowHandle_in;
    sd.Windowed = true;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    device->GetPhysicalDevice()->GetAdapter()->GetDXFactory()->CreateSwapChain(
        device->GetCmdQueueByType(D3D12_COMMAND_LIST_TYPE_DIRECT)->GetDXCommandQueue(),
        &sd,
        mSwapChain.GetAddressOf());
}

void XD3D12Viewport::Resize(
    uint32 size_x_in, 
    uint32 size_y_in)
{
    size_x = size_x_in;
    size_y = size_y_in;

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
        format, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH));
    
    current_back_buffer = 0;

    D3D12_RENDER_TARGET_VIEW_DESC rt_desc;
    rt_desc.Format = format;
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
    }

    directx_ctx->CloseCmdList();

    ID3D12CommandList* cmdsLists[] = { directx_ctx->GetCmdList()->GetDXCmdList()};
    direct_cmd_queue->GetDXCommandQueue()->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);
    direct_cmd_queue->CommandQueueWaitFlush();
}

void XD3D12Viewport::Present()
{
    ThrowIfFailed(mSwapChain->Present(0, 0));
    current_back_buffer = (current_back_buffer + 1) % BACK_BUFFER_COUNT_DX12;
}
