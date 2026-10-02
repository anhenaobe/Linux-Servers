# Linux TCP Messenger

Proyecto universitario de mensajería cliente-servidor para Debian Linux,
desarrollado con C++17, sockets POSIX, TCP y CMake.

El servidor mantiene sesiones persistentes, identifica usuarios mediante
`HELLO`, procesa varios `MSG`, distribuye mensajes a otros usuarios y cierra una
sesión mediante `QUIT`. Cada cliente se atiende en un thread y el estado
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

Interfaz esperada del cliente desarrollado en `feature-client`:

```bash
./build/client <IP-servidor> <puerto>
```

GitHub ya integró el cliente persistente en `main`. El servidor es compatible
con su flujo `HELLO`, varios `MSG` y `QUIT`.

## Protocolo básico

```text
Cliente -> Servidor: HELLO <username>\n
Cliente -> Servidor: MSG <message>\n
Cliente -> Servidor: QUIT\n

Servidor -> Cliente: OK\n
Servidor -> Cliente: ERR <reason>\n
Servidor -> Cliente: FROM <username> <message>\n
```

El emisor recibe `OK` y los demás usuarios identificados reciben `FROM`. Todos
los frames están delimitados por `\n`.

El contrato 0.1 queda temporalmente congelado para desarrollar la futura UI.
Consultar [el protocolo](docs/protocol.md) para límites, errores y eventos
asíncronos. El username es un token de 1–32 bytes, sin espacios ni controles
ASCII. No se han añadido funcionalidades de GUI.

## Estructura

```text
include/protocol.hpp       Constantes y validaciones compartidas
src/common/protocol.cpp    Implementación de validaciones
src/server/                Servidor, sesiones, protocolo y usuarios
src/client/                Cliente (responsabilidad de feature-client)
docs/                      Especificación y documentación técnica
tests/                     Trabajo formal de feature/testing
```

Documentación:

- [Descripción funcional](docs/functional_description.md)
- [Requisitos y casos de uso](docs/requirements.md)
- [Arquitectura](docs/architecture.md)
- [Protocolo](docs/protocol.md)
- [Matriz de verificación](docs/verification_matrix.md)
- [Grafo de llamadas](docs/call_graph.md)
- [Diagramas de interacción](docs/interaction_diagrams.md)

## Ramas de trabajo

- `main`: integración estable.
- `feature-client`: implementación del cliente.
- `feature-server`: servidor, protocolo y documentación técnica.
- `feature/testing`: pruebas formales y validación, responsabilidad separada.

No se añadieron pruebas formales en esta rama. La matriz de verificación solo
registra comprobaciones manuales de desarrollo y trabajo formal pendiente.
