#pragma once
#include "Runtime/RHI/RHIResource.h"
#include "Runtime/HAL/Mch.h"
#include <d3d12.h>
class XD3D12RootSignature;
class XD3D12PSOStoreID3DPSO
{
public:
	XD3D12PSOStoreID3DPSO() {};
	
	inline ID3D12PipelineState** GetID3DPSO_Address()
	{
		return ID3DPSO.GetAddressOf();
	}
	inline ID3D12PipelineState* GetID3DPSO()const
	{
		return ID3DPSO.Get();
	}
private:
	XDxRefCount<ID3D12PipelineState>ID3DPSO;
};

class XD3D12ComputeShader;
class XD3DComputePSO :public XRHIComputePSO
{
public:
	explicit XD3DComputePSO(
		const XD3D12ComputeShader* InComputeShader,
		XD3D12PSOStoreID3DPSO* InXID3DPSO
	) :
		ComputeShader(InComputeShader),
		XID3DPSO(InXID3DPSO) {}
	
	const XD3D12ComputeShader* ComputeShader;
	XD3D12PSOStoreID3DPSO* XID3DPSO;
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