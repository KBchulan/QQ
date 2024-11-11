#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(const std::string& title, int width, int height)
    : window_(nullptr)
    , renderer_(nullptr)
    , isLoggedIn_(false)
    , isRegistering_(false)
    , width_(width)
    , height_(height)
    , networkManager_(std::make_shared<NetworkManager>()) {
    
    if (!init()) {
        throw std::runtime_error("Failed to initialize MainWindow");
    }

    loginWindow_ = std::make_shared<LoginWindow>(renderer_, width, height);
    chatWindow_ = std::make_shared<ChatWindow>(renderer_, width, height);
    registerWindow_ = std::make_shared<RegisterWindow>(renderer_, width, height);
}

bool MainWindow::init() {
    if (!initSDL()) {
        return false;
    }
    return true;
}

bool MainWindow::initSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL初始化失败: " << SDL_GetError() << std::endl;
        return false;
    }

    window_ = SDL_CreateWindow(
        "whxchat",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        width_, height_,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!window_) {
        std::cerr << "窗口创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer_ = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer_) {
        std::cerr << "渲染器创建失败: " << SDL_GetError() << std::endl;
        return false;
    }

    // 初始化SDL_ttf
    if (TTF_Init() < 0) {
        std::cerr << "TTF初始化失败: " << TTF_GetError() << std::endl;
        return false;
    }

    // 初始化SDL_image
    int imgFlags = IMG_INIT_PNG;
    if (!(IMG_Init(imgFlags) & imgFlags)) {
        std::cerr << "SDL_image初始化失败: " << IMG_GetError() << std::endl;
        return false;
    }

    return true;
}

void MainWindow::run() {
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
            handleEvent(event);
        }

        render();
    }
}

void MainWindow::handleEvent(SDL_Event& event) {
    if (event.type == SDL_WINDOWEVENT) {
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
            // 获取新的窗口大小
            SDL_GetWindowSize(window_, &width_, &height_);
        }
    } else if (event.type == SDL_KEYDOWN) {
        // 按F11切换全屏
        if (event.key.keysym.sym == SDLK_F11) {
            static bool isFullscreen = false;
            isFullscreen = !isFullscreen;
            SDL_SetWindowFullscreen(window_, isFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }
    }

    // 处理其他事件
    if (isLoggedIn_) {
        chatWindow_->handleEvent(event);
    } else if (isRegistering_) {
        registerWindow_->handleEvent(event);
        if (registerWindow_->isRegistrationSuccessful() || 
            registerWindow_->shouldBackToLogin()) {
            switchToLogin();
        }
    } else {
        loginWindow_->handleEvent(event);
        if (loginWindow_->isLoginSuccessful()) {
            switchToChat();
        }
        if (loginWindow_->shouldShowRegister()) {
            switchToRegister();
        }
    }
}

void MainWindow::render() {
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderClear(renderer_);

    if (isLoggedIn_) {
        chatWindow_->render();
    } else if (isRegistering_) {
        registerWindow_->render();
    } else {
        loginWindow_->render();
    }

    SDL_RenderPresent(renderer_);
}

void MainWindow::switchToChat() {
    isLoggedIn_ = true;
    isRegistering_ = false;
    
    // 获取登录窗口的NetworkManager和用户信息
    networkManager_ = loginWindow_->getNetworkManager();
    auto user = loginWindow_->getLoggedInUser();
    
    // 设置聊天窗口的NetworkManager和用户信息
    chatWindow_->setNetworkManager(networkManager_);
    chatWindow_->setUser(user);
}

void MainWindow::switchToRegister() {
    isRegistering_ = true;
    isLoggedIn_ = false;
    registerWindow_->reset();
}

void MainWindow::switchToLogin() {
    isRegistering_ = false;
    isLoggedIn_ = false;
    loginWindow_->reset();
}

void MainWindow::cleanup() {
    if (renderer_) {
        SDL_DestroyRenderer(renderer_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

MainWindow::~MainWindow() {
    cleanup();
} 