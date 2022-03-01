#include "D3D12Context.h"
void XD3DDirectContex::Create(XD3D12PhysicDevice* device_in)
{
	SetParentDevice(device_in);
	cmd_direct_alloc.Create(device_in, D3D12_COMMAND_LIST_TYPE_DIRECT);
	cmd_direct_alloc.Reset();

	cmd_dirrect_list.CreateDirectCmdList(device_in, &cmd_direct_alloc);
	cmd_dirrect_list.Close();
	//cmd_dirrect_list.Reset(&cmd_direct_alloc);
	//direct_cmd_allc_manager = new XD3D12CommandAllocManger(D3D12_COMMAND_LIST_TYPE_DIRECT);
	//direct_cmd_allc_manager->Create(device_in);
}

void XD3DDirectContex::OpenCmdList()
{
	cmd_dirrect_list.Reset(&cmd_direct_alloc);
}

void XD3DDirectContex::CloseCmdList()
{
	cmd_dirrect_list.Close();
}

void XD3DDirectContex::RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)
{
	Viewport = { MinX, MinY, (MaxX - MinX), (MaxY - MinY), MinZ, MaxZ };
	ScissorRect = { static_cast<long>(MinX), static_cast<long>(MinY), static_cast<long>(MaxX), static_cast<long>(MaxY) };
	cmd_dirrect_list->RSSetViewports(1, &Viewport);
	cmd_dirrect_list->RSSetScissorRects(1, &ScissorRect);
}

void XD3DDirectContex::ResetCmdAlloc()
{
	cmd_direct_alloc.Reset();
}
