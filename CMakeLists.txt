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
                -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
                -D_SCL_SECURE_NO_WARNINGS
                -DWIN32_LEAN_AND_MEAN
                -DCURL_STATICLIB)

find_package(Boost REQUIRED COMPONENTS filesystem)
find_package(Iconv REQUIRED)
find_package(CURL REQUIRED)

set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:LIBCMTD")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
set(LIB_NAME "io.github.richardchien.coolqhttpapi")
add_library(${LIB_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
target_link_libraries(${LIB_NAME} ${ICONV_AND_DEPS_LIBRARIES})
target_link_libraries(${LIB_NAME} ${CURL_AND_DEPS_LIBRARIES})
target_link_libraries(${LIB_NAME} crypt32 bcrypt Wldap32 Ws2_32)

add_custom_command(TARGET ${LIB_NAME}
                   POST_BUILD
                   COMMAND
                   powershell -ExecutionPolicy Bypass -NoProfile -File "${PROJECT_SOURCE_DIR}/scripts/post_build.ps1" "$<TARGET_FILE:${LIB_NAME}>")
