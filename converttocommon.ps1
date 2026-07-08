# Run this in the workspace root (e.g. S:\vegyes\FalconCalc)
# BACKUP first: copies all .vcxproj and .vcxproj.filters files to .backup_vcxproj
$backupDir = ".backup_vcxproj"
New-Item -ItemType Directory -Path $backupDir -Force | Out-Null
Get-ChildItem -Path . -Recurse -Include *.vcxproj,*.vcxproj.filters |
    ForEach-Object { Copy-Item -Path $_.FullName -Destination (Join-Path $backupDir $_.FullName.Replace(':','').Replace('\','_')) -Force }

# Files to move (names only)
$names = @(
  "calculate.h","calculate.cpp",
  "common.h",
  "defines.h",
  "EngineErrors.h","EngineErrors.cpp",
  "LongNumber.h","LongNumber.cpp",
  "version.h"
)

# Replace patterns: ../src/ or ..\src\  -> ../common/ or ..\common\
Get-ChildItem -Path . -Recurse -Include *.vcxproj,*.vcxproj.filters |
ForEach-Object {
    $file = $_.FullName
    $text = Get-Content -LiteralPath $file -Raw -Encoding UTF8

    foreach($name in $names) {
        # replace backslash style
        $text = $text -replace ("(\.\.\\src\\)" + [regex]::Escape($name)), ("..\\common\\" + $name)
        # replace forward slash style
        $text = $text -replace ("(\.\.\/src\/)" + [regex]::Escape($name)), ("../common/" + $name)
        # also handle cases with different relative prefixes (e.g. ../src/ or src/ )
        $text = $text -replace ("(src\/)" + [regex]::Escape($name)), ("common/" + $name)
        $text = $text -replace ("(src\\)" + [regex]::Escape($name)), ("common\\" + $name)
    }

    # write back only if changed
    if ($text -ne (Get-Content -LiteralPath $file -Raw -Encoding UTF8)) {
        Set-Content -LiteralPath $file -Value $text -Encoding UTF8
        Write-Host "Updated: $file"
    } else {
        Write-Host "No change: $file"
    }
}