#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include "protocol.hpp"

bool send_all(int socket_fd, const std::string& data) {
    std::size_t total_sent = 0;

    while (total_sent < data.size()) {
        ssize_t sent = send(
            socket_fd,
            data.data() + total_sent,
            data.size() - total_sent,
            0
        );

        if (sent <= 0) {
            return false;
        }

        total_sent += static_cast<std::size_t>(sent);
    }

    return true;
}

int main() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        std::cerr << "Error: no se pudo crear el socket\n";
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(protocol::kDefaultPort);

    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        std::cerr << "Error: direccion IP invalida\n";
        close(socket_fd);
        return 1;
    }

    std::cout << "Intentando conectar a 127.0.0.1:"
              << protocol::kDefaultPort << "...\n";

    if (connect(
            socket_fd,
            reinterpret_cast<sockaddr*>(&server_address),
            sizeof(server_address)
        ) < 0) {
        std::cerr << "Error: no se pudo conectar al servidor\n";
        close(socket_fd);
        return 1;
    }

    std::cout << "Conexion establecida\n";
    std::string username;

    std::cout << "Ingrese su nombre de usuario: ";
    std::getline(std::cin, username);

    if (username.empty() ||
        username.size() > protocol::kMaxUsernameLength) {
        std::cerr << "Error: nombre de usuario invalido\n";
        close(socket_fd);
        return 1;
    }

std::string hello_message =
    "HELLO " + username + protocol::kMessageDelimiter;

if (!send_all(socket_fd, hello_message)) {
    std::cerr << "Error: no se pudo enviar HELLO\n";
    close(socket_fd);
    return 1;
}

std::cout << "HELLO enviado\n";

    close(socket_fd);
    return 0;
}