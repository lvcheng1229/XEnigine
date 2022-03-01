//#include "XD3D12CommandListManger.h"

//void XD3D12CommandAllocManger::Create(XD3D12PhysicDevice* device_in)
//{
//	SetParentDevice(device_in);
//
//}
//
//XD3D12CommandAllocator* XD3D12CommandAllocManger::GetFreeCmdAlloc()
//{
//	XD3D12CommandAllocator* ret_alloc = nullptr;
//	if (!free_alloctor.empty())
//	{
//		ret_alloc = free_alloctor.front();
//		free_alloctor.pop();
//	}
//	else
//	{
//		ret_alloc = new XD3D12CommandAllocator();
//		all_new_allocs.push_back(ret_alloc);
//		ret_alloc->Create(GetDXDevice(), Type);
//		ret_alloc->Reset();
//	}
//	return ret_alloc;
//}
//
//void XD3D12CommandAllocManger::ReleaseCmdAlloc(XD3D12CommandAllocator* cmd_alloc)
//{
//	free_alloctor.push(cmd_alloc);
//}

//void XD3D12CommandDirectListManger::Create(XD3D12PhysicDevice* device)
//{
//	SetParentDevice(device);
//}
//
//XD3D12DirectCommandList* XD3D12CommandDirectListManger::GetFreeCmdList(XD3D12CommandAllocator* alloc_in)
//{
//	XD3D12DirectCommandList* ret_cmd_list;
//	if (!cmd_list_queue.empty())
//	{
//		ret_cmd_list = cmd_list_queue.front();
//		cmd_list_queue.pop();
//	}
//	else
//	{
//		ret_cmd_list = new XD3D12DirectCommandList();
//		cmd_list_queue.push(ret_cmd_list);
//		all_new_cmd_lists.push_back(ret_cmd_list);
//	}
//	ret_cmd_list->Reset(alloc_in);
//	return ret_cmd_list;
//}
