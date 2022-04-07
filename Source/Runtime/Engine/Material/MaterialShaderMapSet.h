#pragma once
#include <vector>
class XShaderInfo;
struct XMaterialShaderParameters_ForIndex;
class XMaterialShaderMapSet
{
public:
	std::vector<XShaderInfo*>ShaderInfos;
};

const XMaterialShaderMapSet& GetMaterialShaderMapSet(const XMaterialShaderParameters_ForIndex& MaterialParameters);