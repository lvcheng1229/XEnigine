#pragma once
#include "Runtime/RHI/RHIResource.h"
#include "D3D12Rootsignature.h"
class XD3D12PSOStoreID3DPSO
{
private:
	XDxRefCount<ID3D12PipelineState>ID3DPSO;
};

class XD3DGraphicsPSO : public XRHIGraphicsPSO
{
public:
	explicit XD3DGraphicsPSO(
		const XGraphicsPSOInitializer& Initializer,
		const XD3D12RootSignature* InRootSignature,
		XD3D12PSOStoreID3DPSO* InPipelineState) :
		GraphicsPSOInitializer(Initializer),
		RootSig(InRootSignature),
		XID3DPSO	(InPipelineState) {}

	XGraphicsPSOInitializer GraphicsPSOInitializer;
	const XD3D12RootSignature* RootSig;
	XD3D12PSOStoreID3DPSO* XID3DPSO;
};