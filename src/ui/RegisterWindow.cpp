#include "RegisterWindow.h"
#include <iostream>

RegisterWindow::RegisterWindow(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer)
    , width_(width)
    , height_(height)
    , isUsernameFocused_(false)
    , isPasswordFocused_(false)
    , isConfirmPasswordFocused_(false)
    , isNicknameFocused_(false)
    , registerSuccess_(false)
    , networkManager_(std::make_shared<NetworkManager>()) {
    
    // 加载字体
    font_ = TTF_OpenFont("resources/fonts/simple.ttf", 24);
    if (!font_) {
        std::cerr << "无法加载字体: " << TTF_GetError() << std::endl;
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
    }

    // 设置UI元素位置
    int boxWidth = 300;  // 输入框宽度
    int boxHeight = 40;  // 输入框高度
    int labelWidth = 100; // 标签宽度
    int spacing = 20;    // 间距
    
    // 计算起始位置（垂直居中）
    int startY = (height_ - (4 * (boxHeight + spacing) + boxHeight)) / 2;
    int centerX = width_ / 2;

    // 设置标签和输入框位置
    usernameLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY, labelWidth, boxHeight};
    usernameBox_ = {centerX - boxWidth/2, startY, boxWidth, boxHeight};
    
    passwordLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY + boxHeight + spacing, labelWidth, boxHeight};
    passwordBox_ = {centerX - boxWidth/2, startY + boxHeight + spacing, boxWidth, boxHeight};
    
    confirmPasswordLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY + 2 * (boxHeight + spacing), labelWidth, boxHeight};
    confirmPasswordBox_ = {centerX - boxWidth/2, startY + 2 * (boxHeight + spacing), boxWidth, boxHeight};
    
    nicknameLabel_ = {centerX - boxWidth/2 - spacing - labelWidth, startY + 3 * (boxHeight + spacing), labelWidth, boxHeight};
    nicknameBox_ = {centerX - boxWidth/2, startY + 3 * (boxHeight + spacing), boxWidth, boxHeight};

    // 设置按钮位置
    registerButton_ = {centerX - boxWidth/2, startY + 4 * (boxHeight + spacing), boxWidth/2 - 10, boxHeight};
    backButton_ = {centerX + 10, startY + 4 * (boxHeight + spacing), boxWidth/2 - 10, boxHeight};

    // 设置颜色
    bgColor_ = {240, 240, 240, 255};        // 浅灰色背景
    boxColor_ = {255, 255, 255, 255};       // 白色输入框
    activeBoxColor_ = {220, 220, 255, 255}; // 浅蓝色激活框
    textColor_ = {0, 0, 0, 255};            // 黑色文本
    buttonColor_ = {0, 120, 215, 255};      // 蓝色按钮
    buttonHoverColor_ = {0, 102, 204, 255}; // 深蓝色悬停
    errorColor_ = {255, 0, 0, 255};         // 红色错误提示
}

RegisterWindow::~RegisterWindow() {
    if (font_) {
        TTF_CloseFont(font_);
    }
}

void RegisterWindow::render() {
    // 设置背景色
    SDL_SetRenderDrawColor(renderer_, bgColor_.r, bgColor_.g, bgColor_.b, bgColor_.a);
    SDL_RenderClear(renderer_);

    // 渲染标题
    SDL_Rect titleRect = {width_ / 2 - 100, 20, 200, 40};
    renderText("用户注册", titleRect, textColor_);

    // 渲染标签和输入框
    renderText("账号:", usernameLabel_, textColor_);
    renderTextBox(usernameBox_, username_, false, isUsernameFocused_);
    
    renderText("密码:", passwordLabel_, textColor_);
    renderTextBox(passwordBox_, password_, true, isPasswordFocused_);
    
    renderText("确认密码:", confirmPasswordLabel_, textColor_);
    renderTextBox(confirmPasswordBox_, confirmPassword_, true, isConfirmPasswordFocused_);
    
    renderText("昵称:", nicknameLabel_, textColor_);
    renderTextBox(nicknameBox_, nickname_, false, isNicknameFocused_);

    // 渲染按钮
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    bool registerHovered = isMouseOver(registerButton_, mouseX, mouseY);
    bool backHovered = isMouseOver(backButton_, mouseX, mouseY);
    
    renderButton(registerButton_, "注册", registerHovered);
    renderButton(backButton_, "返回", backHovered);

    // 渲染错误信息
    if (!errorMessage_.empty()) {
        renderErrorMessage();
    }
}

void RegisterWindow::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            
            // 清除之前的错误信息
            errorMessage_.clear();
            
            // 检查输入框焦点
            isUsernameFocused_ = isMouseOver(usernameBox_, mouseX, mouseY);
            isPasswordFocused_ = isMouseOver(passwordBox_, mouseX, mouseY);
            isConfirmPasswordFocused_ = isMouseOver(confirmPasswordBox_, mouseX, mouseY);
            isNicknameFocused_ = isMouseOver(nicknameBox_, mouseX, mouseY);
            
            // 检查按点击
            if (isMouseOver(registerButton_, mouseX, mouseY)) {
                validateInput();
                if (errorMessage_.empty()) {
                    attemptRegister();
                }
            } else if (isMouseOver(backButton_, mouseX, mouseY)) {
                backToLogin_ = true;
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

void RegisterWindow::validateInput() {
    // 检查用户名
    if (username_.empty()) {
        errorMessage_ = "用户名不能为空";
        return;
    }
    if (username_.length() < 4) {
        errorMessage_ = "用户名长度至少4个字符";
        return;
    }
    
    // 检查密码强度
    if (password_.length() < 6) {
        errorMessage_ = "密码长度至少6个字符";
        return;
    }
    bool hasLetter = false, hasNumber = false;
    for (char c : password_) {
        if (isalpha(c)) hasLetter = true;
        if (isdigit(c)) hasNumber = true;
    }
    if (!hasLetter || !hasNumber) {
        errorMessage_ = "密码必须包含字母和数字";
        return;
    }
    
    // 检查密码确认
    if (password_ != confirmPassword_) {
        errorMessage_ = "两次输入的密码不一致";
        return;
    }
    
    // 昵称可选，但如果输入了要检查长度
    if (!nickname_.empty() && nickname_.length() < 2) {
        errorMessage_ = "昵称长度至少2个字符";
        return;
    }
}

bool RegisterWindow::attemptRegister() {
    std::cout << "尝试注册，用户名: " << username_ << std::endl;
    
    // 连接到服务器
    if (!networkManager_->isConnected()) {
        if (!networkManager_->connect("127.0.0.1", 54321)) {
            errorMessage_ = "无法连接到服务器，请检查服务器是否启动";
            std::cout << "连接服务器失败" << std::endl;
            return false;
        }
        std::cout << "成功连接到服务器" << std::endl;
    }

    // 构造注册消息
    Json::Value registerData;
    registerData["username"] = username_;
    registerData["password"] = password_;
    registerData["nickname"] = nickname_;
    
    Message registerMsg(0, 0, registerData.toStyledString(), MessageType::REGISTER);
    networkManager_->sendMessage(registerMsg);
    
    // 等待服务器响应
    bool receivedResponse = false;
    bool registrationSuccess = false;
    
    // 设置超时时间
    auto startTime = std::chrono::steady_clock::now();
    const auto timeout = std::chrono::seconds(5);
    
    while (!receivedResponse) {
        // 检查是否超时
        auto now = std::chrono::steady_clock::now();
        if (now - startTime > timeout) {
            errorMessage_ = "服务器响应超时";
            return false;
        }

        // 处理服务器响应
        if (networkManager_->hasMessage()) {
            Message response = networkManager_->getNextMessage();
            if (response.getType() == MessageType::REGISTER_RESPONSE) {
                Json::Value responseData;
                Json::Reader reader;
                if (reader.parse(response.getContent(), responseData)) {
                    receivedResponse = true;
                    registrationSuccess = responseData["success"].asBool();
                    if (!registrationSuccess) {
                        errorMessage_ = responseData["error"].asString();
                    }
                }
            }
        }
        
        SDL_Delay(100);  // 避免过度占用CPU
    }
    
    if (registrationSuccess) {
        showSuccessAnimation();
        registerSuccess_ = true;
        backToLogin_ = true;
        std::cout << "注册成功" << std::endl;
    }
    
    return registrationSuccess;
}

void RegisterWindow::renderErrorMessage() {
    if (errorMessage_.empty()) return;

    // 创建错误消息背景
    SDL_Rect errorBg = {
        width_ / 2 - 200,
        height_ - 100,
        400,
        60
    };

    // 绘制半透明背景
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 100);
    SDL_RenderFillRect(renderer_, &errorBg);

    // 绘制边框
    SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
    SDL_RenderDrawRect(renderer_, &errorBg);

    // 渲染错误图标（⚠）
    SDL_Rect iconRect = {errorBg.x + 10, errorBg.y + 15, 30, 30};
    renderText("⚠", iconRect, errorColor_);

    // 渲染错误消息
    SDL_Rect textRect = {
        errorBg.x + 50,
        errorBg.y + (errorBg.h - 24) / 2,
        errorBg.w - 60,
        24
    };
    renderText(errorMessage_, textRect, errorColor_);
    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_NONE);
}

void RegisterWindow::handleTextInput(const std::string& text) {
    const size_t maxLength = 20;  // 最大输入长度
    
    if (isUsernameFocused_ && username_.length() < maxLength) {
        username_ += text;
    } else if (isPasswordFocused_ && password_.length() < maxLength) {
        password_ += text;
    } else if (isConfirmPasswordFocused_ && confirmPassword_.length() < maxLength) {
        confirmPassword_ += text;
    } else if (isNicknameFocused_ && nickname_.length() < maxLength) {
        nickname_ += text;
    }
}

void RegisterWindow::handleKeyPress(SDL_Keycode key) {
    switch (key) {
        case SDLK_BACKSPACE:
            if (isUsernameFocused_ && !username_.empty()) {
                username_.pop_back();
            } else if (isPasswordFocused_ && !password_.empty()) {
                password_.pop_back();
            } else if (isConfirmPasswordFocused_ && !confirmPassword_.empty()) {
                confirmPassword_.pop_back();
            } else if (isNicknameFocused_ && !nickname_.empty()) {
                nickname_.pop_back();
            }
            break;
            
        case SDLK_TAB:
            // 在输入框之间循环切换
            if (isUsernameFocused_) {
                isUsernameFocused_ = false;
                isPasswordFocused_ = true;
            } else if (isPasswordFocused_) {
                isPasswordFocused_ = false;
                isConfirmPasswordFocused_ = true;
            } else if (isConfirmPasswordFocused_) {
                isConfirmPasswordFocused_ = false;
                isNicknameFocused_ = true;
            } else {
                isNicknameFocused_ = false;
                isUsernameFocused_ = true;
            }
            break;
            
        case SDLK_RETURN:
            validateInput();
            if (errorMessage_.empty()) {
                attemptRegister();
            }
            break;
    }
}

void RegisterWindow::renderTextBox(const SDL_Rect& box, const std::string& text, bool isPassword, bool isFocused) {
    // 绘制输入框阴影
    SDL_Rect shadowRect = {box.x + 2, box.y + 2, box.w, box.h};
    SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
    SDL_RenderFillRect(renderer_, &shadowRect);

    // 绘制输入框背景
    SDL_SetRenderDrawColor(renderer_, 
        isFocused ? activeBoxColor_.r : boxColor_.r,
        isFocused ? activeBoxColor_.g : boxColor_.g,
        isFocused ? activeBoxColor_.b : boxColor_.b,
        isFocused ? activeBoxColor_.a : boxColor_.a);
    SDL_RenderFillRect(renderer_, &box);

    // 绘制边框（双边框效果）
    if (isFocused) {
        // 外边框
        SDL_SetRenderDrawColor(renderer_, 0, 120, 215, 255);
        SDL_Rect outerBorder = {box.x - 2, box.y - 2, box.w + 4, box.h + 4};
        SDL_RenderDrawRect(renderer_, &outerBorder);
    }
    // 内边框
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

void RegisterWindow::renderButton(const SDL_Rect& button, const std::string& text, bool isHovered) {
    // 绘制按钮阴影
    SDL_Rect shadowRect = {button.x + 2, button.y + 2, button.w, button.h};
    SDL_SetRenderDrawColor(renderer_, 0, 80, 175, 255);
    SDL_RenderFillRect(renderer_, &shadowRect);

    // 绘制按钮主体
    SDL_Rect buttonBody = button;  // 使用完整的按钮区域
    if (isHovered) {
        buttonBody.y -= 2; // 悬停时按钮略微上移
    }
    
    // 绘制按钮背景（整个按钮区域）
    SDL_SetRenderDrawColor(renderer_,
        isHovered ? buttonHoverColor_.r : buttonColor_.r,
        isHovered ? buttonHoverColor_.g : buttonColor_.g,
        isHovered ? buttonHoverColor_.b : buttonColor_.b,
        isHovered ? buttonHoverColor_.a : buttonColor_.a);
    SDL_RenderFillRect(renderer_, &buttonBody);

    // 渲染文本
    SDL_Color textColor = {255, 255, 255, 255};
    int textWidth, textHeight;
    TTF_SizeText(font_, text.c_str(), &textWidth, &textHeight);
    SDL_Rect textRect = {
        buttonBody.x + (buttonBody.w - textWidth) / 2,
        buttonBody.y + (buttonBody.h - textHeight) / 2,
        textWidth,
        textHeight
    };
    renderText(text, textRect, textColor);
}

bool RegisterWindow::isMouseOver(const SDL_Rect& rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void RegisterWindow::renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color) {
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

void RegisterWindow::showSuccessAnimation() {
    // 创建成功消息背景
    SDL_Rect successBg = {
        width_ / 2 - 150,
        height_ / 2 - 50,
        300,
        100
    };

    for (int i = 0; i < 60; i++) {  // 动画持续1秒
        SDL_SetRenderDrawColor(renderer_, 240, 240, 240, 255);
        SDL_RenderClear(renderer_);

        // 绘制半透明背景
        SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 100);
        SDL_RenderFillRect(renderer_, &successBg);

        // 绘制边框
        SDL_SetRenderDrawColor(renderer_, 0, 255, 0, 255);
        SDL_RenderDrawRect(renderer_, &successBg);

        // 渲染成功图标（✓）
        SDL_Rect iconRect = {successBg.x + 10, successBg.y + 35, 30, 30};
        SDL_Color successColor = {0, 255, 0, 255};
        renderText("✓", iconRect, successColor);

        // 渲染成功消息
        SDL_Rect textRect = {
            successBg.x + 50,
            successBg.y + 35,
            successBg.w - 60,
            30
        };
        renderText("注册成功！", textRect, successColor);

        SDL_RenderPresent(renderer_);
        SDL_Delay(16);  // 约60fps
    }
}