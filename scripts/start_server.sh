#!/bin/bash

# 检查项目根目录
PROJECT_ROOT=$(pwd)

# 检查配置文件
if [ ! -f "${PROJECT_ROOT}/config/server_config.json" ]; then
    echo "错误: 找不到配置文件 config/server_config.json"
    exit 1
fi

# 创建日志目录
mkdir -p ${PROJECT_ROOT}/logs

# 从配置文件获取数据库信息
DB_USER=$(grep -o '"user": *"[^"]*"' config/server_config.json | cut -d'"' -f4)
DB_PASS=$(grep -o '"password": *"[^"]*"' config/server_config.json | cut -d'"' -f4)
DB_NAME=$(grep -o '"name": *"[^"]*"' config/server_config.json | cut -d'"' -f4)

# 检查数据库
echo "正在检查数据库..."
mysql -u"$DB_USER" --password="$DB_PASS" -e "USE $DB_NAME" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "初始化数据库..."
    # 使用heredoc创建临时SQL文件
    TMP_SQL=$(mktemp)
    cat > "$TMP_SQL" <<EOF
CREATE DATABASE IF NOT EXISTS $DB_NAME;
USE $DB_NAME;
$(cat ${PROJECT_ROOT}/database/init.sql)
EOF
    
    # 执行SQL文件
    mysql -u"$DB_USER" --password="$DB_PASS" < "$TMP_SQL"
    if [ $? -eq 0 ]; then
        echo "数据库初始化成功"
        rm "$TMP_SQL"
    else
        echo "数据库初始化失败"
        rm "$TMP_SQL"
        exit 1
    fi
fi

# 检查编译
if [ ! -f "${PROJECT_ROOT}/build/qq_server" ]; then
    echo "编译项目..."
    cd ${PROJECT_ROOT}/build
    cmake ..
    make
    cd ${PROJECT_ROOT}
fi

# 启动服务器
cd ${PROJECT_ROOT}
./build/qq_server