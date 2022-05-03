#include "GeomertyData.h"

uint32 GDataBuffer::DataTypeByteSize[(int)EVertexElementType::VET_MAX] =
{
	0,

	4,  
	8,
	12,
	16,

	16,
	32,

	16,
	0,
};

static_assert((int)EVertexElementType::VET_MAX == 9,"(int)EVertexElementType::VET_MAX == 9");

GDataBuffer::~GDataBuffer()
{
}

void GDataBuffer::SetData(uint8* DataStoreIn, uint64 DataNumIn, EVertexElementType DataTypeIn)
{
	DataType = DataTypeIn;
	DataNum = DataNumIn;
	DataByteSize = DataTypeByteSize[(int)DataType] * DataNum;

	DataStore.insert(DataStore.end(), DataStoreIn, DataStoreIn + DataByteSize);
}

void GVertexBuffer::SetData(std::shared_ptr<GDataBuffer> DataBufferIn, EVertexAttributeType EVAIn)
{
	DataBufferPtrArray[(int)EVAIn] = DataBufferIn;
}

void GVertexBuffer::CreateRHIVertexBufferAndLayoutChecked()
{
	if (RHIVertexLayout.get() != nullptr)
	{
		return;
	}

	XRHIVertexLayoutArray LayoutArray;
	uint32 SemanticIndex = 0;
	uint32 ByteOffset = 0;
	
	for (int i = 0; i < (int)EVertexAttributeType::VAT_MAX_NUM; i++)
	{
		std::shared_ptr<GDataBuffer> BufferPtr = DataBufferPtrArray[i];
		if (BufferPtr.get() != nullptr)
		{
			LayoutArray.push_back(XVertexElement(SemanticIndex, BufferPtr->DataType, 0, ByteOffset));
			SemanticIndex++;
			ByteOffset += BufferPtr->DataByteSize;
		}
	}
	
	uint64 ElementNum = ByteOffset * DataBufferPtrArray[(int)EVertexAttributeType::VAT_POSITION]->DataNum;
	for (uint64 i = 0; i < ElementNum; i++)
	{

	}
}
