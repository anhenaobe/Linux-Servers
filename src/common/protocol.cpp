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
    if (username.empty() || username.size() > kMaxUsernameLength) {
        return false;
    }
    for (unsigned char byte : username) {
        // Un solo token hace inequívoco FROM <username> <message>.
        if (byte <= 0x20 || byte == 0x7f) {
            return false;
        }
    }
    return true;
}

bool is_valid_message(std::string_view message)
{
    return !message.empty() && message.size() <= kMaxChatMessageLength
        && !contains_line_break(message)
        && message.find('\0') == std::string_view::npos;
}

}  // namespace protocol
