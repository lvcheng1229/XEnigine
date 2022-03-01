#pragma once
//#include "Runtime/HAL/PlatformTypes.h"
//#include "Runtime/HAL/Mch.h"
//#include "d3dx12.h"

#include "Runtime/RHI/RHI.h"
#include "D3D12PhysicDevice.h"



struct XShaderRegisterCounts
{
	uint8 SamplerCount;
	uint8 ConstantBufferCount;
	uint8 ShaderResourceCount;
	uint8 UnorderedAccessCount;
};

enum ERootParameterKeys
{
	VS_SRVs,
	VS_CBVs,
	VS_RootCBVs,
	VS_Samplers,

	PS_SRVs,
	PS_CBVs,
	PS_RootCBVs,
	PS_Samplers,

	ALL_SRVs,
	ALL_CBVs,
	ALL_RootCBVs,
	ALL_Samplers,

	ALL_UAVs,
	Count,
};

struct XPipelineRegisterBoundCount
{
	XShaderRegisterCounts register_count[EShaderType::SV_ShaderCount];
};

//struct D3D12_ROOT_PARAMETER
//{
//	D3D12_ROOT_PARAMETER_TYPE ParameterType;
//	union
//	{
//		D3D12_ROOT_DESCRIPTOR_TABLE DescriptorTable;
//		D3D12_ROOT_CONSTANTS Constants;
//		D3D12_ROOT_DESCRIPTOR Descriptor;
//	};
//	D3D12_SHADER_VISIBILITY ShaderVisibility;
//};



#define MAX_ROOT_CONSTANT_NUM 16
//#define MAX_DESC_TABLE_NUM
//#define MAX_ROOT_DESC_NUM 0

#define PIPELINE_MAX_ROOT_PARAM_COUNT 32
class XD3D12RootSignature
{
private:
	uint8 ShaderResourceBindSlotIndexArray[ERootParameterKeys::Count];

	D3D12_ROOT_PARAMETER  slot_array[PIPELINE_MAX_ROOT_PARAM_COUNT];
	D3D12_DESCRIPTOR_RANGE  desc_range_array[PIPELINE_MAX_ROOT_PARAM_COUNT];

	D3D12_ROOT_SIGNATURE_DESC root_signature_info;
	XDxRefCount<ID3D12RootSignature> root_signature;
	XDxRefCount<ID3DBlob> serializedRootSig;

	XD3D12PhysicDevice* device;//TODO
public:
	XD3D12RootSignature() :device(nullptr),serializedRootSig(nullptr), root_signature(nullptr) {}
	void Create(XD3D12PhysicDevice* device_in,XPipelineRegisterBoundCount& register_count);

	uint32 GetSRVDescTableBindSlot(EShaderType shader_type)const;
	uint32 GetCBVDescTableBindSlot(EShaderType shader_type)const;
	uint32 GetSampleDescTableBindSlot(EShaderType shader_type)const;
	uint32 GetCBVRootDescBindSlot(EShaderType shader_type)const;
	inline ID3D12RootSignature* GetDXRootSignature() { return root_signature.Get(); }

	void SetSRVDescTableBindSlot(EShaderType shader_type, uint8 RootParameterIndex);
	void SetCBVDescTableBindSlot(EShaderType shader_type, uint8 RootParameterIndex);
	void SetSampleDescTableBindSlot(EShaderType shader_type, uint8 RootParameterIndex);
	inline void SetUAVDescTableTBindSlot(EShaderType SF, uint8 RootParameterIndex)
	{
		X_Assert(false);
	}

	void SetCBVRootDescBindSlot(EShaderType shader_type, uint8 RootParameterIndex);

};

#include "D3D12DescArrayShaderAcesss.h"
#include "D3D12Context.h"

struct XD3D12SRVDescTableManager
{

};
struct XD3D12CBVRootDescManager
{
	D3D12_GPU_VIRTUAL_ADDRESS CurrentGPUVirtualAddress[EShaderType::SV_ShaderCount][MAX_ROOT_CONSTANT_NUM];
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
		D3D12_CPU_DESCRIPTOR_HANDLE cpu_desc_ptr_in,
		uint32& slot_start, uint32 slot_num);

	template<EShaderType shader_type>
	void SetRootDescCBVs(
		const XD3D12RootSignature* root_signature,
		XD3D12CBVRootDescManager& gpu_virtual_ptr_array,
		uint32 slot_num);

	inline XD3D12PipelineCurrentDescArray* GetCurrentDescArray() { return &pipeline_current_desc_array; }
};