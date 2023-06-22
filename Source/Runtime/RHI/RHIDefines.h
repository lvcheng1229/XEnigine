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
	BF_SourceAlpha,
	BF_InverseSourceAlpha,
};

enum class EResourceLockMode
{
	RLM_ReadOnly,
	RLM_WriteOnly,
	RLM_Num
};

enum class ECompareFunction
{
	CF_Greater,
	CF_Less,
	CF_GreaterEqual,
	CF_Always,
};

enum class EFaceCullMode
{
	FC_Back,
	FC_Front,
	FC_None,
};



enum class ERenderTargetLoadAction
{
	ENoAction,
	ELoad,
	EClear,
};

enum class EDepthStencilLoadAction
{
	ENoAction,
	ELoad,
	EClear,
};

enum class ERenderTargetStoreAction
{
	ENoAction,
	EStore,
};

enum class EVertexElementType
{
	VET_None,
	VET_Float1,
	VET_Float2,
	VET_Float3,
	VET_Float4,
	VET_UINT16,
	VET_UINT32,
	VET_Color,
	VET_PackedNormal,	// FPackedNormal
	VET_MAX,
	VET_NumBits = 5,
};

enum class EBufferUsage
{
	BUF_None = 0x0000,
	BUF_Static = 0x0001,
	BUF_Dynamic = 0x0002,
	BUF_Vertex = 0x0002,
	BUF_Index = 0x0002,
	BUF_UnorderedAccess = 0x0008,
	BUF_DrawIndirect = 0x0100,
	BUF_ShaderResource = 0x0200,
	BUF_StructuredBuffer = 0x40000,
	BUF_AnyDynamic = BUF_Dynamic,
};

#define MaxSimultaneousRenderTargets 8