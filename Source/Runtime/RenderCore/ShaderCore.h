#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include "Runtime/Core/Template/XEngineTemplate.h"
class XShaderCodeReader
{
	XArrayView<uint8> ShaderCode;
public:
	XShaderCodeReader(XArrayView<uint8> InShaderCode) :ShaderCode(InShaderCode) {}

	size_t GetActualShaderCodeSize() const
	{
		return ShaderCode.size() - GetOptionalDataSize();
	}

	const uint8* FindOptionalData(uint8 InKey, uint8 ValueSize) const
	{
		const uint8* End = ShaderCode.data() + ShaderCode.size();

		int32 LocalOptionalDataSize = GetOptionalDataSize();

		const uint8* Start = End - LocalOptionalDataSize;
		End = End - sizeof(LocalOptionalDataSize);
		const uint8* Current = Start;

		while (Current < End)
		{
			char Key = *Current; ++Current;
			uint32 Size = *((const int32*)Current);
			Current += sizeof(Size);

			if (Key == InKey && Size == ValueSize)
			{
				return Current;
			}

			Current += Size;
		}

		return 0;
	}

	int32 GetOptionalDataSize()const
	{
		const uint8* End = ShaderCode.data() + ShaderCode.size();
		int32 OptionalDataSize = ((const int32*)End)[-1];
		return OptionalDataSize;
	}
};

struct XShaderResourceCount
{
	static const uint8 Key = 'p';

	uint8 NumSRV;
	uint8 NumCBV;
	uint8 NumUAV;
};