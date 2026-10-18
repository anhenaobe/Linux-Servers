# Grafo de llamadas

El diagrama refleja el código actual de `src/server`.

```mermaid
flowchart TD
    MAIN[main] --> ISH[installSignalHandlers]
    MAIN --> INIT[Server::initialize]
    INIT --> SOCKET[socket / setsockopt / bind / listen]
    MAIN --> RUN[Server::run]
    RUN --> ACCEPT[Server::acceptClient]
    ACCEPT --> POSIX_ACCEPT[accept]
    RUN --> CREATE[make_shared ClientSession]
    RUN --> THREAD[std::thread lambda -> manageClient]

    THREAD --> RECEIVE[ClientSession::receiveMessage]
    RECEIVE --> RECV[recv]
    THREAD --> HANDLE[MessageHandler::handleMessage]
    HANDLE --> VALID_USER[protocol::is_valid_username]
    HANDLE --> VALID_MSG[protocol::is_valid_message]
    HANDLE --> REGISTER[UserManager::registerUser]
    THREAD --> DIRECT[ClientSession::sendMessage]
    THREAD --> RECIPIENTS[UserManager::recipientsExcept]
    THREAD --> BROADCAST[ClientSession::sendMessage para cada destinatario]
    THREAD --> UNREGISTER[UserManager::unregisterUser]
    THREAD --> DISCONNECT[ClientSession::disconnect]

    RUN --> STOP[Server::stop]
    RUN --> REQUEST[ClientSession::requestStop]
    RUN --> REAP[erase Worker / Worker destructor]
    REAP --> JOIN[std::thread::join]
    REAP --> DESTROY[ClientSession destructor -> close]
```

`ClientSession::receiveMessage()` extrae primero cualquier línea ya almacenada
en `pending_input_`; solo llama a `recv()` cuando necesita más bytes.

`UserManager::recipientsExcept()` devuelve referencias a los destinatarios;
`manageClient()` recorre esa colección y llama a `sendMessage()` sin mantener
el mutex del registro tomado.
