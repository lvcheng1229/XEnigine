#include "GeomertyData.h"

uint32 GDataBuffer::DataTypeByteSize[(int)EDataType::DT_MAX_NUM] =
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
}

void GDataBuffer::SetData(uint8* DataStoreIn, uint64 DataNumIn, EDataType DataTypeIn)
{
	DataType = DataTypeIn;
	DataNum = DataNumIn;
	DataByteSize = DataTypeByteSize[(int)DataType] * DataNum;

	DataStore.insert(DataStore.end(), DataStoreIn, DataStoreIn + DataByteSize);
}

void GVertexBuffer::SetData(std::shared_ptr<GDataBuffer> DataBufferIn, EVertexAttributeType EVAIn)
{
	DataBufferPtrArray_LodArray[(int)EVAIn].push_back(DataBufferIn);
}
