#include "Path.h"
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/wstringize.hpp>

const std::wstring& XPath::ProjectResourceSavedDir()
{
	static std::wstring SaveDir = BOOST_PP_CAT(L, BOOST_PP_CAT(BOOST_PP_STRINGIZE(ROOT_DIR_XENGINE), "/ContentSave"));
	return SaveDir;
}

const std::wstring& XPath::ProjectMaterialSavedDir()
{
	static std::wstring SaveDir = BOOST_PP_CAT(L, BOOST_PP_CAT(BOOST_PP_STRINGIZE(ROOT_DIR_XENGINE), "/MaterialShaders"));
	return SaveDir;
}
