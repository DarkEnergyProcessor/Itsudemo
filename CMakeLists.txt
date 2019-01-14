cmake_minimum_required(VERSION 3.0)

project(Itsudemo)
set(ITSUDEMO_VERSION "1.0.7.5")
set(ITSUDEMO_MAJOR 1)
set(ITSUDEMO_MINOR 0)
set(ITSUDEMO_REV 7)
set(ITSUDEMO_PATCH 5)

configure_file("${PROJECT_SOURCE_DIR}/src/Info.rc.in" "${PROJECT_BINARY_DIR}/Info.rc")
add_executable(Itsudemo
	lodepng/lodepng.cpp
	src/Itsudemo.cpp
	src/TEXBFetch.cpp
	src/TEXBLoad.cpp
	src/TEXBModify.cpp
	src/TEXBPixel.cpp
	src/TEXBSave.cpp
	src/TIMG.cpp
	${PROJECT_BINARY_DIR}/Info.rc
)

# zlib
find_package(ZLIB)
if(ZLIB_FOUND)
	message(STATUS "using system zlib")
	include_directories(${ZLIB_INCLUDE_DIRS})
	target_link_libraries(Itsudemo ${ZLIB_LIBRARIES})
elseif(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/zlib")
	message(STATUS "using provided zlib")
	add_subdirectory(zlib "${CMAKE_CURRENT_BINARY_DIR}/zlib_build")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/zlib")
	include_directories("${CMAKE_CURRENT_BINARY_DIR}/zlib_build")
	target_link_libraries(Itsudemo zlibstatic)
else()
	message(FATAL_ERROR "cannot find zlib")
endif()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/tclap-1.2.1")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/lodepng")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
install(TARGETS Itsudemo DESTINATION bin)
