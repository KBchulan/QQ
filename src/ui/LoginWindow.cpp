#include "LoginWindow.h"
#include "../server/UserManager.h"
#include <iostream>
#include <SDL2/SDL_image.h>
#include <chrono>

LoginWindow::LoginWindow(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer)
    , width_(width)
    , height_(height)
    , isUsernameFocused_(false)
    , isPasswordFocused_(false)
    , loginSuccess_(false)
    , showRegister_(false)
    , networkManager_(std::make_shared<NetworkManager>()) {
    
    // 加载菜单图片
    SDL_Surface* surface = IMG_Load("resources/images/menu.jpg");
    if (surface) {
        menuTexture_ = SDL_CreateTextureFromSurface(renderer_, surface);
        if (menuTexture_) {
            // 计算目标大小（保持宽高比）
            double scale = std::min(
                (double)(width_ * 0.8) / surface->w,
                (double)(height_ / 3) / surface->h
            );
            int newWidth = static_cast<int>(surface->w * scale);
            int newHeight = static_cast<int>(surface->h * scale);
            
            // 计算居中位置
            int x = (width_ - newWidth) / 2;
            int y = 20;  // 距离顶部20像素
            menuRect_ = {x, y, newWidth, newHeight};
        }
        SDL_FreeSurface(surface);
    }
    
    // 尝试加载自定义字体
    font_ = TTF_OpenFont("resources/fonts/simple.ttf", 24);
    if (!font_) {
        std::cerr << "无法加载自定义字体: " << TTF_GetError() << std::endl;
        // 尝试加载备用字体
        const char* backupFonts[] = {
            "/usr/share/fonts/truetype/wqy/wqy-zenhei.ttc",
            "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc",
            "/usr/share/fonts/truetype/freefont/FreeSans.ttf"
        };
        
        for (const char* path : backupFonts) {
            font_ = TTF_OpenFont(path, 24);
            if (font_) {
                std::cout << "使用备用字体: " << path << std::endl;
                break;
            }
        }
        
        if (!font_) {
            std::cerr << "警告: 无法加载何字体，界面可能无法正确显示文字" << std::endl;
        }
    } else {
        std::cout << "成功加载自定义字体" << std::endl;
    }
    
    // 设置UI元素位置
    int boxWidth = 200;
    int boxHeight = 30;
    int labelWidth = 70;
    int spacing = 20; // 元素之间的间距
    
    // 计算起始Y坐标（在菜单图片下方）
    int startY = menuRect_.y + menuRect_.h + 50;
    
    // 计算水平居中的X坐标
    int centerX = width_ / 2;
    
    // 设置标签和输入框位置（竖直排列）
    usernameLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY, labelWidth, boxHeight};
    usernameBox_ = {centerX - boxWidth/2, startY, boxWidth, boxHeight};
    
    passwordLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY + boxHeight + spacing, labelWidth, boxHeight};
    passwordBox_ = {centerX - boxWidth/2, startY + boxHeight + spacing, boxWidth, boxHeight};
    
    // 设置按钮位置（竖直排列）
    loginButton_ = {centerX - boxWidth/2, startY + 2 * (boxHeight + spacing), boxWidth, 40};
    registerButton_ = {centerX - boxWidth/2, startY + 2 * (boxHeight + spacing) + 40 + spacing, boxWidth, 40};

    // 设置颜色
    bgColor_ = {240, 240, 240, 255};        // 浅灰色背景
    boxColor_ = {255, 255, 255, 255};       // 白色输入框
    activeBoxColor_ = {220, 220, 255, 255}; // 浅蓝色激活框
    textColor_ = {0, 0, 0, 255};            // 黑色文本
    buttonColor_ = {0, 120, 215, 255};      // 蓝色按钮
    buttonHoverColor_ = {0, 102, 204, 255}; // 深蓝色悬停
}

LoginWindow::~LoginWindow() {
    if (font_) {
        TTF_CloseFont(font_);
    }
    if (menuTexture_) {
        SDL_DestroyTexture(menuTexture_);
    }
}

void LoginWindow::render() {
    // 设置背景色
    SDL_SetRenderDrawColor(renderer_, bgColor_.r, bgColor_.g, bgColor_.b, bgColor_.a);
    SDL_RenderClear(renderer_);

    // 渲染菜单图片
    if (menuTexture_) {
        SDL_RenderCopy(renderer_, menuTexture_, nullptr, &menuRect_);
    }

    // 渲染标签
    renderText("用户名:", usernameLabel_, textColor_);
    renderText("密码:", passwordLabel_, textColor_);

    // 渲染输入框
    renderTextBox(usernameBox_, username_, false, isUsernameFocused_);
    renderTextBox(passwordBox_, password_, true, isPasswordFocused_);

    // 渲染按钮
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    bool loginHovered = isMouseOver(loginButton_, mouseX, mouseY);
    bool registerHovered = isMouseOver(registerButton_, mouseX, mouseY);
    
    renderButton(loginButton_, "登录", loginHovered);
    renderButton(registerButton_, "注册", registerHovered);
}

void LoginWindow::renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color) {
    if (!font_) return;

    SDL_Surface* surface = TTF_RenderUTF8_Blended(font_, text.c_str(), color);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
        if (texture) {
            SDL_RenderCopy(renderer_, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}

void LoginWindow::renderTextBox(const SDL_Rect& box, const std::string& text, bool isPassword, bool isFocused) {
    // 绘制输入框背景
    SDL_SetRenderDrawColor(renderer_, 
        isFocused ? activeBoxColor_.r : boxColor_.r,
        isFocused ? activeBoxColor_.g : boxColor_.g,
        isFocused ? activeBoxColor_.b : boxColor_.b,
        isFocused ? activeBoxColor_.a : boxColor_.a);
    SDL_RenderFillRect(renderer_, &box);

    // 绘制边框
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer_, &box);

    // 绘制文本
    std::string displayText = isPassword ? std::string(text.length(), '*') : text;
    if (!displayText.empty()) {
        // 计算文本大小
        int textWidth, textHeight;
        TTF_SizeText(font_, displayText.c_str(), &textWidth, &textHeight);
        
        // 创建文本渲染区域（左对齐，垂直居中）
        SDL_Rect textRect = {
            box.x + 5,  // 左边距5像素
            box.y + (box.h - textHeight) / 2,  // 垂直居中
            textWidth,  // 使用实际文本宽度
            textHeight  // 使用实际文本高度
        };
        
        // 渲染文本
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font_, displayText.c_str(), textColor_);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            if (texture) {
                SDL_RenderCopy(renderer_, texture, nullptr, &textRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }

    // 绘制光标
    if (isFocused) {
        // 计算光标位置
        int cursorX = box.x + 5; // 起始位置
        if (!displayText.empty()) {
            // 获取文本宽度
            int textWidth = 0;
            TTF_SizeText(font_, displayText.c_str(), &textWidth, nullptr);
            cursorX += textWidth;
        }
        
        // 闪烁效果
        static Uint32 lastTime = 0;
        static bool showCursor = true;
        Uint32 currentTime = SDL_GetTicks();
        
        if (currentTime - lastTime > 500) { // 每500ms切换一次
            showCursor = !showCursor;
            lastTime = currentTime;
        }
        
        if (showCursor) {
            SDL_Rect cursor = {cursorX, box.y + 5, 2, box.h - 10};
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer_, &cursor);
        }
    }
}

void LoginWindow::renderButton(const SDL_Rect& button, const std::string& text, bool isHovered) {
    // 绘制按钮背景
    SDL_SetRenderDrawColor(renderer_,
        isHovered ? buttonHoverColor_.r : buttonColor_.r,
        isHovered ? buttonHoverColor_.g : buttonColor_.g,
        isHovered ? buttonHoverColor_.b : buttonColor_.b,
        isHovered ? buttonHoverColor_.a : buttonColor_.a);
    SDL_RenderFillRect(renderer_, &button);

    // 计算文本大小
    int textWidth, textHeight;
    TTF_SizeText(font_, text.c_str(), &textWidth, &textHeight);

    // 创建居中的文本矩形
    SDL_Rect textRect = {
        button.x + (button.w - textWidth) / 2,  // 水平居中
        button.y + (button.h - textHeight) / 2, // 垂直居中
        textWidth,
        textHeight
    };

    // 绘制按钮文本
    SDL_Color textColor = {255, 255, 255, 255}; // 白色文本
    renderText(text, textRect, textColor);
}

void LoginWindow::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            
            isUsernameFocused_ = isMouseOver(usernameBox_, mouseX, mouseY);
            isPasswordFocused_ = isMouseOver(passwordBox_, mouseX, mouseY);
            
            if (isMouseOver(loginButton_, mouseX, mouseY)) {
                if (attemptLogin()) {
                    std::cout << "登录成功" << std::endl;
                } else {
                    std::cout << "登录失败" << std::endl;
                }
            } else if (isMouseOver(registerButton_, mouseX, mouseY)) {
                showRegister_ = true;
            }
            break;
        }
        case SDL_TEXTINPUT:
            handleTextInput(event.text.text);
            break;
        case SDL_KEYDOWN:
            handleKeyPress(event.key.keysym.sym);
            break;
    }
}

bool LoginWindow::isMouseOver(const SDL_Rect& rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void LoginWindow::handleTextInput(const std::string& text) {
    if (isUsernameFocused_ && username_.length() < 20) {
        username_ += text;
    } else if (isPasswordFocused_ && password_.length() < 20) {
        password_ += text;
    }
}

void LoginWindow::handleKeyPress(SDL_Keycode key) {
    if (key == SDLK_BACKSPACE) {
        if (isUsernameFocused_ && !username_.empty()) {
            username_.pop_back();
        } else if (isPasswordFocused_ && !password_.empty()) {
            password_.pop_back();
        }
    } else if (key == SDLK_RETURN) {
        attemptLogin();
    } else if (key == SDLK_TAB) {
        if (isUsernameFocused_) {
            isUsernameFocused_ = false;
            isPasswordFocused_ = true;
        } else if (isPasswordFocused_) {
            isPasswordFocused_ = false;
            isUsernameFocused_ = true;
        }
    }
}

bool LoginWindow::attemptLogin() {
    std::cout << "尝试登录，用户名: " << username_ << std::endl;
    
    try {
        // 验证输入
        if (username_.empty() || password_.empty()) {
            std::cout << "用户名或密码为空" << std::endl;
            return false;
        }

        // 连接到服务器
        if (!networkManager_->isConnected()) {
            if (!networkManager_->connect("127.0.0.1", 54321)) {
                std::cout << "连接服务器失败" << std::endl;
                return false;
            }
            std::cout << "成功连接到服务器" << std::endl;
        }

        // 发送登录请求
        Json::Value loginData;
        loginData["username"] = username_;
        loginData["password"] = password_;
        
        Message loginMsg(0, 0, loginData.toStyledString(), MessageType::LOGIN);
        networkManager_->sendMessage(loginMsg);
        std::cout << "已发送登录请求" << std::endl;
        
        // 等待服务器响应
        bool receivedResponse = false;
        bool loginSuccess = false;
        
        // 设置超时时间
        auto startTime = std::chrono::steady_clock::now();
        const auto timeout = std::chrono::seconds(5);
        
        while (!receivedResponse) {
            // 检查是否超时
            auto now = std::chrono::steady_clock::now();
            if (now - startTime > timeout) {
                std::cout << "登录超时" << std::endl;
                return false;
            }

            // 处理服务器响应
            if (networkManager_->hasMessage()) {
                Message response = networkManager_->getNextMessage();
                std::cout << "收到服务器响应，类型: " << static_cast<int>(response.getType()) << std::endl;
                
                if (response.getType() == MessageType::LOGIN_RESPONSE) {
                    Json::Value responseData;
                    Json::Reader reader;
                    if (reader.parse(response.getContent(), responseData)) {
                        receivedResponse = true;
                        loginSuccess = responseData["success"].asBool();
                        if (loginSuccess) {
                            // 创建用户对象
                            loggedInUser_ = std::make_shared<User>();
                            loggedInUser_->setUserId(responseData["userId"].asInt64());
                            loggedInUser_->setUsername(username_);
                            loginSuccess_ = true;
                            std::cout << "登录成功: " << username_ 
                                    << ", userId: " << loggedInUser_->getUserId() << std::endl;
                        } else {
                            std::cout << "登录失败: " << responseData["error"].asString() << std::endl;
                        }
                    } else {
                        std::cout << "解析响应数据失败" << std::endl;
                    }
                }
            }
            
            SDL_Delay(100);  // 避免过度占用CPU
        }
        
        return loginSuccess;
    } catch (const std::exception& e) {
        std::cout << "登录过程发生异常: " << e.what() << std::endl;
        return false;
    }
} 