cmake_minimum_required(VERSION 3.9)
project(coolq-http-api)

set(CMAKE_CXX_STANDARD 17)
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

include(UseStaticCRT)
include(FindVcpkgIncludeDir)

include_directories(${VCPKG_INCLUDE_DIR})
include_directories(src)

add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
                -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
                -DWIN32_LEAN_AND_MEAN
                -DCURL_STATICLIB)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Iconv REQUIRED)

set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:LIBCMTD")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
set(LIB_NAME "io.github.richardchien.coolqhttpapi")
add_library(${LIB_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
target_link_libraries(${LIB_NAME} ${ICONV_LIBRARIES})
