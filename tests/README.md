# Plan de Pruebas, Integración y Validación

Este documento detalla la estrategia de validación, la matriz de pruebas y el seguimiento de casos de prueba para el **Sistema de Mensajería TCP en Linux**.

## 1. Topología de Red y Pruebas

Las pruebas se ejecutan considerando la interacción entre cliente(s) y servidor sobre TCP/IP:

```text
Cliente A ----\
Cliente B ----- > Servidor Debian (TCP Port: 5050)
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
| **TC03** | Protocolo | Salida normal | `QUIT\n` | `OK\n`, EOF y servidor activo | PASS | Verificar que el servidor procesa el comando `QUIT` de forma correcta, finalizando el thread del cliente, cerrando el socket TCP sin generar errores de recursos y manteniendo la disponibilidad del servidor para otros clientes |
| **TC04** | Protocolo | Mensaje sin identificar | `MSG Hola\n` antes de `HELLO` | `ERR not_identified\n`; sesión abierta | PENDIENTE |  |
| **TC05** | Protocolo | Comando inválido | `ALGO Hola\n` | `ERR invalid_command\n` | PENDIENTE |  |
| **TC06** | Resiliencia | Desconexión abrupta | Cerrar cliente (Ctrl+C / Kill) | Servidor detecta EOF o error de red; libera la sesión y permanece activo | PENDIENTE |  |
| **TC07** | Framing | Mensaje fragmentado | Recibir `"MSG Ho"` y luego `"la\n"` | Reconstrucción correcta del mensaje | PENDIENTE |  |
| **TC08** | Framing | Múltiples mensajes juntos | Recibir `"MSG 1\nMSG 2\n"` en un bloque | Procesamiento independiente de cada comando | PENDIENTE |  |
| **TC09** | Concurrencia | Multiusuario básico | 3+ clientes conectados enviando | Broadcast correcto sin mezcla de datos | PENDIENTE |  |
| **TC10** | Limites | Username/mensaje largo | Exceder límites de username, texto y línea | `ERR invalid_username` (username >32 con línea ≤1024), `ERR invalid_message` (texto >986 con línea ≤1024); `ERR message_too_long` y cierre (línea >1024). Sin truncamiento | PENDIENTE |  |

---

## 3. Guía de Ejecución Rápida (Prueba Manual)

Para probar manualmente con dos terminales, iniciar un servidor recién
compilado y conectar ambos clientes con `nc <IP_SERVIDOR> 5050`. Enviar
`HELLO Bob` en la segunda terminal antes de ejecutar el flujo de Andrew.
Pulsar Enter tras cada comando (LF); la notación `\n` representa ese byte,
no caracteres que deban escribirse literalmente.

```bash
nc <IP_SERVIDOR> 5050
```

Dentro de la conexión de Andrew, introducir cada línea y esperar su respuesta:

```text
HELLO Andrew
MSG Probando el chat
QUIT
```

Andrew debe recibir `OK` para cada comando y EOF tras QUIT. Bob debe recibir
`FROM Andrew Probando el chat` tras MSG y conservar su conexión abierta.

---

## 4. Evidencia y seguimiento

Los estados PASS/FAIL anteriores se conservan como resultados registrados por
el tester. El documento original no incluye commit probado, identificación del
binario, comandos completos ni logs; por eso no certifican por sí solos el
estado de la versión actual.

**TC02 requiere reproducción formal:** en la auditoría de `1269a59`, con build
nuevo fuera del repositorio, `HELLO Andrew` y varios `MSG` sobre una misma
conexión devolvieron `OK`; un segundo usuario recibió los `FROM` esperados.
También funcionaron QUIT y la continuidad del servidor. Son smoke checks de
desarrollo: no cambian el FAIL registrado ni reemplazan la validación del tester.
La causa del resultado original sigue sin determinarse; usar una rama o binario
antiguo es una hipótesis, no un hecho comprobado.

Para cada ejecución formal registrar:

- SHA (`git rev-parse HEAD`), rama y estado del working tree.
- Comandos de configuración/build y rutas de los ejecutables usados.
- IP/puerto, clientes utilizados y precondiciones (incluidos los HELLO).
- Entradas exactas, salidas/logs, resultado esperado y observado.
- Fecha y responsable de ejecución.

Los límites y errores esperados se definen en
[el contrato 0.1](../docs/protocol.md). La cobertura pendiente se relaciona con
los requisitos en [la matriz de verificación](../docs/verification_matrix.md).

## 5. Flujo de Git y Pruebas

1. Ejecutar `git fetch origin` y comprobar `git status` antes de integrar cambios.
2. Si existe la rama local, usar `git switch feature/testing`; si solo existe
   remotamente, usar `git switch --track origin/feature/testing`.
3. Con el working tree limpio, integrar `origin/main` en la rama de trabajo
   con `git merge origin/main` y resolver cualquier conflicto antes de probar.
4. Ejecutar las pruebas contra el código actualizado y registrar su evidencia.
5. Crear un Pull Request hacia `main` para revisar los cambios de testing.

La rama `develop` no existe en el remoto inspeccionado. No publicar resultados
nuevos directamente en `main` ni cambiar resultados anteriores sin explicar la
nueva ejecución que lo justifica.
