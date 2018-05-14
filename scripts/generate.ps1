$config_type = $args[0]

Set-Location $PSScriptRoot\..\  # enter the parent folder
mkdir build\$config_type -ErrorAction SilentlyContinue  # create build folder if not exists
Set-Location .\build\$config_type  # enter the build folder

$vcpkg_root = "C:\CLI\vcpkg"
$vcpkg_triplet = "x86-windows-static"
$msvc_toolset = "v141"

cmake -G "Visual Studio 15 2017" -T "$msvc_toolset" `
    -DCMAKE_TOOLCHAIN_FILE="$vcpkg_root\scripts\buildsystems\vcpkg.cmake" `
    -DVCPKG_TARGET_TRIPLET="$vcpkg_triplet" `
    -DCMAKE_CONFIGURATION_TYPES="$config_type" `
    -DCMAKE_BUILD_TYPE="$config_type" `
    ..\..
