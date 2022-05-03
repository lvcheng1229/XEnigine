#include "Runtime/Core/Math/Math.h"
#include "ResourcecConverter.h"
#include "ThirdParty/stb_image.h"
#include <d3dcompiler.h>

std::shared_ptr<GTexture2D> CreateTextureFromImageFile(const std::string& FilePath, bool bSRGB)
{
	int SizeX, SizeY, Channel;
	unsigned char* ColorData = stbi_load(FilePath.c_str(), &SizeX, &SizeY, &Channel, 0);
	
	if (Channel == 3)
	{
		int TexSize = SizeX * SizeY * 4;
		unsigned char* FourChannelData = new unsigned char[TexSize];
		for (int i = 0, k = 0; i < TexSize; i += 4, k += 3)
		{
			FourChannelData[i + 0] = ColorData[k + 0];
			FourChannelData[i + 1] = ColorData[k + 1];
			FourChannelData[i + 2] = ColorData[k + 2];
			FourChannelData[i + 3] = 0b11111111;
		}

		std::shared_ptr<GTexture2D> Result = std::make_shared<GTexture2D>(FourChannelData, SizeX, SizeY, 4, bSRGB);
		stbi_image_free(ColorData);
		delete FourChannelData;
		return Result;
	}
	else if (Channel == 4)
	{
		std::shared_ptr<GTexture2D> Result = std::make_shared<GTexture2D>(ColorData, SizeX, SizeY, 4, bSRGB);
		stbi_image_free(ColorData);
		return Result;
	}
	X_Assert(false);
}

std::shared_ptr<GMaterial> CreateMaterialFromCode(const std::wstring& CodePathIn)
{
	std::shared_ptr<GMaterial> MaterialResult = std::make_shared<GMaterial>();
	XDxRefCount<ID3DBlob> BeforeCompressed;
	{
#if defined(DEBUG) || defined(_DEBUG)  
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif

		XDxRefCount<ID3DBlob> Errors;
		std::string Target = "ps_5_1";
		HRESULT hr = D3DCompileFromFile(
			CodePathIn.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE,
			"Empty_PS", Target.c_str(), compileFlags, 0, &BeforeCompressed, &Errors);

		if (Errors != nullptr)OutputDebugStringA((char*)Errors->GetBufferPointer()); ThrowIfFailed(hr);
	}

	{
		ID3D12ShaderReflection* Reflection = NULL;
		D3DReflect(BeforeCompressed->GetBufferPointer(), BeforeCompressed->GetBufferSize(), IID_PPV_ARGS(&Reflection));

		D3D12_SHADER_DESC ShaderDesc;
		Reflection->GetDesc(&ShaderDesc);
		for (uint32 i = 0; i < ShaderDesc.BoundResources; i++)
		{
			D3D12_SHADER_INPUT_BIND_DESC  ResourceDesc;
			Reflection->GetResourceBindingDesc(i, &ResourceDesc);
			D3D_SHADER_INPUT_TYPE ResourceType = ResourceDesc.Type;

			if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_TEXTURE)
			{
				MaterialResult->MaterialTextureArray.push_back(MaterialTexParas{ ResourceDesc.Name ,nullptr });
			}
			else if (ResourceType == D3D_SHADER_INPUT_TYPE::D3D_SIT_CBUFFER)
			{
				ID3D12ShaderReflectionConstantBuffer* ConstantBuffer = Reflection->GetConstantBufferByName(ResourceDesc.Name);
				D3D12_SHADER_BUFFER_DESC CBDesc;
				ConstantBuffer->GetDesc(&CBDesc);
				for (uint32 ConstantIndex = 0; ConstantIndex < CBDesc.Variables; ConstantIndex++)
				{
					ID3D12ShaderReflectionVariable* Variable = ConstantBuffer->GetVariableByIndex(ConstantIndex);
					D3D12_SHADER_VARIABLE_DESC VariableDesc;
					Variable->GetDesc(&VariableDesc);
					MaterialResult->MaterialValueArray.push_back(MaterialValueParas{ VariableDesc.Name ,{},VariableDesc.Size });
				}
			}
		}
	}
	return MaterialResult;
}

std::shared_ptr<GGeomertry> CreateDefualtQuadGeo()
{
	std::vector<XVector4>Positions;
	std::vector<XVector4>TangentX;
	std::vector<XVector4>TangentY;
	std::vector<XVector2>TextureCoords;
	std::vector<uint16>Indices;

	Positions.push_back(XVector4(8.0, 0.0, 8.0, 1.0));
	Positions.push_back(XVector4(8.0, 0.0, -8.0, 1.0));
	Positions.push_back(XVector4(-8.0, 0.0, 8.0, 1.0));
	Positions.push_back(XVector4(-8.0, 0.0, -8.0, 1.0));

	TangentX.push_back(XVector4(1.0, 0.0, 0.0, 1.0));
	TangentX.push_back(XVector4(1.0, 0.0, 0.0, 1.0));
	TangentX.push_back(XVector4(1.0, 0.0, 0.0, 1.0));
	TangentX.push_back(XVector4(1.0, 0.0, 0.0, 1.0));

	TangentY.push_back(XVector4(0.0, 1.0, 0.0, 1.0));
	TangentY.push_back(XVector4(0.0, 1.0, 0.0, 1.0));
	TangentY.push_back(XVector4(0.0, 1.0, 0.0, 1.0));
	TangentY.push_back(XVector4(0.0, 1.0, 0.0, 1.0));

	TextureCoords.push_back(XVector2(1.0, 1.0));
	TextureCoords.push_back(XVector2(1.0, 0.0));
	TextureCoords.push_back(XVector2(0.0, 1.0));
	TextureCoords.push_back(XVector2(0.0, 0.0));

	Indices.push_back(0);
	Indices.push_back(1);
	Indices.push_back(2);

	Indices.push_back(2);
	Indices.push_back(1);
	Indices.push_back(3);

	
	std::shared_ptr<GDataBuffer> PositionDataBuffer = std::make_shared<GDataBuffer>();
	PositionDataBuffer->SetData((uint8*)Positions.data(), Positions.size(), EDataType::DT_FLOAT32_4);

	std::shared_ptr<GDataBuffer> TangentXDataBuffer = std::make_shared<GDataBuffer>();
	TangentXDataBuffer->SetData((uint8*)TangentX.data(), TangentX.size(), EDataType::DT_FLOAT32_4);

	std::shared_ptr<GDataBuffer> TangentYDataBuffer = std::make_shared<GDataBuffer>();
	TangentYDataBuffer->SetData((uint8*)TangentY.data(), TangentY.size(), EDataType::DT_FLOAT32_4);

	std::shared_ptr<GDataBuffer> TextureCoordsDataBuffer = std::make_shared<GDataBuffer>();
	TextureCoordsDataBuffer->SetData((uint8*)TextureCoords.data(), TextureCoords.size(), EDataType::DT_FLOAT32_2);

	std::shared_ptr<GDataBuffer> IndexDataBuffer = std::make_shared<GDataBuffer>();
	IndexDataBuffer->SetData((uint8*)Indices.data(), Indices.size(), EDataType::DT_USHORT);

	std::shared_ptr<GVertexBuffer>VertexBuffer = std::make_shared<GVertexBuffer>();
	VertexBuffer->SetData(PositionDataBuffer,EVertexAttributeType::VAT_POSITION);
	VertexBuffer->SetData(TangentXDataBuffer,EVertexAttributeType::VAT_TANGENT);
	VertexBuffer->SetData(TangentYDataBuffer,EVertexAttributeType::VAT_NORMAL);
	VertexBuffer->SetData(TextureCoordsDataBuffer,EVertexAttributeType::VAT_TEXCOORD);

	std::shared_ptr<GIndexBuffer> IndexBuffer = std::make_shared<GIndexBuffer>();
	IndexBuffer->SetData(IndexDataBuffer);

	std::shared_ptr<GMeshData>MeshData = std::make_shared<GMeshData>();
	MeshData->SetVertexBuffer(VertexBuffer);
	MeshData->SetIndexBuffer(IndexBuffer);

	std::shared_ptr<GGeomertry>Geomertry = std::make_shared<GGeomertry>();
	Geomertry->SetMeshData(MeshData);

	return Geomertry;
}

std::shared_ptr<GGeomertry> TempCreateQuadGeoWithMat()
{
	std::shared_ptr<GGeomertry> Result = CreateDefualtQuadGeo();
	std::shared_ptr<GMaterial> MaterialPtr = CreateMaterialFromCode(L"E:/XEngine/XEnigine/MaterialShaders/Material.hlsl");
	std::shared_ptr<GMaterialInstance> MaterialIns = std::make_shared<GMaterialInstance>(MaterialPtr);
	MaterialIns->SetMaterialValueFloat("Metallic",0.0f);
	MaterialIns->SetMaterialValueFloat("Roughness",0.8f);
	MaterialIns->SetMaterialValueFloat("TextureScale",10.0f);
	
	std::shared_ptr<GTexture2D> TexBaseColor = 
		CreateTextureFromImageFile("E:/XEngine/XEnigine/ContentSave/TextureNoAsset/T_Metal_Gold_D.TGA", true);

	std::shared_ptr<GTexture2D> TexRouhness =
		CreateTextureFromImageFile("E:/XEngine/XEnigine/ContentSave/TextureNoAsset/T_Metal_Gold_D.TGA", false);

	std::shared_ptr<GTexture2D> TexNormal =
		CreateTextureFromImageFile("E:/XEngine/XEnigine/ContentSave/TextureNoAsset/T_MacroVariation.TGA", false);

	MaterialIns->SetMaterialTexture2D("BaseColorMap", TexBaseColor);
	MaterialIns->SetMaterialTexture2D("RoughnessMap", TexRouhness);
	MaterialIns->SetMaterialTexture2D("NormalMap", TexNormal);

	Result->SetMaterialPtr(MaterialIns);
	return std::shared_ptr<GGeomertry>();
}

