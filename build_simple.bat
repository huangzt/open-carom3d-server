@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo   Open Carom3D Server 简化编译脚本
echo ===========================================

REM 设置 Visual Studio 环境（强制使用 x64）
if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" (
    echo 设置 Visual Studio 2022 Community 环境 ^(x64^)...
    call "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\Tools\VsDevCmd.bat" -arch=x64
) else if exist "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" (
    echo 设置 Visual Studio 2022 Professional 环境 ^(x64^)...
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\Tools\VsDevCmd.bat" -arch=x64
) else (
    echo 错误：未找到 Visual Studio 2022
    pause
    exit /b 1
)

echo.
echo 选择编译配置：
echo 1. Debug 版本
echo 2. Release 版本

set /p choice="请输入选择 (1-2): "

if "%choice%"=="1" (
    set BUILD_TYPE=Debug
    set CMAKE_CONFIG=x64-Debug
    echo 选择了 Debug 版本
) else (
    set BUILD_TYPE=RelWithDebInfo  
    set CMAKE_CONFIG=x64-Release
    echo 选择了 Release 版本
)

echo.
echo 进入项目目录...
cd project

echo.
echo 清理旧缓存...
if exist "out\build\%CMAKE_CONFIG%\CMakeCache.txt" (
    del "out\build\%CMAKE_CONFIG%\CMakeCache.txt"
)
if exist "out\build\%CMAKE_CONFIG%\CMakeFiles" (
    rmdir /s /q "out\build\%CMAKE_CONFIG%\CMakeFiles"
)

echo.
echo 创建构建目录...
if not exist "out\build\%CMAKE_CONFIG%" (
    mkdir "out\build\%CMAKE_CONFIG%"
)

echo.
echo 配置项目...
cmake -G "Ninja" -S . -B "out\build\%CMAKE_CONFIG%" -DCMAKE_BUILD_TYPE=%BUILD_TYPE%

if errorlevel 1 (
    echo 配置失败！
    cd ..
    pause
    exit /b 1
)

echo.
echo 编译项目...
cmake --build "out\build\%CMAKE_CONFIG%" --config %BUILD_TYPE%

if errorlevel 1 (
    echo 编译失败！
    cd ..
    pause
    exit /b 1
)

echo.
echo ===========================================
echo   编译成功！
echo ===========================================

if exist "out\build\%CMAKE_CONFIG%\open_carom3d_server.exe" (
    echo 可执行文件：project\out\build\%CMAKE_CONFIG%\open_carom3d_server.exe
    
    echo.
    echo 正在复制必需的 DLL 文件...
    
    REM 复制 MySQL DLL
    set DLL_COPIED=0
    if exist "C:\Program Files\MySQL\MySQL Connector C++ 9.0\lib64\mysqlcppconn-10-vs14.dll" (
        copy "C:\Program Files\MySQL\MySQL Connector C++ 9.0\lib64\mysqlcppconn-10-vs14.dll" "out\build\%CMAKE_CONFIG%\" >nul 2>&1
        if not errorlevel 1 (
            echo ? 复制 mysqlcppconn-10-vs14.dll 成功
            set DLL_COPIED=1
        )
        
        copy "C:\Program Files\MySQL\MySQL Connector C++ 9.0\lib64\libssl-3-x64.dll" "out\build\%CMAKE_CONFIG%\" >nul 2>&1
        copy "C:\Program Files\MySQL\MySQL Connector C++ 9.0\lib64\libcrypto-3-x64.dll" "out\build\%CMAKE_CONFIG%\" >nul 2>&1
    ) else if exist "C:\Program Files\MySQL\MySQL Connector C++ 8.0\lib64\mysqlcppconn-9-vs14.dll" (
        copy "C:\Program Files\MySQL\MySQL Connector C++ 8.0\lib64\mysqlcppconn-9-vs14.dll" "out\build\%CMAKE_CONFIG%\" >nul 2>&1
        if not errorlevel 1 (
            echo ? 复制 mysqlcppconn-9-vs14.dll 成功
            set DLL_COPIED=1
        )
    )
    
    if !DLL_COPIED! == 0 (
        echo ? 未能自动复制 MySQL DLL 文件
        echo   请从 MySQL Connector C++ 安装目录的 lib64 手动复制 DLL 到可执行文件目录
    )
    
) else (
    echo 查找可执行文件...
    dir "out\build\%CMAKE_CONFIG%\*.exe" 2>nul
)

cd ..
echo.
echo 提示：
echo   - 运行服务器：.\run_server.bat
echo.
pause 