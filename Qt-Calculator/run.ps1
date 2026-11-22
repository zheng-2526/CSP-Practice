$ErrorActionPreference = 'Stop'

$qtBin = 'C:\Qt\6.9.1\mingw_64\bin'
$mingwBin = 'C:\Qt\Tools\mingw1310_64\bin'
$env:Path = "$qtBin;$mingwBin;$env:Path"

$application = Join-Path $PSScriptRoot 'build\QtCalculator.exe'
if(-not (Test-Path -LiteralPath $application)) {
    throw '程序尚未编译，请先运行 .\build.ps1'
}

& $application
