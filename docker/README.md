**Docker Compose 配置已创建并启动**

**配置文件位置：** `/home/carom3d/docker-compose.yml`

**当前状态：**
- 容器正在运行
- 游戏服务端程序已成功启动
- 已初始化两个游戏服务器：
  - Novice game server (<SERVER_IP>:9883)
  - Free game server (<SERVER_IP>:9887)

**常用管理命令：**

```bash
# 查看容器状态
docker ps | grep carom3d

# 查看日志
docker logs carom3d-server
docker logs -f carom3d-server  # 实时查看日志

# 停止服务
cd /home/carom3d && docker-compose down

# 启动服务
cd /home/carom3d && docker-compose up -d

# 重启服务
cd /home/carom3d && docker-compose restart

# 进入容器
docker exec -it carom3d-server bash
```

**配置说明：**
- 使用 `network_mode: host` 以便服务端程序可以直接使用主机网络
- 程序目录已挂载到容器的 `/app` 目录
- 设置了 `restart: unless-stopped`，容器会自动重启
- 添加了 `WINEDEBUG=-all` 以减少 Wine 的调试输出

