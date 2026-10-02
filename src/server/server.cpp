#include "server.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

Server::~Server()
{
    stop();
}

bool Server::initialize(std::uint16_t port)
{
    if (running_) {
        return true;
    }

    // El socket servidor solo escucha conexiones nuevas; no se usa para
    // intercambiar mensajes con un cliente concreto.
    server_socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_ < 0) {
        std::cerr << "Error creando socket: " << std::strerror(errno) << '\n';
        return false;
    }

    const int reuse_address = 1;
    if (setsockopt(server_socket_, SOL_SOCKET, SO_REUSEADDR,
                   &reuse_address, sizeof(reuse_address)) < 0) {
        std::cerr << "Error configurando SO_REUSEADDR: "
                  << std::strerror(errno) << '\n';
        stop();
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port);

    if (bind(server_socket_, reinterpret_cast<const sockaddr*>(&address),
             sizeof(address)) < 0) {
        std::cerr << "Error en bind() del puerto " << port << ": "
                  << std::strerror(errno) << '\n';
        stop();
        return false;
    }

    if (listen(server_socket_, SOMAXCONN) < 0) {
        std::cerr << "Error en listen(): " << std::strerror(errno) << '\n';
        stop();
        return false;
    }

    running_ = true;
    std::cout << "Servidor escuchando en 0.0.0.0:" << port << std::endl;
    return true;
}

int Server::acceptClient()
{
    if (!running_) {
        return -1;
    }

    const int client_socket = accept(server_socket_, nullptr, nullptr);

    if (client_socket < 0) {
        // EINTR permite que main observe SIGINT/SIGTERM y cierre limpiamente.
        if (errno != EINTR) {
            std::cerr << "Error en accept(): " << std::strerror(errno) << '\n';
        }
        return -1;
    }

    // accept() crea otro descriptor: el socket servidor continúa escuchando y
    // este nuevo socket representa exclusivamente la conexión con un cliente.
    return client_socket;
}

void Server::stop()
{
    running_ = false;

    if (server_socket_ < 0) {
        return;
    }

    if (close(server_socket_) < 0) {
        std::cerr << "Error cerrando socket servidor: "
                  << std::strerror(errno) << '\n';
    }
    server_socket_ = -1;
}

bool Server::isRunning() const
{
    return running_;
}
