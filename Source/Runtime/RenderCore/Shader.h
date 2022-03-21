#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/HAL/Mch.h"

#include <fstream>
#include <vector>
#include "Runtime/D3D12RHI/d3dx12.h"
#include <d3dcompiler.h>
#include "Runtime/RHI/RHIResource.h"


/////////////////////
#include "Runtime/RenderCore/RenderResource.h"
#include <list>
#include <unordered_map>

/////////////////////

//temp name
class XXShader
{
private:
	int32 IndexToShaderMapStoreRHIShaders;
};

#pragma region ShaderMap

//FShaderMapResource
class XShaderMapStoreRHIShadersInFileUnit : public XRenderResource
{
protected:
	XRHIShader* CreateRHIShaderFromCode(int32 ShaderIndex);
	std::vector<std::shared_ptr<XRHIShader>>RHIShaders;//https://stackoverflow.com/questions/41578021/create-a-vector-of-shared-ptr-to-ints
};

//FShaderMapResourceCode
class XShaderMapStoreCodesInFileUnit
{
public:
	struct XShaderEntry
	{
		std::vector<uint8>Code;
		EShaderType Shadertype;
	};
	std::vector<XShaderEntry>ShaderEntries;
};

//FShaderMapContent
class XShaderMapStoreShadersInfoInFileUnit
{
public:
	XXShader* FindOrAddShader(const std::size_t HashedIndex, XXShader* Shader, int32 PermutationId = 0);
	XXShader* GetShader(const std::size_t HashedIndex, int32 PermutationId = 0)const;
	
private:
	std::vector<XXShader*>ShaderPtrArray;
	std::unordered_map<std::size_t, std::size_t>MapFromHashedEntryIndexToShaderPtrArrayIndex;
};

class XShaderMapBase
{
public:
	inline void AssignShadersInfo(XShaderMapStoreShadersInfoInFileUnit* InShadersInfo)
	{
		ShadersInfosStored = InShadersInfo;
	}
	inline XShaderMapStoreShadersInfoInFileUnit* GetShaderInfo()const { return ShadersInfosStored; }
private:
	std::shared_ptr<XShaderMapStoreRHIShadersInFileUnit>RHIShadersStored;//Raw ptr ???
	std::shared_ptr<XShaderMapStoreCodesInFileUnit>CodesStored;
	XShaderMapStoreShadersInfoInFileUnit* ShadersInfosStored;//Stored In XGlobalShaderMapInProjectUnit
};

template<typename ShadersInfotype>
class TShaderMap: public XShaderMapBase
{
public:
};
#pragma endregion ShaderMap

//class FShaderType
/** An object which is used to serialize/deserialize, compile, and cache a particular shader class. */
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
private:
	std::size_t HashedFileIndex;
	std::size_t HashedEntryIndex;

	EShaderTypeForDynamicCast CastType;
	const char* ShaderName;
	const wchar_t* SourceFileName;
	const char* EntryName;
	EShaderType ShaderType;
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



