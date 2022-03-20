workspace "XEngine"
	architecture "x64"
	startproject "project_xengine"

	configurations
	{
		"Debug",
		"Release",
	}
	


outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"


project "project_xengine"
	location "Source"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")


	includedirs
	{
		"./Source",
		"boost_1_78_0",
	}

	files
	{
		"./Source/**.h",
		"./Source/**.cpp",
	}

	libdirs 
	{ 
		--"./boost_1_78_0/stage/lib",
	}

	links 
	{ 
		"d3dcompiler.lib",
		"D3D12.lib",
		"dxgi.lib",
		--"libboost_log-vc143-mt-gd-x64-1_78.lib",
		--"libboost_log-vc143-mt-x64-1_78.lib",
	}





	filter "system:windows"
		systemversion "latest"

		defines
		{
			"X_PLATFORM_WIN",
			"X_RHI_DX12"
		}

	filter "configurations:Debug"
		defines "XE_DEBUG"
		buildoptions "/MTd"
		symbols "On"

	filter "configurations:Release"
		defines "XE_RELEASE"
		buildoptions "/MT"
		optimize "On"