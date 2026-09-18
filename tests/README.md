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
| **TC01** | Protocolo | Identificación correcta | `HELLO Andrew\n` | `OK\n` | PENDIENTE |  |
| **TC02** | Protocolo | Envío de mensaje tras login | `MSG Hola\n` | `FROM Andrew Hola\n` en otros clientes | PENDIENTE |  |
| **TC03** | Protocolo | Salida normal | `QUIT\n` | Cierre limpio de socket, servidor activo | PENDIENTE |  |
| **TC04** | Protocolo | Mensaje sin identificar | `MSG Hola\n` antes de `HELLO` | `ERR <reason>\n` o desconexión | PENDIENTE |  |
| **TC05** | Protocolo | Comando inválido | `ALGO Hola\n` | `ERR invalid_command\n` | PENDIENTE |  |
| **TC06** | Resiliencia | Desconexión abrupta | Cerrar cliente (Ctrl+C / Kill) | Servidor detecta `recv() == 0` y no cae | PENDIENTE |  |
| **TC07** | Framing | Mensaje fragmentado | Recibir `"MSG Ho"` y luego `"la\n"` | Reconstrucción correcta del mensaje | PENDIENTE |  |
| **TC08** | Framing | Múltiples mensajes juntos | Recibir `"MSG 1\nMSG 2\n"` en un bloque | Procesamiento independiente de cada comando | PENDIENTE |  |
| **TC09** | Concurrencia | Multiusuario básico | 3+ clientes conectados enviando | Broadcast correcto sin mezcla de datos | PENDIENTE |  |
| **TC10** | Limites | Username/mensaje largo | Exceder buffer máximo | Rechazo controlado o truncamiento seguro | PENDIENTE |  |

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
