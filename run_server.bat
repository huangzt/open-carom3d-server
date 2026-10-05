@echo off
setlocal EnableDelayedExpansion

echo ===========================================
echo   Open Carom3D Server 启动脚本
echo ===========================================

REM 查找编译后的可执行文件
set EXE_FOUND=0
set EXE_PATH=""

echo 正在查找可执行文件...

REM 检查 Release 版本（优先）
if exist "project\out\build\x64-Release\open_carom3d_server.exe" (
    set EXE_PATH="project\out\build\x64-Release\open_carom3d_server.exe"
    set EXE_FOUND=1
    echo 找到 Release 版本
) else if exist "project\out\build\x64-Debug\open_carom3d_server.exe" (
    set EXE_PATH="project\out\build\x64-Debug\open_carom3d_server.exe"
    set EXE_FOUND=1
    echo 找到 Debug 版本
)

if !EXE_FOUND! == 0 (
    echo 错误：未找到编译后的可执行文件
    echo 请先运行 .\build_simple.bat 编译项目
    pause
    exit /b 1
)

echo.
echo 正在检查配置文件...

REM 检查并创建 resources 目录
if not exist "resources" (
    echo 创建 resources 目录...
    mkdir resources
)

REM 检查 host.txt 文件
if not exist "resources\host.txt" (
    echo 创建默认主机配置...
    echo 127.0.0.1> resources\host.txt
    echo 已创建：resources\host.txt ^(127.0.0.1^)
)

REM 检查 accounts.txt 文件
if not exist "resources\accounts.txt" (
    echo 创建默认账户文件...
    echo account,password,maxForce,minForce,cueCode> resources\accounts.txt
    echo admin,admin,150,1,3000>> resources\accounts.txt
    echo test,test,150,1,3000>> resources\accounts.txt
    echo player1,123456,150,1,3000>> resources\accounts.txt
    echo player2,123456,150,1,3000>> resources\accounts.txt
    echo 已创建：resources\accounts.txt
    echo   默认账户：admin/admin, test/test, player1/123456, player2/123456
)

echo.
echo 配置信息：
type resources\host.txt | findstr /v "^$" > temp.txt 2>nul && set /p HOST_IP=< temp.txt && del temp.txt 2>nul
if "%HOST_IP%"=="" set HOST_IP=127.0.0.1
echo   服务器 IP：%HOST_IP%
echo   管理端口：9882
echo   游戏端口：9883 ^(新手^), 9887 ^(自由^)
if exist "resources\accounts.txt" (
    echo   账户配置：resources\accounts.txt
)

echo.
echo ===========================================
echo   启动服务器
echo ===========================================
echo 可执行文件：%EXE_PATH%
echo.
echo 提示：
echo   - 服务器启动后，输入 'quit' 并按回车可退出
echo   - 客户端需要修改 hosts 文件连接服务器
echo   - 更多信息请查看 README.txt
echo.

REM 检查 DLL 文件
echo.
echo 检查必需的 DLL 文件...
set DLL_CHECK_DIR=%EXE_PATH:"=%
for %%F in ("%DLL_CHECK_DIR%") do set DLL_CHECK_DIR=%%~dpF

if not exist "%DLL_CHECK_DIR%mysqlcppconn-10-vs14.dll" (
    if not exist "%DLL_CHECK_DIR%mysqlcppconn-9-vs14.dll" (
        echo.
        echo 警告：未找到 MySQL Connector C++ DLL 文件
        echo 服务器可能无法正常启动
        echo.
        echo 解决方案：
        echo 1. 重新运行 .\build_simple.bat（编译后会自动复制 DLL 文件）
        echo 2. 手动复制以下文件到可执行文件目录：
        echo    - mysqlcppconn-10-vs14.dll
        echo    - libssl-3-x64.dll
        echo    - libcrypto-3-x64.dll
        echo.
        set /p continue="是否继续启动服务器？ (y/n): "
        if /i not "%continue%"=="y" (
            echo 已取消启动
            pause
            exit /b 0
        )
    ) else (
        echo ? 找到 mysqlcppconn-9-vs14.dll
    )
) else (
    echo ? 找到 mysqlcppconn-10-vs14.dll
)

REM 启动服务器
echo.
echo 正在启动服务器...
%EXE_PATH%

echo.
echo 服务器已停止
pause 