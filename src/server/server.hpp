#pragma once

#include <cstdint>

class Server
{
public:
    Server() = default;
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool initialize(std::uint16_t port);
    int acceptClient();
    void stop();

    bool isRunning() const;

private:
    int server_socket_{-1};
    bool running_{false};
};
