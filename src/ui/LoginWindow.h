#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <opencv4/opencv2/opencv.hpp>
#include <string>
#include <memory>
#include "../core/User.h"
#include "../core/NetworkManager.h"

class LoginWindow {
private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    SDL_Texture* menuTexture_;
    SDL_Rect menuRect_;
    int width_;
    int height_;
    
    // UI元素
    SDL_Rect usernameLabel_;
    SDL_Rect passwordLabel_;
    SDL_Rect usernameBox_;
    SDL_Rect passwordBox_;
    SDL_Rect loginButton_;
    SDL_Rect registerButton_;
    
    // 输入状态
    std::string username_;
    std::string password_;
    bool isUsernameFocused_;
    bool isPasswordFocused_;
    bool loginSuccess_;
    std::shared_ptr<User> loggedInUser_;

    // 网络管理器
    std::shared_ptr<NetworkManager> networkManager_;

    // UI颜色
    SDL_Color bgColor_;
    SDL_Color boxColor_;
    SDL_Color activeBoxColor_;
    SDL_Color textColor_;
    SDL_Color buttonColor_;
    SDL_Color buttonHoverColor_;

    bool showRegister_;

    std::string errorMessage_;
    SDL_Color errorColor_;

public:
    LoginWindow(SDL_Renderer* renderer, int width, int height);
    ~LoginWindow();

    void handleEvent(SDL_Event& event);
    void render();
    bool isLoginSuccessful() const { return loginSuccess_; }
    std::shared_ptr<User> getLoggedInUser() const { return loggedInUser_; }
    bool shouldShowRegister() const { return showRegister_; }
    void resetRegisterFlag() { showRegister_ = false; }
    void reset() {
        showRegister_ = false;
        loginSuccess_ = false;
    }

    std::shared_ptr<NetworkManager> getNetworkManager() { return networkManager_; }

private:
    void renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color);
    void renderTextBox(const SDL_Rect& box, const std::string& text, bool isPassword, bool isFocused);
    void renderButton(const SDL_Rect& button, const std::string& text, bool isHovered);
    bool attemptLogin();
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    bool isMouseOver(const SDL_Rect& rect, int x, int y);
}; 