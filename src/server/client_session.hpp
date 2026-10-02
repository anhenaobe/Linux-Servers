#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <string_view>

enum class ReceiveResult
{
    MessageReceived,
    Disconnected,
    Error,
    MessageTooLong
};

class ClientSession
{
public:
    explicit ClientSession(int client_socket);
    ~ClientSession();

    ClientSession(const ClientSession&) = delete;
    ClientSession& operator=(const ClientSession&) = delete;

    bool sendMessage(std::string_view message);
    ReceiveResult receiveMessage(std::string& message);
    void requestStop();
    void disconnect();

    bool isConnected() const;

private:
    int client_socket_{-1};
    std::atomic_bool connected_{false};
    mutable std::mutex socket_mutex_;
    std::string pending_input_;
};
