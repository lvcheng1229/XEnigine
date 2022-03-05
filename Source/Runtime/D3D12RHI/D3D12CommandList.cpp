#include  "D3D12CommandList.h"

void XD3D12CommandAllocator::Create(XD3D12PhysicDevice* device, D3D12_COMMAND_LIST_TYPE type)
{
	ThrowIfFailed(
		device->GetDXDevice()->CreateCommandAllocator(
			type,
			IID_PPV_ARGS(&DxCmdAlloc)));
	DxCmdAlloc->SetName(L"cmd allocator");
}

void XD3D12CommandAllocator::Reset()
{
	ThrowIfFailed(DxCmdAlloc->Reset());
}

void XD3D12ResourceBarrierManager::AddTransition(XD3D12Resource* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After)
{
	if (!barriers.empty())
	{
		const D3D12_RESOURCE_BARRIER& Last = barriers.back();
		if (pResource->GetResource() == Last.Transition.pResource &&
			//Subresource == Last.Transition.Subresource &&
			Before == Last.Transition.StateAfter &&
			After == Last.Transition.StateBefore && Last.Type == D3D12_RESOURCE_BARRIER_TYPE_TRANSITION) {
			X_Assert(false);
		}
	}

	D3D12_RESOURCE_BARRIER Barrier;
	Barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	Barrier.Transition.StateBefore = Before;
	Barrier.Transition.StateAfter = After;
	Barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;//NOTE!!
	Barrier.Transition.pResource = pResource->GetResource();
	barriers.push_back(Barrier);
}

void XD3D12ResourceBarrierManager::Flush(ID3D12GraphicsCommandList* pCommandList)
{
	if(barriers.size()>0)
		pCommandList->ResourceBarrier(barriers.size(), barriers.data());
	barriers.resize(0);
}



void XD3D12DirectCommandList::CreateDirectCmdList(XD3D12PhysicDevice* device, XD3D12CommandAllocator* cmd_alloc)
{
	ThrowIfFailed(
		device->GetDXDevice()->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			cmd_alloc->GetDXAlloc(),
			nullptr,
			IID_PPV_ARGS(&d3d12_cmd_list)));

}

void XD3D12DirectCommandList::Reset(XD3D12CommandAllocator* cmd_alloc)
{
	d3d12_cmd_list->Reset(cmd_alloc->GetDXAlloc(), nullptr);
}

void XD3D12DirectCommandList::Close()
{
	ThrowIfFailed(d3d12_cmd_list->Close());
}

