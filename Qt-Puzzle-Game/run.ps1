$ErrorActionPreference = 'Stop'

$exe = Join-Path $PSScriptRoot 'build\QtPuzzleGame.exe'
if(!(Test-Path -LiteralPath $exe)) {
    & (Join-Path $PSScriptRoot 'build.ps1')
    if($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$env:Path = 'C:\Qt\6.9.1\mingw_64\bin;C:\Qt\Tools\mingw1310_64\bin;' + $env:Path
& $exe
