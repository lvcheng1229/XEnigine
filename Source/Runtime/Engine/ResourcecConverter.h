#pragma once
#include "Runtime/Core/Mesh/GeomertyNode.h"
#include "Runtime/Engine/Classes/Texture.h"
#include "Runtime/Engine/Classes/Material.h"

//convert texture
//convert mesh
std::shared_ptr<GTexture2D> CreateTextureFromImageFile(const std::string& FilePath, bool bSRGB = false);
std::shared_ptr<GTexture2D> LoadTextureFromAssetFile(const std::string& FilePath);
std::shared_ptr<GTexture2D> CreateTextureFromImageFileAndSave(const std::string& FilePath, bool bSRGB = false);

std::shared_ptr<GMaterial> CreateMaterialFromCode(const std::wstring& CodePathIn);

std::shared_ptr<GGeomertry> CreateDefualtQuadGeo();


std::shared_ptr<GGeomertry> TempCreateQuadGeoWithMat();
