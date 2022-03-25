#include "Runtime/RenderCore/ShaderCore.h"
#include "ShaderCompiler.h"
#include "Runtime/RenderCore/GlobalShader.h"

static void CompileDX12Shader(XShaderCompileInput& Input, XShaderCompileOutput& Output)
{
#if defined(DEBUG) || defined(_DEBUG)  
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	XDxRefCount<ID3DBlob> CodeGened;
	XDxRefCount<ID3DBlob> Errors;
	{
		std::string Target;
		switch (Input.Shadertype)
		{
		case EShaderType::SV_Vertex:Target = "vs_5_1"; break;
		case EShaderType::SV_Pixel:Target = "ps_5_1"; break;
		case EShaderType::SV_Compute:Target = "cs_5_1"; break;
		default:X_Assert(false); break;
		}

		HRESULT hr = D3DCompileFromFile(
			Input.SourceFilePath.data(),
			nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			Input.EntryPointName.data(),
			Target.data(), compileFlags, 0,
			&CodeGened, &Errors);

		if (Errors != nullptr)OutputDebugStringA((char*)Errors->GetBufferPointer()); ThrowIfFailed(hr);

		Output.Shadertype = Input.Shadertype;
		Output.ShaderCode.clear();
		Output.ShaderCode.insert(
			Output.ShaderCode.end(),
			static_cast<uint8*>(CodeGened->GetBufferPointer()),
			static_cast<uint8*>(CodeGened->GetBufferPointer()) + CodeGened->GetBufferSize());
		Output.SourceCodeHash = std::hash<std::string>{}(
			std::string((const char*)Output.ShaderCode.data(), Output.ShaderCode.size()));


	}

	//Reflect
	{
		ID3D12ShaderReflection* Reflection = NULL;
		D3DReflect(CodeGened->GetBufferPointer(), CodeGened->GetBufferSize(), IID_PPV_ARGS(&Reflection));

		D3D12_SHADER_DESC ShaderDesc;
		Reflection->GetDesc(&ShaderDesc);

		uint8 NumSRVCount = 0;
		uint8 NumCBVCount = 0;
		uint8 NumUAVCount = 0;
		uint8 NumSamplerCount = 0;
		for (uint32 i = 0; i < ShaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
			Reflection->GetResourceBindingDesc(i, &ResourceDesc);
			D3D_SHADER_INPUT_TYPE ResourceType = ResourceDesc.Type;

			switch (ResourceType)
			{
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE:NumSRVCount++; break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER:NumCBVCount++; break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED:NumUAVCount++; break;
			case D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER:NumSamplerCount++; break;
			default:X_Assert(false); break;
			}
		}
		int32 TotalOptionalDataSize = 0;
		XShaderResourceCount ResourceCount = { NumSRVCount ,NumCBVCount ,NumUAVCount };

		int32 ResoucrCountSize = static_cast<int32>(sizeof(XShaderResourceCount));
		Output.ShaderCode.push_back(XShaderResourceCount::Key);
		Output.ShaderCode.insert(Output.ShaderCode.end(), (uint8*)(&ResoucrCountSize), (uint8*)(&ResoucrCountSize) + 4);
		Output.ShaderCode.insert(Output.ShaderCode.end(), (uint8*)(&ResourceCount), (uint8*)(&ResourceCount) + sizeof(XShaderResourceCount));
		
		TotalOptionalDataSize += sizeof(uint8);//XShaderResourceCount::Key
		TotalOptionalDataSize += sizeof(int32);//ResoucrCountSize
		TotalOptionalDataSize += sizeof(XShaderResourceCount);//XShaderResourceCount
		TotalOptionalDataSize += sizeof(int32);//TotalOptionalDataSize
		Output.ShaderCode.insert(Output.ShaderCode.end(), (uint8*)(&TotalOptionalDataSize), (uint8*)(&TotalOptionalDataSize) + 4);
	}
}
void CompileGlobalShaderMap()
{
	if (GGlobalShaderMap == nullptr)
	{
		//ShaderMapInFileUnit has three part , first : source code ,second : shaders info,third : RHIShader 

		GGlobalShaderMap = new XGlobalShaderMapInProjectUnit();
		std::list<XShaderInfos*>& ShaderInfosUsedToCompile_LinkedList = XShaderInfos::GetShaderInfos_LinkedList();
		for (auto iter = ShaderInfosUsedToCompile_LinkedList.begin(); iter != ShaderInfosUsedToCompile_LinkedList.end(); iter++)
		{
			XShaderCompileInput Input;
			Input.SourceFilePath = (*iter)->GetSourceFileName();
			Input.EntryPointName = (*iter)->GetEntryName();
			Input.Shadertype = (*iter)->GetShaderType();
			XShaderCompileOutput Output;
			CompileDX12Shader(Input, Output);
			XGlobalShaderMapInFileUnit* ShaderFileUnit = GGlobalShaderMap->FindOrAddShaderMapFileUnit((*iter));
			
			//first:store source code
			std::size_t HashIndex = ShaderFileUnit->GetShaderMapStoreCodes()->AddShaderCompilerOutput(Output);
			//second: shaders info
			XXShader* Shader = new XXShader(*iter, &Output);
			Shader->SetRHIShaderIndex(HashIndex);
			ShaderFileUnit->GetShaderMapStoreXShaders()->FindOrAddXShader((*iter)->GetHashedEntryIndex(), Shader, 0);
		}
		
		//third: RHIShader
		std::unordered_map<std::size_t, XGlobalShaderMapInFileUnit*>ShaderMapFileUnit = GGlobalShaderMap->GetGlobalShaderMap_HashMap();
		for (auto iter = ShaderMapFileUnit.begin(); iter != ShaderMapFileUnit.end(); iter++)
		{
			iter->second->InitRHIShaders_InlineCode();
		}
	}
}