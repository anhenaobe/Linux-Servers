# Documentación del mini-proyecto

Esta carpeta reúne la documentación principal de **Linux TCP Messenger** para
el primer mini-proyecto.

## Entregables solicitados

| Entregable | Documento | Estado |
|---|---|---|
| Descripción funcional | [functional_description.md](functional_description.md) | Listo |
| Casos de uso o historias de usuario + requisitos del sistema | [requirements.md](requirements.md) | Listo |
| Arquitectura propuesta | [architecture.md](architecture.md) | Listo |

## Documentación técnica complementaria

| Documento | Propósito |
|---|---|
| [protocol.md](protocol.md) | Contrato de comunicación TCP, frames, límites y errores. |
| [interaction_diagrams.md](interaction_diagrams.md) | Secuencias de conexión, mensajes, cierre y errores. |
| [call_graph.md](call_graph.md) | Relación entre las principales funciones del servidor. |
| [SERVER_STATUS.txt](SERVER_STATUS.txt) | Estado técnico del milestone del servidor. |

## Fuera del alcance de este entregable

La **matriz de verificación** y las **pruebas formales** corresponden a la
responsabilidad de `feature/testing`. Se conservan en el repositorio para no
perder el trabajo del equipo, pero no se consideran parte de la documentación
que se entrega desde esta rama.

- [verification_matrix.md](verification_matrix.md)
- [../tests/README.md](../tests/README.md)

## Relación con la implementación

La documentación describe el sistema según el código integrado en `main`.
Cuando una capacidad del servidor todavía no está completamente expuesta por
el cliente CLI, se indica explícitamente en lugar de presentarla como
funcionalidad ya disponible para el usuario final.

La versión actual del contrato de comunicación es la **0.1**, documentada en
[protocol.md](protocol.md).
