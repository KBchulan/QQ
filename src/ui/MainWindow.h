#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include <memory>
#include "LoginWindow.h"
#include "ChatWindow.h"
#include "RegisterWindow.h"
#include "../core/NetworkManager.h"

class MainWindow {
private:
    SDL_Window* window_;
    SDL_Renderer* renderer_;
    std::shared_ptr<LoginWindow> loginWindow_;
    std::shared_ptr<ChatWindow> chatWindow_;
    std::shared_ptr<RegisterWindow> registerWindow_;
    std::shared_ptr<NetworkManager> networkManager_;
    bool isLoggedIn_;
    bool isRegistering_;
    int width_;
    int height_;

public:
    MainWindow(const std::string& title, int width, int height);
    ~MainWindow();

    bool init();
    void run();
    void handleEvent(SDL_Event& event);
    void render();

private:
    bool initSDL();
    void cleanup();
    void switchToChat();
    void switchToRegister();
    void switchToLogin();
}; 