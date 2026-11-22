$ErrorActionPreference = 'Stop'

$cmake = 'C:\Qt\Tools\CMake_64\bin\cmake.exe'
$qtBin = 'C:\Qt\6.9.1\mingw_64\bin'
$mingwBin = 'C:\Qt\Tools\mingw1310_64\bin'
$ninjaBin = 'C:\Qt\Tools\Ninja'

$env:Path = "$qtBin;$mingwBin;$ninjaBin;$env:Path"

& $cmake --preset qt-mingw-debug
if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $cmake --build --preset build-debug
if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

& $cmake --build build --target test
if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

Write-Host "`n构建和测试成功：$PSScriptRoot\build\QtCalculator.exe"
