#pragma once
#include "Runtime/HAL/PlatformTypes.h"
#include <string_view>

class XShaderCodeReader
{
	std::string_view ShaderCode;
public:
	XShaderCodeReader(std::string_view InShaderCode) :ShaderCode(InShaderCode) {}

	size_t GetActualShaderCodeSize() const
	{
		return ShaderCode.size() - GetOptionalDataSize();
	}

	const char* FindOptionalData(uint8 InKey, uint8 ValueSize) const
	{
		const char* End = ShaderCode.data() + ShaderCode.size();

		int32 LocalOptionalDataSize = GetOptionalDataSize();

		const char* Start = End - LocalOptionalDataSize;
		End = End - sizeof(LocalOptionalDataSize);
		const char* Current = Start;

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
		const char* End = ShaderCode.data() + ShaderCode.size();
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