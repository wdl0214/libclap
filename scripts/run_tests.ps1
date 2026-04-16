# PowerShell test runner for Windows
$BuildDir = "build"
$TestDir = "$BuildDir\tests"

Write-Host "=== Running libclap Tests ===" -ForegroundColor Yellow
Write-Host ""

$Passed = 0
$Failed = 0

# Unit Tests
Write-Host "[Unit Tests]" -ForegroundColor Yellow
$UnitTests = @(
    "test_clap", "test_allocator", "test_arena", "test_buffer", "test_trie",
    "test_error", "test_parser", "test_argument", "test_namespace",
    "test_validator", "test_convert", "test_actions", "test_mutex",
    "test_dependency", "test_subparser", "test_find", "test_formatter",
    "test_tokenizer"
)

foreach ($test in $UnitTests) {
    $testPath = "$TestDir\$test.exe"
    if (Test-Path $testPath) {
        Write-Host "  $test ... " -NoNewline
        $result = Start-Process -FilePath $testPath -NoNewWindow -Wait -PassThru
        if ($result.ExitCode -eq 0) {
            Write-Host "PASSED" -ForegroundColor Green
            $Passed++
        } else {
            Write-Host "FAILED" -ForegroundColor Red
            $Failed++
        }
    }
}

# Summary
Write-Host ""
Write-Host "=== Summary ===" -ForegroundColor Yellow
Write-Host "Passed: $Passed" -ForegroundColor Green
Write-Host "Failed: $Failed" -ForegroundColor Red

if ($Failed -eq 0) {
    Write-Host ""
    Write-Host "All tests passed!" -ForegroundColor Green
    exit 0
} else {
    exit 1
}