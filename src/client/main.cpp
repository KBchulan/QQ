#include <iostream>
#include "../ui/MainWindow.h"
#include "../core/NetworkManager.h"

int main(int argc, char* argv[]) {
    try {
        // 创建主窗口
        MainWindow window("QQ Clone", 800, 600);
        window.run();
    }
    catch (const std::exception& e) {
        std::cerr << "错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
} 