@echo off
setlocal
where cmake >nul 2>nul || (echo CMake not found. Install CMake and add it to PATH.& pause & exit /b 1)
if "%QT_DIR%"=="" (
  echo Set QT_DIR to your Qt MSVC kit path, for example:
  echo set QT_DIR=C:\Qt\6.8.3\msvc2022_64
  echo.
  pause
  exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%"
if errorlevel 1 goto failed
cmake --build build --config Release
if errorlevel 1 goto failed
echo.
echo Build complete. Executable: build\Release\SkillOpti.exe
echo To run on another PC, deploy Qt DLLs with windeployqt.
pause
exit /b 0
:failed
echo Build failed. Check the error output above.
pause
exit /b 1
