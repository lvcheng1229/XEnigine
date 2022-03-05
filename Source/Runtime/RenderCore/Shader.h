#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/HAL/Mch.h"

#include <fstream>
#include <vector>
#include "Runtime/D3D12RHI/d3dx12.h"
#include <d3dcompiler.h>

#include "Runtime/RHI/RHIResource.h"
class XShader
{
private:
	uint16 srv_count;
	uint16 cbv_count;
	uint16 uav_count;
	uint16 sampler_count;
	XDxRefCount<ID3DBlob> byteCode;

	std::shared_ptr<XRHIGraphicsShader>RHIShader = nullptr;;
public:
	XShader() :byteCode(nullptr), srv_count(0), cbv_count(0), uav_count(0), sampler_count(0) {};

	void CreateShader(EShaderType shader_type);
	void CompileShader(const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
	void ShaderReflect();
	inline XDxRefCount<ID3DBlob> GetByteCode() { return byteCode; }

	inline std::shared_ptr<XRHIGraphicsShader> GetRHIGraphicsShader() { return RHIShader; }
	inline uint32 GetSRVCount() { return srv_count; }
	inline uint32 GetCBVCount() { return cbv_count; }
	inline uint32 GetUAVCount() { return uav_count; }
	inline uint32 GetSamplerCount() { return sampler_count; }
};
