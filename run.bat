@echo off
echo Building CVM++...
g++ -std=c++17 -Wall -o cvm++.exe main.cpp lexer.cpp parser.cpp compiler.cpp vm.cpp debug.cpp

if %errorlevel% neq 0 (
    echo BUILD FAILED!
    pause
    exit
)

echo Build successful!
echo.
echo Usage:
echo   cvm++.exe calculator.cvm
echo   cvm++.exe -d calculator.cvm
echo.
cmd /k