#include "Runtime/RenderCore/ShaderCore.h"
#include "ShaderCompiler.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include <filesystem>
static const std::filesystem::path ShaderASMPath("E:\\XEngine\\XEnigine\\Cache");
//https://docs.microsoft.com/en-us/windows/win32/direct3dtools/dx-graphics-tools-fxc-syntax

#define CACHE_SHADER 0


class XD3DInclude : public ID3DInclude
{
public:
	std::map<std::string, std::string>* IncludePathToCode;
	bool HasIncludePath;
	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
	{
		auto iter = IncludePathToCode->find(std::string(pFileName));
		if (iter != IncludePathToCode->end())
		{
			*ppData = iter->second.data();
			*pBytes = iter->second.size();

			HasIncludePath = true;
			return S_OK;
		}

		std::string FilePath = GLOBAL_SHADER_PATH + std::string(pFileName);
		if (!std::filesystem::exists(FilePath))
			return E_FAIL;

		//open and get file size
		std::ifstream FileSourceCode(FilePath, std::ios::ate);
		UINT FileSize = static_cast<UINT>(FileSourceCode.tellg());
		FileSourceCode.seekg(0, std::ios_base::beg);
		
		//read data
		*pBytes = FileSize;
		ACHAR* data = static_cast<ACHAR*>(std::malloc(*pBytes));
		if (data)
		{
			//error X3000: Illegal character in shader file
			//https: //zhuanlan.zhihu.com/p/27253604
			memset(data, '\n', FileSize);
		}
		FileSourceCode.read(data, FileSize);


	
		HasIncludePath = false;
		*ppData = data;
		return S_OK;
}

	HRESULT Close(LPCVOID pData) override
	{
		if (HasIncludePath == false)
		{
			std::free(const_cast<void*>(pData));
		}
		return S_OK;
	}
};

static void CompileDX12Shader(XShaderCompileInput& Input, XShaderCompileOutput& Output)
{
#if defined(DEBUG) || defined(_DEBUG)  
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	
	XDxRefCount<ID3DBlob> CodeGened;
	XDxRefCount<ID3DBlob> Errors;
	
#if CACHE_SHADER
	const std::filesystem::path  FilePath = (ShaderASMPath / (Input.ShaderName + ".cso"));
	if (std::filesystem::exists(FilePath))
	{
		std::ifstream fin(FilePath, std::ios::binary);
		fin.seekg(0, std::ios_base::end);
		std::ifstream::pos_type size = (int)fin.tellg();
		fin.seekg(0, std::ios_base::beg);

		ThrowIfFailed(D3DCreateBlob(size, CodeGened.GetAddressOf()));

		fin.read((char*)CodeGened->GetBufferPointer(), size);
		fin.close();

		Output.Shadertype = Input.Shadertype;
		Output.ShaderCode.clear();
		Output.ShaderCode.insert(
			Output.ShaderCode.end(),
			static_cast<uint8*>(CodeGened->GetBufferPointer()),
			static_cast<uint8*>(CodeGened->GetBufferPointer()) + CodeGened->GetBufferSize());
		Output.SourceCodeHash = std::hash<std::string>{}(
			std::string((const char*)Output.ShaderCode.data(), Output.ShaderCode.size()));
	}
	else
#endif
	{
		std::string Target;
		switch (Input.Shadertype)
		{
		case EShaderType::SV_Vertex:Target = "vs_5_1"; break;
		case EShaderType::SV_Pixel:Target = "ps_5_1"; break;
		case EShaderType::SV_Compute:Target = "cs_5_1"; break;
		default:X_Assert(false); break;
		}

		std::vector<D3D_SHADER_MACRO>Macro;
		if (Input.CompileSettings.Defines.size() > 0)
		{
			Macro.resize(Input.CompileSettings.Defines.size() + 1);
			Macro[Input.CompileSettings.Defines.size()].Name = NULL;
			Macro[Input.CompileSettings.Defines.size()].Definition = NULL;
			int index = 0;
			for (auto iter = Input.CompileSettings.Defines.begin(); iter != Input.CompileSettings.Defines.end(); iter++, index++)
			{
				Macro[index].Name = iter->first.c_str();
				Macro[index].Definition = iter->second.c_str();
			};
		}

		XDxRefCount<ID3DBlob> BeforeCompressed;

		XD3DInclude Include;
		Include.IncludePathToCode = &Input.CompileSettings.IncludePathToCode;
		HRESULT hr = D3DCompileFromFile(
			Input.SourceFilePath.data(),
			Macro.data(), 
			&Include,
			//D3D_COMPILE_STANDARD_FILE_INCLUDE,
			Input.EntryPointName.data(),
			Target.data(), compileFlags, 0,
			&BeforeCompressed, &Errors);

		if (Errors != nullptr)OutputDebugStringA((char*)Errors->GetBufferPointer()); ThrowIfFailed(hr);
		
		CodeGened = BeforeCompressed;

		//https: //github.com/TheRealMJP/BakingLab/blob/master/SampleFramework11/v1.02/Graphics/ShaderCompilation.cpp#L131
		//need D3DDecompressShaders
		//D3D_SHADER_DATA ShaderData;
		//ShaderData.pBytecode = BeforeCompressed->GetBufferPointer();
		//ShaderData.BytecodeLength = BeforeCompressed->GetBufferSize();
		//ThrowIfFailed(D3DCompressShaders(1, &ShaderData, D3D_COMPRESS_SHADER_KEEP_ALL_PARTS, &CodeGened));
		

		Output.Shadertype = Input.Shadertype;
		Output.ShaderCode.clear();
		Output.ShaderCode.insert(
			Output.ShaderCode.end(),
			static_cast<uint8*>(CodeGened->GetBufferPointer()),
			static_cast<uint8*>(CodeGened->GetBufferPointer()) + CodeGened->GetBufferSize());
		Output.SourceCodeHash = std::hash<std::string>{}(
			std::string((const char*)Output.ShaderCode.data(), Output.ShaderCode.size()));
#if CACHE_SHADER
		std::ofstream fCodeOut(FilePath, std::ios::binary);
		fCodeOut.write((const char*)Output.ShaderCode.data(), Output.ShaderCode.size());
#endif
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

			XShaderParameterInfo ParameterInfo;
			ParameterInfo.BufferIndex = ResourceDesc.BindPoint;
			ParameterInfo.ResourceCount = ResourceDesc.BindCount;
			
			if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
			{
				NumSRVCount++;
				ParameterInfo.Parametertype = EShaderParametertype::SRV;
				Output.ShaderParameterMap.MapNameToParameter[ResourceDesc.Name] = ParameterInfo;
			}
			else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
			{
				NumCBVCount++;
				ParameterInfo.Parametertype = EShaderParametertype::CBV;
				Output.ShaderParameterMap.MapNameToParameter[ResourceDesc.Name] = ParameterInfo;
				if (strncmp(ResourceDesc.Name, "cbView", 64) != 0)
				{
					ID3D12ShaderReflectionConstantBuffer* ConstantBuffer = Reflection->GetConstantBufferByName(ResourceDesc.Name);
					D3D12_SHADER_BUFFER_DESC CBDesc;
					ConstantBuffer->GetDesc(&CBDesc);
					for (uint32 ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++)
					{
						ID3D12ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
						D3D12_SHADER_VARIABLE_DESC VariableDesc;
						Variable->GetDesc(&VariableDesc);
						ParameterInfo.VariableOffsetInBuffer = VariableDesc.StartOffset;
						ParameterInfo.VariableSize = VariableDesc.Size;
						Output.ShaderParameterMap.MapNameToParameter[VariableDesc.Name] = ParameterInfo;
					}
				}
			}
			else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED)
			{
				NumUAVCount++;
				ParameterInfo.Parametertype = EShaderParametertype::UAV;
				Output.ShaderParameterMap.MapNameToParameter[ResourceDesc.Name] = ParameterInfo;
			}
			else if(ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_SAMPLER)
			{
				NumSamplerCount++;
				ParameterInfo.Parametertype = EShaderParametertype::Sampler;
				Output.ShaderParameterMap.MapNameToParameter[ResourceDesc.Name] = ParameterInfo;
			}
			else
			{
				X_Assert(false);
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

void CompileMaterialShader(XShaderCompileInput& Input, XShaderCompileOutput& Output)
{
	CompileDX12Shader(Input, Output);
}

void CompileGlobalShaderMap()
{
	if (GGlobalShaderMapping == nullptr)
	{
		//ShaderMapInFileUnit has three part , first : source code ,second : shaders info,third : RHIShader 

		GGlobalShaderMapping = new XGlobalShaderMapping_ProjectUnit();
		std::list<XShaderInfo*>& ShaderInfosUsedToCompile_LinkedList = XShaderInfo::GetShaderInfo_LinkedList(XShaderInfo::EShaderTypeForDynamicCast::Global);
		for (auto iter = ShaderInfosUsedToCompile_LinkedList.begin(); iter != ShaderInfosUsedToCompile_LinkedList.end(); iter++)
		{
			XShaderCompileInput Input;
			Input.SourceFilePath = (*iter)->GetSourceFileName();
			Input.EntryPointName = (*iter)->GetEntryName();
			Input.Shadertype = (*iter)->GetShaderType();
			Input.ShaderName = (*iter)->GetShaderName();
			(*iter)->ModifySettingsPtr(Input.CompileSettings);

			XShaderCompileOutput Output;
			CompileDX12Shader(Input, Output);
			XGlobalShaderMapping_FileUnit* ShaderFileUnit = GGlobalShaderMapping->FindOrAddShaderMapping_FileUnit((*iter));
			
			//first:store source code
			std::size_t HashIndex = ShaderFileUnit->GetShaderMapStoreCodes()->AddShaderCompilerOutput(Output);
			
			//second: xshaders
			XXShader* Shader = (*iter)->CtorPtr(XShaderInitlizer(*iter, Output, HashIndex));
			ShaderFileUnit->GetShaderMapStoreXShaders()->FindOrAddXShader((*iter)->GetHashedShaderNameIndex(), Shader, 0);
		}
		
		//third: RHIShader
		std::unordered_map<std::size_t, XGlobalShaderMapping_FileUnit*>ShaderMapFileUnit = GGlobalShaderMapping->GetGlobalShaderMap_HashMap();
		for (auto iter = ShaderMapFileUnit.begin(); iter != ShaderMapFileUnit.end(); iter++)
		{
			iter->second->InitRHIShaders_InlineCode();
		}
	}
}