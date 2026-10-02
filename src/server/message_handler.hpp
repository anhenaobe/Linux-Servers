#pragma once

#include <memory>
#include <string>
#include <string_view>

class ClientSession;
class UserManager;

enum class SessionPhase
{
    Connected,
    Identified,
    Closing
};

struct SessionState
{
    SessionPhase phase{SessionPhase::Connected};
    std::string username;
};

struct MessageResult
{
    std::string response;
    std::string broadcast;
    bool close_session{false};
};

class MessageHandler
{
public:
    explicit MessageHandler(UserManager& user_manager);

    MessageResult handleMessage(
        std::string_view message,
        SessionState& state,
        const std::shared_ptr<ClientSession>& session);

private:
    UserManager& user_manager_;
};
