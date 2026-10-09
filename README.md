# Linux TCP Messenger

Proyecto universitario de mensajería cliente-servidor para Debian Linux,
desarrollado con C++17, sockets POSIX, TCP y CMake.

El servidor mantiene sesiones persistentes, identifica usuarios mediante
`HELLO`, procesa varios `MSG`, distribuye mensajes a otros usuarios y cierra
una sesión mediante `QUIT`. Cada cliente se atiende en un thread y el estado
compartido de usuarios está sincronizado.

## Requisitos

- Debian Linux o sistema compatible con sockets POSIX.
- Compilador con soporte C++17.
- CMake 3.16 o posterior.
- Implementación de threads POSIX disponible para `std::thread`.

## Compilación

```bash
cmake -S . -B build
cmake --build build
```

Los ejecutables se generan como `build/server` y `build/client`.

## Ejecución

Servidor:

```bash
./build/server
```

El puerto predeterminado es `5050`. `Ctrl+C` solicita un cierre controlado.

Cliente:

```bash
./build/client <IP-servidor> <puerto>
```

El cliente CLI actual permite identificarse con `HELLO`, enviar múltiples
`MSG` y cerrar con `QUIT`. El servidor también genera eventos `FROM`
asíncronos para otros usuarios; la recepción continua de esos eventos queda
como evolución pendiente de `feature-client`. Para salir del CLI se escribe
`quit` en minúsculas; el cliente transmite `QUIT`, pero no espera su respuesta.
Las demás limitaciones actuales se detallan en
[la descripción funcional](docs/functional_description.md#cliente-cli).

## Protocolo básico

```text
Cliente -> Servidor: HELLO <username>\n
Cliente -> Servidor: MSG <message>\n
Cliente -> Servidor: QUIT\n

Servidor -> Cliente: OK\n
Servidor -> Cliente: ERR <reason>\n
Servidor -> Cliente: FROM <username> <message>\n
```

El emisor recibe `OK` y los demás usuarios identificados reciben `FROM`.
Todos los frames están delimitados por `\n`.

El contrato 0.1 queda temporalmente congelado. Consultar
[la documentación del protocolo](docs/protocol.md) para límites, errores y
eventos asíncronos.

## Documentación

El [índice de documentación](docs/README.md) describe el propósito de cada
archivo, las fuentes de referencia y las responsabilidades de mantenimiento.
El contrato vigente está en [docs/protocol.md](docs/protocol.md); el plan y los
resultados registrados están en [tests/README.md](tests/README.md).

## Estructura

```text
include/protocol.hpp       Constantes y validaciones compartidas
src/common/protocol.cpp    Implementación de validaciones
src/server/                Servidor, sesiones, protocolo y usuarios
src/client/                Cliente CLI
docs/                      Documentación del sistema
tests/                     Trabajo formal de feature/testing
```

## Ramas de trabajo

- `main`: integración estable.
- `feature-client`: evolución del cliente y documentación asociada a esta rama.
- `feature-server`: servidor y documentación técnica del servidor.
- `feature/testing`: pruebas formales y validación.

Los cambios se revisan mediante PR hacia `main`, coordinando con el responsable
del componente. La documentación de desarrollo y los resultados formales se
distinguen explícitamente.
