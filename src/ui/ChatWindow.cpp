#include "ChatWindow.h"
#include <iostream>

ChatWindow::ChatWindow(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer)
    , width_(width)
    , height_(height)
    , isInputFocused_(false)
    , messageScrollOffset_(0)
    , friendListScrollOffset_(0)
    , networkManager_(std::make_shared<NetworkManager>()) {
    
    // 加载字体
    font_ = TTF_OpenFont("resources/fonts/simple.ttf", 16);
    if (!font_) {
        std::cerr << "无法加载字体: " << TTF_GetError() << std::endl;
    }

    // 设置UI布局
    int topBarHeight = 50;
    int friendListWidth = width_ / 4;
    int inputAreaHeight = 100;
    
    // 顶部信息栏
    topBar_ = {0, 0, width_, topBarHeight};
    
    // 左侧好友列表
    friendListArea_ = {0, topBarHeight, friendListWidth, height_ - topBarHeight};
    
    // 右侧聊天区域
    chatArea_ = {friendListWidth, topBarHeight, 
                 width_ - friendListWidth, height_ - topBarHeight - inputAreaHeight};
    
    // 消息显示区域
    messageArea_ = {friendListWidth, topBarHeight, 
                   width_ - friendListWidth, height_ - topBarHeight - inputAreaHeight};
    
    // 底部输入区域
    inputArea_ = {friendListWidth, height_ - inputAreaHeight, 
                 width_ - friendListWidth - 80, inputAreaHeight};
    
    // 发送按钮
    sendButton_ = {width_ - 80, height_ - inputAreaHeight, 
                  80, inputAreaHeight};

    // 设置颜色
    bgColor_ = {240, 240, 240, 255};        // 浅灰色背景
    topBarColor_ = {30, 144, 255, 255};     // 道奇蓝
    sidebarColor_ = {248, 248, 248, 255};   // 近白色
    inputBoxColor_ = {255, 255, 255, 255};  // 白色输入框
    activeBoxColor_ = {220, 220, 255, 255}; // 浅蓝色激活框
    textColor_ = {0, 0, 0, 255};            // 黑色文本
    buttonColor_ = {0, 120, 215, 255};      // 蓝色按钮
    buttonHoverColor_ = {0, 102, 204, 255}; // 深蓝色悬停
}

void ChatWindow::render() {
    // 计算布局
    float scale = std::min(width_ / 1024.0f, height_ / 768.0f);  // 基准分辨率
    
    // 顶部栏
    int topBarHeight = static_cast<int>(50 * scale);
    topBar_ = {0, 0, width_, topBarHeight};
    
    // 好友列表区域（左侧20%）
    int friendListWidth = static_cast<int>(width_ * 0.2f);
    friendListArea_ = {0, topBarHeight, friendListWidth, height_ - topBarHeight};
    
    // 聊天区域（右侧80%）
    int chatAreaWidth = width_ - friendListWidth;
    int inputAreaHeight = static_cast<int>(100 * scale);
    chatArea_ = {friendListWidth, topBarHeight, 
                 chatAreaWidth, height_ - topBarHeight - inputAreaHeight};
    
    // 输入区域
    int sendButtonWidth = static_cast<int>(80 * scale);
    inputArea_ = {friendListWidth, height_ - inputAreaHeight,
                 chatAreaWidth - sendButtonWidth, inputAreaHeight};
    
    // 发送按钮
    sendButton_ = {width_ - sendButtonWidth, height_ - inputAreaHeight,
                  sendButtonWidth, inputAreaHeight};
    
    // 添加好友按钮
    int addFriendButtonHeight = static_cast<int>(40 * scale);
    addFriendButton_ = {10, height_ - addFriendButtonHeight - 10,
                       friendListWidth - 20, addFriendButtonHeight};

    // 渲染各个区域
    renderTopBar();
    renderFriendList();
    if (selectedFriend_) {
        renderChatArea();
        renderInputArea();
    } else {
        renderWelcomeMessage();
    }
}

void ChatWindow::renderWelcomeMessage() {
    SDL_Color tipColor = {128, 128, 128, 255};
    float scale = std::min(width_ / 1024.0f, height_ / 768.0f);
    int fontSize = static_cast<int>(24 * scale);
    
    SDL_Rect tipRect = {
        chatArea_.x + (chatArea_.w - static_cast<int>(300 * scale)) / 2,
        chatArea_.y + (chatArea_.h - static_cast<int>(30 * scale)) / 2,
        static_cast<int>(300 * scale),
        static_cast<int>(30 * scale)
    };
    
    renderText("请选择一个好友开始聊天", tipRect, tipColor);
}

void ChatWindow::renderTopBar() {
    // 绘制顶部栏背景
    SDL_SetRenderDrawColor(renderer_, 
        topBarColor_.r, topBarColor_.g, topBarColor_.b, topBarColor_.a);
    SDL_RenderFillRect(renderer_, &topBar_);

    // 计算用户信息显示区域
    SDL_Rect userInfoRect = {
        topBar_.x + 20,  // 左边距20像素
        topBar_.y + (topBar_.h - 30) / 2,  // 垂直居中
        200,  // 固定宽度
        30    // 固定高度
    };

    // 构造显示文本
    std::string userInfo = "当前用户: ";
    if (currentUser_) {
        userInfo += currentUser_->getNickname().empty() ? 
                   currentUser_->getUsername() : 
                   currentUser_->getNickname();
    }

    // 使用白色渲染文本
    SDL_Color textColor = {255, 255, 255, 255};
    
    // 渲染文本（使用固定宽度的字符渲染）
    renderText(userInfo, userInfoRect, textColor);

    // 如果有选中的好友，显示聊天对象
    if (selectedFriend_) {
        SDL_Rect chatWithRect = {
            topBar_.x + topBar_.w / 2 - 100,  // 水平居中
            topBar_.y + (topBar_.h - 30) / 2,  // 垂直居中
            200,  // 固定宽度
            30    // 固定高度
        };
        
        std::string chatWithText = "正在与 " + 
            (selectedFriend_->getNickname().empty() ? 
             selectedFriend_->getUsername() : 
             selectedFriend_->getNickname()) + 
            " 聊天";
        
        renderText(chatWithText, chatWithRect, textColor);
    }
}

void ChatWindow::renderFriendList() {
    // 绘制好友列表背景
    SDL_SetRenderDrawColor(renderer_, 
        sidebarColor_.r, sidebarColor_.g, sidebarColor_.b, sidebarColor_.a);
    SDL_RenderFillRect(renderer_, &friendListArea_);

    // 渲染好友列表标题
    SDL_Rect titleRect = {friendListArea_.x + 10, friendListArea_.y + 5, 
                         friendListArea_.w - 20, 30};
    renderText("好友列表", titleRect, textColor_);

    // 渲染好友列表
    int y = friendListArea_.y + 40;
    for (const auto& friend_ : friendList_) {
        SDL_Rect friendRect = {friendListArea_.x + 5, y, 
                              friendListArea_.w - 10, 50};
        
        // 如果是当前选中的好友，绘制高亮背景
        if (selectedFriend_ && selectedFriend_->getUserId() == friend_->getUserId()) {
            SDL_SetRenderDrawColor(renderer_, 200, 200, 255, 255);
            SDL_RenderFillRect(renderer_, &friendRect);
        }

        // 渲染好友名称
        SDL_Rect nameRect = {friendRect.x + 5, y + 5, 
                            friendRect.w - 10, 20};
        renderText(friend_->getNickname(), nameRect, textColor_);

        // 渲染在线状态
        SDL_Rect statusRect = {friendRect.x + 5, y + 25, 
                              friendRect.w - 10, 20};
        SDL_Color statusColor = friend_->isOnline() ? 
            SDL_Color{0, 255, 0, 255} : SDL_Color{128, 128, 128, 255};
        renderText(friend_->isOnline() ? "在线" : "离线", 
                  statusRect, statusColor);

        // 渲染未读消息数
        if (unreadCounts_[friend_->getUserId()] > 0) {
            SDL_Rect unreadRect = {friendRect.x + friendRect.w - 25, y + 5, 20, 20};
            SDL_SetRenderDrawColor(renderer_, 255, 0, 0, 255);
            SDL_RenderFillRect(renderer_, &unreadRect);
            renderText(std::to_string(unreadCounts_[friend_->getUserId()]), 
                      unreadRect, {255, 255, 255, 255});
        }

        y += 55;
    }

    // 添加"添加好友"按钮
    addFriendButton_ = {
        friendListArea_.x + 10,
        friendListArea_.y + friendListArea_.h - 40,
        friendListArea_.w - 20,
        30
    };

    // 渲染按钮
    SDL_SetRenderDrawColor(renderer_, buttonColor_.r, buttonColor_.g, buttonColor_.b, buttonColor_.a);
    SDL_RenderFillRect(renderer_, &addFriendButton_);
    
    // 渲染按钮文本
    SDL_Color textColor = {255, 255, 255, 255};
    SDL_Rect textRect = {
        addFriendButton_.x + (addFriendButton_.w - 100) / 2,
        addFriendButton_.y + (addFriendButton_.h - 24) / 2,
        100,
        24
    };
    renderText("添加好友", textRect, textColor);
}

void ChatWindow::renderChatArea() {
    // 绘制聊天区域背景
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer_, &chatArea_);

    // 渲染消息历史
    int y = chatArea_.y + chatArea_.h - 30;
    for (auto it = chatHistory_.rbegin(); it != chatHistory_.rend(); ++it) {
        const Message& msg = *it;
        bool isSentByMe = (msg.getSenderId() == currentUser_->getUserId());
        
        // 计算消息气泡的大小
        int textWidth, textHeight;
        TTF_SizeText(font_, msg.getContent().c_str(), &textWidth, &textHeight);
        int bubbleWidth = std::min(textWidth + 20, chatArea_.w - 100);
        int bubbleHeight = textHeight + 20;

        // 计算气泡位置
        SDL_Rect bubbleRect;
        if (isSentByMe) {
            bubbleRect = {chatArea_.x + chatArea_.w - bubbleWidth - 20, 
                         y - bubbleHeight, bubbleWidth, bubbleHeight};
        } else {
            bubbleRect = {chatArea_.x + 20, y - bubbleHeight, 
                         bubbleWidth, bubbleHeight};
        }

        // 渲染消息气泡
        renderMessageBubble(msg, bubbleRect);

        y -= bubbleHeight + 10;
        if (y < chatArea_.y) break;
    }
}

void ChatWindow::renderMessageBubble(const Message& msg, const SDL_Rect& rect) {
    bool isSentByMe = (msg.getSenderId() == currentUser_->getUserId());
    
    // 绘制气泡背景
    SDL_Color bubbleColor = isSentByMe ? 
        SDL_Color{150, 200, 255, 255} : SDL_Color{220, 220, 220, 255};
    SDL_SetRenderDrawColor(renderer_, 
        bubbleColor.r, bubbleColor.g, bubbleColor.b, bubbleColor.a);
    
    // 绘制圆角矩形
    // TODO: 实现圆角矩形绘制
    SDL_RenderFillRect(renderer_, &rect);

    // 渲染消息内容
    SDL_Rect textRect = {
        rect.x + 10,
        rect.y + 10,
        rect.w - 20,
        rect.h - 20
    };
    renderText(msg.getContent(), textRect, textColor_);

    // 渲染消息状态（已发送/已读）
    if (isSentByMe) {
        SDL_Rect statusRect = {
            rect.x + rect.w - 20,
            rect.y + rect.h - 15,
            15,
            10
        };
        SDL_Color statusColor = messageRead_[msg.getMessageId()] ? 
            SDL_Color{0, 255, 0, 255} : 
            (messageDelivered_[msg.getMessageId()] ? 
             SDL_Color{0, 0, 255, 255} : SDL_Color{128, 128, 128, 255});
        renderText("✓", statusRect, statusColor);
    }
}

void ChatWindow::renderInputArea() {
    // 绘制输入框背景
    SDL_SetRenderDrawColor(renderer_,
        isInputFocused_ ? activeBoxColor_.r : inputBoxColor_.r,
        isInputFocused_ ? activeBoxColor_.g : inputBoxColor_.g,
        isInputFocused_ ? activeBoxColor_.b : inputBoxColor_.b,
        isInputFocused_ ? activeBoxColor_.a : inputBoxColor_.a);
    SDL_RenderFillRect(renderer_, &inputArea_);

    // 渲染输入的文本
    if (!inputText_.empty()) {
        SDL_Rect textRect = {
            inputArea_.x + 10,
            inputArea_.y + 5,
            inputArea_.w - 20,
            inputArea_.h - 10
        };
        renderText(inputText_, textRect, textColor_);
    }

    // 渲染光标
    if (isInputFocused_) {
        static Uint32 lastTime = 0;
        static bool showCursor = true;
        Uint32 currentTime = SDL_GetTicks();
        
        if (currentTime - lastTime > 500) {
            showCursor = !showCursor;
            lastTime = currentTime;
        }
        
        if (showCursor) {
            // 计算光标位置
            int cursorX = inputArea_.x + 10 + (inputText_.length() * 20); // 每个字符20像素宽
            SDL_Rect cursor = {
                cursorX,
                inputArea_.y + 5,
                2,
                inputArea_.h - 10
            };
            SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
            SDL_RenderFillRect(renderer_, &cursor);
        }
    }
}

void ChatWindow::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            
            // 检查好友列���点击
            if (isMouseOver(friendListArea_, mouseX, mouseY)) {
                int itemHeight = 55; // 每个好友项的高度
                int index = (mouseY - friendListArea_.y - 40) / itemHeight;
                if (index >= 0 && index < friendList_.size()) {
                    selectedFriend_ = friendList_[index];
                    // 清除未读消息计数
                    unreadCounts_[selectedFriend_->getUserId()] = 0;
                    // 加载聊天历史
                    loadChatHistory();
                }
            }
            
            // 检查输入框焦点
            isInputFocused_ = isMouseOver(inputArea_, mouseX, mouseY);
            
            // 检查发送按钮
            if (isMouseOver(sendButton_, mouseX, mouseY)) {
                sendMessage();
            }

            if (isMouseOver(addFriendButton_, mouseX, mouseY)) {
                showAddFriendDialog();
            }
            break;
        }
        case SDL_MOUSEWHEEL: {
            // 处理滚轮事件
            if (isMouseOver(messageArea_, event.wheel.x, event.wheel.y)) {
                messageScrollOffset_ -= event.wheel.y * 20;
                messageScrollOffset_ = std::max(0, messageScrollOffset_);
            } else if (isMouseOver(friendListArea_, event.wheel.x, event.wheel.y)) {
                friendListScrollOffset_ -= event.wheel.y * 20;
                friendListScrollOffset_ = std::max(0, friendListScrollOffset_);
            }
            break;
        }
        case SDL_TEXTINPUT:
            if (isInputFocused_) {
                handleTextInput(event.text.text);
            }
            break;
        case SDL_KEYDOWN:
            if (isInputFocused_) {
                handleKeyPress(event.key.keysym.sym);
            }
            break;
    }
}

void ChatWindow::sendMessage() {
    if (inputText_.empty() || !selectedFriend_) {
        return;
    }

    // 创建消息
    Message msg(currentUser_->getUserId(), 
               selectedFriend_->getUserId(),
               inputText_);
    
    // 发送消息
    networkManager_->sendMessage(msg);
    
    // 添加到本地聊天记录
    chatHistory_.push_back(msg);
    
    // 清空输入框
    inputText_.clear();
}

void ChatWindow::loadFriendList() {
    // 发送获取好友列表请求
    Json::Value request;
    request["userId"] = currentUser_->getUserId();
    
    Message msg(currentUser_->getUserId(), 0, 
               request.toStyledString(), MessageType::GET_FRIEND_LIST);
    networkManager_->sendMessage(msg);
}

void ChatWindow::loadChatHistory() {
    if (!selectedFriend_) return;
    
    // TODO: 从服务器获取聊天记录
    chatHistory_.clear();
    // 临时添加测试消息
    Message msg(currentUser_->getUserId(), selectedFriend_->getUserId(), 
               "这是一条测试消息");
    chatHistory_.push_back(msg);
}

void ChatWindow::setUser(std::shared_ptr<User> user) {
    currentUser_ = user;
    // 加载好友列表
    loadFriendList();
}

void ChatWindow::renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color) {
    if (!font_) return;

    // 量定义移到类的私有成员中
    static const int kCharWidth = 16;     // 英文字符宽度
    static const int kWideCharWidth = 24; // 中文字符宽度
    static const int kCharHeight = 24;    // 字符高度

    int x = rect.x;
    int y = rect.y + (rect.h - kCharHeight) / 2;  // 垂直居中

    std::string::const_iterator it = text.begin();
    while (it != text.end()) {
        int charLen = 1;
        if ((*it & 0x80) != 0) {  // UTF-8多字节字符
            if ((*it & 0xE0) == 0xC0) charLen = 2;
            else if ((*it & 0xF0) == 0xE0) charLen = 3;
            else if ((*it & 0xF8) == 0xF0) charLen = 4;
        }

        // 检查是否超出显示区域
        int charWidth = (charLen > 1) ? kWideCharWidth : kCharWidth;
        if (x + charWidth > rect.x + rect.w) break;

        // 渲染当前字符
        std::string currentChar(it, it + charLen);
        SDL_Surface* surface = TTF_RenderUTF8_Blended(font_, currentChar.c_str(), color);
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
            if (texture) {
                SDL_Rect charRect = {x, y, charWidth, kCharHeight};
                SDL_RenderCopy(renderer_, texture, nullptr, &charRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }

        x += charWidth;
        it += charLen;
    }
}

void ChatWindow::renderButton(const SDL_Rect& button, const std::string& text, bool isHovered) {
    // 绘制按钮背景
    SDL_SetRenderDrawColor(renderer_,
        isHovered ? buttonHoverColor_.r : buttonColor_.r,
        isHovered ? buttonHoverColor_.g : buttonColor_.g,
        isHovered ? buttonHoverColor_.b : buttonColor_.b,
        isHovered ? buttonHoverColor_.a : buttonColor_.a);
    SDL_RenderFillRect(renderer_, &button);

    // 计算文本位置（居中）
    int textWidth, textHeight;
    TTF_SizeText(font_, text.c_str(), &textWidth, &textHeight);
    SDL_Rect textRect = {
        button.x + (button.w - textWidth) / 2,
        button.y + (button.h - textHeight) / 2,
        textWidth,
        textHeight
    };

    // 渲染文本
    SDL_Color textColor = {255, 255, 255, 255}; // 白色文本
    renderText(text, textRect, textColor);
}

bool ChatWindow::isMouseOver(const SDL_Rect& rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void ChatWindow::handleTextInput(const std::string& text) {
    if (isInputFocused_ && inputText_.length() < 1000) { // 限制输入长度
        inputText_ += text;
    }
}

void ChatWindow::handleKeyPress(SDL_Keycode key) {
    if (!isInputFocused_) return;

    switch (key) {
        case SDLK_BACKSPACE:
            if (!inputText_.empty()) {
                inputText_.pop_back();
            }
            break;
        case SDLK_RETURN:
            sendMessage();
            break;
    }
}

ChatWindow::~ChatWindow() {
    if (font_) {
        TTF_CloseFont(font_);
    }
}

void ChatWindow::handleNewMessage(const Message& msg) {
    chatHistory_.push_back(msg);
    
    // 如果消息不是当前选中的好友发送的，增加未读计数
    if (!selectedFriend_ || msg.getSenderId() != selectedFriend_->getUserId()) {
        unreadCounts_[msg.getSenderId()]++;
    }
    
    // 标记消息为已送达
    messageDelivered_[msg.getMessageId()] = true;
}

void ChatWindow::refreshFriendList() {
    // 发送获取好友列表请求
    Json::Value request;
    request["userId"] = currentUser_->getUserId();
    
    Message msg(currentUser_->getUserId(), 0, 
               request.toStyledString(), MessageType::GET_FRIEND_LIST);
    networkManager_->sendMessage(msg);
}

void ChatWindow::updateFriendStatus(int64_t friendId, bool online) {
    for (auto& friend_ : friendList_) {
        if (friend_->getUserId() == friendId) {
            friend_->setOnline(online);
            break;
        }
    }
}

void ChatWindow::showAddFriendDialog() {
    // 创建输入框
    std::string username;
    bool running = true;
    bool confirmed = false;
    SDL_StartTextInput();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_RETURN) {
                    confirmed = true;
                    running = false;
                }
                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                else if (event.key.keysym.sym == SDLK_BACKSPACE && !username.empty()) {
                    username.pop_back();
                }
            }
            else if (event.type == SDL_TEXTINPUT) {
                username += event.text.text;
            }
        }

        // 渲染对话框背景
        SDL_SetRenderDrawColor(renderer_, 200, 200, 200, 255);
        SDL_Rect dialogBg = {width_/2 - 200, height_/2 - 100, 400, 200};
        SDL_RenderFillRect(renderer_, &dialogBg);

        // 渲染对话框边框
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer_, &dialogBg);

        // 渲染标题
        SDL_Rect titleRect = {dialogBg.x + 20, dialogBg.y + 20, 360, 30};
        renderText("添加好友", titleRect, {0, 0, 0, 255});

        // 渲染输入框背景
        SDL_Rect inputBg = {dialogBg.x + 20, dialogBg.y + 70, 360, 40};
        SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer_, &inputBg);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer_, &inputBg);

        // 渲染输入的文本
        SDL_Rect inputRect = {inputBg.x + 10, inputBg.y + 5, 340, 30};
        renderText(username, inputRect, {0, 0, 0, 255});

        // 渲染提示文本
        SDL_Rect hintRect = {dialogBg.x + 20, dialogBg.y + 120, 360, 30};
        renderText("输入要添加的好友用户名，按Enter确认", hintRect, {128, 128, 128, 255});

        SDL_RenderPresent(renderer_);
        SDL_Delay(16);  // 限制帧率
    }

    SDL_StopTextInput();

    if (confirmed && !username.empty()) {
        // 发送添加好友请求
        Json::Value request;
        request["from_user_id"] = currentUser_->getUserId();
        request["to_username"] = username;

        Message msg(currentUser_->getUserId(), 0, request.toStyledString(), MessageType::FRIEND_REQUEST);
        if (networkManager_->isConnected()) {
            networkManager_->sendMessage(msg);
            std::cout << "已发送添加好友请求: " << username << std::endl;
        } else {
            std::cerr << "未连接到服务器，无法添加好友" << std::endl;
        }
    }
}

void ChatWindow::handleMessage(const Message& msg) {
    switch (msg.getType()) {
        case MessageType::FRIEND_REQUEST_NOTIFICATION: {
            Json::Value notification;
            Json::Reader reader;
            if (reader.parse(msg.getContent(), notification)) {
                int64_t fromUserId = notification["from_user_id"].asInt64();
                std::string fromUsername = notification["from_username"].asString();
                
                // 显示好友请求对话框
                showFriendRequestDialog(fromUserId, fromUsername);
            }
            break;
        }
        // ... 其他消息处理 ...
    }
}

void ChatWindow::showFriendRequestDialog(int64_t fromUserId, const std::string& fromUsername) {
    // 创建好友请求对话框
    SDL_Rect dialogRect = {width_/2 - 200, height_/2 - 100, 400, 200};
    SDL_Rect acceptButton = {dialogRect.x + 50, dialogRect.y + 140, 100, 40};
    SDL_Rect rejectButton = {dialogRect.x + 250, dialogRect.y + 140, 100, 40};
    
    bool running = true;
    bool accepted = false;
    
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX = event.button.x;
                int mouseY = event.button.y;
                
                if (isMouseOver(acceptButton, mouseX, mouseY)) {
                    accepted = true;
                    running = false;
                }
                else if (isMouseOver(rejectButton, mouseX, mouseY)) {
                    running = false;
                }
            }
        }

        // 渲染对话框
        SDL_SetRenderDrawColor(renderer_, 240, 240, 240, 255);
        SDL_RenderFillRect(renderer_, &dialogRect);
        SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
        SDL_RenderDrawRect(renderer_, &dialogRect);

        // 渲染文本
        SDL_Rect textRect = {dialogRect.x + 20, dialogRect.y + 20, 360, 30};
        std::string message = fromUsername + " 请求添加您为好友";
        renderText(message, textRect, textColor_);

        // 渲染按钮
        renderButton(acceptButton, "接受", false);
        renderButton(rejectButton, "拒绝", false);

        SDL_RenderPresent(renderer_);
    }

    if (accepted) {
        // 发送接受好友请求的消息
        Json::Value response;
        response["from_user_id"] = currentUser_->getUserId();
        response["to_user_id"] = fromUserId;
        response["accepted"] = true;

        Message responseMsg(currentUser_->getUserId(), fromUserId,
                          response.toStyledString(),
                          MessageType::FRIEND_RESPONSE);
        networkManager_->sendMessage(responseMsg);
    }
}

// ... 继续实现其他方法 ...