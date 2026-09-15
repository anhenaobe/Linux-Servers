# Especificación inicial del protocolo

**Protocol Version 0.1**

Esta es una propuesta inicial para guiar la implementación y puede evolucionar durante el proyecto. El parser completo todavía no está implementado.

## Transporte y codificación

- Transporte: TCP.
- Codificación inicial: UTF-8 compatible con ASCII.
- Framing: cada mensaje de aplicación termina con el carácter newline `\n`.

## Operaciones propuestas

### Cliente -> Servidor

```text
HELLO <username>
MSG <message>
QUIT
```

- `HELLO` propone registrar el nombre del cliente en la conexión.
- `MSG` propone enviar un mensaje de texto.
- `QUIT` propone finalizar la sesión de forma ordenada.

### Servidor -> Cliente

```text
OK
ERR <reason>
FROM <username> <message>
```

- `OK` propone confirmar una operación válida.
- `ERR` propone informar un error con su motivo.
- `FROM` propone entregar un mensaje indicando el usuario de origen.

Cada línea mostrada representa un mensaje lógico y debe enviarse terminada en `\n`.

## Framing sobre TCP

TCP es un flujo de bytes y **no preserva las fronteras entre llamadas a `send()` y `recv()`**. Un solo `recv()` puede devolver una parte de un mensaje, varios mensajes completos o una combinación de mensajes completos y parciales.

Por ello, el receptor deberá conservar los bytes recibidos en un buffer acumulativo. Solo deberá extraer y procesar mensajes completos delimitados por `\n`; cualquier fragmento posterior al último delimitador permanecerá en el buffer hasta recibir más bytes.

La validación de comandos, los límites, el manejo de errores y el parser completo se definirán e implementarán en etapas posteriores.
