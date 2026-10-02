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
| FR-004 | Probar usernames vacío, válido y largo | Unit/Integración | Solo el válido recibe `OK` | Pending formal test | Testing partner |
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
