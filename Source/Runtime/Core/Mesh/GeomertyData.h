#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/Spatial/Spatial.h"
#include "Runtime/Engine/Classes/Material.h"

enum class XDataType
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
	static uint32 DataTypeByteSize[(int)XDataType::DT_MAX_NUM];
	~GDataBuffer();
	void SetData(uint8* DataStoreIn, uint64 DataNumIn, XDataType DataTypeIn);
protected:
	XDataType  DataType;
	uint8* DataStore;
	uint64 DataNum;
	uint64 DataByteSize;
};

class GVertexBuffer : public GObject
{
public:
};


class GGeomertryData :public GSpatial
{
public:

private:
	GMaterialInstance* MaterialInstance;
};