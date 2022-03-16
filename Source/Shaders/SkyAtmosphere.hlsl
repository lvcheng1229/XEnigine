#include "Common.hlsl"
#include "FastMath.hlsl"


#define CM_TO_SKY_UNIT 0.00001f

cbuffer SkyAtmosphere
{
    float4 SkyAtmosphere_TransmittanceLutSizeAndInvSize;
    float4 SkyAtmosphere_MultiScatteredLuminanceLutSizeAndInvSize;
    float4 SkyAtmosphere_SkyViewLutSizeAndInvSize;

	float4 Atmosphere_RayleighScattering;
    
    float4 Atmosphere_MieScattering;
    float4 Atmosphere_MieAbsorption;
    float4 Atmosphere_MieExtinction;
    float4 Atmosphere_GroundAlbedo;

    float Atmosphere_TopRadiusKm;
    float Atmosphere_BottomRadiusKm;
    float Atmosphere_MieDensityExpScale;
    float Atmosphere_RayleighDensityExpScale;

	float Atmosphere_TransmittanceSampleCount;
	float SkyAtmosphere_MultiScatteringSampleCount;
    float Atmosphere_MiePhaseG;
	float3 Atmosphere_padding;
}



/**
 * Returns near intersection in x, far intersection in y, or both -1 if no intersection.
 * RayDirection does not need to be unit length.
 */
float2 RayIntersectSphere(float3 RayOrigin, float3 RayDirection, float4 Sphere)
{
    float3 LocalPosition = RayOrigin - Sphere.xyz;
    float LocalPositionSqr = dot(LocalPosition, LocalPosition);

    float3 QuadraticCoef;
    QuadraticCoef.x = dot(RayDirection, RayDirection);
    QuadraticCoef.y = 2 * dot(RayDirection, LocalPosition);
    QuadraticCoef.z = LocalPositionSqr - Sphere.w * Sphere.w;

    float Discriminant = QuadraticCoef.y * QuadraticCoef.y - 4 * QuadraticCoef.x * QuadraticCoef.z;

    float2 Intersections = -1;

	// Only continue if the ray intersects the sphere
    [flatten]
    if (Discriminant >= 0)
    {
        float SqrtDiscriminant = sqrt(Discriminant);
        Intersections = (-QuadraticCoef.y + float2(-1, 1) * SqrtDiscriminant) / (2 * QuadraticCoef.x);
    }

    return Intersections;
}

// - RayOrigin: ray origin
// - RayDir: normalized ray direction
// - SphereCenter: sphere center
// - SphereRadius: sphere radius
// - Returns distance from RayOrigin to closest intersecion with sphere,
//   or -1.0 if no intersection.
float RaySphereIntersectNearest(float3 RayOrigin, float3 RayDir, float3 SphereCenter, float SphereRadius)
{
	float2 Sol = RayIntersectSphere(RayOrigin, RayDir, float4(SphereCenter, SphereRadius));
	float Sol0 = Sol.x;
	float Sol1 = Sol.y;
	if (Sol0 < 0.0f && Sol1 < 0.0f)
	{
		return -1.0f;
	}
	if (Sol0 < 0.0f)
	{
		return max(0.0f, Sol1);
	}
	else if (Sol1 < 0.0f)
	{
		return max(0.0f, Sol0);
	}
	return max(0.0f, min(Sol0, Sol1));
}


struct MediumSampleRGB
{
    float3 Scattering;
    float3 Absorption;
    float3 Extinction;

    float3 ScatteringMie;
    float3 AbsorptionMie;
    float3 ExtinctionMie;

    float3 ScatteringRay;
    float3 AbsorptionRay;
    float3 ExtinctionRay;

    float3 Albedo;
};

float3 GetAlbedo(float3 Scattering, float3 Extinction)
{
    return Scattering / max(0.001f, Extinction);
}

MediumSampleRGB SampleMediumRGB(in float3 WorldPos)
{
    const float SampleHeight = max(0.0, (length(WorldPos) - Atmosphere_BottomRadiusKm));
    const float DensityMie = exp(Atmosphere_MieDensityExpScale * SampleHeight);
    const float DensityRay = exp(Atmosphere_RayleighDensityExpScale * SampleHeight);

    MediumSampleRGB s;

    s.ScatteringMie = DensityMie * Atmosphere_MieScattering.rgb;
    s.AbsorptionMie = DensityMie * Atmosphere_MieAbsorption.rgb;
    s.ExtinctionMie = DensityMie * Atmosphere_MieExtinction.rgb; // equals to  ScatteringMie + AbsorptionMie

    s.ScatteringRay = DensityRay * Atmosphere_RayleighScattering.rgb;
    s.AbsorptionRay = 0.0f;
    s.ExtinctionRay = s.ScatteringRay + s.AbsorptionRay;

    s.Scattering = s.ScatteringMie + s.ScatteringRay;
    s.Absorption = s.AbsorptionMie + s.AbsorptionRay;
    s.Extinction = s.ExtinctionMie + s.ExtinctionRay;
    s.Albedo = GetAlbedo(s.Scattering, s.Extinction);

    return s;
}

////////////////////////////////////////////////////////////
// LUT functions
////////////////////////////////////////////////////////////

// Transmittance LUT function parameterisation from Bruneton 2017 https://github.com/ebruneton/precomputed_atmospheric_scattering
// uv in [0,1]
// ViewZenithCosAngle in [-1,1]
// ViewHeight in [bottomRAdius, topRadius]


//NOTE : See https://ebruneton.github.io/precomputed_atmospheric_scattering/atmosphere/functions.glsl.html
//explaned in Part Transmittance -> Part Precomputation
void UvToLutTransmittanceParams(out float ViewHeight, out float ViewZenithCosAngle, in float2 UV)
{
	//UV = FromSubUvsToUnit(UV, SkyAtmosphere.TransmittanceLutSizeAndInvSize); // No real impact so off
	float Xmu = UV.x;
	float Xr = UV.y;

	float H = sqrt(Atmosphere_TopRadiusKm * Atmosphere_TopRadiusKm - Atmosphere_BottomRadiusKm * Atmosphere_BottomRadiusKm);
	float Rho = H * Xr;
	ViewHeight = sqrt(Rho * Rho + Atmosphere_BottomRadiusKm * Atmosphere_BottomRadiusKm);

	float Dmin = Atmosphere_TopRadiusKm - ViewHeight;
	float Dmax = Rho + H;
	float D = Dmin + Xmu * (Dmax - Dmin);
	ViewZenithCosAngle = D == 0.0f ? 1.0f : (H * H - Rho * Rho - D * D) / (2.0f * ViewHeight * D);
	ViewZenithCosAngle = clamp(ViewZenithCosAngle, -1.0f, 1.0f);
}

#define DEFAULT_SAMPLE_OFFSET 0.3f

//WorldDir : should be normalized
//to solve tau(s)= -int_0^s (extinction(p))dt
float3 ComputeOpticalDepth(in float3 WorldPos,in float3 WorldDir , in float SampleCount)
{
    float t = 0.0f, tMax = 0.0f;
	float3 PlanetO = float3(0.0f, 0.0f, 0.0f);
	float tBottom = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_BottomRadiusKm);
	float tTop = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_TopRadiusKm);
	
	if (tBottom < 0.0f)
	{
		if (tTop < 0.0f)	{	tMax = 0.0f; return 0;	} // No intersection with planet nor its atmosphere: stop right away  
		else				{	tMax = tTop;			}
	}
	else
	{
		if (tTop > 0.0f)	{ tMax = min(tTop, tBottom); }
	}

	float dt = tMax / SampleCount;
    float3 OpticalDepth = 0.0f;
	for (float SampleI = 0.0f; SampleI < SampleCount; SampleI += 1.0f)
	{
		
		t = tMax * (SampleI + DEFAULT_SAMPLE_OFFSET) / SampleCount;
		float3 P = WorldPos + t * WorldDir;
		MediumSampleRGB Medium = SampleMediumRGB(P);
        const float3 SampleOpticalDepth = Medium.Extinction * dt;
		OpticalDepth += SampleOpticalDepth;		
	}
	return OpticalDepth;
}


////////////////////////////////////////////////////////////
// Transmittance LUT
////////////////////////////////////////////////////////////

#ifndef THREADGROUP_SIZE
#define THREADGROUP_SIZE 1
#endif

RWTexture2D<float3> TransmittanceLutUAV;

[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void RenderTransmittanceLutCS(uint3 ThreadId : SV_DispatchThreadID)
{
    float2 PixPos = float2(ThreadId.xy) + 0.5f;

    // Compute camera position from LUT coords
	float2 UV = (PixPos) * SkyAtmosphere_TransmittanceLutSizeAndInvSize.zw;
    float ViewHeight;
	float ViewZenithCosAngle;
	UvToLutTransmittanceParams(ViewHeight, ViewZenithCosAngle, UV);

    //  A few extra needed constants
	float3 WorldPos = float3(0.0f, 0.0f, ViewHeight);
	float3 WorldDir = float3(0.0f, sqrt(1.0f - ViewZenithCosAngle * ViewZenithCosAngle), ViewZenithCosAngle);

	//tau(s)= -int_0^s (extinction(p))dt
	float3 OpticalDepth = ComputeOpticalDepth(WorldPos,WorldDir,Atmosphere_TransmittanceSampleCount);
	
	//Tr = exp(- tau(s))
	float3 transmittance = exp(-OpticalDepth);

    TransmittanceLutUAV[ThreadId.xy]=transmittance;
}



///////////////////////////////////////////////////////////////////////////////////////

void getTransmittanceLutUvs(
	in float viewHeight, in float viewZenithCosAngle, in float BottomRadius, in float TopRadius,
	out float2 UV)
{
    float H = sqrt(max(0.0f, TopRadius * TopRadius - BottomRadius * BottomRadius));
    float Rho = sqrt(max(0.0f, viewHeight * viewHeight - BottomRadius * BottomRadius));

    float Discriminant = viewHeight * viewHeight * (viewZenithCosAngle * viewZenithCosAngle - 1.0f) + TopRadius * TopRadius;
    float D = max(0.0f, (-viewHeight * viewZenithCosAngle + sqrt(Discriminant))); // Distance to atmosphere boundary

    float Dmin = TopRadius - viewHeight;
    float Dmax = Rho + H;
    float Xmu = (D - Dmin) / (Dmax - Dmin);
    float Xr = Rho / H;

    UV = float2(Xmu, Xr);
}

void LutTransmittanceParamsToUv(in float ViewHeight, in float ViewZenithCosAngle, out float2 UV)
{
    getTransmittanceLutUvs(ViewHeight, ViewZenithCosAngle, Atmosphere_BottomRadiusKm, Atmosphere_TopRadiusKm, UV);
}

//#define PI 3.14159265358979
#define PLANET_RADIUS_OFFSET 0.001f
RWTexture2D<float3> MultiScatteredLuminanceLutUAV;
Texture2D<float3> TransmittanceLutTexture;
//SamplerState gsamLinearClamp : register(s5, space1000);

void ComputeScattedLightLuminanceAndMultiScatAs1in(
    in float3 WorldPos, 
    in float3 WorldDir, 
    in float3 Light0Dir,
    in float3 Light0Illuminance,
    in float SampleCount,
	out float3 OutL,
	out float3 OutMultiScatAs1
)
{
    float3 L = 0, MultiScatAs1 = 0;

    float t = 0.0f, tMax = 0.0f;
    float3 PlanetO = float3(0.0f, 0.0f, 0.0f);
    float tBottom = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_BottomRadiusKm);
    float tTop = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_TopRadiusKm);
	
    if (tBottom < 0.0f)
    {
        if      (tTop < 0.0f)   {   tMax = 0.0f;    return;} // No intersection with planet nor its atmosphere: stop right away  
        else                    {   tMax = tTop;}
    }
    else
    {
        if  (tTop > 0.0f)       {   tMax = min(tTop, tBottom); }
    }

    float dt = tMax / SampleCount;
    float3 Throughput = 1.0f;

    const float uniformPhase = 1.0f / (4.0f * PI);
	for (float SampleI = 0.0f; SampleI < SampleCount; SampleI += 1.0f)
	{
        t = tMax * (SampleI + DEFAULT_SAMPLE_OFFSET) / SampleCount;
        float3 P = WorldPos + t * WorldDir;
        float PHeight = length(P);

		MediumSampleRGB Medium = SampleMediumRGB(P);
        const float3 SampleOpticalDepth = Medium.Extinction * dt * 1.0f;
        const float3 SampleTransmittance = exp(-SampleOpticalDepth);

        // 1 is the integration of luminance over the 4pi of a sphere, and assuming an isotropic phase function of 1.0/(4*PI) 
		MultiScatAs1 += Throughput * Medium.Scattering * 1.0f * dt;

        //Compute TransmittanceToLight0 by look up TransmittanceLutTexture ,which is computed in last step
        float2 UV;
        const float3 UpVector = P / PHeight;
        float Light0ZenithCosAngle = dot(Light0Dir, UpVector);
        LutTransmittanceParamsToUv(PHeight, Light0ZenithCosAngle, UV);
        float3 TransmittanceToLight0 = TransmittanceLutTexture.SampleLevel(gsamLinearClamp, UV, 0).rgb;
		
        //Compute PlanetShadow0
        //if Light0 is intersected with plant0 , then PlanetShadow0 is euqals to 0
        float tPlanet0 = RaySphereIntersectNearest(P, Light0Dir, PlanetO + PLANET_RADIUS_OFFSET * UpVector, Atmosphere_BottomRadiusKm);
        float PlanetShadow0 = tPlanet0 >= 0.0f ? 0.0f : 1.0f;

        //Compute Phase 
        //phase funtion become isotropic when scattering time increaseing , so we times uniformPhase
        float3 PhaseTimesScattering0 = Medium.Scattering * uniformPhase;

        //S euals to in point p , how many light scatters to eye dir(PhaseTimesScattering0)
        //and then times(x) transmittance form p to light pos
        //if light dir is intersected with plant , then planet shaow equals to 0
        
        //NOTE : we ingnore MultiScatteredLuminance
        float3 S = Light0Illuminance * PlanetShadow0 * TransmittanceToLight0 * PhaseTimesScattering0;

        // See slide 28 at http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/ 
        // L = /int_ Tr(x_eye, x_sample) * S dx
        // S is single scattered light , which is computed in last step
        // but Tr(x_eye, x_sample) is unkown
        // /int_0^tmax (e ^(- extinction * pos) x S) dx = (S - S* e ^(- extinction * pos))/ extinction
        float3 Sint = (S - S * SampleTransmittance) / Medium.Extinction; // integrate along the current step segment 
        
        // accumulate and also take into account the transmittance from previous steps
        L += Throughput * Sint; 
        Throughput *= SampleTransmittance;
    }

	//Ground Scattering
	if (tMax == tBottom)
	{
		float3 P = WorldPos + tBottom * WorldDir;
		float PHeight = length(P);

        float2 UV;
		const float3 UpVector = P / PHeight;
		float Light0ZenithCosAngle = dot(Light0Dir, UpVector);
        LutTransmittanceParamsToUv(PHeight, Light0ZenithCosAngle, UV);
        float3 TransmittanceToLight0 = TransmittanceLutTexture.SampleLevel(gsamLinearClamp, UV, 0).rgb;

		const float NdotL0 = saturate(dot(UpVector, Light0Dir));
		L += Light0Illuminance * TransmittanceToLight0 * Throughput * NdotL0 * Atmosphere_GroundAlbedo.rgb / PI;
	}

	OutL=L;
	OutMultiScatAs1=MultiScatAs1;
}

//x in [0,1] CosLightZenithAngle
//y in [0 1] ViewHeight
[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void RenderMultiScatteredLuminanceLutCS(uint3 ThreadId : SV_DispatchThreadID)
{
	float2 PixPos = float2(ThreadId.xy) + 0.5f;
	float CosLightZenithAngle = (PixPos.x * SkyAtmosphere_MultiScatteredLuminanceLutSizeAndInvSize.z) * 2.0f - 1.0f;
    float3 LightDir = float3(0.0f, sqrt(saturate(1.0f - CosLightZenithAngle * CosLightZenithAngle)), CosLightZenithAngle);
    
    float ViewHeight = Atmosphere_BottomRadiusKm +
	(PixPos.y * SkyAtmosphere_MultiScatteredLuminanceLutSizeAndInvSize.w) *
	(Atmosphere_TopRadiusKm - Atmosphere_BottomRadiusKm);

    float3 WorldPos = float3(0.0f, 0.0f, ViewHeight);
    float3 WorldDir = float3(0.0f, 0.0f, 1.0f);

	const float3 OneIlluminance = float3(1.0f, 1.0f, 1.0f);

    float3 OutL_0, OutMultiScatAs1_0, OutL_1, OutMultiScatAs1_1;
    ComputeScattedLightLuminanceAndMultiScatAs1in(
        WorldPos, WorldDir, LightDir, OneIlluminance,
        SkyAtmosphere_MultiScatteringSampleCount, OutL_0, OutMultiScatAs1_0);
    ComputeScattedLightLuminanceAndMultiScatAs1in(
        WorldPos, -WorldDir, LightDir, OneIlluminance,
        SkyAtmosphere_MultiScatteringSampleCount, OutL_1, OutMultiScatAs1_1);

    const float SphereSolidAngle = 4.0f * PI;
    const float IsotropicPhase = 1.0f / SphereSolidAngle;

    float3 IntegratedIlluminance = (SphereSolidAngle / 2.0f) * (OutL_0 + OutL_1);
    float3 MultiScatAs1 = (1.0f / 2.0f) * (OutMultiScatAs1_0 + OutMultiScatAs1_1);
    float3 InScatteredLuminance = IntegratedIlluminance * IsotropicPhase;
	
	// For a serie, sum_{n=0}^{n=+inf} = 1 + r + r^2 + r^3 + ... + r^n = 1 / (1.0 - r)
    const float3 R = MultiScatAs1;
    const float3 SumOfAllMultiScatteringEventsContribution = 1.0f / (1.0f - R);
    float3 L = InScatteredLuminance * SumOfAllMultiScatteringEventsContribution;

    MultiScatteredLuminanceLutUAV[PixPos] = L;
}



////////////////////////////////////////////////////////////
// Sky View LUT
////////////////////////////////////////////////////////////
RWTexture2D<float3> SkyViewLutUAV;
Texture2D<float3> MultiScatteredLuminanceLutTexture;
// This is the camera position relative to the virtual planet center.
// This is convenient because for all the math in this file using world position relative to the virtual planet center.
float3 GetCameraPlanetPos()
{
	return (View.SkyWorldCameraOrigin - View.SkyPlanetCenterAndViewHeight.xyz)* CM_TO_SKY_UNIT;
}

float2 FromSubUvsToUnit(float2 uv, float4 SizeAndInvSize) { return (uv - 0.5f * SizeAndInvSize.zw) * (SizeAndInvSize.xy / (SizeAndInvSize.xy - 1.0f)); }


// SkyViewLut is a new texture used for fast sky rendering.
// It is low resolution of the sky rendering around the camera,
// basically a lat/long parameterisation with more texel close to the horizon for more accuracy during sun set.
void UvToSkyViewLutParams(out float3 ViewDir, in float ViewHeight, in float2 UV)
{
	// Constrain uvs to valid sub texel range (avoid zenith derivative issue making LUT usage visible)
    UV = FromSubUvsToUnit(UV, SkyAtmosphere_SkyViewLutSizeAndInvSize);

	float Vhorizon = sqrt(ViewHeight * ViewHeight - Atmosphere_BottomRadiusKm * Atmosphere_BottomRadiusKm);
	float CosBeta = Vhorizon / ViewHeight;				// cos of zenith angle from horizon to zeniht
	float Beta = acosFast4(CosBeta);
	float ZenithHorizonAngle = PI - Beta;

	float ViewZenithAngle;
	if (UV.y < 0.5f)
	{
		float Coord = 2.0f * UV.y;
		Coord = 1.0f - Coord;
		Coord *= Coord;
		Coord = 1.0f - Coord;
		ViewZenithAngle = ZenithHorizonAngle * Coord;
	}
	else
	{
		float Coord = UV.y * 2.0f - 1.0f;
		Coord *= Coord;
		ViewZenithAngle = ZenithHorizonAngle + Beta * Coord;
	}
	float CosViewZenithAngle = cos(ViewZenithAngle);
	float SinViewZenithAngle = sqrt(1.0 - CosViewZenithAngle * CosViewZenithAngle) * (ViewZenithAngle > 0.0f ? 1.0f : -1.0f); // Equivalent to sin(ViewZenithAngle)

	float LongitudeViewCosAngle = UV.x * 2.0f * PI;

	// Make sure those values are in range as it could disrupt other math done later such as sqrt(1.0-c*c)
	float CosLongitudeViewCosAngle = cos(LongitudeViewCosAngle);
	float SinLongitudeViewCosAngle = sqrt(1.0 - CosLongitudeViewCosAngle * CosLongitudeViewCosAngle) * (LongitudeViewCosAngle <= PI ? 1.0f : -1.0f); // Equivalent to sin(LongitudeViewCosAngle)
	ViewDir = float3(
		SinViewZenithAngle * CosLongitudeViewCosAngle,
		SinViewZenithAngle * SinLongitudeViewCosAngle,
		CosViewZenithAngle
		);
}



float RayleighPhase(float CosTheta)
{
    float Factor = 3.0f / (16.0f * PI);
    return Factor * (1.0f + CosTheta * CosTheta);
}

float HgPhase(float G, float CosTheta)
{
    float Numer = 1.0f - G * G;
    float Denom = 1.0f + G * G + 2.0f * G * CosTheta;
    return Numer / (4.0f * PI * Denom * sqrt(Denom));
}

void ComputeLForRenderSkyViewLut(
	in float3 WorldPos, in float3 WorldDir,
	in float MinSampleCount, in float MaxSampleCount,
	in float3 Light0Dir, in float3 Light0Illuminance,
	in float DistanceToSampleCountMaxInv,
)
{
	float t = 0.0f, tMax = 0.0f;
    float3 L = 0.0f;
    float3 Throughput = 1.0f;

    float3 PlanetO = float3(0.0f, 0.0f, 0.0f);
    float tBottom = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_BottomRadiusKm);
    float tTop = RaySphereIntersectNearest(WorldPos, WorldDir, PlanetO, Atmosphere_TopRadiusKm);
	
    if (tBottom < 0.0f)
    {
        if      (tTop < 0.0f)   {   tMax = 0.0f;    return;     } // No intersection with planet nor its atmosphere: stop right away  
        else                    {   tMax = tTop;}
    }
    else
    {
        if  (tTop > 0.0f)       {   tMax = min(tTop, tBottom);  }
    }
    tMax = min(tMax, 9000000.0f);
	
	//variable sample count by max distant to top sky atmosphere
	float SampleCount = lerp(MinSampleCount, MaxSampleCount, saturate(tMax*DistanceToSampleCountMaxInv));
	float SampleCountFloor = floor(SampleCount);
	float tMaxFloor = tMax * SampleCountFloor / SampleCount;	// rescale tMax to map to the last entire step segment.

    // Phase functions
    const float uniformPhase = 1.0f / (4.0f * PI);
    const float3 wi = Light0Dir;
    const float3 wo = WorldDir;
    float cosTheta = dot(wi, wo);
    float MiePhaseValueLight0 = HgPhase(Atmosphere.MiePhaseG, -cosTheta); // negate cosTheta because due to WorldDir being a "in" direction. 
    float RayleighPhaseValueLight0 = RayleighPhase(cosTheta);

    float dt = 0;
	for (float SampleI = 0.0f; SampleI < SampleCount; SampleI += 1.0f)
	{
		{
			float t0 = (SampleI) / SampleCountFloor;
			float t1 = (SampleI + 1.0f) / SampleCountFloor;
            
			t0 = t0 * t0; // Non linear distribution of samples within the range.
			t1 = t1 * t1;
            t0 = tMaxFloor * t0; // Make t0 and t1 world space distances.

			if (t1 > 1.0f)	{	t1 = tMax;			}
			else			{	t1 = tMaxFloor * t1;}

			t = t0 + (t1 - t0) * DEFAULT_SAMPLE_OFFSET;
            dt = t1 - t0;
        }
		
		float3 P = WorldPos + t * WorldDir;
        float PHeight = length(P);

		// Sample the medium
		MediumSampleRGB Medium = SampleMediumRGB(P);
		const float3 SampleOpticalDepth = Medium.Extinction * dt;
		const float3 SampleTransmittance = exp(-SampleOpticalDepth);

		//TODOOOOOOOOOOOOOOOOO Shadow
		
        float2 UV;
        const float3 UpVector = P / PHeight;
        float Light0ZenithCosAngle = dot(Light0Dir, UpVector);
        LutTransmittanceParamsToUv(PHeight, Light0ZenithCosAngle, UV);
        float3 TransmittanceToLight0 = TransmittanceLutTexture.SampleLevel(gsamLinearClamp, UV, 0).rgb;
        float3 PhaseTimesScattering0 = Medium.ScatteringMie * MiePhaseValueLight0 + Medium.ScatteringRay * RayleighPhaseValueLight0;

        float2 MultiSactterUV = saturate(float2(Light0ZenithCosAngle * 0.5f + 0.5f,
                                (length(P) - Atmosphere_BottomRadiusKm) /
                                (Atmosphere_TopRadiusKm - Atmosphere_BottomRadiusKm)));
	    
        float3 MultiScatteredLuminance0 = MultiScatteredLuminanceLutTexture.SampleLevel(gsamLinearClamp, MultiSactterUV, 0).rgb;
        

		// Planet shadow
        float tPlanet0 = RaySphereIntersectNearest(P, Light0Dir, PlanetO + PLANET_RADIUS_OFFSET * UpVector, Atmosphere_BottomRadiusKm);
        float PlanetShadow0 = tPlanet0 >= 0.0f ? 0.0f : 1.0f;
        float3 S = Light0Illuminance * 
            (PlanetShadow0 * TransmittanceToLight0 * PhaseTimesScattering0 + 
            MultiScatteredLuminance0 * Medium.Scattering);

		//details in ComputeScattedLightLuminanceAndMultiScatAs1in
		float3 Sint = (S - S * SampleTransmittance) / Medium.Extinction;
		L += Throughput * Sint;		
		Throughput *= SampleTransmittance;
	}
}


[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
void RenderSkyViewLutCS(uint3 ThreadId : SV_DispatchThreadID)
{
    float2 PixPos = float2(ThreadId.xy) + 0.5f;
    float2 UV = PixPos * SkyAtmosphere_SkyViewLutSizeAndInvSize.zw;

    float3 WorldPos = GetCameraPlanetPos();
    float ViewHeight = length(WorldPos);
    WorldPos = float3(0.0, 0.0, ViewHeight); // transform to local referential

    float3 WorldDir;
    UvToSkyViewLutParams(WorldDir, ViewHeight, UV); // UV to lat/long	

	// For the sky view lut to work, and not be distorted, we need to transform the view and light directions 
	// into a referential with UP being perpendicular to the ground. And with origin at the planet center.
    float3x3 LocalReferencial = GetSkyViewLutReferential(View.SkyViewLutReferential);
    float3 AtmosphereLightDirection0 = View_AtmosphereLightDirection.xyz;
    AtmosphereLightDirection0 = mul(LocalReferencial, AtmosphereLightDirection0);

	//if ((Ray is not intersecting the atmosphere)) {}


}

//[numthreads(THREADGROUP_SIZE, THREADGROUP_SIZE, 1)]
//void RenderSkyViewLutCS(uint3 ThreadId : SV_DispatchThreadID)
//{
//    SkyViewLutUAV[ThreadId.xy]=float3(1.0,0.0,0.0);
//}