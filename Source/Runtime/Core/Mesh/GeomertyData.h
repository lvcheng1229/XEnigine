#pragma once
#include <memory>
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/Spatial/Spatial.h"

class GMaterialInstance;
enum class EDataType
{
	DT_FLOAT32_1 = 0,	// 1D float expanded to (value, 0., 0., 1.)
	DT_FLOAT32_2,		// 2D float expanded to (value.x, value.y, 0., 1.)
	DT_FLOAT32_3,		// 3D float expanded to (value.x, value.y, value.z, 1.)
	DT_FLOAT32_4,		// 4D float expanded to (value.x, value.y, value.z, value.w.)

	DT_USHORT,	// Index buffer 
	DT_UINT,	// Index buffer		

	DT_MAX_NUM,
};


class GDataBuffer :public GObject
{
public:
	static uint32 DataTypeByteSize[(int)EDataType::DT_MAX_NUM];
	~GDataBuffer();
	void SetData(uint8* DataStoreIn, uint64 DataNumIn, EDataType DataTypeIn);
protected:
	EDataType  DataType;
	std::vector<uint8>DataStore;
	uint64 DataNum;
	uint64 DataByteSize;
};

enum class EVertexAttributeType
{
	VAT_POSITION,
	VAT_TEXCOORD,
	VAT_NORMAL,
	VAT_TANGENT,
	VAT_MAX_NUM,
};

class GVertexBuffer : public GObject
{
public:
	void SetData(std::shared_ptr<GDataBuffer>DataBufferIn, EVertexAttributeType EVAIn);
private:
	std::vector <std::shared_ptr<GDataBuffer>> DataBufferPtrArray_LodArray[(int)EVertexAttributeType::VAT_MAX_NUM];
};

class GIndexBuffer : public GObject
{
public:
	inline void SetData(std::shared_ptr<GDataBuffer>DataBufferIn)
	{
		IndexBufferPtr = DataBufferIn;
	}
private:
	std::shared_ptr<GDataBuffer> IndexBufferPtr;
};

class GMeshData :public GObject
{
public:
	friend class GGeomertry;
	inline void SetVertexBuffer(std::shared_ptr<GVertexBuffer>VertexBufferPtrIn)
	{
		VertexBufferPtr = VertexBufferPtrIn;
	}
	inline void SetIndexBuffer(std::shared_ptr<GIndexBuffer>IndexBufferPtrIn)
	{
		IndexBufferPtr = IndexBufferPtrIn;
	}
private:
	std::shared_ptr<GVertexBuffer>VertexBufferPtr;
	std::shared_ptr<GIndexBuffer>IndexBufferPtr;
};

class GGeomertry :public GSpatial
{
public:

	inline void SetMeshData(std::shared_ptr<GMeshData>MeshDataPtrIn)
	{
		MeshDataPtr = MeshDataPtrIn;
	}

	inline void SetMaterialPtr(std::shared_ptr<GMaterialInstance>MaterialInstancePtrIn)
	{
		MaterialInstancePtr = MaterialInstancePtrIn;
	}
private:
	std::shared_ptr<GMeshData>MeshDataPtr;
	std::shared_ptr<GMaterialInstance>MaterialInstancePtr;
};