#include "Material.h"
#include <d3dcompiler.h>

void GMaterial::CreateGMaterial(const std::wstring& CodePathIn)
{
	XDxRefCount<ID3DBlob> BeforeCompressed;
	{
#if defined(DEBUG) || defined(_DEBUG)  
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		XDxRefCount<ID3DBlob> Errors;
		std::string Target = "ps_5_1";
		HRESULT hr = D3DCompileFromFile(
			CodePathIn.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"Empty_PS", Target.c_str(), compileFlags, 0, &BeforeCompressed, &Errors);

		if (Errors != nullptr)OutputDebugStringA((char*)Errors->GetBufferPointer()); ThrowIfFailed(hr);
	}

	{
		ID3D12ShaderReflection* Reflection = NULL;
		D3DReflect(BeforeCompressed->GetBufferPointer(), BeforeCompressed->GetBufferSize(), IID_PPV_ARGS(&Reflection));

		D3D12_SHADER_DESC ShaderDesc;
		Reflection->GetDesc(&ShaderDesc);
		for (uint32 i = 0; i < ShaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
			Reflection->GetResourceBindingDesc(i, &ResourceDesc);
			D3D_SHADER_INPUT_TYPE ResourceType = ResourceDesc.Type;

			if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
			{
				MaterialTextureArray.push_back(MaterialTexParas{ ResourceDesc.Name ,nullptr });
			}
			else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
			{
				ID3D12ShaderReflectionConstantBuffer* ConstantBuffer = Reflection->GetConstantBufferByName(ResourceDesc.Name);
				D3D12_SHADER_BUFFER_DESC CBDesc;
				ConstantBuffer->GetDesc(&CBDesc);
				for (uint32 ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++)
				{
					ID3D12ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
					D3D12_SHADER_VARIABLE_DESC VariableDesc;
					Variable->GetDesc(&VariableDesc);
					MaterialValueArray.push_back(MaterialValueParas{ VariableDesc.Name ,{0,0,0,0},VariableDesc.Size});
				}
			}
		}
	}
}
