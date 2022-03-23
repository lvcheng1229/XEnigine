#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/HAL/Mch.h"

#include <fstream>


#include "Runtime/D3D12RHI/d3dx12.h"
#include <d3dcompiler.h>
#include "Runtime/RHI/RHIResource.h"


/////////////////////
#include "Runtime/RenderCore/RenderResource.h"
#include <list>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Runtime/Engine/ShaderCompiler/ShaderCompiler.h"
/////////////////////

class XXShader;

#pragma region ShaderMap



class XShaderMapStoreRHIShaders //: public XRenderResource
{
public:
<<<<<<< HEAD
	XShaderMapStoreRHIShaders(std::size_t NumShaders)
	{
		RHIShaders.resize(NumShaders);
	}
	inline XRHIShader* GetRHIShader(int32 ShaderIndex)
	{
		if (RHIShaders[ShaderIndex].get() == nullptr)
		{
			RHIShaders[ShaderIndex] = CreateRHIShaderFromCode(ShaderIndex);
		}
		return RHIShaders[ShaderIndex].get();
	}
=======
	//XShaderMapStoreRHIShadersInFileUnit(std::size_t NumShaders)
	//{
	//	RHIShaders.resize(NumShaders,std::shared_ptr<XRHIShader>(nullptr));
	//}
	//inline XRHIShader* GetRHIShader(int32 ShaderIndex)
	//{
	//	if (RHIShaders[ShaderIndex].get() == nullptr)
	//	{
	//		RHIShaders[ShaderIndex] = CreateRHIShaderFromCode(ShaderIndex);
	//	}
	//	return RHIShaders[ShaderIndex].get();
	//	return nullptr;
	//}
>>>>>>> parent of 13cbd5a (~~~)
protected:
	virtual std::shared_ptr<XRHIShader> CreateRHIShaderFromCode(int32 ShaderIndex) = 0;
	std::vector<std::shared_ptr<XRHIShader>>RHIShaders;
};

class XShaderMapStoreCodes
{
public:
<<<<<<< HEAD
=======
	//inline std::size_t GetEntryIndexByCodeHash(std::size_t CodeHash)
	//{
	//	return MapFromCodeHashToEntry[CodeHash];
	//}
	//void AddShaderCompilerOutput(XShaderCompileOutput& OutputInfo);
>>>>>>> parent of 13cbd5a (~~~)
	struct XShaderEntry
	{
		std::vector<uint8>Code;
		EShaderType Shadertype;
	};

	void AddShaderCompilerOutput(XShaderCompileOutput& OutputInfo);

	inline std::size_t GetEntryIndexByCodeHash(std::size_t CodeHash)
	{
		return MapFCodeHashToEntryIndex[CodeHash];
	}

	std::vector<XShaderEntry>ShaderEntries;
	std::unordered_map<std::size_t, std::size_t>MapFCodeHashToEntryIndex;
};

class XShaderMapStoreRHIShaders_InlineCode :public XShaderMapStoreRHIShaders
{
public:
<<<<<<< HEAD
	XShaderMapStoreRHIShaders_InlineCode(XShaderMapStoreCodes* InCode)
		: XShaderMapStoreRHIShaders(InCode->ShaderEntries.size()),
		Code(InCode) {}

	std::shared_ptr<XRHIShader> CreateRHIShaderFromCode(int32 ShaderIndex)override;
	XShaderMapStoreCodes* Code;//Stored In ShaderMapBase
=======
	//XShaderMapStoreRHIShaders_InlineCode(XShaderMapStoreCodesInFileUnit* InCode)
	//	: XShaderMapStoreRHIShadersInFileUnit(InCode->ShaderEntries.size())
	//{
	//	Code = std::make_shared<XShaderMapStoreCodesInFileUnit>(InCode);
	//}
	//std::shared_ptr<XRHIShader> CreateRHIShaderFromCode(int32 ShaderIndex)override;
	//std::shared_ptr<XShaderMapStoreCodesInFileUnit>Code;
>>>>>>> parent of 13cbd5a (~~~)
};

class XShaderMapStoreXShaders
{
public:
<<<<<<< HEAD
	XXShader* FindOrAddXShader(const std::size_t HashedIndex, XXShader* Shader, int32 PermutationId = 0);
	XXShader* GetXShader(const std::size_t HashedEntryIndex, int32 PermutationId = 0)const;
=======
	//XXShader* FindOrAddShader(const std::size_t HashedIndex, XXShader* Shader, int32 PermutationId = 0);
	//XXShader* GetShader(const std::size_t HashedEntryIndex, int32 PermutationId = 0)const;
	
>>>>>>> parent of 13cbd5a (~~~)
private:
	std::vector<std::shared_ptr<XXShader>>ShaderPtrArray;
	std::unordered_map<std::size_t, std::size_t>MapHashedEntryIndexToXShaderIndex;
};

class XShaderMapBase
{
public:
<<<<<<< HEAD
	~XShaderMapBase();

	inline void AssignShadersInfo(XShaderMapStoreXShaders* InXShadersStored)
=======
	inline void AssignShadersInfo(XShaderMapStoreShadersInfoInFileUnit* InShadersInfo)
>>>>>>> parent of 13cbd5a (~~~)
	{
		XShadersStored = InXShadersStored;
	}
<<<<<<< HEAD

	inline void InitRHIShaders_InlineCode()
	{
		RHIShadersStored.reset();
		if (CodesStored.get() != nullptr)
		{
			XShaderMapStoreRHIShaders_InlineCode* RHIShader = new XShaderMapStoreRHIShaders_InlineCode(CodesStored.get());
			RHIShadersStored = std::shared_ptr<XShaderMapStoreRHIShaders>
				(static_cast<XShaderMapStoreRHIShaders*>(RHIShader));
		}
	}
	
	inline XShaderMapStoreRHIShaders* GetShaderMapStoreRHIShaders() const
	{ 
		return RHIShadersStored.get();
	}

	inline XShaderMapStoreXShaders* GetShaderMapStoreXShaders()const 
	{ 
		return XShadersStored;
	}

	inline XShaderMapStoreCodes* GetShaderMapStoreCodes()
	{ 
		if (CodesStored.get() == nullptr)
		{
			CodesStored = std::make_shared<XShaderMapStoreCodes>();
		}
		return CodesStored.get();
	}

private:
	std::shared_ptr<XShaderMapStoreRHIShaders>RHIShadersStored;
	std::shared_ptr<XShaderMapStoreCodes>CodesStored;
	XShaderMapStoreXShaders* XShadersStored;//Stored In XGlobalShaderMapInProjectUnit
=======
	//inline void InitRHIShaders_InlineCode()
	//{
	//	XShaderMapStoreRHIShaders_InlineCode* RHICode = new XShaderMapStoreRHIShaders_InlineCode(CodesStored.get());
	//	RHIShadersStored = std::shared_ptr<XShaderMapStoreRHIShadersInFileUnit>(RHICode);
	//}
	//
	//inline XShaderMapStoreRHIShadersInFileUnit* GetShaderMapRHIShaders() const{ return RHIShadersStored.get(); }
	//inline XShaderMapStoreShadersInfoInFileUnit* GetShaderInfo()const { return ShadersInfosStored; }
	//inline XShaderMapStoreCodesInFileUnit* GetResourceCode()
	//{ 
	//	if (CodesStored.get() == nullptr)
	//	{
	//		CodesStored = std::make_shared<XShaderMapStoreCodesInFileUnit>(new XShaderMapStoreCodesInFileUnit());
	//	}
	//	return CodesStored.get();
	//}
private:
	std::shared_ptr<XShaderMapStoreRHIShadersInFileUnit>RHIShadersStored;//Raw ptr ???
	std::shared_ptr<XShaderMapStoreCodesInFileUnit>CodesStored;
	XShaderMapStoreShadersInfoInFileUnit* ShadersInfosStored;//Stored In XGlobalShaderMapInProjectUnit
>>>>>>> parent of 13cbd5a (~~~)
};

template<typename ShadersInfotype>
class TShaderMap: public XShaderMapBase
{
public:
};
//
//
//#pragma endregion ShaderMap
//
///** An object which is used to serialize/deserialize, compile, and cache a particular shader class. */
//
////class FShaderType
class XShaderInfosUsedToCompile
{
public:
	enum class EShaderTypeForDynamicCast
	{
		Global,
		Material,
		MeshMaterial
	};

	XShaderInfosUsedToCompile(
		EShaderTypeForDynamicCast InCastType,
		const char* InShaderName,
		const wchar_t* InSourceFileName,
		const char* InEntryName,
		EShaderType InShaderType
		);
	static std::list<XShaderInfosUsedToCompile*>& GetShaderInfosUsedToCompile_LinkedList();

	inline const wchar_t* GetSourceFileName()const { return SourceFileName; }
	inline const char* GetEntryName()const { return EntryName; }
	inline std::size_t GetHashedFileIndex()const { return HashedFileIndex; }
	inline std::size_t GetHashedEntryIndex()const { return HashedEntryIndex; }
	inline EShaderType GetShaderType()const { return ShaderType; }
private:
	std::size_t HashedFileIndex;
	std::size_t HashedEntryIndex;

	EShaderTypeForDynamicCast CastType;
	const char* ShaderName;
	const wchar_t* SourceFileName;
	const char* EntryName;
	EShaderType ShaderType;
};



template<typename XXShaderClass>
class TShaderReference
{
public:
	TShaderReference() :ShaderPtr(nullptr), ShaderMapFileUnitPtr(nullptr) {}
	TShaderReference(XXShaderClass* InShaderPtr, const XShaderMapBase* InShaderMapFileUnitPtr)
		:ShaderPtr(InShaderPtr),
		ShaderMapFileUnitPtr(InShaderMapFileUnitPtr) {}

	template<typename OtherXXShaderClass>
	static TShaderReference<XXShaderClass> Cast(const TShaderReference<OtherXXShaderClass>& Rhs)
	{
		return TShaderReference<XXShaderClass>(static_cast<XXShaderClass*>(Rhs.GetShader()), Rhs.GetShaderMapFileUnit());
	}

	inline XXShaderClass* GetShader()const { return ShaderPtr; }
	inline const XShaderMapBase* GetShaderMapFileUnit()const { return ShaderMapFileUnitPtr; }
	
	inline XRHIShader* GetOrCreateRHIShaderFromMap(EShaderType ShaderType) const
	{
		return ShaderMapFileUnitPtr->GetShaderMapStoreRHIShaders()->GetRHIShader(ShaderPtr->GetRHIShaderIndex());
	}
	
	inline XRHIVertexShader* GetVertexShader() const
	{
		return static_cast<XRHIVertexShader*>(GetOrCreateRHIShaderFromMap(EShaderType::SV_Vertex));
	}

	inline XRHIPixelShader* GetPixelShader() const
	{
		return static_cast<XRHIPixelShader*>(GetOrCreateRHIShaderFromMap(EShaderType::SV_Pixel));
	}
private:
	XXShaderClass* ShaderPtr;
	const XShaderMapBase* ShaderMapFileUnitPtr;
};



////temp name
class XXShader
{
public:
	XXShader(XShaderInfosUsedToCompile* InShadersInfoPtr, XShaderCompileOutput* Output) :
		ShadersInfoPtr(InShadersInfoPtr),
		CodeHash(Output->SourceCodeHash),
		RHIShaderIndex(0) {}
	
	inline void SetRHIShaderIndex(std::size_t Index)
	{
		RHIShaderIndex = Index;
	}
	inline std::size_t GetCodeHash()const { return CodeHash; };
	inline std::size_t GetRHIShaderIndex()const { return RHIShaderIndex; };
private:
	XShaderInfosUsedToCompile* ShadersInfoPtr;
	std::size_t CodeHash;
	std::size_t RHIShaderIndex;
};



class XShader
{
private:
	uint16 srv_count;
	uint16 cbv_count;
	uint16 uav_count;
	uint16 sampler_count;
	XDxRefCount<ID3DBlob> byteCode;

	EShaderType ShaderType;
	std::shared_ptr<XRHIGraphicsShader>RHIShader = nullptr;
	std::shared_ptr<XRHIComputeShader>RHIComputeShader = nullptr;

public:
	XShader() :byteCode(nullptr), srv_count(0), cbv_count(0), uav_count(0), sampler_count(0) {};

	void CreateShader(EShaderType shader_type);
	void CompileShader(const std::wstring& filename,
		const D3D_SHADER_MACRO* defines,
		const std::string& entrypoint,
		const std::string& target);
	void ShaderReflect();
	inline XDxRefCount<ID3DBlob> GetByteCode() { return byteCode; }

	inline EShaderType GetShaderType() { return ShaderType; }
	inline std::shared_ptr<XRHIGraphicsShader> GetRHIGraphicsShader() { return RHIShader; }
	inline std::shared_ptr<XRHIComputeShader> GetRHIComputeShader() { return RHIComputeShader; }

	inline uint32 GetSRVCount() { return srv_count; }
	inline uint32 GetCBVCount() { return cbv_count; }
	inline uint32 GetUAVCount() { return uav_count; }
	inline uint32 GetSamplerCount() { return sampler_count; }
};



