#pragma once
#include <memory>
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/Spatial/Spatial.h"
#include "Runtime/RHI/RHIResource.h"

class GMaterialInstance;

//enum class EDataType
//{
//	DT_FLOAT32_1 = 0,	// 1D float expanded to (value, 0., 0., 1.)
//	DT_FLOAT32_2,		// 2D float expanded to (value.x, value.y, 0., 1.)
//	DT_FLOAT32_3,		// 3D float expanded to (value.x, value.y, value.z, 1.)
//	DT_FLOAT32_4,		// 4D float expanded to (value.x, value.y, value.z, value.w.)
//
//	DT_USHORT,	// Index buffer 
//	DT_UINT,	// Index buffer		
//
//	DT_MAX_NUM,
//};


class GDataBuffer :public GObject
{
	friend class GVertexBuffer;
	friend class GIndexBuffer;
public:
	
	static uint32 DataTypeByteSize[(int)EVertexElementType::VET_MAX];
	~GDataBuffer();
	void SetData(uint8* DataStoreIn, uint64 DataNumIn, EVertexElementType DataTypeIn);
protected:
	EVertexElementType  DataType;
	std::vector<uint8>DataStore;
	uint64 DataNum;
	uint64 DataByteSize;
};

enum class EVertexAttributeType
{
	VAT_POSITION,
	VAT_TANGENT,
	VAT_NORMAL,
	VAT_TEXCOORD,
	VAT_MAX_NUM,
};

struct DataPerVertex
{
	XVector4 Position;
	XVector3 TangentX;
	XVector4 TangentY;
	XVector2 TextureCoord;
};

class GVertexBuffer : public GObject
{
public:
	void PushVertex(DataPerVertex& DataPerVertexIn);
	void SetData(std::shared_ptr<GDataBuffer>DataBufferIn, EVertexAttributeType EVAIn);
	void CreateRHIVertexBufferAndLayoutChecked();
	inline std::shared_ptr<XRHIVertexLayout> GetRHIVertexLayout();
	inline std::shared_ptr<XRHIVertexBuffer> GetRHIVertexBuffer();
private:
	std::shared_ptr<GDataBuffer> DataBufferPtrArray[(int)EVertexAttributeType::VAT_MAX_NUM];

	std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
	std::shared_ptr<XRHIVertexBuffer> RHIVertexBuffer;
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