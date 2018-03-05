find_path(CURL_INCLUDE_DIR curl/curl.h)

find_library(CURL_LIBRARY NAMES libcurl curllib curl curllib_static)
find_library(LIBSSH_LIBRARY NAMES libssh2 libssh ssh)
find_library(ZLIB_LIBRARY NAMES zlib)
find_library(SSLEAY_LIBRARY NAMES ssleay ssleay32)
find_library(EAY_LIBRARY NAMES eay eay32 libeay32)

if(CURL_INCLUDE_DIR AND CURL_LIBRARY)
  set(CURL_FOUND TRUE)
endif(CURL_INCLUDE_DIR AND CURL_LIBRARY)

mark_as_advanced(
  CURL_INCLUDE_DIR
  CURL_LIBRARY
  LIBSSH_LIBRARY
  ZLIB_LIBRARY
  SSLEAY_LIBRARY
)

if(CURL_FOUND)
  set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
  set(CURL_LIBRARIES ${CURL_LIBRARY})
  set(CURL_AND_DEPS_LIBRARIES ${CURL_LIBRARY} ${LIBSSH_LIBRARY} ${ZLIB_LIBRARY} ${SSLEAY_LIBRARY} ${EAY_LIBRARY})
  message(STATUS "Found CURL and it's dependencies: ${CURL_AND_DEPS_LIBRARIES}")
else(CURL_FOUND)
  message(FATAL_ERROR "Could not find CURL")
endif(CURL_FOUND)
