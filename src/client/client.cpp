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
bool receive_message(int socket_fd, std::string& response) {
    char buffer[protocol::kMaxMessageLength];

    ssize_t received = recv(
        socket_fd,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (received <= 0) {
        return false;
    }

    buffer[received] = '\0';
    response = std::string(buffer);

    return true;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0]
                  << " <IP-servidor> <puerto>\n";
        return 1;
    }

    std::string server_ip = argv[1];
    int server_port = std::stoi(argv[2]);
    
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (socket_fd < 0) {
        std::cerr << "Error: no se pudo crear el socket\n";
        return 1;
    }

    sockaddr_in server_address{};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip.c_str(), &server_address.sin_addr) <= 0) {
        std::cerr << "Error: direccion IP invalida\n";
        close(socket_fd);
        return 1;
    }

    std::cout << "Intentando conectar a " << server_ip << ":"
              << server_port << "...\n";

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

std::string server_response;

if (!receive_message(socket_fd, server_response)) {
    std::cerr << "Error: no se pudo recibir respuesta del servidor\n";
    close(socket_fd);
    return 1;
}

std::cout << "Respuesta del servidor: "
          << server_response << "\n";


while (true) {
    std::string message;

    std::cout << "Mensaje: ";
    std::getline(std::cin, message);

    if (message == "quit") {
        std::string quit_message =
            "QUIT" + std::string(1, protocol::kMessageDelimiter);

        if (!send_all(socket_fd, quit_message)) {
            std::cerr << "Error: no se pudo enviar QUIT\n";
        }

        break;
    }

    std::string msg_command =
        "MSG " + message + protocol::kMessageDelimiter;

    if (!send_all(socket_fd, msg_command)) {
        std::cerr << "Error: no se pudo enviar mensaje\n";
        break;
    }

    std::string response;

    if (!receive_message(socket_fd, response)) {
        std::cerr << "Error: conexión cerrada por el servidor\n";
        break;
    }

    std::cout << "Servidor: "
              << response;
}

close(socket_fd);
return 0;

}