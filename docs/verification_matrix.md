# Matriz de verificación propuesta

Esta matriz no representa una suite formal. Los estados **Development check**
corresponden únicamente a comprobaciones manuales de la rama del servidor. El
responsable de `feature/testing` deberá diseñar, automatizar y registrar las
pruebas formales.

| Requisito | Método propuesto | Tipo futuro | Resultado esperado | Estado actual | Responsable formal |
|---|---|---|---|---|---|
| FR-001 | Conectar al puerto activo | Integración | Conexión aceptada | Development check | Testing partner |
| FR-002 | Enviar varios comandos por un socket | Integración | Todos reciben respuesta | Development check | Testing partner |
| FR-003 | Fragmentar y agrupar líneas | Unit/Integración | Una línea por lectura lógica | Development check | Testing partner |
| FR-004 | Probar usernames vacío, válido, con espacios y largo | Unit/Integración | Solo el válido recibe `OK` | Development check | Testing partner |
| FR-005 | Conectar dos clientes con el mismo nombre | Integración | Segundo recibe `ERR username_in_use` | Development check | Testing partner |
| FR-006 | Enviar `MSG` antes de `HELLO` | Integración | `ERR not_identified` | Development check | Testing partner |
| FR-007 | Enviar `MSG` con dos usuarios | Integración | Emisor recibe `OK`; otro recibe `FROM` | Development check | Testing partner |
| FR-008 | Enviar `QUIT` | Integración | `OK`, EOF y servidor activo | Development check | Testing partner |
| FR-009 | Cerrar abruptamente y reutilizar nombre | Integración | Username disponible de nuevo | Development check | Testing partner |
| FR-010 | Mantener dos conexiones activas | Integración | Ambas progresan independientemente | Development check | Testing partner |
| FR-011 | Enviar más de 1024 bytes sin `\n` | Integración | `ERR message_too_long` y cierre | Development check | Testing partner |
| FR-012 | Enviar `SIGINT` | Sistema | Workers terminan y proceso retorna 0 | Development check | Testing partner |
| NFR-001 | Configurar y compilar con CMake | Build | Ambos ejecutables se generan | Development check | Testing partner |
| NFR-002 | Inspección de dependencias | Revisión | Solo C++17/POSIX/Threads | Pending formal review | Testing partner |
| NFR-003 | Ejecutar carga concurrente y detector de carreras | Sistema | Sin carreras ni interbloqueos | Pending formal test | Testing partner |
| NFR-004 | Revisión arquitectónica | Revisión | Responsabilidades separadas | Pending formal review | Team review |
| NFR-005 | Revisar cierre de recursos | Sistema | Sin descriptores/threads activos | Pending formal test | Testing partner |
| NFR-006 | Repetir conexiones y apagar con clientes bloqueados | Sistema | Workers se recolectan y apagado termina | Development check | Testing partner |

## Comprobaciones del milestone (2026-10-02)

Build CMake y smoke checks temporales fuera del repositorio: tres usuarios,
HELLO fragmentado, dos MSG juntos, broadcast a dos destinatarios, errores de
orden/comando, HELLO repetido, nombres inválidos y duplicados, QUIT con EOF,
desconexión abrupta y reutilización del nombre. Se comprobaron límites de 32
bytes de username, 986 de texto y rechazo de 1025 bytes sin delimitador.

Después de 30 sesiones cortas, el número de descriptores regresó al nivel
inicial y no quedaron sus threads. SIGTERM terminó con usuarios inactivos;
SIGINT y el reinicio inmediato también funcionaron. El cliente CLI existente
completó HELLO, dos MSG y QUIT.
El apagado durante envíos grandes a un peer que no lee terminó correctamente;
el error Broken pipe se aisló en la sesión.

Todos los casos siguen **Pending formal test** para `feature/testing`, incluso
los marcados Development check. No se ejecutaron detectores de carreras ni una
suite formal. Validar especialmente emisiones simultáneas, resets de TCP,
clientes lentos, límites de bytes UTF-8 y el último frame de QUIT.
