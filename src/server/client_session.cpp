#include "client_session.hpp"

#include "protocol.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <sys/socket.h>
#include <unistd.h>

ClientSession::ClientSession(int client_socket)
    : client_socket_(client_socket), connected_(client_socket >= 0)
{
}

ClientSession::~ClientSession()
{
    disconnect();
}

bool ClientSession::sendMessage(std::string_view message)
{
    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (client_socket_ < 0 || !connected_) {
        return false;
    }

    // Varios threads pueden emitir mensajes hacia una misma sesión. El mutex
    // evita que sus llamadas parciales a send() mezclen bytes.
    std::size_t bytes_sent = 0;
    while (bytes_sent < message.size()) {
        const ssize_t sent = send(client_socket_, message.data() + bytes_sent,
                                  message.size() - bytes_sent, MSG_NOSIGNAL);
        if (sent < 0 && errno == EINTR) {
            continue;
        }
        if (sent <= 0) {
            if (sent < 0) {
                std::cerr << "Error en send(): " << std::strerror(errno) << '\n';
            } else {
                std::cerr << "Error en send(): no se enviaron bytes\n";
            }
            connected_ = false;
            shutdown(client_socket_, SHUT_RDWR);
            return false;
        }
        bytes_sent += static_cast<std::size_t>(sent);
    }

    return true;
}

ReceiveResult ClientSession::receiveMessage(std::string& message)
{
    message.clear();

    while (connected_) {
        const std::size_t delimiter_position =
            pending_input_.find(protocol::kMessageDelimiter);

        if (delimiter_position != std::string::npos) {
            if (delimiter_position > protocol::kMaxMessageLength) {
                pending_input_.clear();
                return ReceiveResult::MessageTooLong;
            }

            message = pending_input_.substr(0, delimiter_position);
            pending_input_.erase(0, delimiter_position + 1);
            return ReceiveResult::MessageReceived;
        }

        if (pending_input_.size() > protocol::kMaxMessageLength) {
            pending_input_.clear();
            return ReceiveResult::MessageTooLong;
        }

        std::array<char, protocol::kMaxMessageLength> buffer{};
        ssize_t bytes_received;
        do {
            bytes_received = recv(client_socket_, buffer.data(), buffer.size(), 0);
        } while (bytes_received < 0 && errno == EINTR && connected_);

        if (bytes_received < 0) {
            if (connected_) {
                std::cerr << "Error en recv(): " << std::strerror(errno) << '\n';
                connected_ = false;
                return ReceiveResult::Error;
            }
            return ReceiveResult::Disconnected;
        }

        if (bytes_received == 0) {
            connected_ = false;
            return ReceiveResult::Disconnected;
        }

        pending_input_.append(buffer.data(),
                              static_cast<std::size_t>(bytes_received));
    }

    return ReceiveResult::Disconnected;
}

void ClientSession::requestStop()
{
    connected_ = false;

    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (client_socket_ >= 0) {
        // shutdown() despierta al thread que pueda estar bloqueado en recv().
        shutdown(client_socket_, SHUT_RDWR);
    }
}

void ClientSession::disconnect()
{
    requestStop();

    std::lock_guard<std::mutex> lock(socket_mutex_);
    if (client_socket_ < 0) {
        return;
    }

    if (close(client_socket_) < 0) {
        std::cerr << "Error cerrando socket cliente: "
                  << std::strerror(errno) << '\n';
    }
    client_socket_ = -1;
}

bool ClientSession::isConnected() const
{
    return connected_;
}
