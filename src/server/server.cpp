#include "server.hpp"
#include "client_session.hpp"
#include "message_handler.hpp"
#include "user_manager.hpp"

#include <atomic>
#include <cerrno>
#include <cstring>
#include <exception>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <thread>

#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

namespace {

void manageClient(const std::shared_ptr<ClientSession>& session,
                  MessageHandler& handler, UserManager& users)
{
    SessionState state;
    try {
        while (session->isConnected()) {
            std::string message;
            const auto received = session->receiveMessage(message);
            if (received == ReceiveResult::MessageTooLong) {
                session->sendMessage("ERR message_too_long\n", true);
                break;
            }
            if (received != ReceiveResult::MessageReceived) {
                break;
            }
            const auto result = handler.handleMessage(message, state, session);
            if (!session->sendMessage(result.response, result.close_session)) {
                break;
            }
            if (!result.broadcast.empty()) {
                // Ningún mutex del registro permanece tomado durante send().
                for (const auto& recipient : users.recipientsExcept(state.username)) {
                    recipient->sendMessage(result.broadcast);
                }
            }
            if (result.close_session) {
                break;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error en sesión: " << error.what() << '\n';
    } catch (...) {
        std::cerr << "Error desconocido en sesión\n";
    }
    if (!state.username.empty()) {
        users.unregisterUser(state.username, session);
    }
    session->disconnect();
}

struct Worker
{
    std::shared_ptr<ClientSession> session;
    std::atomic_bool finished{false};
    std::thread thread;

    ~Worker()
    {
        if (thread.joinable()) {
            session->requestStop();
            thread.join();
        }
    }
};

}  // namespace

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

    // Permite revisar señales y recolectar workers aunque no lleguen clientes.
    // Evita perder una señal recibida entre la condición del loop y accept().
    const timeval accept_timeout{0, 200000};
    if (setsockopt(server_socket_, SOL_SOCKET, SO_RCVTIMEO,
                   &accept_timeout, sizeof(accept_timeout)) < 0) {
        std::cerr << "Error configurando espera de accept(): "
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
        // run() observa interrupciones y timeouts para revisar el apagado.
        if (errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Error en accept(): " << std::strerror(errno) << '\n';
        }
        return -1;
    }

    // accept() crea otro descriptor: el socket servidor continúa escuchando y
    // este nuevo socket representa exclusivamente la conexión con un cliente.
    return client_socket;
}

int Server::run(const volatile std::sig_atomic_t& stop_requested)
{
    UserManager users;
    MessageHandler handler(users);
    std::list<Worker> workers;
    int exit_code = 0;

    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);

    try {
        while (running_ && !stop_requested) {
            for (auto worker = workers.begin(); worker != workers.end();) {
                if (worker->finished.load()) {
                    worker = workers.erase(worker);
                } else {
                    ++worker;
                }
            }
            const int fd = acceptClient();
            if (fd < 0) {
                if (stop_requested) {
                    break;
                }
                if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK
                    || errno == ECONNABORTED) {
                    continue;
                }
                exit_code = 1;
                break;
            }

            // Un receptor lento no puede bloquear un broadcast indefinidamente.
            const timeval send_timeout{2, 0};
            // Linux hereda opciones del listener: la sesión debe poder esperar
            // indefinidamente entrada, aunque accept() tenga espera limitada.
            const timeval receive_timeout{0, 0};
            if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
                           &receive_timeout, sizeof(receive_timeout)) < 0
                || setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
                           &send_timeout, sizeof(send_timeout)) < 0) {
                std::cerr << "Error configurando envío del cliente\n";
                close(fd);
                continue;
            }

            std::shared_ptr<ClientSession> session;
            try {
                session = std::make_shared<ClientSession>(fd);
            } catch (...) {
                close(fd);
                throw;
            }
            workers.emplace_back();
            Worker& worker = workers.back();
            worker.session = std::move(session);

            // Los workers heredan la máscara bloqueada; las señales llegan al
            // principal. Siempre restauramos la máscara si std::thread falla.
            sigset_t previous_mask;
            const int mask_error = pthread_sigmask(SIG_BLOCK, &signals, &previous_mask);
            if (mask_error != 0) {
                std::cerr << "Error bloqueando señales: " << std::strerror(mask_error) << '\n';
                exit_code = 1;
                break;
            }
            try {
                worker.thread = std::thread([&worker, &handler, &users] {
                    manageClient(worker.session, handler, users);
                    worker.finished.store(true);
                });
            } catch (...) {
                pthread_sigmask(SIG_SETMASK, &previous_mask, nullptr);
                throw;
            }
            const int restore_error = pthread_sigmask(SIG_SETMASK, &previous_mask, nullptr);
            if (restore_error != 0) {
                std::cerr << "Error restaurando señales: " << std::strerror(restore_error) << '\n';
                exit_code = 1;
                break;
            }
        }
    } catch (const std::exception& error) {
        std::cerr << "Error iniciando sesión: " << error.what() << '\n';
        exit_code = 1;
    }

    stop();
    for (const auto& worker : workers) {
        if (worker.session) {
            worker.session->requestStop();
        }
    }
    workers.clear(); // Los destructores unen threads antes de destruir users.
    std::cout << "Servidor detenido\n";
    return exit_code;
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
