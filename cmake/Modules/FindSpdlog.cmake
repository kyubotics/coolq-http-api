find_path(SPDLOG_INCLUDE_DIR spdlog/spdlog.h)

find_library(FMT_LIBRARY NAMES fmt libfmt)

if(SPDLOG_INCLUDE_DIR)
   set(SPDLOG_FOUND TRUE)
endif(SPDLOG_INCLUDE_DIR)

mark_as_advanced(
  SPDLOG_INCLUDE_DIR
  FMT_LIBRARY
)

if(SPDLOG_FOUND)
  set(SPDLOG_INCLUDE_DIRS ${SPDLOG_INCLUDE_DIR})
  set(SPDLOG_LIBRARIES)
  set(SPDLOG_AND_DEPS_LIBRARIES ${FMT_LIBRARY})
  message(STATUS "Found Spdlog and it's dependencies: ${SPDLOG_AND_DEPS_LIBRARIES}")
else(SPDLOG_FOUND)
  message(FATAL_ERROR "Could not find Spdlog")
endif(SPDLOG_FOUND)
