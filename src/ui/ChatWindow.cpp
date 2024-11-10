#include "ChatWindow.h"
#include "../core/NetworkManager.h"
#include <iostream>

ChatWindow::ChatWindow(SDL_Renderer* renderer, int width, int height)
    : renderer_(renderer)
    , width_(width)
    , height_(height)
    , isInputFocused_(false) {
    
    // 设置UI布局
    contactListArea_ = {0, 0, width_ / 4, height_};
    chatHistoryArea_ = {width_ / 4, 0, 3 * width_ / 4, height_ - 100};
    inputArea_ = {width_ / 4, height_ - 100, 3 * width_ / 4 - 100, 80};
    sendButton_ = {width_ - 100, height_ - 100, 80, 80};

    // 设置颜色
    bgColor_ = {240, 240, 240, 255};        // 浅灰色背景
    sidebarColor_ = {230, 230, 230, 255};   // 侧边栏颜色
    inputBoxColor_ = {255, 255, 255, 255};  // 白色输入框
    activeBoxColor_ = {220, 220, 255, 255}; // 浅蓝色激活框
    textColor_ = {0, 0, 0, 255};            // 黑色文本
    buttonColor_ = {0, 120, 215, 255};      // 蓝色按钮
    buttonHoverColor_ = {0, 102, 204, 255}; // 深蓝色悬停
}

void ChatWindow::render() {
    // 设置背景色
    SDL_SetRenderDrawColor(renderer_, bgColor_.r, bgColor_.g, bgColor_.b, bgColor_.a);
    SDL_RenderClear(renderer_);

    // 渲染联系人列表
    renderContactList();

    // 渲染聊天历史
    renderChatHistory();

    // 渲染输入区域
    renderInputArea();

    // 渲染发送按钮
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    bool sendHovered = isMouseOver(sendButton_, mouseX, mouseY);
    renderButton(sendButton_, "发送", sendHovered);
}

void ChatWindow::renderContactList() {
    // 绘制侧边栏背景
    SDL_SetRenderDrawColor(renderer_, 
        sidebarColor_.r, sidebarColor_.g, sidebarColor_.b, sidebarColor_.a);
    SDL_RenderFillRect(renderer_, &contactListArea_);

    // TODO: 渲染联系人列表
}

void ChatWindow::renderChatHistory() {
    // 绘制聊天历史区域背景
    SDL_SetRenderDrawColor(renderer_, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer_, &chatHistoryArea_);

    // TODO: 渲染聊天消息
}

void ChatWindow::renderInputArea() {
    // 绘制输入框背景
    SDL_SetRenderDrawColor(renderer_,
        isInputFocused_ ? activeBoxColor_.r : inputBoxColor_.r,
        isInputFocused_ ? activeBoxColor_.g : inputBoxColor_.g,
        isInputFocused_ ? activeBoxColor_.b : inputBoxColor_.b,
        isInputFocused_ ? activeBoxColor_.a : inputBoxColor_.a);
    SDL_RenderFillRect(renderer_, &inputArea_);

    // 绘制边框
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer_, &inputArea_);

    // TODO: 渲染输入的文本
}

void ChatWindow::handleEvent(SDL_Event& event) {
    switch (event.type) {
        case SDL_MOUSEBUTTONDOWN: {
            int mouseX = event.button.x;
            int mouseY = event.button.y;
            
            isInputFocused_ = isMouseOver(inputArea_, mouseX, mouseY);
            
            if (isMouseOver(sendButton_, mouseX, mouseY)) {
                sendMessage();
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

void ChatWindow::handleTextInput(const std::string& text) {
    if (inputText_.length() < 1000) { // 限制输入长度
        inputText_ += text;
        std::cout << "输入文本: " << inputText_ << std::endl;
    }
}

void ChatWindow::handleKeyPress(SDL_Keycode key) {
    if (key == SDLK_BACKSPACE && !inputText_.empty()) {
        inputText_.pop_back();
    } else if (key == SDLK_RETURN) {
        sendMessage();
    }
}

void ChatWindow::sendMessage() {
    if (inputText_.empty() || !selectedContact_) {
        std::cout << "无法发送消息：文本为空或未选择联系人" << std::endl;
        return;
    }

    Message msg(currentUser_->getUserId(),
               selectedContact_->getUserId(),
               inputText_);
    
    networkManager_.sendMessage(msg);
    chatHistory_.push_back(msg);
    std::cout << "发送消息: " << inputText_ << std::endl;
    inputText_.clear();
}

void ChatWindow::setUser(std::shared_ptr<User> user) {
    currentUser_ = user;
    std::cout << "当前用户设置为: " << user->getUsername() << std::endl;
}

void ChatWindow::addMessage(const Message& msg) {
    chatHistory_.push_back(msg);
    std::cout << "收到新消息，来自: " << msg.getSenderId() << std::endl;
}

bool ChatWindow::isMouseOver(const SDL_Rect& rect, int x, int y) {
    return (x >= rect.x && x <= rect.x + rect.w &&
            y >= rect.y && y <= rect.y + rect.h);
}

void ChatWindow::renderButton(const SDL_Rect& button, const std::string& text, bool isHovered) {
    // 绘制按钮背景
    SDL_SetRenderDrawColor(renderer_,
        isHovered ? buttonHoverColor_.r : buttonColor_.r,
        isHovered ? buttonHoverColor_.g : buttonColor_.g,
        isHovered ? buttonHoverColor_.b : buttonColor_.b,
        isHovered ? buttonHoverColor_.a : buttonColor_.a);
    SDL_RenderFillRect(renderer_, &button);

    // TODO: 使用SDL_ttf渲染文本
    // 暂时不渲染文本
} 