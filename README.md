# QQce - 即时通讯系统

## 前言

这是一个完全基于cursor的项目，也就是说是纯ai，作者只解决了一些语法问题，整体的设计完全由cursor完成

## 项目简介
QQce是一个基于SDL2的跨平台即时通讯系统，支持用户注册、登录、好友管理、即时通讯等功能。采用C++开发，使用Boost.Asio实现网络通信，MySQL进行数据持久化存储。

## 技术栈

| 类别 | 技术 | 版本 | 说明 |
|------|------|------|------|
| 图形界面 | SDL2 | 2.0+ | 跨平台图形库 |
| 网络通信 | Boost.Asio | 1.74+ | 异步IO网络库 |
| 数据库 | MySQL | 8.0+ | 数据持久化存储 |
| 加密 | OpenSSL | 3.0+ | 安全通信和密码加密 |
| 序列化 | JsonCpp | 1.9+ | JSON数据序列化 |
| 构建工具 | CMake | 3.10+ | 跨平台构建系统 |

## 环境配置

### Ubuntu/Debian依赖安装
```
sudo apt-get update
sudo apt-get install -y \
build-essential \
cmake \
libsdl2-dev \
libsdl2-ttf-dev \
libsdl2-image-dev \
libboost-all-dev \
libmysqlclient-dev \
libjsoncpp-dev \
libssl-dev
```

### 数据库配置
```
# sql
mysql -u root -p
CREATE DATABASE qq_db;
```

## 项目结构

├── CMakeLists.txt &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 项目构建配置  
├── src/ # 源代码目录  
│ &ensp;&ensp;&ensp;├── client/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 客户端代码  
│ &ensp;&ensp;&ensp;├── server/ &ensp;&ensp;&ensp;&ensp;&ensp;# 服务器代码  
│ &ensp;&ensp;&ensp;├── core/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 核心功能模块  
│ &ensp;&ensp;&ensp;└── ui/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 用户界面代码  
├── resources/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 资源文件  
│ &ensp;&ensp;&ensp;├── fonts/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 字体文件  
│ &ensp;&ensp;&ensp;└── images/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 图片资源  
├── database/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 数据库脚本  
├── config/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 配置文件  
├── scripts/ &ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;&ensp;# 启动脚本  
└── README.md # 项目文档  

## 功能模块

### 服务器模块
- 用户认证与会话管理
- 消息转发与存储
- 好友关系管理
- 数据持久化
- 心跳检测与连接维护

### 客户端模块
- 登录注册界面
- 主聊天界面
- 好友列表管理
- 消息发送接收
- 网络连接维护

### 核心功能
- 用户注册与登录
- 好友添加与管理
- 实时消息通信
- 离线消息存储
- 心跳保活机制

## 编译运行

1. 编译项目
```
mkdir build
cd build
cmake ..
make -j4
```

2. 启动服务器
```
cd ..
./scripts/start_server.sh
```

3. 启动客户端
```
./build/qq_client
```
## 使用说明

### 用户注册
1. 启动客户端程序
2. 点击注册按钮
3. 填写用户名、密码等信息
4. 提交注册信息

### 用户登录
1. 输入用户名和密码
2. 点击登录按钮
3. 等待验证完成

### 好友管理
1. 点击"添加好友"按钮
2. 输入对方用户名
3. 发送好友请求
4. 等待对方接受

### 聊天功能
1. 在好友列表选择好友
2. 在输入框输入消息
3. 点击发送或按Enter发送消息

## 开发文档

### 网络协议
- 消息格式：
  - 消息头：4字节长度
  - 消息体：JSON格式
- 心跳包：30秒一次
- 重连机制：自动重试

### 数据库设计
- users表：用户基本信息
- friendships表：好友关系
- messages表：聊天记录

### 代码规范
- 类名：大驼峰命名（ChatWindow）
- 方法名：小驼峰命名（sendMessage）
- 变量名：下划线分隔（user_id）
- 常量：全大写（MAX_CONNECTIONS）

## 注意事项
1. 确保MySQL服务正在运行
2. 检查数据库连接配置
3. 确保54321端口未被占用
4. 检查字体文件是否存在

## 调试信息
- 服务器日志：logs/server.log
- 客户端控制台输出
- 数据库查询：
  ```sql
  USE qq_db;
  SELECT * FROM users;
  SELECT * FROM friendships;
  ```

## 常见问题
1. 连接超时：检查服务器是否运行
2. 登录失败：验证用户名密码
3. 发送消息失败：检查网络连接
4. 界面显示异常：检查字体文件

## 后续开发计划
- [ ] 群聊功能
- [ ] 文件传输
- [ ] 语音通话
- [ ] 视频聊天
- [ ] 表情包系统

## 许可证
MIT License
