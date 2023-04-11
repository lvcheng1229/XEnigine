cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

project(XEngine VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(XENGINE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

add_definitions(-D ROOT_DIR_XENGINE=${XENGINE_ROOT_DIR})

file(GLOB_RECURSE HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")
file(GLOB_RECURSE SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp")
file(GLOB_RECURSE SOURCE_FILES_CC "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cc")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/Source" FILES ${HEADER_FILES} ${SOURCE_FILES} ${SOURCE_FILES_CC})

link_libraries("d3dcompiler.lib")
link_libraries("D3D12.lib")
link_libraries("dxgi.lib")

include_directories(
  ${XENGINE_ROOT_DIR}/Source
)

add_definitions(-DX_PLATFORM_WIN)
add_definitions(-DX_RHI_DX12)
add_definitions(-DUNICODE)
add_definitions(-D_UNICODE)#for imgui

##ImGUI Begin
#set(imgui_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThridParty/imgui)
#file(GLOB imgui_sources "${imgui_SOURCE_DIR}/*.cpp")
#file(GLOB imgui_impl
#"${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp" 
#"${imgui_SOURCE_DIR}/backends/imgui_impl_win32.h"
#"${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.cpp" 
#"${imgui_SOURCE_DIR}/backends/imgui_impl_dx12.h")
#add_library(imgui STATIC ${imgui_sources} ${imgui_impl})
#target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
#link_libraries(imgui)
##ImGUI End


#BOOST Begin
set(BOOST_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ThridParty/boost_1_78_0)
include_directories(${BOOST_SOURCE_DIR})
#BOOST End


add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${SOURCE_FILES_CC})