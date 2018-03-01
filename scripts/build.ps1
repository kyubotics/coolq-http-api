$config_type = $args[0]

Set-Location $PSScriptRoot\..\build  # enter the parent folder
cmake --build .\$config_type --config $config_type  # build the project
