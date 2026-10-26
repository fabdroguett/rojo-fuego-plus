# Auditoría del proyecto

Fecha de inspección: 2026-10-26. Revisión inicial de solo lectura.

## Identidad y arquitectura

- Proyecto: Rojo Fuego Plus, hack ROM de Pokémon Rojo Fuego en español.
- Base: decompilación de `pokeemerald-expansion`, configurada como `BUILD=firered`, código de juego BPRE. No es una ROM tradicional editada únicamente con parches ni el repositorio puro `pret/pokefirered`.
- La README identifica la base como expansión 1.17.1; `include/constants/expansion.h` define 1.17.2 y marca la versión como no etiquetada. Conviene tratar el encabezado compilado como la evidencia del árbol actual y revisar la README cuando se retome su mantenimiento.
- Lenguajes: C, ensamblador ARM, Python, PowerShell y Bash. Build con GNU Make, `arm-none-eabi-gcc` y herramientas de conversión de recursos del propio repositorio.
- El repositorio contiene aproximadamente 30.413 archivos rastreados; `src/` incluye unos 399 C, 174 headers y 5 archivos `.s`. Hay 939 directorios de mapa con `map.json` y 91 scripts de evento `.inc` (conteos por extensión/ruta, pueden variar con cambios locales).

## Sistemas y recursos

- Combate, especies, movimientos, habilidades, evoluciones, objetos, mapas, eventos, scripts, guardado y menús residen principalmente en `src/`, `include/`, `data/` y `constants/`.
- Extensiones propias identificables: `rf_collection`, `rf_exp_reserve`, `rf_legends`, `rf_quest_events`, `rf_quest_menu`, `rf_quests`, `rf_starter_menu`, `rf_training`, `rf_traveler`, `rf_ui_theme` y `rf_wild_loot`, junto con cambios a sistemas base.
- Catálogos actuales aproximados: 1.673 identificadores de especies/formas, 956 movimientos, 320 habilidades y 937 objetos, según escaneo de constantes; estos conteos no equivalen necesariamente a entidades jugables únicas.
- Inventario aproximado: `graphics/` 19.234 archivos (incluye PNG y paletas); `sound/` 2.077 archivos (WAV, MIDI, PCM e inclusiones). El hardware GBA limita el formato y tamaño efectivo de los recursos compilados.
- `SaveBlock3` tiene un límite/assert de 1.624 bytes en `include/global.h`. Los documentos locales describen almacenamiento SAVE1 y una migración SAVE2/SAVE3 aún por resolver; cualquier cambio exige revisar datos existentes y compatibilidad.

## Herramientas, build y pruebas detectadas

- Windows ofrece WSL Ubuntu-24.04 y Docker Desktop. El flujo documentado del proyecto sincroniza a `/work` en WSL y compila allí; GNU Make no está disponible directamente en Windows.
- En WSL se detectaron GNU Make 4.3, Python 3.12.3 y `arm-none-eabi-gcc` 13.2.1. En Windows, Python 3.15, Node 26 y Git 2.56. Las versiones son la observación de esta máquina, no requisitos fijados.
- mGBA está instalado bajo `C:\Program Files\mGBA`; hay harnesses y scripts de captura/verificación, pero su presencia no demuestra que cada escenario sea automatizable sin intervención.
- Pruebas existentes: unittest Python bajo `tools/rojo_fuego/tests`, validación de misiones `tools/rojo_fuego/validate_quests.py`, harness C, `make check`/runner de la decompilación y wrapper WSL `tools/rojo_fuego/wsl.ps1` (`test`, `build`, `check`). No se ejecutaron pruebas durante esta auditoría inicial.
- En `build_artifacts/wsl/` se encontró una ROM anterior de 32 MiB; es un artefacto previo, no una validación nueva. El historial de trabajo previo registra EWRAM 232.944/262.144 B e IWRAM 28.632/32.768 B; tratarlo como referencia histórica y volver a medir en la siguiente build.

## Estado Git y conservación

La rama activa es `rojo-fuego`, HEAD `cdd26189` (`Rewrite README for Rojo Fuego Plus`). El árbol ya tenía 39 entradas modificadas/no rastreadas al iniciar. Incluyen cambios de mochila, misiones, interfaz, validadores, harnesses y scripts de WSL; se consideran trabajo preexistente y deben conservarse íntegros. No hay submódulos registrados.

También existen `.impeccable/` y `.taste-skill/` ignorados, con archivos de dirección visual/capturas. Se conservan; no son una instalación verificable de las skills homónimas. No se encontró `AGENTS.md`, `CLAUDE.md` ni `SKILL.md` de proyecto antes de esta suite.

## Dependencias y licencias

- Las dependencias de build se enumeran en `INSTALL.md`; no se instalaron herramientas ni paquetes durante esta auditoría.
- Hay licencias individuales para varias herramientas bajo `tools/`; se debe respetar cada licencia y `CREDITS.md` para recursos. No se halló un `LICENSE` único en la raíz; esto no significa que todo el árbol comparta una licencia.
- La ROM de origen debe ser aportada legalmente por la persona que compila; el repositorio declara no distribuir ROM ni partidas.
- Skills externas revisadas se comparan en `01-skills-inventory.md`. No se descargaron, copiaron ni instalaron.

## Límites y riesgos

1. El margen de EWRAM/IWRAM histórico es reducido; toda expansión debe medir memoria enlazada y no solo ROM.
2. Cambios de guardado pueden invalidar partidas; crear una política de versión/migración y fixtures antes de modificar el formato.
3. `README.md` y encabezado de expansión discrepan en versión.
4. La sincronización WSL elimina archivos del snapshot `/work` que desaparecieron del origen; Windows es fuente de verdad según el script. No ejecutar `stop` ni modificar el snapshot sin necesidad.
5. El árbol ya contiene cambios no confirmados, incluyendo fuentes y tests. Evitar checkout/reset/clean y respaldar cualquier destino que un instalador actualice.
6. El volumen de assets y mapas requiere validadores incrementales; un escaneo completo puede ser lento.
7. Godot/SDL/raylib son alternativas para un futuro runtime, no dependencias actuales. No iniciar port o reescritura sin análisis de equivalencia y viabilidad.

## Clasificación preliminar para port

| Área | Acoplamiento GBA | Siguiente paso seguro |
|---|---|---|
| Reglas, datos y progresión | Mixto; lógica C reutilizable con macros globales | aislar API, construir pruebas de comportamiento |
| Renderizado/tiles/paletas | Alto | definir adaptador de presentación y mapeo de recursos |
| Audio | Alto | inventariar secuenciador/formatos y definir interfaz de reproducción |
| Entrada | Medio/alto | extraer acciones abstractas sin cambiar controles actuales |
| Temporización | Alto | localizar VBlank/ticks/contadores y encapsular reloj |
| Memoria/DMA/VRAM | Directo | documentar límites y diseñar backend específico de plataforma |
| Guardado | Mixto | especificar esquema serializado y pruebas de ida/vuelta |
| Scripts/mapas | Mixto | inventariar formatos y referencias antes de transformarlos |

## Decisiones recomendadas

- Mantener FireRed decomp como producto de referencia y ejecutar infraestructura de calidad en WSL.
- Hacer que cada skill local apunte a rutas y comandos reales; cargar solo skills relacionadas con la solicitud.
- Comenzar el port con inventario y una prueba de equivalencia de un subsistema pequeño, no una reescritura.
- Mantener Godot como candidato para prototipo visual; SDL3 como opción C de bajo nivel; considerar raylib para prototipos/tools. Decisión final después de medir integración, plataformas objetivo y coste de migración.
- No instalar colecciones externas ahora: algunas requieren plugins/hooks, dependencias o convenciones ajenas al proyecto. Mantener un inventario fechado y una futura evaluación explícita.

## Evidencia pendiente

La siguiente ejecución solicitada debe correr: `python3 tools/rojo_fuego/validate_quests.py`, `python3 -m unittest discover -s tools/rojo_fuego/tests -v`, `bash tools/rojo_fuego/verify_code.sh` y, si las fuentes/entorno lo permiten, build FireRed. Registrar estado y salida real en `TESTING.md`.
