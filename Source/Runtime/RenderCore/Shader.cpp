#include "Shader.h"
#include "Runtime/RHI/RHICommandList.h"

<<<<<<< HEAD
std::shared_ptr<XRHIShader> XShaderMapStoreRHIShaders_InlineCode::CreateRHIShaderFromCode(int32 ShaderIndex)
{
	const XShaderMapStoreCodes::XShaderEntry& ShaderEntry = Code->ShaderEntries[ShaderIndex];
	
	const EShaderType ShaderType = ShaderEntry.Shadertype;
	std::shared_ptr<XRHIShader> RHIShaderRef;
	XArrayView<uint8> CodeView(ShaderEntry.Code.data(), ShaderEntry.Code.size());
	switch (ShaderType)
	{
	case EShaderType::SV_Vertex:RHIShaderRef = std::static_pointer_cast<XRHIShader>(RHICreateVertexShader(CodeView)); break;
	case EShaderType::SV_Pixel:RHIShaderRef = std::static_pointer_cast<XRHIShader>(RHICreatePixelShader(CodeView)); break;
	default:X_Assert(false); break;
	}
	return RHIShaderRef;
}


XShaderMapBase::~XShaderMapBase()
{
	delete XShadersStored;
}


void XShaderMapStoreCodes::AddShaderCompilerOutput(XShaderCompileOutput& OutputInfo)
{
	XShaderEntry ShaderEntry;
	ShaderEntry.Code = OutputInfo.ShaderCode;
	ShaderEntry.Shadertype = OutputInfo.Shadertype;
	ShaderEntries.push_back(std::move(ShaderEntry));

	MapFCodeHashToEntryIndex[OutputInfo.SourceCodeHash] = ShaderEntries.size() - 1;
}

XXShader* XShaderMapStoreXShaders::FindOrAddXShader(const std::size_t HashedEntryIndex, XXShader* Shader, int32 PermutationId)
{
	const std::size_t Index = HashedEntryIndex;
	auto iter = MapHashedEntryIndexToXShaderIndex.find(Index);
	if (iter != MapHashedEntryIndexToXShaderIndex.end())
	{
		return ShaderPtrArray[iter->second].get();
	}
	ShaderPtrArray.push_back(std::shared_ptr<XXShader>(Shader));
	MapHashedEntryIndexToXShaderIndex[Index] = ShaderPtrArray.size() - 1;
	return ShaderPtrArray.back().get();
}

XXShader* XShaderMapStoreXShaders::GetXShader(const std::size_t HashedEntryIndex, int32 PermutationId) const
{
	auto iter = MapHashedEntryIndexToXShaderIndex.find(HashedEntryIndex);
	X_Assert(iter != MapHashedEntryIndexToXShaderIndex.end());
	std::size_t ShaderPtrArrayIndex = iter->second;
	return ShaderPtrArray[ShaderPtrArrayIndex].get();
}




XShaderInfos::XShaderInfos(
=======
//std::shared_ptr<XRHIShader> XShaderMapStoreRHIShaders_InlineCode::CreateRHIShaderFromCode(int32 ShaderIndex)
//{
//	const XShaderMapStoreCodesInFileUnit::XShaderEntry& ShaderEntry = Code->ShaderEntries[ShaderIndex];
//	
//	const EShaderType ShaderType = ShaderEntry.Shadertype;
//	std::shared_ptr<XRHIShader> RHIShaderRef;
//	std::string_view CodeView((char*)ShaderEntry.Code.data());
//	
//	switch (ShaderType)
//	{
//	case EShaderType::SV_Vertex:RHIShaderRef = RHICreateVertexShader(CodeView); break;
//	case EShaderType::SV_Pixel:RHIShaderRef = RHICreatePixelShader(CodeView); break;
//	default:X_Assert(false); break;
//	}
//	return RHIShaderRef;
//}
//

//
//
//void XShaderMapStoreCodesInFileUnit::AddShaderCompilerOutput(XShaderCompileOutput& OutputInfo)
//{
//	XShaderEntry ShaderEntry;
//	ShaderEntry.Code = OutputInfo.ShaderCode;
//	ShaderEntry.Shadertype = OutputInfo.Shadertype;
//	ShaderEntries.push_back(std::move(ShaderEntry));
//
//	MapFromCodeHashToEntry[OutputInfo.SourceCodeHash] = ShaderEntries.size() - 1;
//}
//
//XXShader* XShaderMapStoreShadersInfoInFileUnit::FindOrAddShader(const std::size_t HashedEntryIndex, XXShader* Shader, int32 PermutationId)
//{
//	const std::size_t Index = HashedEntryIndex;
//	auto iter = MapFromHashedEntryIndexToShaderPtrArrayIndex.find(Index);
//	if (iter != MapFromHashedEntryIndexToShaderPtrArrayIndex.end())
//	{
//		return ShaderPtrArray[iter->second].get();
//	}
//	ShaderPtrArray.push_back(std::make_shared<XXShader>(Shader));
//	MapFromHashedEntryIndexToShaderPtrArrayIndex[Index] = ShaderPtrArray.size() - 1;
//	return ShaderPtrArray.back().get();
//}
//
//XXShader* XShaderMapStoreShadersInfoInFileUnit::GetShader(const std::size_t HashedEntryIndex, int32 PermutationId) const
//{
//	auto iter = MapFromHashedEntryIndexToShaderPtrArrayIndex.find(HashedEntryIndex);
//	X_Assert(iter != MapFromHashedEntryIndexToShaderPtrArrayIndex.end());
//	std::size_t ShaderPtrArrayIndex = iter->second;
//	return ShaderPtrArray[ShaderPtrArrayIndex].get();
//}




XShaderInfosUsedToCompile::XShaderInfosUsedToCompile(
>>>>>>> parent of 13cbd5a (~~~)
	EShaderTypeForDynamicCast InCastType,
	const char* InShaderName,
	const wchar_t* InSourceFileName,
	const char* InEntryName,
	EShaderType InShaderType) :
	CastType(InCastType),
	ShaderName(InShaderName),
	SourceFileName(InSourceFileName),
	EntryName(InEntryName),
	ShaderType(InShaderType)
{
	GetShaderInfosUsedToCompile_LinkedList().push_back(this);
	HashedFileIndex = std::hash<std::wstring>{}(InSourceFileName);
	HashedEntryIndex = std::hash<std::string>{}(InEntryName);
}

std::list<XShaderInfosUsedToCompile*>& XShaderInfosUsedToCompile::GetShaderInfosUsedToCompile_LinkedList()
{
	static std::list<XShaderInfosUsedToCompile*> GloablShaderInfosUsedToCompile_LinkedList;
	return GloablShaderInfosUsedToCompile_LinkedList;
}




















void XShader::CreateShader(EShaderType shader_type)
{
	if (shader_type == EShaderType::SV_Compute)
	{
		RHIComputeShader = std::make_shared<XRHIComputeShader>(shader_type);
	}
	else
	{
		RHIShader = std::make_shared<XRHIGraphicsShader>(shader_type);
	}
	ShaderType = shader_type;
}

void XShader::CompileShader(const std::wstring& filename, const D3D_SHADER_MACRO* defines, const std::string& entrypoint, const std::string& target)
{
	UINT compileFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)  
	compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	
	XDxRefCount<ID3DBlob> errors;
	HRESULT hr = D3DCompileFromFile(filename.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entrypoint.c_str(), target.c_str(), compileFlags, 0, &byteCode, &errors);

	if (errors != nullptr)
		OutputDebugStringA((char*)errors->GetBufferPointer());

	ThrowIfFailed(hr);
}



void XShader::ShaderReflect()
{
	ID3D12ShaderReflection* Reflection = NULL;
	D3DReflect(byteCode->GetBufferPointer(), byteCode->GetBufferSize(), IID_PPV_ARGS(&Reflection));

	D3D12_SHADER_DESC ShaderDesc;
	Reflection->GetDesc(&ShaderDesc);

	for (uint32 i = 0; i < ShaderDesc.BoundResources; i++)
	{
		D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
		Reflection->GetResourceBindingDesc(i, &ResourceDesc);

		//LPCSTR ShaderVarName = ResourceDesc.Name;
		D3D_SHADER_INPUT_TYPE ResourceType = ResourceDesc.Type;
		//UINT RegisterSpace = ResourceDesc.Space;
		//UINT BindPoint = ResourceDesc.BindPoint;
		//UINT BindCount = ResourceDesc.BindCount;

		//D3D_SIT_CBUFFER = 0,
		//D3D_SIT_TBUFFER = (D3D_SIT_CBUFFER + 1),
		//D3D_SIT_TEXTURE = (D3D_SIT_TBUFFER + 1),
		//D3D_SIT_SAMPLER = (D3D_SIT_TEXTURE + 1),
		//D3D_SIT_UAV_RWTYPED = (D3D_SIT_SAMPLER + 1),
		//D3D_SIT_STRUCTURED = (D3D_SIT_UAV_RWTYPED + 1),
		//D3D_SIT_UAV_RWSTRUCTURED = (D3D_SIT_STRUCTURED + 1),
		//D3D_SIT_BYTEADDRESS = (D3D_SIT_UAV_RWSTRUCTURED + 1),
		//D3D_SIT_UAV_RWBYTEADDRESS = (D3D_SIT_BYTEADDRESS + 1),
		//D3D_SIT_UAV_APPEND_STRUCTURED = (D3D_SIT_UAV_RWBYTEADDRESS + 1),
		//D3D_SIT_UAV_CONSUME_STRUCTURED = (D3D_SIT_UAV_APPEND_STRUCTURED + 1),
		//D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER = (D3D_SIT_UAV_CONSUME_STRUCTURED + 1),
		//D3D_SIT_RTACCELERATIONSTRUCTURE = (D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER + 1),
		//D3D_SIT_UAV_FEEDBACKTEXTURE = (D3D_SIT_RTACCELERATIONSTRUCTURE + 1),

		if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
		{
			srv_count++;
			//XShaderSRVParameterInfo info;
			//info.name = ShaderVarName;
			//info.shader_type = shader_type;
			//info.base_register_index = BindPoint;
			//info.num = BindCount;
			//info.register_spacee = RegisterSpace;
			//
			//srv_paramter_infos.push_back(info);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
		{
			cbv_count++;
			//XShaderCBVParameterInfo info;
			//info.name = ShaderVarName;
			//info.shader_type = shader_type;
			//info.base_register_index = BindPoint;
			//info.register_spacee = RegisterSpace;
			//
			//cbv_paramter_infos.push_back(info);
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D10_SIT_SAMPLER)
		{
			sampler_count++;
		}
		else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D11_SIT_UAV_RWTYPED)
		{
			uav_count++;
		}
		else
		{
			X_Assert(false);
		}

	}
}


