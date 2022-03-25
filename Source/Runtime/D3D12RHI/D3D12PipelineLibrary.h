#pragma once
#include <d3d12.h>
#include <string>
#include "Runtime/HAL/Mch.h"

class XD3D12PhysicDevice;
class MemoryMappedFile
{
public:
    MemoryMappedFile();
    ~MemoryMappedFile();

    void Init(std::wstring filename, UINT filesize = 64);
    void Destroy(bool deleteFile);
    void GrowMapping(UINT size);

    void SetSize(UINT size)
    {
        if (m_mapAddress)
        {
            static_cast<UINT*>(m_mapAddress)[0] = size;
        }
    }

    UINT GetSize() const
    {
        if (m_mapAddress)
        {
            return static_cast<UINT*>(m_mapAddress)[0];
        }
        return 0;
    }

    void* GetData()
    {
        if (m_mapAddress)
        {
            return &static_cast<UINT*>(m_mapAddress)[1];// The actual data comes after the length.
        }
        return nullptr;
    }

public:
    bool IsMapped() const { return m_mapAddress != nullptr; }

protected:
    HANDLE m_mapFile;
    HANDLE m_file;
    LPVOID m_mapAddress;
    std::wstring m_filename;

    UINT m_currentFileSize;
};

class XD3D12PipelineLibrary
{
public:
    ~XD3D12PipelineLibrary();
	
    //ID3D12PipelineLibrary::LoadGraphics/ComputePipeline
    bool LoadPSOFromLibrary(LPCWSTR pName, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* pDesc, ID3D12PipelineState** PtrAddress);
	void StorePSOToLibrary(LPCWSTR pName,ID3D12PipelineState* pPipeline);//ID3D12PipelineLibrary::StorePipeline
	void SerializingPSOLibrary();//ID3D12PipelineLibrary::Serialize			
	void DeserializingPSOLibrary(XD3D12PhysicDevice* PhyDevice);//ID3D12Device1::CreatePipelineLibrary

	inline ID3D12PipelineLibrary* GetID3D12PipelineLibrary() { return m_pipelineLibrary.Get(); }
private:
    MemoryMappedFile MMappedFile;
	XDxRefCount<ID3D12PipelineLibrary> m_pipelineLibrary;
	XD3D12PhysicDevice* PhyDevice;
	bool Changed;
};