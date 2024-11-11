#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>
#include <vector>
#include <memory>
#include <chrono>
#include "../core/User.h"
#include "../core/Message.h"
#include "../core/NetworkManager.h"

class ChatWindow {
private:
    SDL_Renderer* renderer_;
    TTF_Font* font_;
    int width_;
    int height_;
    
    // UI布局区域
    SDL_Rect topBar_;           // 顶部信息栏
    SDL_Rect friendListArea_;   // 左侧好友列表
    SDL_Rect chatArea_;         // 右侧聊天区域
    SDL_Rect messageArea_;      // 消息显示区域
    SDL_Rect inputArea_;        // 底部输入框
    SDL_Rect sendButton_;       // 发送按钮
    SDL_Rect addFriendButton_;

    // 用户信息
    std::shared_ptr<User> currentUser_;
    std::shared_ptr<User> selectedFriend_;
    std::vector<std::shared_ptr<User>> friendList_;
    
    // 消息相关
    std::string inputText_;
    std::vector<Message> chatHistory_;
    bool isInputFocused_;
    
    // 滚动相关
    int messageScrollOffset_;
    int friendListScrollOffset_;

    // UI颜色
    SDL_Color bgColor_;
    SDL_Color topBarColor_;
    SDL_Color sidebarColor_;
    SDL_Color inputBoxColor_;
    SDL_Color activeBoxColor_;
    SDL_Color textColor_;
    SDL_Color buttonColor_;
    SDL_Color buttonHoverColor_;

    // 网络管理器
    std::shared_ptr<NetworkManager> networkManager_;

    // 添加新的成员
    bool showEmojiPanel_;
    SDL_Rect emojiButton_;
    SDL_Rect fileButton_;
    SDL_Rect userInfoButton_;
    
    // 消息时间戳显示
    bool showTimestamp_;
    std::chrono::system_clock::time_point lastMessageTime_;
    
    // 未读消息提醒
    std::unordered_map<int64_t, int> unreadCounts_;
    
    // 消息状态
    std::unordered_map<int64_t, bool> messageDelivered_;
    std::unordered_map<int64_t, bool> messageRead_;

public:
    ChatWindow(SDL_Renderer* renderer, int width, int height);
    ~ChatWindow();

    void handleEvent(SDL_Event& event);
    void render();
    void setUser(std::shared_ptr<User> user);
    void addMessage(const Message& msg);

    // 添加新的方法
    void handleNewMessage(const Message& msg);
    void markMessageAsRead(int64_t messageId);
    void updateFriendStatus(int64_t friendId, bool online);
    void refreshFriendList();

    void setNetworkManager(std::shared_ptr<NetworkManager> networkManager) {
        networkManager_ = networkManager;
    }

private:
    void renderTopBar();
    void renderFriendList();
    void renderChatArea();
    void renderInputArea();
    void renderButton(const SDL_Rect& button, const std::string& text, bool isHovered);
    void renderText(const std::string& text, const SDL_Rect& rect, const SDL_Color& color);
    void renderWelcomeMessage();
    bool isMouseOver(const SDL_Rect& rect, int x, int y);
    void handleTextInput(const std::string& text);
    void handleKeyPress(SDL_Keycode key);
    void sendMessage();
    void loadFriendList();
    void loadChatHistory();

    // 添加新的私有方法
    void renderMessageBubble(const Message& msg, const SDL_Rect& rect);
    void renderTimestamp(const std::chrono::system_clock::time_point& time, const SDL_Rect& rect);
    void renderEmojiPanel();
    void handleEmojiSelection(int x, int y);
    void handleFileSelection();
    void showUserProfile();
    void showAddFriendDialog();
    void handleAddFriend(const std::string& username);
    void handleMessage(const Message& msg);
    void showFriendRequestDialog(int64_t fromUserId, const std::string& fromUsername);
}; 