#include "D3D12PlatformRHI.h"
#include "D3D12Shader.h"
#include <functional>
#include <unordered_map>
struct XD3D12VertexDeclarationKey
{
	D3DVertexLayoutArray VertexLayoutArray;
	std::size_t Hash;
	explicit XD3D12VertexDeclarationKey(const XRHIVertexLayoutArray& Elements)
	{
		VertexLayoutArray.resize(Elements.size());
		for (int i = 0; i < Elements.size(); i++)
		{
			const XVertexElement& Element = Elements[i];
			VertexLayoutArray[i].SemanticName = "ATTRIBUTE";
			VertexLayoutArray[i].SemanticIndex = Element.SemanticIndex;
			
			switch (Element.Format)
			{
			case EVertexElementType::VET_Float1:		VertexLayoutArray[i].Format = DXGI_FORMAT_R32_FLOAT; break;
			case EVertexElementType::VET_Float2:		VertexLayoutArray[i].Format = DXGI_FORMAT_R32G32_FLOAT; break;
			case EVertexElementType::VET_Float3:		VertexLayoutArray[i].Format = DXGI_FORMAT_R32G32B32_FLOAT; break;
			case EVertexElementType::VET_Float4:		VertexLayoutArray[i].Format = DXGI_FORMAT_R32G32B32A32_FLOAT; break;
			case EVertexElementType::VET_PackedNormal:	VertexLayoutArray[i].Format = DXGI_FORMAT_R8G8B8A8_SNORM; break;
			default:X_Assert(false);
			}
			
			VertexLayoutArray[i].InputSlot = Element.InputSlot;
			VertexLayoutArray[i].AlignedByteOffset = Element.AlignedByteOffset;
			VertexLayoutArray[i].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			VertexLayoutArray[i].InstanceDataStepRate = 0;
		}
		Hash = std::hash<std::string>{}((char*)VertexLayoutArray.data());
	}
};

struct XVertexDeclarationCache
{
	std::shared_ptr<XRHIVertexLayout> FindOrAdd(const XD3D12VertexDeclarationKey& Key)
	{
		auto iter = MapHashToLayout.find(Key.Hash);
		if (iter == MapHashToLayout.end())
		{
			MapHashToLayout[Key.Hash] = std::make_shared<XD3D12VertexLayout>(Key.VertexLayoutArray);
			return MapHashToLayout[Key.Hash];
		}
		return iter->second;
	}
	std::unordered_map<std::size_t, std::shared_ptr<XRHIVertexLayout>>MapHashToLayout;
};

XVertexDeclarationCache GVertexLayoutCache;

std::shared_ptr<XRHIVertexLayout> XD3D12PlatformRHI::RHICreateVertexDeclaration(const XRHIVertexLayoutArray& Elements)
{
	XD3D12VertexDeclarationKey Key(Elements);
	return GVertexLayoutCache.FindOrAdd(Key);
}