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

Envía `HELLO`, mantiene un loop para `MSG` y finaliza mediante `QUIT`, leyendo
respuestas terminadas en `\n`. Esta rama integra los commits recientes de
`origin/feature-client` sin reescribir su implementación. El archivo
`project_status.txt` indicado como fuente funcional no está versionado en
ninguna rama remota inspeccionada.

## Límites actuales

- No hay contraseñas, cifrado, cuentas persistentes ni historial.
- Los usernames son sensibles a mayúsculas y pueden contener espacios, pero no
  saltos de línea.
- No existen timeouts de conexión o de envío.
- Los objetos de sesión y los threads finalizados se conservan hasta detener el
  servidor para poder hacer `join`; los sockets y usernames sí se liberan al
  terminar cada sesión.
- El cliente necesita una estrategia de recepción apropiada para mostrar
  broadcasts que puedan llegar fuera de una solicitud inmediata.
- Las pruebas formales pertenecen a `feature/testing` y siguen pendientes.
