#pragma once
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

class CaptchaGenerator {
private:
    static const int WIDTH = 200;
    static const int HEIGHT = 80;
    static const int CODE_LENGTH = 4;
    
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    std::string currentCode_;

public:
    CaptchaGenerator(SDL_Renderer* renderer);
    ~CaptchaGenerator();

    // 生成新的验证码
    std::string generateNewCode();
    
    // 渲染验证码图像
    SDL_Texture* renderCaptcha();
    
    // 验证用户输入
    bool verify(const std::string& input);

private:
    void drawNoise();
    void drawLines();
    std::string generateRandomCode();
}; 