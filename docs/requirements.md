# Casos de uso, historias y requisitos

## Casos de uso

| ID | Caso de uso | Resultado principal |
|---|---|---|
| UC-001 | Conectarse al servidor | Se establece una conexión TCP. |
| UC-002 | Registrar username | Un nombre válido y libre queda asociado a la sesión. |
| UC-003 | Enviar mensajes | La sesión identificada puede enviar varios `MSG`. |
| UC-004 | Recibir mensajes | Otros usuarios reciben una línea `FROM`. |
| UC-005 | Cerrar sesión | `QUIT` cierra solo la conexión solicitante. |
| UC-006 | Rechazar username duplicado | El segundo cliente recibe `ERR username_in_use`. |
| UC-007 | Recuperarse de una desconexión | El username se libera sin detener el servidor. |

## Historias de usuario

- **US-001:** Como usuario quiero conectarme usando IP y puerto para iniciar una sesión.
- **US-002:** Como usuario quiero identificarme con un nombre único.
- **US-003:** Como usuario identificado quiero enviar varios mensajes en la misma conexión.
- **US-004:** Como usuario quiero recibir mensajes de otros usuarios conectados.
- **US-005:** Como usuario quiero cerrar mi sesión mediante `QUIT`.
- **US-006:** Como operador quiero que un cliente defectuoso no detenga el servidor.
- **US-007:** Como operador quiero detener el servidor de manera controlada.

## Requisitos funcionales

| ID | Requisito | Origen |
|---|---|---|
| FR-001 | El servidor debe escuchar conexiones TCP en el puerto configurado. | UC-001, US-001 |
| FR-002 | Una conexión debe aceptar múltiples comandos delimitados por `\n`. | UC-003, US-003 |
| FR-003 | El receptor debe reconstruir líneas fragmentadas y conservar líneas sobrantes. | UC-003, US-003 |
| FR-004 | `HELLO` debe registrar un username no vacío de hasta `kMaxUsernameLength`. | UC-002, US-002 |
| FR-005 | El servidor debe rechazar usernames activos duplicados. | UC-006, US-002 |
| FR-006 | `MSG` antes de un `HELLO` válido debe producir un error. | UC-003, US-002 |
| FR-007 | Un `MSG` válido debe producir `OK` al emisor y `FROM` para los demás usuarios. | UC-003, UC-004, US-003, US-004 |
| FR-008 | `QUIT` debe responder `OK`, cerrar esa sesión y liberar su username. | UC-005, US-005 |
| FR-009 | Una desconexión o error debe eliminar el usuario sin cerrar el servidor. | UC-007, US-006 |
| FR-010 | El servidor debe aceptar clientes simultáneos. | UC-001, US-006 |
| FR-011 | Una línea superior a `kMaxMessageLength` debe rechazarse de forma controlada. | UC-003, US-006 |
| FR-012 | `SIGINT` y `SIGTERM` deben iniciar el cierre del servidor y sus sesiones. | US-007 |

## Requisitos no funcionales

| ID | Requisito | Origen |
|---|---|---|
| NFR-001 | El proyecto debe compilar con C++17 y CMake en Debian Linux. | Restricción técnica |
| NFR-002 | La comunicación debe usar TCP y sockets POSIX sin frameworks externos. | Restricción técnica |
| NFR-003 | Los datos compartidos y los envíos concurrentes no deben tener data races conocidas. | US-006 |
| NFR-004 | Las responsabilidades de transporte, protocolo y usuarios deben permanecer separadas. | Mantenibilidad |
| NFR-005 | Los descriptores y threads deben cerrarse o unirse durante el cierre controlado. | US-007 |
