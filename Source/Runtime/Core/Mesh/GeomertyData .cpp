#include "GeomertyData.h"

uint32 GDataBuffer::DataTypeByteSize[(int)XDataType::DT_MAX_NUM] =
{
	4,  
	8,
	12,
	16,

	16,
	32,
};

GDataBuffer::~GDataBuffer()
{
	std::free(DataStore);
}

void GDataBuffer::SetData(uint8* DataStoreIn, uint64 DataNumIn, XDataType DataTypeIn)
{
	DataType = DataTypeIn;
	DataNum = DataNumIn;
	DataByteSize = DataTypeByteSize[(int)DataType] * DataNum;

	DataStore = (uint8*)std::malloc(DataByteSize);
	memcpy(DataStore, DataStoreIn, DataByteSize);
}