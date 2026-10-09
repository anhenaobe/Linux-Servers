# Documentación del mini-proyecto

Esta carpeta reúne la documentación de **Linux TCP Messenger**. Cada documento
tiene un propósito distinto; los resúmenes deben enlazar la fuente de detalle
para evitar mantener copias independientes del mismo estado.

| Documento | Propósito | Responsable |
|---|---|---|
| [functional_description.md](functional_description.md) | Alcance funcional, comportamiento implementado y limitaciones actuales del cliente. | Server/client, revisión del equipo |
| [requirements.md](requirements.md) | Casos de uso, historias y requisitos esperados; no certifica su validación. | Equipo |
| [architecture.md](architecture.md) | Componentes, concurrencia, propiedad de recursos y flujo del servidor. | Server |
| [protocol.md](protocol.md) | Fuente de referencia del contrato 0.1: comandos, framing, límites y errores. | Server, acuerdo con client/testing |
| [interaction_diagrams.md](interaction_diagrams.md) | Secuencias de identificación, broadcast, cierre y errores. | Server |
| [call_graph.md](call_graph.md) | Llamadas principales de la implementación del servidor. | Server |
| [SERVER_STATUS.txt](SERVER_STATUS.txt) | Informe histórico del milestone estable del 2026-10-02, con comprobaciones de desarrollo y límites de ese estado. | Server |
| [verification_matrix.md](verification_matrix.md) | Relación entre requisitos y verificaciones propuestas; distingue comprobaciones de desarrollo de validación formal pendiente. | Testing, revisión del equipo |
| [../tests/README.md](../tests/README.md) | Plan de pruebas, resultados registrados y evidencia necesaria para reproducirlos. | Testing |

## Criterios de mantenimiento

- Comparar las afirmaciones de implementación con el código del commit revisado.
- Mantener el detalle del protocolo en `protocol.md`; los ejemplos resumidos
  deben respetar ese contrato. Cambiarlo requiere acuerdo con client y testing.
- Registrar las limitaciones del cliente en la descripción funcional y enlazarlas
  desde los resúmenes.
- Conservar `SERVER_STATUS.txt` como informe fechado; no presentarlo como una
  ejecución reciente de pruebas ni sobrescribir sus resultados históricos.
- Registrar commit, build, comandos y logs al declarar PASS/FAIL. Un smoke check
  de desarrollo no sustituye una prueba formal.
- Actualizar los documentos afectados en el mismo PR que cambie el comportamiento,
  con revisión del responsable correspondiente y destino `main`.

El servidor implementa el contrato de mensajería, pero el CLI todavía no ofrece
recepción continua de eventos `FROM`. Los requisitos describen el comportamiento
esperado; su existencia en un documento no implica que esté implementado o probado.
