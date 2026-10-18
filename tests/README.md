# Plan de Pruebas, Integración y Validación

Este documento detalla la estrategia de validación, la matriz de pruebas y el seguimiento de casos de prueba para el **Sistema de Mensajería TCP en Linux**.

---
1. Topología de Red y Pruebas
Las pruebas se ejecutan considerando la interacción entre cliente(s) y servidor sobre TCP/IP:

```text
Cliente A ----\
Cliente B ----- > Servidor Debian (TCP Port: 8080)
Cliente C ----/

```

### Comprobaciones Previas de Infraestructura

* **Red (IP/ICMP):** Validar conectividad con `ping <IP_SERVIDOR>`.
* **Puerto TCP:** Verificar socket activo en escucha usando `ss -tuln | grep <PUERTO>`.

---

## 2. Checklist de Casos de Prueba
| ID | Categoria | Descripción del Test | Entrada / Acción | Resultado Esperado | Estado | Observaciones |
| --- | --- | --- | --- | --- | --- | --- |
| **TC01** | Protocolo | Identificación correcta | `HELLO Andrew\n` | `OK\n` | PASS | Cumple con lo esperado |
| **TC02** | Protocolo | Envío de mensaje tras login | `MSG Hola\n` | `OK\n` al emisor y `FROM Andrew Hola\n` a otros clientes identificados | PASS | Verificar que el servidor difunde correctamente las tramas MSG enviadas por un usuario autenticado hacia los demás clientes conectados a la sesión |
| **TC03** | Protocolo | Salida normal | `QUIT\n` | `OK\n` , EOF y servidor activo | PASS | Verificar que el servidor procesa el comando QUIT de forma correcta, finalizando el thread del cliente, cerrando el socket TCP sin generar errores de recursos y manteniendo la disponibilidad del servidor para otros clientes |
| **TC04** | Protocolo | Mensaje sin identificar | `MSG Hola\n` antes de `HELLO` | `ERR not_identified\n` ; sesión abierta | PASS | Verificar que el servidor bloquea la ejecución del comando MSG si el cliente no ha enviado previamente la trama de registro e identificación HELLO el servidor dara respuesta de la siguiente manera ERR invalid_username |
| **TC05** | Protocolo | Comando inválido | `ALGO Hola\n` | `ERR invalid_command\n` | PASS | Verificar que el servidor valide la sintaxis del protocolo y el nombre de usuario, retornando las tramas de error ERR correspondientes sin cerrar la conexión ni provocar una caída del sistema; al ejecutar los comandos -> HELLO Carlos <- Servidor: ERR invalid_username y -> PING<- Servidor: ERR invalid_command |
| **TC06** | Resiliencia | Desconexión abrupta | Cerrar cliente (Ctrl+C / Kill) | Servidor detecta EOF o error de red; libera la sesión y permanece activo | PASS | Confirmar que el servidor detecta correctamente la interrupción repentina de la conexión TCP (por ejemplo, interrupción del proceso cliente vía Ctrl+C) y maneja el error de socket de forma segura sin colapsar el servicio. El servidor debe responder de la siguiente manera "Error en recv(): Connection reset by peer" |
| **TC07** | Framing | Mensaje fragmentado | Recibir `"MSG Ho"` y luego `"la\n"` | Reconstrucción correcta del mensaje | PASS | Se envía el comando dividido en dos partes con sleep intermedio. El servidor espera el salto de línea \n antes de procesar el mensaje completo de forma correcta. |
| **TC08** | Framing | Múltiples mensajes juntos | Recibir `"MSG 1\nMSG 2\n"` en un bloque | Procesamiento independiente de cada comando | PASS | Se envían varios comandos juntos en un solo bloque. El servidor procesa e interpreta cada instrucción de manera independiente. |
| **TC09** | Concurrencia | Multiusuario básico | 3+ clientes conectados enviando | Broadcast correcto sin mezcla de datos | PASS | Se abrieron 3 terminales en simultáneo enviando mensajes. El servidor realizó la difusión (broadcast) correctamente entre todos los clientes conectados. |
| **TC10** | Limites | Username/mensaje largo | Exceder límites de username, texto y línea | `ERR invalid_username` (username >32 con línea ≤1024), `ERR invalid_message` (texto >986 con línea ≤1024); `ERR message_too_long` y cierre (línea >1024). Sin truncamiento | PASS | El servidor identificó que el mensaje superaba el tamaño máximo permitido, rechazó la trama respondiendo ERR message_too_long y la conexión continuó activa para procesar subsiguientes mensajes correctamente. |
 

---

## 3. Guía de Ejecución Rápida (Prueba Manual)

Para probar manualmente el puerto TCP sin compilar el cliente:

```bash
# Conectar al servidor
nc <IP_SERVIDOR> <PUERTO>

# Flujo básico de prueba
HELLO Andrew
MSG Probrando el chat
QUIT

```

---

## 4. Flujo de Git y Pruebas

1. Crear rama de trabajo: `git checkout -b feature/testing`
2. Actualizar la tabla de pruebas pasando el estado de `PENDIENTE` a `PASS` o `FAIL`.
3. Crear **Pull Request** apuntando hacia la rama `develop`.

```

### Pasos para guardarlo en GitHub:
1. Borra el texto actual en el editor web de `tests/README.md`.
2. Pega el código Markdown de arriba.
3. Haz clic en el botón verde **Commit changes...** arriba a la derecha.

```
