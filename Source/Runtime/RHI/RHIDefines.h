#pragma once
enum class EShaderType
{
	SV_Vertex = 0,
	SV_Pixel,
	SV_Compute,
	SV_ShaderCount
};

enum ETextureCreateFlags
{
	TexCreate_None = 0,
	TexCreate_RenderTargetable = 1 << 0,
	TexCreate_DepthStencilTargetable = 1 << 1,
	TexCreate_ShaderResource = 1 << 2,
	TexCreate_SRGB = 1 << 3,
	TexCreate_UAV = 1 << 4,
};


enum class EBlendOperation
{
	BO_Add,
};

enum class EBlendFactor
{
	BF_Zero,
	BF_One,
};

enum class ECompareFunction
{
	CF_Greater,
	CF_Always,
};

enum class ERenderTargetLoadAction :uint8
{
	ENoAction,
	ELoad,
	EClear,
};

enum class EDepthStencilLoadAction :uint8
{
	ENoAction,
	ELoad,
	EClear,
};

enum class EVertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_PackedNormal,	// FPackedNormal
	VET_MAX,
	VET_NumBits = 5,
};