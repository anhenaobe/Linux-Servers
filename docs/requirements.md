# Casos de uso, historias de usuario y requisitos del sistema

## 1. Alcance

Este documento define las funciones y restricciones principales de **Linux TCP
Messenger**. Los requisitos describen el comportamiento esperado del sistema y
sirven como base para el diseño arquitectónico y la implementación.

La validación formal corresponde a `feature/testing` y se registra en
[el plan de pruebas](../tests/README.md). La
[matriz de verificación](verification_matrix.md) relaciona estos requisitos
con comprobaciones propuestas y distingue su estado de validación.

## 2. Actores

| Actor | Responsabilidad |
|---|---|
| Usuario | Se conecta, se identifica, envía mensajes y termina su sesión. |
| Operador | Inicia y detiene el servidor. |
| Cliente | Proporciona la interfaz de comunicación del usuario con el servidor. |
| Servidor | Administra sesiones, usuarios y distribución de mensajes. |

## 3. Casos de uso

| ID | Caso de uso | Actor principal | Resultado |
|---|---|---|---|
| UC-001 | Conectarse al servidor | Usuario | Se establece una conexión TCP. |
| UC-002 | Registrar username | Usuario | Un nombre válido y libre queda asociado a la sesión. |
| UC-003 | Enviar mensajes | Usuario | Un mensaje válido es aceptado y distribuido. |
| UC-004 | Recibir mensajes | Usuario | El cliente recibe mensajes enviados por otros usuarios. |
| UC-005 | Cerrar sesión | Usuario | La conexión solicitante termina y su username se libera. |
| UC-006 | Rechazar username duplicado | Usuario | La segunda sesión recibe `ERR username_in_use`. |
| UC-007 | Recuperarse de una desconexión | Servidor | La sesión termina sin detener las demás conexiones. |
| UC-008 | Detener el servidor | Operador | El servidor cierra sus sesiones y libera sus recursos. |

### UC-001 — Conectarse al servidor

**Precondición:** el servidor está iniciado y escuchando.

**Flujo principal:**

1. El usuario ejecuta el cliente con IP y puerto.
2. El cliente crea un socket TCP.
3. El cliente establece la conexión con el servidor.
4. El servidor acepta la conexión y crea una sesión independiente.

**Resultado:** existe una sesión TCP lista para recibir comandos.

### UC-002 — Registrar username

**Precondición:** existe una conexión TCP activa.

**Flujo principal:**

1. El cliente envía `HELLO <username>\n`.
2. El servidor valida el username.
3. El servidor comprueba que no esté activo en otra sesión.
4. El servidor responde `OK\n`.

**Alternativas:** username inválido o duplicado producen
`ERR invalid_username` o `ERR username_in_use`.

### UC-003 — Enviar mensajes

**Precondición:** la sesión está identificada.

**Flujo principal:**

1. El usuario introduce un mensaje.
2. El cliente envía `MSG <message>\n`.
3. El servidor valida el contenido.
4. El servidor responde `OK\n` al emisor.
5. El servidor envía `FROM <username> <message>\n` a los demás usuarios.

### UC-004 — Recibir mensajes

**Precondición:** el usuario está identificado y existen otros usuarios
conectados.

**Flujo principal:**

1. Otro usuario envía un `MSG`.
2. El servidor genera un evento `FROM`.
3. El cliente destinatario recibe el frame y debe mostrarlo.

**Restricción:** `FROM` puede llegar sin que el usuario destinatario haya
enviado recientemente un comando.

**Estado de implementación:** el servidor emite estos eventos; el CLI todavía
no los recibe de forma continua ni distingue eventos de respuestas. Ver
[las limitaciones del cliente](functional_description.md#cliente-cli).

### UC-005 — Cerrar sesión

**Precondición:** existe una conexión activa.

**Flujo principal:**

1. El cliente envía `QUIT\n`.
2. El servidor responde `OK\n`.
3. El servidor libera el username.
4. La sesión termina.

### UC-006 — Rechazar username duplicado

**Precondición:** otro cliente ya utiliza el username solicitado.

**Flujo principal:**

1. Un segundo cliente envía `HELLO <username>\n`.
2. El servidor detecta que el nombre está en uso.
3. Responde `ERR username_in_use\n`.
4. La sesión permanece disponible para otro `HELLO` o `QUIT`.

### UC-007 — Recuperarse de una desconexión

**Precondición:** existe una sesión activa.

**Flujo principal:**

1. El peer se desconecta sin ejecutar `QUIT`.
2. El servidor detecta EOF o un error de red.
3. La sesión se elimina del registro.
4. El username vuelve a estar disponible.
5. Las demás sesiones continúan activas.

### UC-008 — Detener el servidor

**Precondición:** el servidor está ejecutándose.

**Flujo principal:**

1. El operador envía `SIGINT` o `SIGTERM`.
2. El servidor deja de aceptar conexiones y cierra el socket de escucha.
3. Solicita el cierre de las sesiones activas.
4. Une los workers terminados.
5. Libera los recursos restantes y finaliza.

## 4. Historias de usuario

- **US-001:** Como usuario quiero conectarme usando IP y puerto para iniciar una sesión.
- **US-002:** Como usuario quiero identificarme con un nombre único para participar en el chat.
- **US-003:** Como usuario identificado quiero enviar varios mensajes en la misma conexión.
- **US-004:** Como usuario quiero recibir mensajes de otros usuarios conectados.
- **US-005:** Como usuario quiero cerrar mi sesión mediante `QUIT`.
- **US-006:** Como usuario quiero recibir un error claro cuando una operación no sea válida.
- **US-007:** Como operador quiero que un cliente defectuoso no detenga las demás sesiones.
- **US-008:** Como operador quiero detener el servidor de manera controlada.

## 5. Requisitos funcionales

| ID | Requisito | Origen |
|---|---|---|
| FR-001 | El servidor debe escuchar conexiones TCP en el puerto configurado. | UC-001, US-001 |
| FR-002 | Una conexión debe aceptar múltiples comandos delimitados por `\n`. | UC-003, US-003 |
| FR-003 | El receptor debe reconstruir líneas fragmentadas y conservar líneas sobrantes. | UC-003, US-003 |
| FR-004 | `HELLO` debe registrar un username de 1–32 bytes, sin espacios ni controles ASCII. | UC-002, US-002 |
| FR-005 | El servidor debe rechazar usernames activos duplicados. | UC-006, US-002 |
| FR-006 | `MSG` antes de un `HELLO` válido debe producir un error. | UC-003, US-002 |
| FR-007 | Un `MSG` válido debe producir `OK` al emisor y `FROM` para los demás usuarios. | UC-003, UC-004 |
| FR-008 | `QUIT` debe responder `OK`, cerrar esa sesión y liberar su username. | UC-005, US-005 |
| FR-009 | Una desconexión o error debe eliminar el usuario sin cerrar el servidor. | UC-007, US-007 |
| FR-010 | El servidor debe aceptar clientes simultáneos. | UC-001, US-007 |
| FR-011 | Una línea superior a `kMaxMessageLength` debe rechazarse de forma controlada. | UC-003, US-006 |
| FR-012 | `SIGINT` y `SIGTERM` deben iniciar el cierre del servidor y sus sesiones. | UC-008, US-008 |

## 6. Requisitos no funcionales

| ID | Requisito | Origen |
|---|---|---|
| NFR-001 | El proyecto debe compilar con C++17 y CMake en Debian Linux. | Restricción técnica |
| NFR-002 | La comunicación debe utilizar TCP y sockets POSIX sin frameworks externos. | Restricción técnica |
| NFR-003 | El acceso concurrente al estado compartido debe estar sincronizado para evitar data races conocidas. | US-007 |
| NFR-004 | Las responsabilidades de transporte, protocolo y gestión de usuarios deben permanecer separadas. | Mantenibilidad |
| NFR-005 | Los descriptores y threads deben cerrarse o unirse durante el cierre controlado. | US-008 |
| NFR-006 | Los workers terminados deben poder recolectarse durante la ejecución y el cierre debe despertar operaciones bloqueadas. | US-007, US-008 |

## 7. Restricciones del sistema

- Sistema operativo objetivo: Debian Linux o un sistema compatible con
  sockets POSIX.
- Lenguaje: C++17.
- Construcción: CMake 3.16 o posterior.
- Transporte: TCP sobre IPv4.
- Framing: líneas terminadas en LF.
- Puerto predeterminado: `5050`.
- Username máximo: 32 bytes.
- Mensaje de chat máximo: 986 bytes.
- Línea de protocolo máxima: 1024 bytes sin incluir el LF.
