#include "client_session.hpp"
#include "message_handler.hpp"
#include "server.hpp"
#include "user_manager.hpp"

#include "protocol.hpp"

#include <csignal>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <cerrno>
#include <pthread.h>

namespace {

volatile std::sig_atomic_t stop_requested = 0;

void requestServerStop(int)
{
    stop_requested = 1;
}

bool installSignalHandlers()
{
    struct sigaction action {};
    action.sa_handler = requestServerStop;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    return sigaction(SIGINT, &action, nullptr) == 0
        && sigaction(SIGTERM, &action, nullptr) == 0;
}

void manageClient(const std::shared_ptr<ClientSession>& session,
                  MessageHandler& message_handler,
                  UserManager& user_manager)
{
    SessionState state;

    try {
        while (session->isConnected()) {
            std::string message;
            const ReceiveResult receive_result = session->receiveMessage(message);

            if (receive_result == ReceiveResult::MessageTooLong) {
                session->sendMessage("ERR message_too_long\n");
                break;
            }
            if (receive_result != ReceiveResult::MessageReceived) {
                break;
            }

            MessageResult result =
                message_handler.handleMessage(message, state, session);

            if (!result.response.empty()
                && !session->sendMessage(result.response)) {
                break;
            }

            if (!result.broadcast.empty()) {
                const auto recipients =
                    user_manager.recipientsExcept(state.username);
                for (const auto& recipient : recipients) {
                    recipient->sendMessage(result.broadcast);
                }
            }

            if (result.close_session) {
                break;
            }
        }
    } catch (const std::exception& exception) {
        std::cerr << "Error inesperado en sesión: " << exception.what() << '\n';
    }

    if (!state.username.empty()) {
        user_manager.unregisterUser(state.username, session);
    }
    session->disconnect();
}

sigset_t terminationSignals()
{
    sigset_t signals;
    sigemptyset(&signals);
    sigaddset(&signals, SIGINT);
    sigaddset(&signals, SIGTERM);
    return signals;
}

}  // namespace

int main()
{
    if (!installSignalHandlers()) {
        std::cerr << "No se pudieron instalar los manejadores de señales\n";
        return 1;
    }

    Server server;
    if (!server.initialize(protocol::kDefaultPort)) {
        return 1;
    }

    UserManager user_manager;
    MessageHandler message_handler(user_manager);
    std::vector<std::shared_ptr<ClientSession>> sessions;
    std::vector<std::thread> workers;
    int exit_code = 0;

    while (server.isRunning() && !stop_requested) {
        const int client_socket = server.acceptClient();
        if (client_socket < 0) {
            if (stop_requested) {
                break;
            }
            if (errno == EINTR || errno == ECONNABORTED) {
                continue;
            }
            exit_code = 1;
            break;
        }

        auto session = std::make_shared<ClientSession>(client_socket);
        sessions.push_back(session);

        // Los workers bloquean las señales de cierre. Así SIGINT/SIGTERM se
        // atienden en el thread principal y pueden interrumpir accept().
        const sigset_t signals = terminationSignals();
        sigset_t previous_mask;
        pthread_sigmask(SIG_BLOCK, &signals, &previous_mask);
        workers.emplace_back(manageClient, session,
                             std::ref(message_handler),
                             std::ref(user_manager));
        pthread_sigmask(SIG_SETMASK, &previous_mask, nullptr);
    }

    server.stop();
    for (const auto& session : sessions) {
        session->requestStop();
    }
    for (std::thread& worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    std::cout << "Servidor detenido\n";
    return exit_code;
}
