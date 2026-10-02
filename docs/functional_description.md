# Descripción funcional

## Propósito

Linux TCP Messenger es un sistema educativo de mensajería cliente-servidor para
Debian Linux. Usa sockets POSIX, TCP y un protocolo de texto delimitado por
saltos de línea. El servidor centraliza las conexiones y mantiene en memoria
los usuarios activos durante la ejecución.

## Funcionamiento general

1. El servidor abre el puerto TCP configurado en `protocol::kDefaultPort`.
2. Un cliente establece una conexión y envía `HELLO <username>\n`.
3. El servidor acepta el nombre si es válido y no está en uso.
4. La misma conexión puede transportar varios comandos `MSG`.
5. Los mensajes se entregan a los otros usuarios identificados como
   `FROM <username> <message>\n`.
6. `QUIT`, un cierre remoto o un error termina solamente esa sesión y libera el
   username.
7. `SIGINT` o `SIGTERM` detiene el servidor y solicita el cierre de las sesiones.

## Servidor implementado

- Acepta conexiones mientras permanece activo.
- Crea un thread joinable por conexión.
- Reconstruye líneas aunque lleguen fragmentadas y conserva líneas adicionales.
- Distingue sesiones conectadas, identificadas y en cierre.
- Mantiene usernames únicos con acceso protegido por mutex.
- Serializa los envíos concurrentes hacia cada socket.
- El emisor de `MSG` recibe `OK`; el broadcast se envía solo a los demás.

## Cliente

La interfaz asumida para `feature-client` es:

```bash
./build/client <IP-servidor> <puerto>
```

La interfaz envía `HELLO`, mantiene un loop para `MSG` y finaliza mediante
`QUIT`, leyendo respuestas terminadas en `\n`. GitHub integró ese cliente en
`main` antes de este milestone. No se modificó su implementación. El archivo
`project_status.txt` indicado como fuente funcional no está versionado en las
ramas inspeccionadas; se mantiene la interfaz funcional indicada por el equipo.

## Límites actuales

- No hay contraseñas, cifrado, cuentas persistentes ni historial.
- Los usernames son sensibles a mayúsculas, de 1–32 bytes y sin espacios ni
  controles ASCII; esta regla permite interpretar FROM sin ambigüedad.
- No hay timeout de inactividad de sesión. La espera de aceptación es de
  200 ms y un envío bloqueado tiene timeout de 2 s por llamada.
- Se recolectan sesiones terminadas durante la ejecución, sin esperar al cierre
  global. No hay un máximo configurado de conexiones simultáneas.
- El cliente necesita una estrategia de recepción apropiada para mostrar
  broadcasts que puedan llegar fuera de una solicitud inmediata.
- Las pruebas formales pertenecen a `feature/testing` y siguen pendientes.
- La desconexión física de un peer sin FIN/RST puede tardar en ser detectada por
  TCP. No hay heartbeat ni garantía de entrega del broadcast.
