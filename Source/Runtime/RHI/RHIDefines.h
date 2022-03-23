#pragma once
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