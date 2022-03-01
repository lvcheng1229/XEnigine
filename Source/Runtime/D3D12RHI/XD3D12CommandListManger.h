#pragma once
//#include <vector>
//#include <queue>
//#include "D3D12CommandList.h"

//class XD3D12CommandAllocManger : public XD3D12DeviceChild
//{
//public:
//	XD3D12CommandAllocManger(const D3D12_COMMAND_LIST_TYPE type_in)
//		:Type(type_in) {}
//	void Create(XD3D12PhysicDevice* device_in);
//	inline ~XD3D12CommandAllocManger() { for (auto t : all_new_allocs) { delete t; } }
//	XD3D12CommandAllocator* GetFreeCmdAlloc();
//	void ReleaseCmdAlloc(XD3D12CommandAllocator* cmd_alloc);
//private:
//	std::queue<XD3D12CommandAllocator*> free_alloctor;
//	std::vector<XD3D12CommandAllocator*> all_new_allocs;
//	const D3D12_COMMAND_LIST_TYPE Type;
//};



//class XD3D12CommandDirectListManger : public XD3D12DeviceChild
//{
//public:
//	void Create(XD3D12PhysicDevice* device);
//	~XD3D12CommandDirectListManger() { for (auto t : all_new_cmd_lists) { delete t; }; }
//	XD3D12DirectCommandList* GetFreeCmdList(XD3D12CommandAllocator *alloc_in);
//	void ResetCmdList();
//private:
//	std::queue<XD3D12DirectCommandList*>cmd_list_queue;
//	std::vector<XD3D12DirectCommandList*> all_new_cmd_lists;
//};