@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo   Open Carom3D Server Ninja 编译脚本
echo ===========================================

REM 检查 Visual Studio 2022
set VS_FOUND=0
set VS_PATH=""
set MSVC_PATH=""

if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    set VS_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat"
    set MSVC_PATH="C:/Program Files/Microsoft Visual Studio/2022/Community/VC/Tools/MSVC"
    set VS_FOUND=1
    echo 找到 Visual Studio 2022 Community
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    set VS_PATH="C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat"
    set MSVC_PATH="C:/Program Files/Microsoft Visual Studio/2022/Professional/VC/Tools/MSVC"
    set VS_FOUND=1
    echo 找到 Visual Studio 2022 Professional
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" (
    set VS_PATH="C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat"
    set MSVC_PATH="C:/Program Files/Microsoft Visual Studio/2022/Enterprise/VC/Tools/MSVC"
    set VS_FOUND=1
    echo 找到 Visual Studio 2022 Enterprise
)

if !VS_FOUND! == 0 (
    echo 错误：未找到 Visual Studio 2022 安装
    echo 此脚本需要 Visual Studio 2022
    pause
    exit /b 1
)

echo.
echo 选择编译配置：
echo 1. Debug 版本
echo 2. Release 版本
echo 3. 退出

set /p choice="请输入选择 (1-3): "

if "%choice%"=="1" (
    set BUILD_TYPE=Debug
    set CMAKE_CONFIG=x64-Debug
    echo 选择了 Debug 版本
) else if "%choice%"=="2" (
    set BUILD_TYPE=RelWithDebInfo
    set CMAKE_CONFIG=x64-Release
    echo 选择了 Release 版本
) else if "%choice%"=="3" (
    echo 退出编译
    exit /b 0
) else (
    echo 无效选择，默认使用 Debug 版本
    set BUILD_TYPE=Debug
    set CMAKE_CONFIG=x64-Debug
)

echo.
echo 正在设置 Visual Studio 环境...
call %VS_PATH%

REM 获取 MSVC 版本号（最新版本）
for /f "delims=" %%i in ('dir %MSVC_PATH% /b /ad /o-n 2^>nul') do (
    set MSVC_VERSION=%%i
    goto found_msvc
)
:found_msvc
if "%MSVC_VERSION%"=="" (
    echo 错误：无法找到 MSVC 版本
    echo 检查路径：%MSVC_PATH%
    pause
    exit /b 1
)

echo 使用 MSVC 版本：%MSVC_VERSION%

REM 设置编译器路径（去掉外层引号）
set MSVC_PATH_CLEAN=%MSVC_PATH:"=%
set C_COMPILER=%MSVC_PATH_CLEAN%/%MSVC_VERSION%/bin/Hostx64/x64/cl.exe
set CXX_COMPILER=%MSVC_PATH_CLEAN%/%MSVC_VERSION%/bin/Hostx64/x64/cl.exe

echo C 编译器：%C_COMPILER%
echo C++ 编译器：%CXX_COMPILER%

REM 检查 CMake
set CMAKE_PATH=""
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    echo 使用 VS2022 内置 CMake
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" (
    set CMAKE_PATH="C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
    echo 使用 VS2022 内置 CMake
) else (
    cmake --version >nul 2>&1
    if errorlevel 1 (
        echo 错误：未找到 CMake
        pause
        exit /b 1
    ) else (
        set CMAKE_PATH=cmake
        echo 使用系统 CMake
    )
)

REM 设置 Ninja 路径
set NINJA_PATH=""
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" (
    set NINJA_PATH="C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    echo 使用 VS2022 内置 Ninja
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe" (
    set NINJA_PATH="C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
    echo 使用 VS2022 内置 Ninja
) else (
    ninja --version >nul 2>&1
    if errorlevel 1 (
        echo 警告：未找到 Ninja，将使用默认生成器
        set NINJA_PATH=""
    ) else (
        set NINJA_PATH=ninja
        echo 使用系统 Ninja
    )
)

echo.
echo 正在进入项目目录...
cd project

REM 清理旧的构建缓存（模仿 VS2022 行为）
set BUILD_DIR=out\build\%CMAKE_CONFIG%
if exist "%BUILD_DIR%\CMakeCache.txt" (
    echo 删除旧的 CMake 缓存...
    del "%BUILD_DIR%\CMakeCache.txt" 2>nul
)
if exist "%BUILD_DIR%\CMakeFiles" (
    echo 删除旧的 CMakeFiles 目录...
    rmdir /s /q "%BUILD_DIR%\CMakeFiles" 2>nul
)

REM 创建构建目录
if not exist "%BUILD_DIR%" (
    echo 创建构建目录...
    mkdir "%BUILD_DIR%"
)

echo.
echo 正在配置项目...
echo 工作目录：%CD%\%BUILD_DIR%

REM 配置 CMake（模仿 VS2022 的命令）
if "%NINJA_PATH%"=="" (
    %CMAKE_PATH% -S . -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=%C_COMPILER% -DCMAKE_CXX_COMPILER=%CXX_COMPILER%
) else (
    %CMAKE_PATH% -G "Ninja" -S . -B "%BUILD_DIR%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_C_COMPILER=%C_COMPILER% -DCMAKE_CXX_COMPILER=%CXX_COMPILER% -DCMAKE_MAKE_PROGRAM=%NINJA_PATH%
)

if errorlevel 1 (
    echo 错误：CMake 配置失败
    cd ..
    pause
    exit /b 1
)

echo.
echo 正在编译项目...
%CMAKE_PATH% --build "%BUILD_DIR%" --config %BUILD_TYPE%

if errorlevel 1 (
    echo 编译失败！请检查代码错误
    cd ..
    pause
    exit /b 1
)

echo.
echo ===========================================
echo   编译完成！
echo ===========================================

REM 查找可执行文件
set EXE_FOUND=0
if exist "%BUILD_DIR%\open_carom3d_server.exe" (
    echo 可执行文件：project\%BUILD_DIR%\open_carom3d_server.exe
    set EXE_FOUND=1
) else if exist "%BUILD_DIR%\%BUILD_TYPE%\open_carom3d_server.exe" (
    echo 可执行文件：project\%BUILD_DIR%\%BUILD_TYPE%\open_carom3d_server.exe
    set EXE_FOUND=1
)

if !EXE_FOUND! == 0 (
    echo 警告：未找到生成的可执行文件
    echo 请检查 project\%BUILD_DIR% 目录
)

cd ..
echo.
echo 按任意键退出...
pause 