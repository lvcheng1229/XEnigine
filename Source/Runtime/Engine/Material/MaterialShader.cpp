#include "MaterialShared.h"
#include "MaterialShaderMapSet.h"


void XMaterialShaderMapping_MatUnit::Compile(const XShaderCompileSetting& ShaderCompileSetting)
{
	//SubmitCompileJobs()
	{
		
		XMaterialShaderParameters_ForIndex MaterialParameters;
		//const XMaterialShaderParameters_ForIndex& MaterialParameters;

		//Temp
		{
			memset(&MaterialParameters, 0, sizeof(XMaterialShaderParameters_ForIndex));
			MaterialParameters.MaterialDomain = EMaterialDomain::Surface;
		}


		const XMaterialShaderMapSet& MapSet = GetMaterialShaderMapSet(MaterialParameters);
		for (auto ShaderInfo : MapSet.ShaderInfos)
		{
			XShaderCompileInput Input;
			Input.SourceFilePath = ShaderInfo->GetSourceFileName();
			Input.EntryPointName = ShaderInfo->GetEntryName();
			Input.Shadertype = ShaderInfo->GetShaderType();
			Input.ShaderName = ShaderInfo->GetShaderName();
			Input.CompileSettings = ShaderCompileSetting;
			
			XShaderCompileOutput Output;//TODO if(!GetContent().HashShader()), then Compile

			CompileMaterialShader(Input, Output);
		}
	}
}