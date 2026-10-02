#pragma once

#include <cstdint>
#include <csignal>

class Server
{
public:
    Server() = default;
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    bool initialize(std::uint16_t port);
    int acceptClient();
    int run(const volatile std::sig_atomic_t& stop_requested);
    void stop();

    bool isRunning() const;

private:
    int server_socket_{-1};
    bool running_{false};
};
