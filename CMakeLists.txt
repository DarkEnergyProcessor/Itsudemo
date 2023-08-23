cmake_minimum_required(VERSION 3.13)

project(Itsudemo)

set(LIBTEXB_VERSION "2.0.0")
set(LIBTEXB_VERSION_MAJOR 2)
set(LIBTEXB_VERSION_MINOR 0)
set(LIBTEXB_VERSION_PATCH 0)

# zlib
find_package(ZLIB)
if(ZLIB_FOUND)
	message(STATUS "using system zlib")
	set(ZLIB_LIBRARY_TARGET ZLIB::ZLIB)
	set(ZLIB_INCLUDE_DIRECTORIES "")
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/zlib")
	message(STATUS "using provided zlib")
	add_subdirectory(zlib "${CMAKE_CURRENT_BINARY_DIR}/zlib_build")
	set(ZLIB_LIBRARY_TARGET zlibstatic)
	set(ZLIB_INCLUDE_DIRECTORIES
		zlib
		"${CMAKE_CURRENT_BINARY_DIR}/zlib_build"
	)
else()
	message(FATAL_ERROR "cannot find zlib")
endif()

# LibTEXB
configure_file(src/TEXBVersion.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/TEXB/TEXBVersion.h")
add_library(libtexb
	include/TEXB/TEXBLib.h
	src/TEXB.h
	src/TEXBFetch.cpp
	src/TEXBLib.cpp
	src/TEXBLoad.cpp
	src/TEXBModify.cpp
	src/TEXBPixel.cpp
	src/TEXBPixel.h
	src/TEXBSave.cpp
	src/TIMG.cpp
	src/xy2uv.h
	"${CMAKE_CURRENT_BINARY_DIR}/include/TEXB/TEXBVersion.h"
)
target_include_directories(libtexb PUBLIC include "${CMAKE_CURRENT_BINARY_DIR}/include" ${ZLIB_INCLUDE_DIRECTORIES})
target_link_libraries(libtexb PRIVATE ${ZLIB_LIBRARY_TARGET})
target_compile_definitions(libtexb PRIVATE LIBTEXB_DURING_COMPILATION)
set_target_properties(libtexb PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED TRUE
	OUTPUT_NAME "TEXB"
)
if(MSVC)
	target_compile_definitions(libtexb PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

install(TARGETS)

# Itsudemo program
set(ITSUDEMO_BUILD_PROGRAM_DEFAULT ON)
get_directory_property(HAS_PARENT PARENT_DIRECTORY)
if(HAS_PARENT)
	set(ITSUDEMO_BUILD_PROGRAM_DEFAULT OFF)
endif()

option(ITSUDEMO_BUILD_PROGRAM "Build Itsudemo front-end program?" ${ITSUDEMO_BUILD_PROGRAM_DEFAULT})
if(ITSUDEMO_BUILD_PROGRAM)
	set(ITSUDEMO_VERSION "1.1.0.0")
	set(ITSUDEMO_MAJOR 1)
	set(ITSUDEMO_MINOR 1)
	set(ITSUDEMO_REV 0)
	set(ITSUDEMO_PATCH 0) # deprecated

	# lodepng
	add_library(lodepng STATIC
		lodepng/lodepng.h
		lodepng/lodepng.cpp
	)
	target_include_directories(lodepng PUBLIC lodepng)

	configure_file(src/Info.rc.in "${CMAKE_CURRENT_BINARY_DIR}/Info.rc")
	add_executable(Itsudemo
		src/Itsudemo.cpp
		"${CMAKE_CURRENT_BINARY_DIR}/Info.rc"
	)
	target_link_libraries(Itsudemo PRIVATE lodepng libtexb)
endif()
