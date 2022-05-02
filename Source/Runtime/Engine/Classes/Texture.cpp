#include "Texture.h"

//#ifndef STB_IMAGE_IMPLEMENTATION
//#define STB_IMAGE_IMPLEMENTATION
//#endif // !STB_IMAGE_IMPLEMENTATION

#include "ThirdParty/stb_image.h"

void GTexture::LoadTextureFromImage(const char* Name)
{
	unsigned char* ColorData = stbi_load(Name, &SizeX, &SizeY, &Channel, 0);
	if (Channel != 4) { X_Assert(false); }
	TextureDataArray.insert(TextureDataArray.end(), ColorData, ColorData + SizeX * SizeY * Channel);
	stbi_image_free(ColorData);
}