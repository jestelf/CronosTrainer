param([string]$Out = '', [string]$VcVars = 'C:\BuildTools\VC\Auxiliary\Build\vcvars64.bat')
$ErrorActionPreference = 'Stop'
$root = $PSScriptRoot
if (-not $Out) { $Out = Join-Path $root 'dist\CronosTrainer.exe' }
New-Item -ItemType Directory -Force (Split-Path $Out) | Out-Null
$csc = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
$assets = Join-Path $root 'assets'
New-Item -ItemType Directory -Force $assets | Out-Null

$tmp = Join-Path $env:TEMP 'ctb'
New-Item -ItemType Directory -Force $tmp | Out-Null
Copy-Item "$root\src\CronosBridge.cpp" $tmp -Force
$vcvars = $VcVars
Set-Content "$tmp\b.bat" -Encoding ASCII -Value "@call `"$vcvars`" >nul 2>nul`r`ncd /d `"$tmp`"`r`ncl /nologo /O2 /LD /EHa /std:c++17 /utf-8 /W3 CronosBridge.cpp /link /OUT:CronosBridge.dll user32.lib"
cmd /c "$tmp\b.bat"
if ($LASTEXITCODE -ne 0) { throw 'CronosBridge.dll: compilation failed' }
Copy-Item "$tmp\CronosBridge.dll" "$assets\CronosBridge.dll" -Force

$gen = Join-Path $env:TEMP 'CronosTrainerIconGen.exe'
& $csc /nologo /target:exe /out:$gen /r:System.Drawing.dll "$root\src\IconGen.cs"
if ($LASTEXITCODE -ne 0) { throw 'IconGen: compilation failed' }
& $gen $assets
if ($LASTEXITCODE -ne 0) { throw 'IconGen: generation failed' }
Remove-Item $gen -ErrorAction SilentlyContinue

& $csc /nologo /target:winexe /platform:x64 /optimize+ "/out:$Out" "/win32icon:$assets\cronos.ico" "/win32manifest:$root\src\app.manifest" `
  "/resource:$assets\cronos.png,cronos.png" "/resource:$assets\CronosBridge.dll,CronosBridge.dll" `
  /r:System.Windows.Forms.dll /r:System.Drawing.dll `
  "$root\src\CronosTrainer.cs" "$root\src\CronosLink.cs"
if ($LASTEXITCODE -ne 0) { throw 'Build failed' }
Get-Item $Out | Select-Object Name, Length, LastWriteTime
