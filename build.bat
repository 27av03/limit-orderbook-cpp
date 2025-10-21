@echo off
REM Windows build script for OrderBook project

echo Building OrderBook project...

REM Create build directory if it doesn't exist
if not exist build mkdir build
cd build

REM Configure with CMake
echo Configuring with CMake...
cmake .. -G "MinGW Makefiles"
if %ERRORLEVEL% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

REM Build the project
echo Building project...
mingw32-make
if %ERRORLEVEL% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Build completed successfully!
echo.
echo Running basic tests...
orderbook_tests.exe

echo.
echo Running demo...
orderbook_main.exe

cd ..
echo All done!
