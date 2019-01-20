if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    # fix Vcpkg debug library lookup bug, see https://github.com/Microsoft/vcpkg/issues/1626
    list(APPEND CMAKE_IGNORE_PATH ${_VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/lib)
endif()
