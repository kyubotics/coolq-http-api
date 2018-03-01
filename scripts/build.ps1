Set-Location $PSScriptRoot\..\  # enter the parent folder
cmake --build build --config $args  # build the project

$post_build_script = "$PSScriptRoot\post_build.ps1"
if ([System.IO.File]::Exists($post_build_script)) {
    & $post_build_script $args[0]  # run post build script
}
