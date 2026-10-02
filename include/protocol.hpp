#pragma once

#include <cstddef>
#include <string_view>

namespace protocol {

inline constexpr unsigned short kDefaultPort = 5050;
inline constexpr std::size_t kMaxUsernameLength = 32;
// Longitud máxima de una línea de protocolo, sin incluir '\n'.
inline constexpr std::size_t kMaxMessageLength = 1024;
// Reserva espacio para "FROM ", el username y el espacio separador.
inline constexpr std::size_t kMaxChatMessageLength =
    kMaxMessageLength - kMaxUsernameLength - 6;
inline constexpr char kMessageDelimiter = '\n';
inline constexpr std::string_view kVersion = "0.1";

bool is_valid_username(std::string_view username);
bool is_valid_message(std::string_view message);

}  // namespace protocol
