# Descripción funcional

## 1. Propósito y alcance

**Linux TCP Messenger** es un sistema educativo de mensajería cliente-servidor
para Debian Linux. Permite que varios clientes establezcan conexiones TCP con
un servidor central, se identifiquen mediante un username y envíen mensajes de
texto a los demás usuarios conectados.

El sistema se implementa con C++17, sockets POSIX y TCP sobre IPv4. La
comunicación utiliza un protocolo de texto delimitado por saltos de línea
(`\\n`).

El alcance de este mini-proyecto comprende:

- establecimiento y cierre de conexiones TCP;
- identificación de usuarios mediante `HELLO`;
- validación y unicidad de usernames activos;
- envío de múltiples mensajes por una misma sesión;
- distribución de mensajes a los demás usuarios identificados;
- manejo de desconexiones y cierre controlado del servidor;
- definición de los requisitos y de la arquitectura del sistema.

Quedan fuera del alcance actual la autenticación, el cifrado, las cuentas
persistentes, el historial de mensajes, las salas, los mensajes privados y
una interfaz gráfica.

## 2. Actores

| Actor | Descripción |
|---|---|
| Usuario | Utiliza un cliente para conectarse, identificarse, enviar mensajes y cerrar su sesión. |
| Operador | Inicia y detiene el servidor durante la ejecución del sistema. |
| Cliente | Aplicación que representa al usuario y se comunica con el servidor mediante TCP. |
| Servidor | Componente central que administra las conexiones, sesiones y distribución de mensajes. |

## 3. Entradas y salidas

### Entradas

- Dirección IPv4 y puerto del servidor proporcionados al cliente.
- Username enviado mediante `HELLO <username>\\n`.
- Mensajes enviados mediante `MSG <message>\\n`.
- Solicitud de cierre mediante `QUIT\\n`.
- Señales `SIGINT` o `SIGTERM` dirigidas al servidor.

### Salidas

- `OK\\n` para confirmar una operación aceptada.
- `ERR <reason>\\n` para informar operaciones rechazadas.
- `FROM <username> <message>\\n` para distribuir un mensaje a los demás
  usuarios identificados.
- Cierre de la conexión cuando una sesión termina o el servidor se apaga.

## 4. Funcionamiento general

### 4.1 Inicio del servidor

1. El servidor crea un socket TCP IPv4.
2. Configura el socket para reutilizar la dirección y establece una espera
   limitada para revisar señales y workers terminados.
3. Asocia el socket al puerto configurado, por defecto `5050`.
4. Comienza a escuchar conexiones entrantes.
5. Permanece aceptando clientes mientras no se solicite el cierre.

### 4.2 Conexión e identificación

1. El cliente establece una conexión TCP.
2. El servidor crea una sesión independiente para esa conexión.
3. El cliente envía `HELLO <username>\\n`.
4. El servidor valida el username y comprueba que no esté siendo utilizado por
   otra sesión.
5. Si es válido, responde `OK\\n` y asocia el username con la sesión.
6. Si es inválido o está ocupado, responde con el error correspondiente y la
   sesión puede continuar para intentar otra identificación o ejecutar `QUIT`.

### 4.3 Envío de mensajes

1. Un cliente identificado envía `MSG <message>\\n`.
2. El servidor valida el mensaje.
3. El emisor recibe `OK\\n`.
4. El servidor obtiene una copia de los demás usuarios conectados.
5. Cada destinatario recibe `FROM <username> <message>\\n`.
6. La misma conexión puede repetir el proceso para múltiples mensajes.

Los mensajes recibidos por un destinatario son eventos asíncronos respecto de
su propia entrada. Por ello, un cliente completo debe mantener una operación
de recepción independiente de la entrada del usuario.

### 4.4 Cierre de sesión

1. El cliente envía `QUIT\\n`.
2. El servidor responde `OK\\n`.
3. El username se libera.
4. La sesión se cierra y el servidor conserva las demás conexiones activas.

Una desconexión inesperada produce el mismo efecto de limpieza de la sesión,
sin detener el servidor.

### 4.5 Cierre del servidor

Ante `SIGINT` o `SIGTERM`, el servidor:

1. deja de aceptar nuevas conexiones;
2. solicita el cierre de las sesiones activas mediante `shutdown()`;
3. espera la terminación de los workers;
4. libera los recursos y cierra el socket de escucha.

## 5. Reglas funcionales principales

- El username debe tener entre 1 y 32 bytes, sin espacios ni controles ASCII.
- Los usernames distinguen mayúsculas y minúsculas.
- Un username activo no puede estar registrado simultáneamente en dos sesiones.
- `MSG` requiere una identificación válida previa.
- El texto de un mensaje debe tener entre 1 y 986 bytes y no contener NUL,
  CR ni LF.
- Cada línea de protocolo utiliza `\\n` como delimitador.
- El servidor reconstruye líneas fragmentadas y puede procesar varias líneas
  recibidas en un mismo bloque TCP.
- `QUIT` puede ejecutarse incluso antes de `HELLO`.
- Un error de protocolo no debe detener el servidor completo.
- Una sesión defectuosa se aísla de las demás conexiones.

## 6. Estado actual de las implementaciones

### Servidor

El servidor implementa actualmente las funciones descritas: conexiones
simultáneas, sesiones persistentes, identificación, mensajes, broadcast,
desconexión, framing, validaciones y apagado controlado.

### Cliente CLI

El cliente CLI integrado en `main` permite:

- recibir IP y puerto mediante argumentos;
- establecer la conexión;
- enviar `HELLO`;
- enviar múltiples `MSG`;
- solicitar `QUIT`;
- recibir las respuestas directas del servidor.

El servidor ya soporta eventos `FROM` asíncronos, pero el cliente CLI actual
todavía utiliza un flujo de entrada/respuesta secuencial y no muestra esos
eventos mientras el usuario permanece sin enviar un mensaje. Esta limitación
queda explícitamente identificada para la evolución de `feature-client`.

## 7. Límites y funcionalidades no incluidas

Actualmente no existen:

- autenticación o contraseñas;
- TLS o cifrado de transporte;
- almacenamiento persistente;
- historial de mensajes;
- salas o mensajería privada;
- límite configurable de conexiones simultáneas;
- heartbeat o timeout de inactividad;
- garantía de entrega del broadcast;
- interfaz gráfica.

El servidor no valida la codificación UTF-8; los clientes deben acordar la
codificación utilizada.
