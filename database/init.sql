-- 创建数据库
CREATE DATABASE IF NOT EXISTS qq_db;
USE qq_db;

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    user_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) NOT NULL UNIQUE,
    password_hash VARCHAR(255) NOT NULL,
    nickname VARCHAR(50),
    avatar_url VARCHAR(255),
    status TINYINT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    last_login TIMESTAMP
);

-- 创建消息表
CREATE TABLE IF NOT EXISTS messages (
    msg_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    sender_id BIGINT,
    receiver_id BIGINT,
    content TEXT,
    msg_type TINYINT,
    status TINYINT DEFAULT 0,
    send_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sender_id) REFERENCES users(user_id),
    FOREIGN KEY (receiver_id) REFERENCES users(user_id)
);

-- 创建好友关系表
CREATE TABLE IF NOT EXISTS friendships (
    user_id BIGINT,
    friend_id BIGINT,
    status TINYINT DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (user_id, friend_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id),
    FOREIGN KEY (friend_id) REFERENCES users(user_id)
);

-- 创建群组表（使用chat_groups代替groups）
CREATE TABLE IF NOT EXISTS chat_groups (
    group_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    group_name VARCHAR(50) NOT NULL,
    owner_id BIGINT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (owner_id) REFERENCES users(user_id)
);

-- 创建群组成员表
CREATE TABLE IF NOT EXISTS group_members (
    group_id BIGINT,
    user_id BIGINT,
    role TINYINT DEFAULT 0,
    joined_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (group_id, user_id),
    FOREIGN KEY (group_id) REFERENCES chat_groups(group_id),
    FOREIGN KEY (user_id) REFERENCES users(user_id)
); 