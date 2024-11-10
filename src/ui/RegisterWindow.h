#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <memory>
#include "../core/NetworkManager.h"

class RegisterWindow {
private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    int width_;
    int height_;
    
    // UI元素
    SDL_Rect usernameBox_;
    SDL_Rect passwordBox_;
    SDL_Rect confirmPasswordBox_;
    SDL_Rect nicknameBox_;
    SDL_Rect registerButton_;
    SDL_Rect backButton_;
    
    // 输入状态
    std::string username_;
    std::string password_;
    std::string confirmPassword_;
    std::string nickname_;
    bool isUsernameFocused_;
    bool isPasswordFocused_;
    bool isConfirmPasswordFocused_;
    bool isNicknameFocused_;
    bool registerSuccess_;

    // 网络管理器（改为智能指针）
    std::shared_ptr<NetworkManager> networkManager_;

    // UI颜色
    SDL_Color bgColor_;
    SDL_Color boxColor_;
    SDL_Color activeBoxColor_;
    SDL_Color textColor_;
    SDL_Color buttonColor_;
    SDL_Color buttonHoverColor_;
    SDL_Color errorColor_;

    // 错误信息
    std::string errorMessage_;

    bool backToLogin_;
    SDL_Rect usernameLabel_;
    SDL_Rect passwordLabel_;
    SDL_Rect confirmPasswordLabel_;
    SDL_Rect nicknameLabel_;

public:
    RegisterWindow(SDL_Renderer* renderer, int width, int height);
    ~RegisterWindow();

    void handleEvent(SDL_Event& event);
    void render();
    bool isRegistrationSuccessful() const { return registerSuccess_; }
    bool shouldBackToLogin() const { return backToLogin_; }
    void reset() {
        registerSuccess_ = false;
        backToLogin_ = false;
        errorMessage_.clear();
        username_.clear();
        password_.clear();
        confirmPassword_.clear();
        nickname_.clear();
        isUsernameFocused_ = false;
        isPasswordFocused_ = false;
        isConfirmPasswordFocused_ = false;
        isNicknameFocused_ = false;
    }

private:
    void renderTextBox(const SDL_Rect& box, const std::string& text, bool isPassword, bool isFocused);
    void renderButton(const SDL_Rect& button, const std::string& text, bool isHovered);
    void renderErrorMessage();
    bool attemptRegister();
    void validateInput();
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    bool isMouseOver(const SDL_Rect& rect, int x, int y);
    void renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color);
    void showSuccessAnimation();
}; 