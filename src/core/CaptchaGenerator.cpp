#include "CaptchaGenerator.h"
#include <random>
#include <algorithm>

CaptchaGenerator::CaptchaGenerator(SDL_Renderer* renderer) 
    : renderer_(renderer) {
    font_ = TTF_OpenFont("/usr/share/fonts/truetype/freefont/FreeSans.ttf", 36);
    if (!font_) {
        throw std::runtime_error("Failed to load font for captcha");
    }
}

CaptchaGenerator::~CaptchaGenerator() {
    if (font_) TTF_CloseFont(font_);
}

std::string CaptchaGenerator::generateNewCode() {
    currentCode_ = generateRandomCode();
    return currentCode_;
}

SDL_Texture* CaptchaGenerator::renderCaptcha() {
    // 创建表面
    SDL_Surface* surface = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
    if (!surface) return nullptr;

    // 设置白色背景
    SDL_FillRect(surface, nullptr, SDL_MapRGB(surface->format, 255, 255, 255));

    // 添加干扰线
    drawLines(surface);

    // 添加噪点
    drawNoise(surface);

    // 渲染文本
    SDL_Color textColor = {0, 0, 0, 255};
    SDL_Surface* textSurface = TTF_RenderText_Blended(font_, currentCode_.c_str(), textColor);
    if (textSurface) {
        SDL_Rect destRect = {
            (WIDTH - textSurface->w) / 2,
            (HEIGHT - textSurface->h) / 2,
            textSurface->w,
            textSurface->h
        };
        SDL_BlitSurface(textSurface, nullptr, surface, &destRect);
        SDL_FreeSurface(textSurface);
    }

    // 创建纹理
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
    SDL_FreeSurface(surface);
    return texture;
}

void CaptchaGenerator::drawLines(SDL_Surface* surface) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, WIDTH);
    std::uniform_int_distribution<> disY(0, HEIGHT);
    
    for (int i = 0; i < 5; ++i) {
        SDL_Point points[2] = {
            {disX(gen), disY(gen)},
            {disX(gen), disY(gen)}
        };
        // 使用SDL_gfx库画线
        // lineRGBA(surface, points[0].x, points[0].y, points[1].x, points[1].y,
        //          0, 0, 0, 128);
    }
}

void CaptchaGenerator::drawNoise(SDL_Surface* surface) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disX(0, WIDTH);
    std::uniform_int_distribution<> disY(0, HEIGHT);
    
    for (int i = 0; i < 100; ++i) {
        int x = disX(gen);
        int y = disY(gen);
        SDL_Rect point = {x, y, 2, 2};
        SDL_FillRect(surface, &point, SDL_MapRGB(surface->format, 0, 0, 0));
    }
}

std::string CaptchaGenerator::generateRandomCode() {
    const std::string chars = "23456789ABCDEFGHJKLMNPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, chars.length() - 1);
    
    std::string code;
    for (int i = 0; i < CODE_LENGTH; ++i) {
        code += chars[dis(gen)];
    }
    return code;
}

bool CaptchaGenerator::verify(const std::string& input) {
    return input == currentCode_;
} 