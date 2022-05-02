#pragma once
#include "Runtime/CoreGObject/GObjtect/Object.h"
#include "Runtime/RHI/RHIResource.h"
#include "Runtime/RHI/RHICommandList.h"

#include <memory>
#include <vector>





class GTexture :public GObject
{
	//Declare Reflection Begin
public:
	static void InitialReflectionInfo(XReflectionInfo* InfoPtr);
	static XReflectionInfo StaticReflectionInfo;

	using SuperClass = GObject;
	//Declare Reflection Begin

	//Declare Initial Begin
	static bool HasReisterClass;
	static bool PushInitFunToGloablFunArray();
private:
	//Declare Initial End

public:

	virtual void ArchiveImpl(XArchiveBase& Archvice)
	{
		SuperClass::ArchiveImpl(Archvice);
		Archvice.ArchiveFun(SizeX);
		Archvice.ArchiveFun(SizeY);
		Archvice.ArchiveFun(Channel);
		Archvice.ArchiveFun(TextureDataArray);
	}

	virtual void CreateRHITexture(bool bSRGB)
	{
	
	}

	void LoadTextureFromImage(const char* Name);

	int32 SizeX;
	int32 SizeY;
	int32 Channel;
protected:
	
	std::vector<uint8>TextureDataArray;//Temp Data Array , NO MipMap Now
};

class GTexture2D :public GTexture
{
public:
	virtual void CreateRHITexture(bool bSRGB)override
	{
		if (bSRGB)
		{
			RHITexturePrivate = RHICreateTexture2D(SizeX, SizeY, 1, false, false, EPixelFormat::FT_R8G8B8A8_UNORM_SRGB
				, ETextureCreateFlags(TexCreate_SRGB), 1, TextureDataArray.data());
		}
		else
		{
			RHITexturePrivate = RHICreateTexture2D(SizeX, SizeY, 1, false, false, EPixelFormat::FT_R8G8B8A8_UNORM
				, ETextureCreateFlags(TexCreate_None), 1, TextureDataArray.data());
		}
	}

	inline std::shared_ptr<XRHITexture2D> GetRHITexture2D()
	{
		return RHITexturePrivate;
	}
private:
	std::shared_ptr<XRHITexture2D>RHITexturePrivate;
};