# Protocolo de mensajería

**Versión 0.1**

Contrato temporalmente congelado para el milestone del servidor (2026-10-02).
Cambios de tramas o reglas requieren acuerdo previo con cliente y testing.

## Transporte y framing

- Transporte: TCP sobre IPv4.
- Codificación prevista: texto compatible con UTF-8/ASCII.
- Cada frame termina en `\n`.
- El delimitador no forma parte del comando entregado al parser.
- Una línea, sin `\n`, no puede superar `protocol::kMaxMessageLength`.
- Los límites son bytes, no caracteres: username 32, línea 1024, texto MSG 986.
- Los comandos distinguen mayúsculas y usan exactamente los espacios indicados.
- Se usa LF (`\n`), sin normalizar CRLF (`\r\n`). El servidor no valida la
  codificación UTF-8; emisor y UI deben acordar usarla.

TCP no conserva fronteras entre `send()` y `recv()`. El servidor acumula bytes
por sesión, reconstruye líneas fragmentadas y conserva frames adicionales para
la siguiente llamada a `receiveMessage()`.

## Cliente hacia servidor

```text
HELLO <username>\n
MSG <message>\n
QUIT\n
```

### `HELLO`

Identifica la sesión. El username debe ser no vacío, medir como máximo
`kMaxUsernameLength`, no contener espacios ni controles ASCII (bytes 0–32 y
127) y no estar registrado por otra sesión. Es un único token, sin recortar
espacios. Los nombres son sensibles a mayúsculas. Un segundo `HELLO` después de
identificarse se rechaza.

### `MSG`

Solo está permitido después de `HELLO`. El texto debe ser no vacío y no superar
`kMaxChatMessageLength`. El límite garantiza que la línea `FROM` resultante no
supere `kMaxMessageLength` incluso con el username más largo.
El texto conserva espacios, no puede contener NUL, CR ni LF. No se trunca.

### `QUIT`

Responde `OK` y finaliza solamente la sesión solicitante. También puede usarse
antes de `HELLO`.

## Servidor hacia cliente

```text
OK\n
ERR <reason>\n
FROM <username> <message>\n
```

Para un `MSG` válido, el emisor recibe `OK`. Los demás usuarios identificados
reciben `FROM`; el emisor no recibe su propio broadcast.

Razones de error implementadas:

| Razón | Condición |
|---|---|
| `invalid_username` | Username vacío, demasiado largo, con espacios o controles ASCII. |
| `username_in_use` | Otro cliente mantiene ese username. |
| `already_identified` | La sesión ya completó `HELLO`. |
| `not_identified` | Se recibió `MSG` antes de `HELLO`. |
| `invalid_message` | Texto vacío, demasiado largo o con NUL/CR/LF. |
| `invalid_command` | La línea no coincide con un comando. |
| `message_too_long` | El framing superó el máximo antes de obtener una línea válida. |

`message_too_long` se considera irrecuperable para esa conexión: el servidor
envía el error y la cierra. Los demás errores permiten continuar la sesión.

## Eventos y respuestas para una UI

- Por cada comando se genera una respuesta directa `OK` o `ERR`.
- `FROM` es un evento asíncrono: puede llegar intercalado entre respuestas
  directas. La UI debe seguir leyendo aunque el usuario no esté enviando texto.
- Para interpretar `FROM`, extraer el token username y conservar el resto de
  la línea como texto; los espacios dentro del texto no son delimitadores nuevos.
- Para asociar respuestas sin IDs, mantener como máximo un comando pendiente;
  un `FROM` no completa ese comando. `HELLO` solo completa el registro con `OK`.
- `OK` de `MSG` confirma aceptación, no entrega garantizada a todos los clientes.
- Se conserva el orden por conexión emisora; no se garantiza un orden global
  entre mensajes emitidos simultáneamente por distintos usuarios.
- `OK` de `QUIT` es la última trama enviada a esa sesión; después llega EOF.
- EOF o error de red indica desconexión. No hay trama adicional de despedida
  cuando un peer desaparece o cuando el operador detiene el servidor.
- Un cliente que no lee puede ser desconectado ante un fallo o timeout de envío.

Compatibilidad conocida: el cliente CLI actual rechaza una respuesta que mida
exactamente 1024 bytes antes del LF. El servidor permite ese límite inclusivo;
la futura UI debe admitirlo. La corrección del receptor CLI corresponde a
`feature-client`.
