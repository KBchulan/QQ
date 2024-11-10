#include <iostream>
#include "ui/MainWindow.h"
#include "core/NetworkManager.h"

int main(int argc, char* argv[]) {
    try {
        NetworkManager networkManager;
        MainWindow mainWindow;
        
        // 连接到服务器
        if (!networkManager.connect("localhost", 8888)) {
            std::cerr << "无法连接到服务器" << std::endl;
            return 1;
        }

        // 启动主窗口
        mainWindow.render();
        
    } catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 