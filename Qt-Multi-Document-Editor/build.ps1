$ErrorActionPreference = 'Stop'
Push-Location $PSScriptRoot
try {
    $cmake = 'C:\Qt\Tools\CMake_64\bin\cmake.exe'
    $env:Path = 'C:\Qt\6.9.1\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;' + $env:Path
    & $cmake --preset qt-mingw-debug
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $cmake --build --preset build-debug
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
    & $cmake --build build --target test
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} finally {
    Pop-Location
}
