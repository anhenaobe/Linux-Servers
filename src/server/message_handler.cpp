#include "message_handler.hpp"

#include "client_session.hpp"
#include "protocol.hpp"
#include "user_manager.hpp"

namespace {

constexpr std::string_view kHelloPrefix = "HELLO ";
constexpr std::string_view kMessagePrefix = "MSG ";

bool startsWith(std::string_view value, std::string_view prefix)
{
    return value.size() >= prefix.size()
        && value.substr(0, prefix.size()) == prefix;
}

MessageResult errorResult(std::string_view reason)
{
    return {"ERR " + std::string(reason) + "\n", {}, false};
}

}  // namespace

MessageHandler::MessageHandler(UserManager& user_manager)
    : user_manager_(user_manager)
{
}

MessageResult MessageHandler::handleMessage(
    std::string_view message,
    SessionState& state,
    const std::shared_ptr<ClientSession>& session)
{
    if (message == "QUIT") {
        state.phase = SessionPhase::Closing;
        return {"OK\n", {}, true};
    }

    if (message == "HELLO" || startsWith(message, kHelloPrefix)) {
        if (state.phase == SessionPhase::Identified) {
            return errorResult("already_identified");
        }

        const std::string username = message == "HELLO"
            ? std::string{}
            : std::string(message.substr(kHelloPrefix.size()));

        if (!protocol::is_valid_username(username)) {
            return errorResult("invalid_username");
        }
        if (!user_manager_.registerUser(username, session)) {
            return errorResult("username_in_use");
        }

        state.username = username;
        state.phase = SessionPhase::Identified;
        return {"OK\n", {}, false};
    }

    if (message == "MSG" || startsWith(message, kMessagePrefix)) {
        if (state.phase != SessionPhase::Identified) {
            return errorResult("not_identified");
        }

        const std::string_view payload = message == "MSG"
            ? std::string_view{}
            : message.substr(kMessagePrefix.size());
        if (!protocol::is_valid_message(payload)) {
            return errorResult("invalid_message");
        }

        MessageResult result;
        result.response = "OK\n";
        result.broadcast = "FROM " + state.username + " "
            + std::string(payload) + "\n";
        return result;
    }

    return errorResult("invalid_command");
}
