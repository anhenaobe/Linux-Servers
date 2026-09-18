#include "protocol.hpp"

#include <array>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <string_view>

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
    std::array<char, protocol::kMaxMessageLength> buffer{};

    // recv() lee bytes enviados por TCP, sin exigir un terminador nulo.
    // TCP es un flujo de bytes: una lectura no garantiza un mensaje completo.
    ssize_t bytes_received;
    do {
        bytes_received = recv(client_fd, buffer.data(), buffer.size(), 0);
    } while (bytes_received < 0 && errno == EINTR);

    if (bytes_received < 0) {
        std::cerr << "Error en recv(): " << std::strerror(errno) << '\n';
        close_socket(client_fd);
        close_socket(server_fd);
        return 1;
    }
    if (bytes_received == 0) {
        std::cout << "El cliente cerró la conexión sin enviar datos\n";
        const bool client_closed = close_socket(client_fd);
        const bool server_closed = close_socket(server_fd);
        return client_closed && server_closed ? 0 : 1;
    }

    std::cout << "Mensaje recibido: ";
    std::cout.write(buffer.data(), bytes_received);
    std::cout << std::endl;

    constexpr std::string_view response = "OK: mensaje recibido\n";
    std::size_t bytes_sent = 0;
    // send() transmite bytes al cliente; puede enviar solo parte de la respuesta.
    while (bytes_sent < response.size()) {
        const ssize_t sent = send(client_fd, response.data() + bytes_sent,
                                  response.size() - bytes_sent, MSG_NOSIGNAL);
        if (sent < 0 && errno == EINTR) {
            continue;
        }
        if (sent <= 0) {
            if (sent < 0) {
                std::cerr << "Error en send(): " << std::strerror(errno) << '\n';
            } else {
                std::cerr << "Error en send(): no se enviaron bytes\n";
            }
            close_socket(client_fd);
            close_socket(server_fd);
            return 1;
        }
        bytes_sent += static_cast<std::size_t>(sent);
    }

    const bool client_closed = close_socket(client_fd);
    const bool server_closed = close_socket(server_fd);
    return client_closed && server_closed ? 0 : 1;
}
