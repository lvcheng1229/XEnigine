#pragma once
#include "Runtime/RHI/RHIContext.h"
#include "D3D12CommandList.h"
//#include "XD3D12CommandListManger.h"

class XD3D12Context :public IRHIContext, public XD3D12DeviceChild
{

};

class XD3DDirectContex :public XD3D12Context
{
public:
	XD3DDirectContex(bool ctx_is_default_in) :ctx_is_default(ctx_is_default_in) {};
	void Create(XD3D12PhysicDevice* device_in);
	
	void OpenCmdList()override;
	void CloseCmdList()override;

	void RHISetViewport(float MinX, float MinY, float MinZ, float MaxX, float MaxY, float MaxZ)override;
	//void RHISetScissorRect(uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)override;

	void ResetCmdAlloc();
private:
public:
	inline XD3D12DirectCommandList* GetCmdList() { return &cmd_dirrect_list; };
	inline XD3D12CommandAllocator* GetCmdAlloc() { return &cmd_direct_alloc; };
private:
	//XD3D12CommandAllocManger* direct_cmd_allc_manager;
	//XD3D12CommandDirectListManger* direct_cmd_lsit_manager;
	const bool ctx_is_default;
	XD3D12CommandAllocator cmd_direct_alloc;
	XD3D12DirectCommandList cmd_dirrect_list;

private://TODO
	D3D12_VIEWPORT Viewport;
	D3D12_RECT ScissorRect;
};

