#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include "../server/User.h"
#include "../core/Message.h"
#include "../core/NetworkManager.h"

class ChatWindow {
private:
    SDL_Renderer* renderer_;
    int width_;
    int height_;
    
    // UI元素
    SDL_Rect contactListArea_;    // 联系人列表区域
    SDL_Rect chatHistoryArea_;    // 聊天历史区域
    SDL_Rect inputArea_;          // 输入框区域
    SDL_Rect sendButton_;         // 发送按钮
    
    // 状态
    std::shared_ptr<User> currentUser_;
    std::shared_ptr<User> selectedContact_;
    std::string inputText_;
    std::vector<Message> chatHistory_;
    bool isInputFocused_;

    // 网络管理器
    NetworkManager networkManager_;

    // UI颜色
    SDL_Color bgColor_;
    SDL_Color sidebarColor_;
    SDL_Color inputBoxColor_;
    SDL_Color activeBoxColor_;
    SDL_Color textColor_;
    SDL_Color buttonColor_;
    SDL_Color buttonHoverColor_;

public:
    ChatWindow(SDL_Renderer* renderer, int width, int height);
    ~ChatWindow() = default;

    void handleEvent(SDL_Event& event);
    void render();
    void setUser(std::shared_ptr<User> user);
    void addMessage(const Message& msg);

private:
    void renderContactList();
    void renderChatHistory();
    void renderInputArea();
    void renderButton(const SDL_Rect& button, const std::string& text, bool isHovered);
    bool isMouseOver(const SDL_Rect& rect, int x, int y);
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    void sendMessage();
}; 