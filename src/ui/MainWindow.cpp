#include "MainWindow.h"
#include <iostream>

MainWindow::MainWindow(const std::string& title, int width, int height)
    : window_(nullptr)
    , renderer_(nullptr)
    , isLoggedIn_(false)
    , isRegistering_(false)
    , width_(width)
    , height_(height) {
    
    if (!init()) {
        throw std::runtime_error("Failed to initialize MainWindow");
    }

    loginWindow_ = std::make_unique<LoginWindow>(renderer_, width, height);
    chatWindow_ = std::make_unique<ChatWindow>(renderer_, width, height);
    registerWindow_ = std::make_unique<RegisterWindow>(renderer_, width, height);
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
    if (isLoggedIn_) {
        chatWindow_->handleEvent(event);
    } else if (isRegistering_) {
        registerWindow_->handleEvent(event);
        if (registerWindow_->isRegistrationSuccessful()) {
            switchToLogin();
        } else if (registerWindow_->shouldBackToLogin()) {
            switchToLogin();
        }
    } else {
        loginWindow_->handleEvent(event);
        if (loginWindow_->isLoginSuccessful()) {
            switchToChat();
        } else if (loginWindow_->shouldShowRegister()) {
            switchToRegister();
            loginWindow_->resetRegisterFlag();
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
    chatWindow_->setUser(loginWindow_->getLoggedInUser());
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