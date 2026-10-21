# Matriz de verificación

Esta matriz relaciona los requisitos del sistema con las verificaciones
previstas durante el desarrollo y la integración del proyecto.

Los estados **Development check** corresponden únicamente a comprobaciones
manuales realizadas durante el desarrollo del servidor y no sustituyen las
pruebas formales del equipo de testing.

Los resultados registrados por el responsable de testing se mantienen en
[`tests/README.md`](../tests/README.md). Esta matriz no modifica ni reinterpreta
los estados PASS/FAIL allí registrados.

Las pruebas unitarias descritas en este documento se consideran propuestas
para completar la verificación del sistema y permanecen pendientes de
implementación o automatización por el responsable de testing.

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

## Pruebas unitarias propuestas

Las siguientes pruebas corresponden a componentes que pueden verificarse de
forma aislada. Su inclusión en esta matriz define la cobertura deseada, pero no
implica que actualmente exista una suite automatizada.

| ID | Componente | Verificación propuesta | Requisitos relacionados | Estado |
|---|---|---|---|---|
| UT-001 | `protocol::is_valid_username()` | Verificar usernames vacíos, válidos, de longitud límite, mayores de 32 bytes y con espacios o controles ASCII. | FR-004 | Pendiente de implementación por testing |
| UT-002 | `protocol::is_valid_message()` | Verificar mensajes vacíos, válidos, de longitud límite, mayores de 986 bytes y con caracteres no permitidos por el protocolo. | FR-007, FR-011 | Pendiente de implementación por testing |

## Pruebas de integración

Las pruebas de integración y sistema ejecutadas por el responsable de testing
se documentan en [`tests/README.md`](../tests/README.md).

Ese documento contiene actualmente los casos `TC01` a `TC10`, que cubren entre
otros:

- identificación mediante `HELLO`;
- envío de `MSG` y broadcast;
- cierre mediante `QUIT`;
- rechazo de mensajes antes de identificación;
- comandos inválidos;
- desconexión abrupta;
- framing de mensajes fragmentados;
- múltiples mensajes dentro de un mismo bloque TCP;
- múltiples clientes concurrentes;
- límites del protocolo.

Los estados y observaciones de estos casos pertenecen al registro de testing y
no se duplican en esta matriz para evitar mantener dos fuentes distintas de
resultados.

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

Las comprobaciones anteriores corresponden al milestone de desarrollo del
servidor y no sustituyen los resultados registrados posteriormente por el
responsable de testing.

Los estados actuales de los casos `TC01` a `TC10` deben consultarse en
[`tests/README.md`](../tests/README.md). La automatización de pruebas unitarias,
de integración y de análisis de concurrencia permanece como trabajo de testing.

Se recomienda mantener cobertura sobre emisiones simultáneas, resets TCP,
clientes lentos, límites de bytes UTF-8 y liberación de recursos.
