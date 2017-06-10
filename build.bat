call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\Common7\Tools\VsDevCmd.bat"
chcp 65001 > nul
msbuild /t:Build /p:Configuration=Release
exit