Set-Location $PSScriptRoot\..\  # enter the parent folder
mkdir build -ErrorAction SilentlyContinue  # create build folder if not exists
Set-Location .\build  # enter the build folder

$vcpkg_root = "C:\CLI\vcpkg"
$vcpkg_triplet = "x86-windows-static"

cmake -G "Visual Studio 15 2017" `
    -DCMAKE_TOOLCHAIN_FILE=$vcpkg_root\scripts\buildsystems\vcpkg.cmake `
    -DVCPKG_TARGET_TRIPLET=$vcpkg_triplet `
    ..
