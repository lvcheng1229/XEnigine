#pragma once
#include "D3D12Rootsignature.h"
#include "D3D12PipelineCurrentDescArray.h"

#include "D3D12View.h"

class XD3DDirectContex;
struct XD3D12SRVDescTableManager
{

};

struct XD3D12CBVRootDescManager
{
	D3D12_GPU_VIRTUAL_ADDRESS CurrentGPUVirtualAddress[EShaderType::SV_ShaderCount][MAX_ROOT_CONSTANT_NUM];
};



struct XD3D12PassShaderResourceManager
{
	XD3D12ShaderResourceView* Views[EShaderType::SV_ShaderCount][MAX_SHADER_RESOURCE_NUM];
};

class XD3D12PipelineCurrentDescArrayManager
{
private:
	XD3D12PhysicDevice* device;
	XD3DDirectContex* direct_ctx;
	XD3D12PipelineCurrentDescArray pipeline_current_desc_array;
public:
	XD3D12PipelineCurrentDescArrayManager() :device(nullptr), direct_ctx(nullptr) {};
	void Create(XD3D12PhysicDevice* device_in,XD3DDirectContex* direct_ctx_in);
	
	template<EShaderType shader_type>
	void SetDescTableSRVs(
		const XD3D12RootSignature* root_signature, 
		XD3D12PassShaderResourceManager* SRVManager,
		uint32& slot_start, uint32 slot_num);

	template<EShaderType shader_type>
	void SetRootDescCBVs(
		const XD3D12RootSignature* root_signature,
		XD3D12CBVRootDescManager* gpu_virtual_ptr_array,
		uint32 slot_num);

	inline XD3D12PipelineCurrentDescArray* GetCurrentDescArray() { return &pipeline_current_desc_array; }
};