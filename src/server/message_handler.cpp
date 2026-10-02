#include "message_handler.hpp"

#include "client_session.hpp"
#include "protocol.hpp"
#include "user_manager.hpp"

namespace {

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
    if (message == protocol::kQuitCommand) {
        state.phase = SessionPhase::Closing;
        return {std::string(protocol::kOkResponse), {}, true};
    }

    if (message == protocol::kHelloCommand || startsWith(message, protocol::kHelloPrefix)) {
        if (state.phase == SessionPhase::Identified) {
            return errorResult("already_identified");
        }

        const std::string username = message == protocol::kHelloCommand
            ? std::string{}
            : std::string(message.substr(protocol::kHelloPrefix.size()));

        if (!protocol::is_valid_username(username)) {
            return errorResult("invalid_username");
        }
        state.username = username;
        if (!user_manager_.registerUser(username, session)) {
            state.username.clear();
            return errorResult("username_in_use");
        }

        state.phase = SessionPhase::Identified;
        return {std::string(protocol::kOkResponse), {}, false};
    }

    if (message == protocol::kMessageCommand || startsWith(message, protocol::kMessagePrefix)) {
        if (state.phase != SessionPhase::Identified) {
            return errorResult("not_identified");
        }

        const std::string_view payload = message == protocol::kMessageCommand
            ? std::string_view{}
            : message.substr(protocol::kMessagePrefix.size());
        if (!protocol::is_valid_message(payload)) {
            return errorResult("invalid_message");
        }

        MessageResult result;
        result.response = protocol::kOkResponse;
        result.broadcast = "FROM " + state.username + " "
            + std::string(payload) + "\n";
        return result;
    }

    return errorResult("invalid_command");
}
