# Linux TCP Messenger

Proyecto universitario para aprender los fundamentos de una aplicación cliente-servidor en Debian Linux. El equipo desarrollará en C++17 un servidor y varios clientes que se comunicarán mediante TCP y sockets POSIX.

En esta etapa el repositorio contiene solamente la infraestructura inicial y programas placeholder. La comunicación por sockets, la concurrencia, el protocolo completo y la gestión multiusuario todavía no están implementados.

## Arquitectura prevista

```text
Client 1 ----\
Client 2 ----- > TCP Server (Debian Linux)
Client 3 ----/
```

El servidor será el punto central de comunicación. Los clientes establecerán conexiones TCP con él y, en etapas posteriores, intercambiarán mensajes de acuerdo con la especificación de [`docs/protocol.md`](docs/protocol.md).

## Tecnologías

- Debian Linux como plataforma de desarrollo y ejecución.
- C++17 como lenguaje.
- TCP como protocolo de transporte.
- API de sockets POSIX para la comunicación de red futura.
- CMake para configurar y compilar el proyecto.
- Git y GitHub para control de versiones y revisión mediante Pull Requests.
- VS Code con Remote SSH para trabajar en el entorno Linux remoto.

## Estructura del repositorio

```text
.
├── CMakeLists.txt
├── README.md
├── docs/
│   └── protocol.md
├── include/
│   └── protocol.hpp
├── src/
│   ├── client/
│   │   └── client.cpp
│   ├── common/
│   │   └── protocol.cpp
│   └── server/
│       └── server.cpp
└── tests/
    └── README.md
```

## Compilación

Desde la raíz del repositorio:

```bash
cmake -S . -B build
cmake --build build
```

CMake configura C++17, compila el código compartido del protocolo y genera los ejecutables `server` y `client` dentro de `build/`.

## Ejecución actual

Después de compilar en Debian Linux:

```bash
./build/server
./build/client
```

Por ahora ambos programas solo imprimen un mensaje placeholder; aún no abren conexiones de red.

## Flujo de trabajo Git

La rama estable es `main` y la rama de integración es `develop`. Cada integrante debe trabajar en una rama `feature/*` creada desde `develop`:

- `feature/server`: trabajo relacionado con el servidor.
- `feature/client`: trabajo relacionado con el cliente.
- `feature/testing`: pruebas y soporte de validación.

El flujo acordado es:

```text
feature/* -> Pull Request -> develop
develop   -> Pull Request -> main
```

No se trabaja directamente sobre `main`. Antes de abrir un Pull Request, cada integrante debe compilar sus cambios, mantener sus commits enfocados y actualizar su rama con los cambios recientes de `develop` cuando sea necesario.
