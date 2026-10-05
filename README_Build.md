# Open Carom3D Server 构建指南

## 概述
这是一个基于 C++ 的 Open Carom3D 多人在线游戏服务器项目。现在支持自动复制 MySQL Connector C++ DLL 文件。

## 系统要求
- Windows 10/11
- Visual Studio 2022 Community (包含 C++ 工具)
- CMake 3.15+
- MySQL Connector C++ 8.0 或 9.0

## 快速开始

### 1. 编译项目
```bash
# 简化编译脚本（推荐）
.\build_simple.bat

# 或使用 Ninja 编译脚本（高级用户）
.\build_ninja.bat
```

### 2. 运行服务器
```bash
.\run_server.bat
```

## 新功能：自动 DLL 复制

现在 CMakeLists.txt 已配置为自动复制必需的 MySQL Connector C++ DLL 文件到输出目录：

### 支持的版本
- **MySQL Connector C++ 9.0**（推荐）
  - `mysqlcppconn-10-vs14.dll`
  - `libssl-3-x64.dll`
  - `libcrypto-3-x64.dll`

- **MySQL Connector C++ 8.0**（兼容）
  - `mysqlcppconn-9-vs14.dll`

### 自动检测和复制
编译时会自动：
1. 检测系统中安装的 MySQL Connector C++ 版本
2. 复制相应的 DLL 文件到输出目录
3. 显示复制状态信息

## 编译脚本说明

### build_simple.bat（推荐）
- 简化的编译流程
- 自动设置 Visual Studio 环境
- 支持 Debug/Release 配置选择
- 自动清理旧缓存
- **自动复制 DLL 文件**

### build_ninja.bat（高级）
- 使用 Ninja 构建系统
- 更快的编译速度
- 支持并行编译

### 清理脚本
```bash
# 清理所有构建文件
.\clean_build.bat
```

## 服务器配置

### 默认端口
- **管理服务器**: 9882
- **游戏服务器（新手）**: 9883  
- **游戏服务器（自由）**: 9887

### 默认账户
| 用户名 | 密码 | 说明 |
|--------|------|------|
| admin | admin | 管理员 |
| test | test | 测试用户 |
| player1 | 123456 | 玩家1 |
| player2 | 123456 | 玩家2 |

### 配置文件位置
- `resources/host.txt` - 服务器主机配置
- `resources/accounts.txt` - 用户账户
- `resources/mysql.txt` - 数据库配置

## 目录结构
```
open-carom3d-server/
├── src/                    # 源代码
│   ├── business/          # 业务逻辑
│   ├── core/              # 核心功能
│   └── server_starter.cpp # 服务器启动入口
├── project/               # CMake 项目配置
│   └── CMakeLists.txt     # 构建配置（自动复制 DLL）
├── resources/             # 配置文件和资源
├── build_simple.bat       # 简化编译脚本
├── build_ninja.bat        # Ninja 编译脚本
├── clean_build.bat        # 清理脚本
└── run_server.bat         # 服务器启动脚本
```

## 编译输出
- **可执行文件**: `project\out\build\x64-Release\open_carom3d_server.exe`
- **DLL 文件**: 自动复制到与可执行文件相同目录
- **资源文件**: 自动复制到 `resources/` 子目录

## 故障排除

### 1. DLL 文件未找到
如果编译时显示警告"未找到 MySQL Connector C++ DLL 文件"：
- 确认已安装 MySQL Connector C++ 8.0 或 9.0
- 检查安装路径是否为标准路径
- 从安装目录的 lib64 手动复制 DLL 到可执行文件目录

### 2. 编译错误
```bash
# 清理构建缓存后重试
.\clean_build.bat
.\build_simple.bat
```

### 3. 服务器启动失败
- 检查端口是否被占用：`netstat -an | findstr "988"`
- 确认 DLL 文件在输出目录中
- 检查 `resources/` 目录是否完整

## 开发说明

### 修改 CMakeLists.txt 的好处
1. **自动化**: 编译时自动处理依赖
2. **版本兼容**: 支持多个 MySQL Connector C++ 版本
3. **简化部署**: 无需手动复制 DLL 文件
4. **跨平台**: 使用 CMake 标准命令

## 网络测试
```bash
# 检查服务器端口
netstat -an | findstr "988"

# 停止服务器
taskkill /f /im "open_carom3d_server.exe"
```

---
**注意**: 自动 DLL 复制功能现在已集成到 CMakeLists.txt 中，编译时会自动处理所有依赖文件。 