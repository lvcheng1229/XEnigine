#pragma once
#include <memory>
#include <fstream>
#include "Runtime/HAL/PlatformTypes.h"
struct TGAHeader
{
	uint8 ImageIDLength;
	uint8 ColorMapType;
	uint8 CompressionType;
	
	uint32 ColorMapInfo;
	//uint8 ColorMapBitNum;

	uint16 LowerLeftX;
	uint16 LowerLeftY;
	uint16 width;
	uint16 height;

	uint8 BitsPerPixel;
	uint8 ImageDesc;
};


#include <iostream>
bool LoadTGATexture(const wchar_t* fileName, uint32& width, uint32& height, std::unique_ptr<uint8[]>& o_data,uint32& channel)
{
	std::ifstream file_stream;
	file_stream.open(fileName, std::ifstream::binary | std::ifstream::in);
	
	uint32 file_size_bit;
	file_stream.seekg(0, std::ios::end);
	file_size_bit = file_stream.tellg();
	uint8* data = new uint8[file_size_bit];
	file_stream.seekg(0, std::ios::beg);
	
	file_stream.read(reinterpret_cast<char*>(data), file_size_bit);

	TGAHeader* header = reinterpret_cast<TGAHeader*>(data);


	if (header->ImageIDLength != 0) { return false; };
	if (header->ColorMapType != 0) { return false; };
	if (header->CompressionType != 2) { return false; }
	if (header->ColorMapInfo != 0) { return false; }
	//if (header->ColorMapBitNum != 0) { return false; }
	if ((header->ImageDesc)>=0) { return false; }
	
	
	width = header->width;
	height = header->height;
	
	channel = header->BitsPerPixel / 8;

	uint32 image_size = width * height * header->BitsPerPixel / 8;
	uint32 PiexlNums = width * height;

	o_data.reset(new uint8[image_size]);

	std::cout << "header->ImageIDLength:" << (uint32)header->ImageIDLength << std::endl;
	std::cout << "header->ColorMapType:" << (uint32)header->ColorMapType << std::endl;
	std::cout << "header->CompressionType:" << (uint32)header->CompressionType << std::endl;
	std::cout << "header->leftDownX:" << (uint32)header->LowerLeftX << std::endl;
	std::cout << "header->leftDownY:" << (uint32)header->LowerLeftY << std::endl;
	std::cout << "header->width:" << (uint32)header->width << std::endl;
	std::cout << "header->height:" << (uint32)header->height << std::endl;
	std::cout << "header->BitsPerPixel:" << (uint32)header->BitsPerPixel << std::endl;
	std::cout << "header->ImageDesc:" << (uint32)header->ImageDesc << std::endl;
	std::cout << "image_size:" << (uint32)image_size << std::endl;

	if (header->ImageIDLength != 0) {}
	if (header->ColorMapType != 0) {};
	if (header->CompressionType == 2)
	{
		memcpy(o_data.get(), data + sizeof(TGAHeader), image_size);
	}

	uint32 index = 0;
	for (int i = 0; i < PiexlNums; i++)
	{
		uint8 temp = o_data.get()[index];
		o_data.get()[index] = o_data.get()[index + 2];
		o_data.get()[index + 2] = temp;
		index += header->BitsPerPixel / 8;


		//std::cout << (uint32)o_data.get()[index] << " " << (uint32)o_data.get()[index + 1] << " " << (uint32)o_data.get()[index + 2] << std::endl;
	}
	
	delete[] data;
	return true;
}