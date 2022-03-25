#pragma once
#include <d3d12.h>
#include <string>
#include "Runtime/HAL/Mch.h"
class XD3D12PhysicDevice;
class XD3D12PipelineLibrary
{
public:

	//ID3D12Device1::CreatePipelineLibrary
	void DeserializingPSOLibrary(XD3D12PhysicDevice* PhyDevice);
	
	//ID3D12PipelineLibrary::Serialize
	void SerializingPSOLibrary();					
	
	//ID3D12PipelineLibrary::StorePipeline
	//void StorePSOToLibrary(
	
	//ID3D12PipelineLibrary::LoadGraphics/ComputePipeline
	//bool LoadPSOFromLibrary(

	inline ID3D12PipelineLibrary* GetID3D12PipelineLibrary() { return m_pipelineLibrary.Get(); }
private:
	XDxRefCount<ID3D12PipelineLibrary> m_pipelineLibrary;
};