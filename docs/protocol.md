# Protocolo de mensajería

**Versión 0.1**

## Transporte y framing

- Transporte: TCP sobre IPv4.
- Codificación prevista: texto compatible con UTF-8/ASCII.
- Cada frame termina en `\n`.
- El delimitador no forma parte del comando entregado al parser.
- Una línea, sin `\n`, no puede superar `protocol::kMaxMessageLength`.

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
`kMaxUsernameLength`, no contener `\r`/`\n` y no estar registrado por otra
sesión. Los nombres son sensibles a mayúsculas. Un segundo `HELLO` después de
identificarse se rechaza.

### `MSG`

Solo está permitido después de `HELLO`. El texto debe ser no vacío y no superar
`kMaxChatMessageLength`. El límite garantiza que la línea `FROM` resultante no
supere `kMaxMessageLength` incluso con el username más largo.

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
| `invalid_username` | Username vacío, demasiado largo o con salto de línea. |
| `username_in_use` | Otro cliente mantiene ese username. |
| `already_identified` | La sesión ya completó `HELLO`. |
| `not_identified` | Se recibió `MSG` antes de `HELLO`. |
| `invalid_message` | Texto vacío o demasiado largo. |
| `invalid_command` | La línea no coincide con un comando. |
| `message_too_long` | El framing superó el máximo antes de obtener una línea válida. |

`message_too_long` se considera irrecuperable para esa conexión: el servidor
envía el error y la cierra. Los demás errores permiten continuar la sesión.
