#include "protocol.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    // El descriptor representa el socket TCP IPv4 del servidor.
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Error creando socket: " << std::strerror(errno) << '\n';
        return 1;
    }

    const auto close_socket = [](int fd) {
        if (close(fd) < 0) {
            std::cerr << "Error cerrando socket: " << std::strerror(errno) << '\n';
            return false;
        }
        return true;
    };

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(protocol::kDefaultPort);

    // bind() asocia el socket al puerto y a todas las interfaces locales.
    if (bind(server_fd, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        std::cerr << "Error en bind() del puerto " << protocol::kDefaultPort
                  << ": " << std::strerror(errno) << '\n';
        close_socket(server_fd);
        return 1;
    }

    // listen() habilita la recepción de solicitudes de conexión.
    if (listen(server_fd, 1) < 0) {
        std::cerr << "Error en listen(): " << std::strerror(errno) << '\n';
        close_socket(server_fd);
        return 1;
    }

    std::cout << "Servidor escuchando en 0.0.0.0:" << protocol::kDefaultPort << std::endl;

    // accept() devuelve un nuevo descriptor para la conexión del cliente.
    const int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) {
        std::cerr << "Error en accept(): " << std::strerror(errno) << '\n';
        close_socket(server_fd);
        return 1;
    }

    std::cout << "Conexión de cliente aceptada\n";
    const bool client_closed = close_socket(client_fd);
    const bool server_closed = close_socket(server_fd);
    return client_closed && server_closed ? 0 : 1;
}
