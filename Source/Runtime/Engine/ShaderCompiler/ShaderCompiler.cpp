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

		Output.ShaderCode.clear();
		//https://stackoverflow.com/questions/259297/how-do-you-copy-the-contents-of-an-array-to-a-stdvector-in-c-without-looping
		Output.ShaderCode.insert(
			Output.ShaderCode.end(),
			static_cast<uint8*>(CodeGened->GetBufferPointer()),
			static_cast<uint8*>(CodeGened->GetBufferPointer()) + CodeGened->GetBufferSize());
	}

	//Reflect
	{
		ID3D12ShaderReflection* Reflection = NULL;
		D3DReflect(CodeGened->GetBufferPointer(), CodeGened->GetBufferSize(), IID_PPV_ARGS(&Reflection));

		D3D12_SHADER_DESC ShaderDesc;
		Reflection->GetDesc(&ShaderDesc);

		uint32 NumSRVCount = 0;
		uint32 NumCBVCount = 0;
		uint32 NumUAVCount = 0;
		uint32 NumSamplerCount = 0;
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
		Output.ShaderCode.insert(Output.ShaderCode.end(), (uint8*)(&TotalOptionalDataSize), (uint8*)(&TotalOptionalDataSize) + 4);
	}
}

void CompileGlobalShaderMap()
{
	if (GGlobalShaderMap == nullptr)
	{
		GGlobalShaderMap = new XGlobalShaderMapInProjectUnit();
		std::list<XShaderInfosUsedToCompile*>& ShaderInfosUsedToCompile_LinkedList = XShaderInfosUsedToCompile::GetShaderInfosUsedToCompile_LinkedList();
		for (auto iter = ShaderInfosUsedToCompile_LinkedList.begin(); iter != ShaderInfosUsedToCompile_LinkedList.end(); iter++)
		{
			XShaderCompileInput Input;
			Input.SourceFilePath = (*iter)->GetSourceFileName();
			Input.EntryPointName = (*iter)->GetEntryName();
			XShaderCompileOutput Output;
			CompileDX12Shader(Input, Output);
			//FGlobalShaderMapSection* Section = GGlobalShaderMap->FindOrAddSection(ShaderType);
			//Section->GetResourceCode()->AddShaderCompilerOutput(CurrentJob.Output);
			GGlobalShaderMap->GlobalShaderMapFindOrAddShader(,0,)


		}
	}
}