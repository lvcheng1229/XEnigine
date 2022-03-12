#include "Common.hlsl"


Texture2D SceneTexturesStruct_GBufferATexture;
Texture2D SceneTexturesStruct_GBufferBTexture;
Texture2D SceneTexturesStruct_GBufferCTexture;
Texture2D SceneTexturesStruct_GBufferDTexture;

Texture2D SceneTexturesStruct_SceneDepthTexture;

Texture2D LightAttenuationTexture;
struct FGBufferData
{
	float3 WorldNormal;
	float3 BaseColor;
	float Metallic;
	float Specular;
	float Roughness;
	uint ShadingModelID;

	// 0..1 (derived from BaseColor, Metalness, Specular)
	float3 DiffuseColor;
	float3 SpecularColor;
};

// GGX / Trowbridge-Reitz
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX( float Roughness, float NoH )
{
	float a = Roughness * Roughness;
	float a2 = a * a;
	float d = ( NoH * a2 - NoH ) * NoH + 1;	// 2 mad
	return a2 / ( PI*d*d );					// 4 mul, 1 rcp
}

// Appoximation of joint Smith term for GGX
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox( float Roughness, float NoV, float NoL )
{
	float a = Roughness*Roughness;
	float Vis_SmithV = NoL * ( NoV * ( 1 - a ) + a );
	float Vis_SmithL = NoV * ( NoL * ( 1 - a ) + a );
	// Note: will generate NaNs with Roughness = 0.  MinRoughness is used to prevent this
	return 0.5 * rcp( Vis_SmithV + Vis_SmithL );
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float3 F_Schlick( float3 SpecularColor, float VoH )
{
	float Fc = Pow5( 1 - VoH );					// 1 sub, 3 mul
	//return Fc + (1 - Fc) * SpecularColor;		// 1 add, 3 mad
	
	// Anything less than 2% is physically impossible and is instead considered to be shadowing
	return saturate( 50.0 * SpecularColor.g ) * Fc + (1 - Fc) * SpecularColor;
}

float3 Diffuse_Lambert( float3 DiffuseColor )
{
	return DiffuseColor * (1 / PI);
}

float3 StandardShading( float3 DiffuseColor, float3 SpecularColor, float Roughness, float3 L, float3 V, half3 N )
{
	float3 H = normalize(V + L);
	float NoL = saturate( dot(N, L) );
	float NoV = saturate( abs( dot(N, V) ) + 1e-5 );
	float NoH = saturate( dot(N, H) );
	float VoH = saturate( dot(V, H) );

	// Generalized microfacet specular
	float D = D_GGX(Roughness, NoH ) * 1.0f;
	float Vis = Vis_SmithJointApprox(Roughness, NoV, NoL );
	float3 F = F_Schlick( SpecularColor, VoH );
	float3 Diffuse = Diffuse_Lambert( DiffuseColor );


	return Diffuse * 1.0f + (D * Vis) * F;
}

float3 SurfaceShading( FGBufferData GBuffer, float Roughness, float3 L, float3 V, half3 N)
{
	return StandardShading(GBuffer.DiffuseColor, GBuffer.SpecularColor, Roughness, L, V, N );
	//return GBuffer.DiffuseColor;
	//switch(GBuffer.ShadingModelID)
	//{
	//	case 1:
	//		return StandardShading(GBuffer.DiffuseColor, GBuffer.SpecularColor, LobeRoughness, L, V, N );
	//	default:
	//		return float3(1.0f,0.0f,1.0f);
	//}
}

float4 GetDynamicLighting(float3 CameraVector, FGBufferData GBuffer, uint ShadingModelID)
{
	float3 L = normalize(float3(-1,1,1));
	float3 V = -CameraVector;
	float3 N = GBuffer.WorldNormal;

	float3 SurfaceLighting = SurfaceShading(GBuffer, GBuffer.Roughness,  L, V, N);
	return float4(SurfaceLighting.xyz,1.0f);
}

void DeferredLightPixelMain(
	float2 ScreenUV			: TEXCOORD0,
	float3 ScreenVector		: TEXCOORD1,
	float4 SVPos			: SV_POSITION,
	out float4 OutColor		: SV_Target0
	)
{
	float DeviceZ=SceneTexturesStruct_SceneDepthTexture.Sample(gsamPointWarp,ScreenUV).r;
	float WorldZ=ConvertFromDeviceZ(DeviceZ);
	float3 WorldPosition = ScreenVector * WorldZ + View_WorldCameraOrigin;
	float3 CameraVector = normalize(ScreenVector);

	float tempres=0.0f;
	FGBufferData GBuffer=(FGBufferData)0;
	{
		float4 GbufferA= SceneTexturesStruct_GBufferATexture.Load(int3(SVPos.xy,0));
		float4 GbufferB= SceneTexturesStruct_GBufferBTexture.Load(int3(SVPos.xy,0));
		float4 GbufferC= SceneTexturesStruct_GBufferCTexture.Load(int3(SVPos.xy,0));
		float4 GbufferD= SceneTexturesStruct_GBufferDTexture.Load(int3(SVPos.xy,0));
		GBuffer.WorldNormal=normalize(GbufferA.xyz);
		GBuffer.BaseColor=GbufferC.xyz;
		GBuffer.Metallic=GbufferB.x;
		GBuffer.Specular=GbufferB.y;
		GBuffer.Roughness=GbufferB.z;
		GBuffer.ShadingModelID=uint(GbufferB.w);

		//EncodeGBuffer in DeferredShadingCommon.usf
		GBuffer.SpecularColor = lerp( 0.08 * GBuffer.Specular.xxx, GBuffer.BaseColor, GBuffer.Metallic );
		GBuffer.DiffuseColor = GBuffer.BaseColor - GBuffer.BaseColor * GBuffer.Metallic;
		tempres+=(GbufferA.x+GbufferB.x+GbufferA.x+GbufferC.x+GbufferD.x);
	}
	tempres+=WorldPosition.x*0.0001f+WorldPosition.y*0.0001f+WorldPosition.z*0.0001f;
	tempres*=0.00000000000f;
	//DeferredLightingCommon.ush
	float4 SurfaceLighting;
	{
		float Shadow=LightAttenuationTexture.Sample(gsamPointWarp,ScreenUV).r;
		SurfaceLighting=(1.0-Shadow)*GetDynamicLighting(CameraVector,GBuffer,GBuffer.ShadingModelID);
	}
    OutColor=float4(SurfaceLighting.xyz,1.0f);
	OutColor.z+=tempres;
}