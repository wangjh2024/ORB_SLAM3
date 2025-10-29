@echo off
cd /d "E:\VS2022Project\ORB_SLAM3"

echo Building ORB-SLAM3...
if not exist build mkdir build
cd build

cmake .. -DCMAKE_BUILD_TYPE=Release
if %errorlevel% neq 0 exit /b %errorlevel%

cmake --build . --config Release --parallel
if %errorlevel% neq 0 exit /b %errorlevel%

echo Build completed successfully!
dir lib
dir bin
pause