#include "CommonRenderRresource.h"
TGlobalResource<RFullScreenQuadVertexLayout> GFullScreenLayout;

RFullScreenQuadVS::ShaderInfos RFullScreenQuadVS::StaticShaderInfos(
	"RFullScreenQuadVS", L"E:/XEngine/XEnigine/Source/Shaders/FullScreenVertexShader.hlsl",
	"VS", EShaderType::SV_Vertex, RFullScreenQuadVS::CustomConstrucFunc);