# ROTS 云服务器

基于Node.js + MQTT + MySQL的云服务器，用于ROTS远程气味传输系统。

## 功能特性

- **设备管理**: 注册和监控发送端/接收端设备
- **MQTT通信**: 实时消息路由和设备通信
- **命令发送**: 通过Web界面发送气味命令
- **状态监控**: 实时显示设备状态和日志
- **数据存储**: MySQL数据库存储所有数据

## 快速开始

### 1. 安装依赖

```bash
cd cloud-server
npm install
```

### 2. 配置环境变量

创建 `.env` 文件：

```env
# 数据库配置
DB_HOST=localhost
DB_USER=root
DB_PASSWORD=your_password
DB_NAME=rots_db

# MQTT配置
MQTT_URL=mqtt://localhost:1883

# 服务器配置
PORT=3000
```

### 3. 启动服务

```bash
# 开发模式
npm run dev

# 生产模式
npm start
```

### 4. 访问管理界面

打开浏览器访问: http://localhost:3000

## API接口

### 设备管理

- `GET /api/devices` - 获取所有设备
- `POST /api/devices/register` - 注册设备

### 命令管理

- `POST /api/commands/send` - 发送气味命令
- `GET /api/commands/history` - 获取命令历史

### 日志管理

- `GET /api/devices/:deviceId/logs` - 获取设备日志

## MQTT主题

### 设备状态
- `rots/status/{device_id}` - 设备状态上报
- `rots/error/{device_id}` - 设备错误报告
- `rots/heartbeat/{device_id}` - 设备心跳

### 命令发送
- `rots/command/{device_id}` - 发送给特定设备的命令

## 数据库结构

### devices表
- id: 主键
- device_id: 设备ID
- device_type: 设备类型 (sender/receiver)
- location: 设备位置
- status: 设备状态 (online/offline/error)
- last_seen: 最后在线时间

### commands表
- id: 主键
- sender_id: 发送端设备ID
- receiver_id: 接收端设备ID
- odor_type: 气味类型
- intensity: 强度 (0-100)
- duration: 持续时间 (秒)
- status: 命令状态

### logs表
- id: 主键
- device_id: 设备ID
- log_type: 日志类型 (info/warning/error)
- message: 日志消息
- created_at: 创建时间

## 部署到阿里云

### 1. 创建ECS实例
- 选择Ubuntu 20.04 LTS
- 1核1GB内存
- 开放端口: 22, 80, 3000, 1883

### 2. 安装Node.js
```bash
curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash -
sudo apt-get install -y nodejs
```

### 3. 安装MySQL
```bash
sudo apt update
sudo apt install mysql-server
```

### 4. 安装Mosquitto MQTT
```bash
sudo apt install mosquitto mosquitto-clients
```

### 5. 部署应用
```bash
git clone your-repo
cd cloud-server
npm install
npm start
```

### 6. 使用PM2管理进程
```bash
sudo npm install -g pm2
pm2 start app.js --name rots-server
pm2 save
pm2 startup
```

## 监控和维护

### 查看服务状态
```bash
pm2 status
pm2 logs rots-server
```

### 重启服务
```bash
pm2 restart rots-server
```

### 数据库备份
```bash
mysqldump -u root -p rots_db > backup_$(date +%Y%m%d).sql
```

## 故障排除

### 常见问题

1. **MQTT连接失败**
   - 检查Mosquitto服务是否运行
   - 检查防火墙设置

2. **数据库连接失败**
   - 检查MySQL服务状态
   - 验证数据库配置

3. **设备无法连接**
   - 检查网络连接
   - 验证设备ID和认证

### 日志查看
```bash
# 应用日志
pm2 logs rots-server

# 系统日志
sudo journalctl -u mosquitto
sudo journalctl -u mysql
```

## 扩展功能

### 添加新功能
1. 在 `app.js` 中添加新的API路由
2. 在 `public/index.html` 中添加前端界面
3. 更新数据库结构（如需要）

### 性能优化
- 使用Redis缓存
- 数据库连接池
- 负载均衡

### 安全加固
- HTTPS证书
- API认证
- 防火墙配置
