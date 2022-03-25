#include "D3D12PipelineLibrary.h"
#include "D3D12PhysicDevice.h"
void XD3D12PipelineLibrary::DeserializingPSOLibrary(XD3D12PhysicDevice* PhyDevice)
{
		//step 1
	std::wstring FileName = L"E:/XEngine/XEnigine/graphics_pso_cache.cache";
	
	WIN32_FIND_DATA FIndFileData;
	HANDLE handle = FindFirstFileEx(FileName.c_str(), FindExInfoBasic, &FIndFileData, FindExSearchNameMatch, nullptr, 0);
	bool found = handle != INVALID_HANDLE_VALUE;

	if (found)
	{
		FindClose(handle);
	}

	
	HANDLE M_File = CreateFile2(FileName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, (found) ? OPEN_EXISTING : CREATE_NEW, nullptr);
	X_Assert(M_File != INVALID_HANDLE_VALUE);
	
	LARGE_INTEGER RealFileSize = {};
	BOOL flag = GetFileSizeEx(M_File, &RealFileSize);
	unsigned int m_currentFileSize = RealFileSize.LowPart;
	
	if (m_currentFileSize == 0)
	{
		m_currentFileSize = 64U;// File mapping files with a size of 0 produces an error.
	}
	else if (64U > m_currentFileSize)
	{
		m_currentFileSize = 64U;// Grow to the specified size.
	}

	HANDLE m_mapFile = CreateFileMapping(M_File, nullptr, PAGE_READWRITE, 0, m_currentFileSize, nullptr); X_Assert(m_mapFile != nullptr);
	void* m_mapAddress = MapViewOfFile(m_mapFile, FILE_MAP_ALL_ACCESS, 0, 0, m_currentFileSize); X_Assert(m_mapAddress != nullptr);
	
	//https://github.com/microsoft/DirectX-Graphics-Samples/blob/master/Samples/Desktop/D3D12PipelineStateCache/src/MemoryMappedPipelineLibrary.cpp
	PhyDevice->GetDXDevice1()->CreatePipelineLibrary(
		,
		,
		IID_PPV_ARGS(&m_pipelineLibrary));

	ThrowIfFailed(device1->CreatePipelineLibrary(&static_cast<UINT*>(m_mapAddress)[1], static_cast<UINT*>(m_mapAddress)[0], IID_PPV_ARGS(&m_pipelineLibrary)));

}