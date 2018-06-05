cmake_minimum_required(VERSION 3.9)
project(coolq-http-api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "/utf-8 ${CMAKE_CXX_FLAGS}")  # use UTF-8 source files
set(CMAKE_CXX_FLAGS "/MP ${CMAKE_CXX_FLAGS}")  # build with object level parallelism

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/../../cmake/Modules/")

include(UseStaticCRT)
include(FindVcpkgIncludeDir)

include_directories(${VCPKG_INCLUDE_DIR})
include_directories(../../src)
include_directories(src)

# add_definitions(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE
#                 -D_SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#                 -D_SCL_SECURE_NO_WARNINGS
#                 -D_WIN32_WINNT=0x0501
#                 -DWIN32_LEAN_AND_MEAN
#                 -DCURL_STATICLIB)

# find_package(Boost REQUIRED COMPONENTS filesystem)
# find_package(Iconv REQUIRED)
# find_package(CURL REQUIRED)
# find_package(Spdlog REQUIRED)

set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /NODEFAULTLIB:LIBCMT")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} /NODEFAULTLIB:LIBCMTD")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
set(LIB_NAME "test")
add_library(${LIB_NAME} SHARED ${SOURCE_FILES})

# target_link_libraries(${LIB_NAME} ${Boost_LIBRARIES})
# target_link_libraries(${LIB_NAME} ${ICONV_AND_DEPS_LIBRARIES})
# target_link_libraries(${LIB_NAME} ${CURL_AND_DEPS_LIBRARIES})
# target_link_libraries(${LIB_NAME} ${SPDLOG_AND_DEPS_LIBRARIES})
# target_link_libraries(${LIB_NAME} crypt32 bcrypt Wldap32 Ws2_32)
