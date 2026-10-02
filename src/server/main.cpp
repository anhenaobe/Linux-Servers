#include "server.hpp"
#include "protocol.hpp"

#include <csignal>
#include <iostream>

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
    return sigaction(SIGINT, &action, nullptr) == 0
        && sigaction(SIGTERM, &action, nullptr) == 0;
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
    return server.run(stop_requested);
}
