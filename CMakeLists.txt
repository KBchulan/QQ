cmake_minimum_required(VERSION 3.10)
project(QQClone)

# 添加pkg-config支持
include(FindPkgConfig)

# 查找必要的包
find_package(Boost REQUIRED COMPONENTS system)
find_package(OpenSSL REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
find_package(OpenCV REQUIRED)

# MySQL查找
find_package(PkgConfig REQUIRED)
pkg_check_modules(MYSQL REQUIRED mysqlclient)

# 设置MySQL包含目录
include_directories(${MYSQL_INCLUDE_DIRS})

# 添加服务器源文件
add_executable(qq_server
    src/server/main.cpp
    src/server/Server.cpp
    src/server/Session.cpp
    src/server/Encryption.cpp
    src/server/DatabaseManager.cpp
    src/server/Logger.cpp
    src/server/Config.cpp
    src/server/UserManager.cpp
    src/server/MessageManager.cpp
    src/server/FriendManager.cpp
    src/core/Message.cpp
)

# 添加客户端源文件
add_executable(qq_client
    src/client/main.cpp
    src/ui/MainWindow.cpp
    src/ui/LoginWindow.cpp
    src/ui/ChatWindow.cpp
    src/ui/RegisterWindow.cpp
    src/core/NetworkManager.cpp
    src/core/Message.cpp
    src/core/User.cpp
)

# 链接服务器依赖
target_link_libraries(qq_server
    PRIVATE
    Boost::system
    OpenSSL::SSL
    OpenSSL::Crypto
    ${MYSQL_LIBRARIES}
    jsoncpp
)

# 链接客户端依赖
target_link_libraries(qq_client
    PRIVATE
    Boost::system
    SDL2::SDL2
    SDL2_image::SDL2_image
    SDL2_ttf::SDL2_ttf
    jsoncpp
    ${OpenCV_LIBS}
)

# 添加包含目录
target_include_directories(qq_server
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${MYSQL_INCLUDE_DIRS}
)

target_include_directories(qq_client
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
)

# 打印MySQL信息用于调试
message(STATUS "MySQL Include Dirs: ${MYSQL_INCLUDE_DIRS}")
message(STATUS "MySQL Libraries: ${MYSQL_LIBRARIES}") 