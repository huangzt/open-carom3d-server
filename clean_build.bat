@echo off
echo ===========================================
echo   Open Carom3D Server 构建清理脚本
echo ===========================================

echo 正在清理构建缓存和临时文件...

REM 清理项目构建目录
if exist "project\out" (
    echo 删除 project\out 目录...
    rmdir /s /q "project\out" 2>nul
    if errorlevel 1 (
        echo 警告：部分文件可能正在使用中，无法删除
    ) else (
        echo ? 已删除 project\out 目录
    )
)

REM 清理 CMake 缓存文件
if exist "project\CMakeCache.txt" (
    echo 删除 CMakeCache.txt...
    del "project\CMakeCache.txt" 2>nul
    echo ? 已删除 CMakeCache.txt
)

REM 清理 CMakeFiles 目录
if exist "project\CMakeFiles" (
    echo 删除 CMakeFiles 目录...
    rmdir /s /q "project\CMakeFiles" 2>nul
    echo ? 已删除 CMakeFiles 目录
)

REM 清理 Visual Studio 临时文件
if exist ".vs" (
    echo 删除 .vs 目录...
    rmdir /s /q ".vs" 2>nul
    if not errorlevel 1 (
        echo ? 已删除 .vs 目录
    )
)

REM 清理其他临时文件
if exist "*.tmp" (
    echo 删除临时文件...
    del "*.tmp" 2>nul
)

echo.
echo ===========================================
echo   清理完成！
echo ===========================================
echo.
echo 现在可以重新运行编译脚本：
echo   .\build_ninja.bat
echo.
pause 