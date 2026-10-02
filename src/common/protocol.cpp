#include "protocol.hpp"

namespace protocol {

namespace {

bool contains_line_break(std::string_view value)
{
    return value.find_first_of("\r\n") != std::string_view::npos;
}

}  // namespace

bool is_valid_username(std::string_view username)
{
    return !username.empty() && username.size() <= kMaxUsernameLength
        && !contains_line_break(username);
}

bool is_valid_message(std::string_view message)
{
    return !message.empty() && message.size() <= kMaxChatMessageLength
        && !contains_line_break(message);
}

}  // namespace protocol
