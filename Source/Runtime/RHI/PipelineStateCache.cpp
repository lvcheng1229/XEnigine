#include "PipelineStateCache.h"
#include "Runtime/HAL/Mch.h"
#include <unordered_map>

void SetComputePipelineStateFromCS(XRHICommandList& RHICmdList, const XRHIComputeShader* CSShader)
{
	std::shared_ptr <XRHIComputePSO> RHIComputePSO = PipelineStateCache::GetAndOrCreateComputePipelineState(RHICmdList, CSShader);
	X_Assert(RHIComputePSO.get() != nullptr);
	RHICmdList.SetComputePipelineState(RHIComputePSO.get());

}

void SetGraphicsPipelineStateFromPSOInit(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& Initializer)
{
	std::shared_ptr <XRHIGraphicsPSO> RHIGraphicsPSO = PipelineStateCache::GetAndOrCreateGraphicsPipelineState(RHICmdList, Initializer);
	X_Assert(RHIGraphicsPSO.get() != nullptr);
	RHICmdList.SetGraphicsPipelineState(RHIGraphicsPSO.get());
}

class PipelineStateMap
{
public:
	PipelineStateMap() :size(0) {};
	bool find(std::size_t HashIndex, std::shared_ptr<XRHIGraphicsPSO>& PSORet)
	{
		X_Assert(size < 50);
		auto iter = HashIndexToPSOPtr.find(HashIndex);
		if (iter == HashIndexToPSOPtr.end())
		{
			return false;
		}
		PSORet = iter->second;
	}

	bool Add(std::size_t HashIndex, std::shared_ptr<XRHIGraphicsPSO> PSOPtr)
	{
		auto iter = HashIndexToPSOPtr.find(HashIndex);
		if (iter != HashIndexToPSOPtr.end())
			return false;
		HashIndexToPSOPtr[HashIndex] = PSOPtr;
		size++;
		return true;
	}

private:
	uint32 size;
	std::unordered_map<std::size_t, std::shared_ptr<XRHIGraphicsPSO>>HashIndexToPSOPtr;
};
static std::unordered_map<std::size_t, std::shared_ptr<XRHIComputePSO>>GComputePSOMap;
PipelineStateMap GPipelineStateMap;

namespace PipelineStateCache
{
	std::shared_ptr <XRHIComputePSO> GetAndOrCreateComputePipelineState(XRHICommandList& RHICmdList, const XRHIComputeShader* ComputeShader)
	{
		std::size_t HashIndex = ComputeShader->GetHash();
		auto iter = GComputePSOMap.find(HashIndex);
		if (iter == GComputePSOMap.end())
		{
			std::shared_ptr <XRHIComputePSO> ComputePSO = RHICreateComputePipelineState(ComputeShader);
			GComputePSOMap[HashIndex] = ComputePSO;
		}
		return GComputePSOMap[HashIndex];
	}

	std::shared_ptr <XRHIGraphicsPSO> GetAndOrCreateGraphicsPipelineState(XRHICommandList& RHICmdList, const XGraphicsPSOInitializer& OriginalInitializer)
	{
		const XGraphicsPSOInitializer* Initializer = &OriginalInitializer;
		std::size_t HashIndex = OriginalInitializer.GetHashIndex();
		std::shared_ptr<XRHIGraphicsPSO>PSOOut;
		bool bFound = GPipelineStateMap.find(HashIndex, PSOOut);
		if (bFound == false)
		{
			//FPipelineFileCache::CacheGraphicsPSO(GetTypeHash(*Initializer), *Initializer);
			PSOOut = RHICreateGraphicsPipelineState(OriginalInitializer);
			GPipelineStateMap.Add(HashIndex, PSOOut);
		}
		return PSOOut;
	}

	//std::shared_ptr<XRHIVertexLayout> GetOrCreateVertexDeclaration(const XRHIVertexLayoutArray& Elements)
	//{
	//
	//	//return std::shared_ptr<XRHIVertexLayout>();
	//}
}

