param(
    [string]$ExePath = "build\bin\x64\Debug\MiniCXX.exe"
)

$ErrorActionPreference = "Continue"

if (-not (Test-Path $ExePath)) {
    Write-Error "Compiler executable not found: $ExePath"
    exit 1
}

$failed = 0

function Run-Case {
    param(
        [string]$Path,
        [bool]$ShouldPass
    )

    $output = & $ExePath $Path 2>&1 | Out-String
    $passed = ($LASTEXITCODE -eq 0)

    if ($passed -ne $ShouldPass) {
        $script:failed += 1
        $expected = if ($ShouldPass) { "pass" } else { "fail" }
        $actual = if ($passed) { "pass" } else { "fail" }
        Write-Host "[FAIL] $Path expected=$expected actual=$actual"
        if ($output) {
            Write-Host $output
        }
        return
    }

    $expectPath = "$Path.expect"
    if ($ShouldPass -and (Test-Path $expectPath)) {
        $missing = @()
        Get-Content $expectPath | ForEach-Object {
            $line = $_.Trim()
            if ($line.Length -gt 0 -and -not $output.Contains($line)) {
                $missing += $line
            }
        }
        if ($missing.Count -gt 0) {
            $script:failed += 1
            Write-Host "[FAIL] $Path missing expected output:"
            $missing | ForEach-Object { Write-Host "  $_" }
            return
        }
    }

    if ($passed -eq $ShouldPass) {
        Write-Host "[PASS] $Path"
    }
}

Get-ChildItem "tests\sysy\valid" -Filter "*.sy" | Sort-Object Name | ForEach-Object {
    Run-Case $_.FullName $true
}

Get-ChildItem "tests\sysy\invalid" -Filter "*.sy" | Sort-Object Name | ForEach-Object {
    Run-Case $_.FullName $false
}

if ($failed -ne 0) {
    Write-Error "$failed regression case(s) failed"
    exit 1
}

Write-Host "All regression cases passed."
