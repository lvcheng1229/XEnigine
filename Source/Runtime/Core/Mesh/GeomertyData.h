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
	inline uint32 GetDataTypeSize()const
	{
		return DataTypeByteSize[(int)DataType];
	}
	inline const uint8* GetData()const
	{
		return DataStore.data();
	}
	inline uint64 GetDataNum()
	{
		return DataNum;
	}
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


class GVertexBuffer : public GObject
{
public:
	void SetData(std::shared_ptr<GDataBuffer>DataBufferIn, EVertexAttributeType EVAIn);
	void CreateRHIBufferChecked();
	inline std::shared_ptr<XRHIVertexLayout> GetRHIVertexLayout()const
	{
		return RHIVertexLayout;
	}
	inline std::shared_ptr<XRHIVertexBuffer> GetRHIVertexBuffer()const
	{
		return RHIVertexBuffer;
	}
	//inline std::shared_ptr<GDataBuffer> GetPositionPtr()const
	//{
	//	return DataBufferPtrArray[(int)EVertexAttributeType::VAT_POSITION];
	//}
	//inline std::shared_ptr<GDataBuffer> GetTangentPtr()const
	//{
	//	return DataBufferPtrArray[(int)EVertexAttributeType::VAT_TANGENT];
	//}
	//inline std::shared_ptr<GDataBuffer> GetNormalPtr()const
	//{
	//	return DataBufferPtrArray[(int)EVertexAttributeType::VAT_NORMAL];
	//}
	//inline std::shared_ptr<GDataBuffer> GetTextureCoordPtr()const
	//{
	//	return DataBufferPtrArray[(int)EVertexAttributeType::VAT_TEXCOORD];
	//}
private:
	std::shared_ptr<GDataBuffer> DataBufferPtrArray[(int)EVertexAttributeType::VAT_MAX_NUM];
	std::shared_ptr<XRHIVertexLayout> RHIVertexLayout;
	std::shared_ptr<XRHIVertexBuffer> RHIVertexBuffer;
};

class GIndexBuffer : public GObject
{
	friend class GGeomertry;
public:
	void CreateRHIBufferChecked();
	inline void SetData(std::shared_ptr<GDataBuffer>DataBufferIn)
	{
		IndexBufferPtr = DataBufferIn;
	}
	inline std::shared_ptr<XRHIIndexBuffer> GetRHIIndexBuffer()const
	{
		return RHIIndexBuffer;
	}
private:
	std::shared_ptr<GDataBuffer> IndexBufferPtr;
	std::shared_ptr<XRHIIndexBuffer> RHIIndexBuffer;
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

struct VertexCBufferStruct
{
	XMatrix WorldMatrix;

	XVector3 BoundBoxMax;
	float padding0;
	XVector3 BoundBoxMin;
	float padding1;
};

class GGeomertry :public GSpatial
{
public:
	inline uint64 GetIndexCount()const
	{
		return MeshDataPtr->IndexBufferPtr->IndexBufferPtr->GetDataNum();
	}

	inline std::shared_ptr<GVertexBuffer> GetGVertexBuffer()const
	{
		return MeshDataPtr->VertexBufferPtr;
	}
	inline std::shared_ptr<XRHIVertexLayout> GetRHIVertexLayout()const
	{
		return MeshDataPtr->VertexBufferPtr->GetRHIVertexLayout();
	}
	inline std::shared_ptr<XRHIVertexBuffer> GetRHIVertexBuffer()const
	{
		return MeshDataPtr->VertexBufferPtr->GetRHIVertexBuffer();
	}


	inline std::shared_ptr<GIndexBuffer> GetGIndexBuffer()const
	{
		return MeshDataPtr->IndexBufferPtr;
	}
	inline std::shared_ptr<XRHIIndexBuffer> GetRHIIndexBuffer()const
	{
		return MeshDataPtr->IndexBufferPtr->GetRHIIndexBuffer();
	}




	inline void SetMeshData(std::shared_ptr<GMeshData>MeshDataPtrIn)
	{
		MeshDataPtr = MeshDataPtrIn;
	}

	inline void SetMaterialPtr(std::shared_ptr<GMaterialInstance>MaterialInstancePtrIn)
	{
		MaterialInstancePtr = MaterialInstancePtrIn;
	}

	inline std::shared_ptr<GMaterialInstance>& GetMaterialInstance()
	{
		return MaterialInstancePtr;
	}

	std::shared_ptr<XRHIConstantBuffer> GetPerObjectVertexCBuffer();
	std::shared_ptr<GGeomertry> CreateGeoInstanceNoMatAndTrans();
	std::shared_ptr<GGeomertry> CreateGeoInstancewithMat();
private:
	std::shared_ptr<GMeshData>MeshDataPtr;
	std::shared_ptr<GMaterialInstance>MaterialInstancePtr;
	std::shared_ptr<XRHIConstantBuffer>PerObjectVertexCBuffer;
};
