#include <Windows.h>
#include <memory>
#include <fstream>
#include "Runtime/HAL/PlatformTypes.h"

const uint32 DDS_MAGIC = 0x20534444; // "DDS "

struct DDS_PIXELFORMAT
{
    uint32    size;
    uint32    flags;
    uint32    fourCC;
    uint32    RGBBitCount;
    uint32    RBitMask;
    uint32    GBitMask;
    uint32    BBitMask;
    uint32    ABitMask;
};

struct DDS_HEADER
{
    uint32        size;
    uint32        flags;
    uint32        height;
    uint32        width;
    uint32        pitchOrLinearSize;
    uint32        depth; // only if DDS_HEADER_FLAGS_VOLUME is set in flags
    uint32        mipMapCount;
    uint32        reserved1[11];
    DDS_PIXELFORMAT ddspf;
    uint32        caps;
    uint32        caps2;
    uint32        caps3;
    uint32        caps4;
    uint32        reserved2;
};

HRESULT XLoadTextureDataFromFile(_In_z_ const wchar_t* fileName,
    std::unique_ptr<uint8[]>& ddsData,
    DDS_HEADER** header,
    uint8** bitData,
    uint32 file_size_bit
)
{
    uint32 magic_num;
    std::ifstream file_stream;
    file_stream.open(fileName, std::ifstream::binary | std::ifstream::in);
    
    file_stream.seekg(0, std::ios::end);
    file_size_bit = file_stream.tellg();
    ddsData.reset(new uint8[file_size_bit]);
    file_stream.seekg(0, std::ios::beg);
    
    file_stream.read(reinterpret_cast<char*>(ddsData.get()), file_size_bit);
    
    magic_num = reinterpret_cast<uint32*>(ddsData.get())[0];
    *header = reinterpret_cast<DDS_HEADER*>(ddsData.get() + sizeof(uint32));
    *bitData = reinterpret_cast<uint8*>(ddsData.get() + sizeof(uint32) + sizeof(DDS_HEADER));

    file_stream.close();

    return S_OK;
}