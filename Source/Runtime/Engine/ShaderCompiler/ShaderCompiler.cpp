#include "Runtime/RenderCore/ShaderCore.h"
#include "ShaderCompiler.h"
#include "Runtime/RenderCore/GlobalShader.h"
#include <filesystem>
#include "Runtime/Core/Misc/Path.h"


#define CACHE_PATH BOOST_PP_CAT(BOOST_PP_STRINGIZE(ROOT_DIR_XENGINE), "/Cache")
static const std::filesystem::path ShaderASMPath(CACHE_PATH);

//static const std::filesystem::path ShaderASMPath("E:\\XEngine\\XEnigine\\Cache");
//https://docs.microsoft.com/en-us/windows/win32/direct3dtools/dx-graphics-tools-fxc-syntax

#define CACHE_SHADER 0
class XD3DInclude : public ID3DInclude
{
public:
	std::map<std::string, std::string>* IncludePathToCode;
	bool HasIncludePath;
	HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
	{
		std::string FileNameStr = std::string(pFileName);
		auto iter = IncludePathToCode->find(FileNameStr);
		if (iter != IncludePathToCode->end())
		{
			*ppData = iter->second.data();
			*pBytes = iter->second.size();

			if (iter->second.data()[0] == '*')
			{
				FileNameStr = iter->second.substr(1, iter->second.size() - 1);
			}
			else
			{
				HasIncludePath = true;
				return S_OK;
			}
		}

		std::string FilePath = GLOBAL_SHADER_PATH + FileNameStr;
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
		default:XASSERT(false); break;
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
			
			if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE || ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_STRUCTURED)
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
			else if (
				ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWTYPED || 
				ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_APPEND_STRUCTURED || 
				ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER)
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
				XASSERT(false);
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





#include "dxc\dxcapi.h"

/*class XDXCInclude : public IDxcIncludeHandler
{
public:
	std::map<std::string, std::string>* IncludePathToCode;
	bool HasIncludePath;

	HRESULT STDMETHODCALLTYPE LoadSource(
		_In_z_ LPCWSTR pFilename,                                 // Candidate filename.
		_COM_Outptr_result_maybenull_ IDxcBlob** ppIncludeSource  // Resultant source object for included file, nullptr if not found.
	)override
	{
		std::string FileNameStr = std::string(pFilename);
		auto iter = IncludePathToCode->find(FileNameStr);
		if (iter != IncludePathToCode->end())
		{
			*ppData = iter->second.data();
			*pBytes = iter->second.size();

			if (iter->second.data()[0] == '*')
			{
				FileNameStr = iter->second.substr(1, iter->second.size() - 1);
			}
			else
			{
				HasIncludePath = true;
				return S_OK;
			}
		}

		std::string FilePath = GLOBAL_SHADER_PATH + FileNameStr;
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
};

static void CompileDX12ShaderDXC(XShaderCompileInput& Input, XShaderCompileOutput& Output)
{
	XDxRefCount<IDxcUtils> pUtils;
	XDxRefCount<IDxcCompiler3> pCompiler;
	DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&pUtils));
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&pCompiler));

	XDxRefCount<IDxcIncludeHandler> pIncludeHandler;
	pUtils->CreateDefaultIncludeHandler(&pIncludeHandler);


	LPCWSTR pszArgs[] =
	{
		filename.c_str(),            // Optional shader source file name for error reporting and for PIX shader source view.  
		L"-E", entryPoint.c_str(),              // Entry point.
		L"-T", pTarget.c_str(),            // Target.
		L"-Zi",                      // Enable debug information.
		L"-D", L"MYDEFINE=1",        // A single define.
		L"-Fo", L"myshader.bin",     // Optional. Stored in the pdb. 
		L"-Fd", L"myshader.pdb",     // The file name of the pdb. This must either be supplied or the autogenerated file name must be used.
		L"-Qstrip_reflect",          // Strip reflection into a separate blob. 
	};


	XDxRefCount<IDxcBlobEncoding> pSource = nullptr;
	pUtils->LoadFile(filename.c_str(), nullptr, &pSource);
	DxcBuffer Source;
	Source.Ptr = pSource->GetBufferPointer();
	Source.Size = pSource->GetBufferSize();
	Source.Encoding = DXC_CP_ACP; // Assume BOM says UTF8 or UTF16 or this is ANSI text.

	//
	// Compile it with specified arguments.
	//
	XDxRefCount<IDxcResult> pResults;
	pCompiler->Compile(
		&Source,                // Source buffer.
		pszArgs,                // Array of pointers to arguments.
		_countof(pszArgs),      // Number of arguments.
		pIncludeHandler.Get(),        // User-provided interface to handle #include directives (optional).
		IID_PPV_ARGS(&pResults) // Compiler output status, buffer, and errors.
	);

	//
	// Print errors if present.
	//
	XDxRefCount<IDxcBlobUtf8> pErrors = nullptr;
	pResults->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&pErrors), nullptr);
	// Note that d3dcompiler would return null if no errors or warnings are present.  
	// IDxcCompiler3::Compile will always return an error buffer, but its length will be zero if there are no warnings or errors.
	if (pErrors != nullptr && pErrors->GetStringLength() != 0)
		wprintf(L"Warnings and Errors:\n%S\n", pErrors->GetStringPointer());

	//
	// Quit if the compilation failed.
	//
	HRESULT hrStatus;
	pResults->GetStatus(&hrStatus);
	if (FAILED(hrStatus))
	{
		wprintf(L"Compilation Failed\n");
		return false;
	}

	//
	// Save shader binary.
	//
	XDxRefCount<IDxcBlob> pShader = nullptr;
	XDxRefCount<IDxcBlobUtf16> pShaderName = nullptr;
	pResults->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&pShader), &pShaderName);
	if (pShader != nullptr)
	{
		// create pipeline state object 
	}

	XDxRefCount< ID3D12ShaderReflection > pReflection;

	XDxRefCount<IDxcBlob> pReflectionData;
	pResults->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&pReflectionData), nullptr);
	if (pReflectionData != nullptr)
	{
		// Optionally, save reflection blob for later here.

		// Create reflection interface.
		DxcBuffer ReflectionData;
		ReflectionData.Encoding = DXC_CP_ACP;
		ReflectionData.Ptr = pReflectionData->GetBufferPointer();
		ReflectionData.Size = pReflectionData->GetBufferSize();

		pUtils->CreateReflection(&ReflectionData, IID_PPV_ARGS(&pReflection));

		// Use reflection interface here.

	}


	D3D12_SHADER_DESC shader_desc;
	pReflection->GetDesc(&shader_desc);

	for (int i = 0; i < shader_desc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC  resource_desc;
		pReflection->GetResourceBindingDesc(i, &resource_desc);


		auto shaderVarName = resource_desc.Name;
		auto registerSpace = resource_desc.Space;
		auto resourceType = resource_desc.Type;
		auto bindPoint = resource_desc.BindPoint;

		std::cout << "var name is " << shaderVarName << std::endl;
		std::cout << "type name is " << resourceType << std::endl;
		std::cout << "bind point is " << bindPoint << std::endl;
		std::cout << "register space is " << registerSpace << std::endl;

	}

}*/















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