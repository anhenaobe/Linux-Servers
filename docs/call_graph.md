# Grafo de llamadas

El diagrama refleja el código actual de `src/server`.

```mermaid
flowchart TD
    MAIN[main] --> ISH[installSignalHandlers]
    MAIN --> INIT[Server::initialize]
    INIT --> SOCKET[socket / setsockopt / bind / listen]
    MAIN --> ACCEPT[Server::acceptClient]
    ACCEPT --> POSIX_ACCEPT[accept]
    MAIN --> CREATE[make_shared ClientSession]
    MAIN --> THREAD[std::thread manageClient]

    THREAD --> RECEIVE[ClientSession::receiveMessage]
    RECEIVE --> RECV[recv]
    THREAD --> HANDLE[MessageHandler::handleMessage]
    HANDLE --> VALID_USER[protocol::is_valid_username]
    HANDLE --> VALID_MSG[protocol::is_valid_message]
    HANDLE --> REGISTER[UserManager::registerUser]
    THREAD --> DIRECT[ClientSession::sendMessage]
    THREAD --> RECIPIENTS[UserManager::recipientsExcept]
    RECIPIENTS --> BROADCAST[ClientSession::sendMessage]
    THREAD --> UNREGISTER[UserManager::unregisterUser]
    THREAD --> DISCONNECT[ClientSession::disconnect]

    MAIN --> STOP[Server::stop]
    MAIN --> REQUEST[ClientSession::requestStop]
    MAIN --> JOIN[std::thread::join]
```

`ClientSession::receiveMessage()` extrae primero cualquier línea ya almacenada
en `pending_input_`; solo llama a `recv()` cuando necesita más bytes.
