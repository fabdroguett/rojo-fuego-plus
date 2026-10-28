# Motor de misiones (referencia para contenido)

Estado al 28-10-2026, tras la Fase B. Código: `src/rf_quests.c`, `src/rf_quest_events.c`,
`include/rf_quests.h`, `include/constants/rf_quests.h`; datos en `src/data/rf_quests*.h` y
`src/data/rf_quest_links.h`. Validador: `python tools/rojo_fuego/validate_quests.py`.

## Capacidad y guardado

- IDs estables 0–649. Los estados de 0–499 están en `rfQuestStates`; los de 500–649, en
  `rfQuestStatesExt`. Ambos usan 2 bits por misión (disponible, en curso, lista, reclamada).
- Extensión al final de `SaveBlock3` con magic propio (`QUEST_EXT_MAGIC`): una partida anterior
  conserva todos sus estados y solo inicializa la extensión. `SaveBlock3` ocupa 1620 de 1624 bytes.
- 4 contadores compartidos para objetivos con cantidad (ID en 10 bits, avance 0–63 en 6 bits).
  Solo los ocupan misiones contadas en curso; al completarse, el hueco se libera. Si los cuatro
  están ocupados, `RFQuests_Accept` rechaza otra misión contada (`RFQuests_CounterSlotsFull`).
- 32 bits de decisiones de historia (`RFQuests_SetChoice`/`GetChoice`, specials
  `Script_RF_QuestSetChoice`/`GetChoice` con `VAR_0x8004`). Son permanentes.

## Condiciones (`struct RFQuest.condition`)

| Condición | Argumentos | Se completa |
|---|---|---|
| `QUEST_VISIT` | mapa | al estar en el mapa |
| `QUEST_PARTY_EITHER` / `QUEST_PARTY_BOTH` | especies | con esas especies en el equipo |
| `QUEST_ITEM_OR_FOSSIL` / `QUEST_REVIVED_FOSSIL` | objeto | fósiles (casos originales) |
| `QUEST_EVENT` | evento, filtro | al recibir el evento mientras está en curso |
| `QUEST_DELIVER_ITEM` | objeto, cantidad | entrega explícita por script |
| `QUEST_LEGENDS` | saga | toda la saga capturada |
| `QUEST_DEX_CAUGHT` | especie | registrada como capturada |
| `QUEST_DEX_SEEN` | especie | vista (observar sin capturar) |
| `QUEST_COLLECT_ITEM` / `QUEST_USE_ITEM` | objeto | tenerlo / usarlo con éxito |
| `QUEST_BADGES` | 1–8 | con ese número de medallas |
| `QUEST_ACHIEVEMENT` | ID de logro | logro desbloqueado |
| `QUEST_COUNT` | evento, filtro, `target` 2–63 | tras `target` eventos que coincidan |

`objectiveCount = 2` añade un segundo requisito consultable (no un evento).

## Eventos (`RF_QUEST_EVENT_*`, nunca reordenar)

1 informe de Oak · 2 victoria contra Noa · 3 victoria contra entrenador (filtro: entrenador) ·
4 escena de campaña · 5 captura (especie base) · 6 captura en mapa (filtro obligatorio) ·
7 captura por tipo (filtro obligatorio; un tipo doble cuenta una vez por tipo) · 8 evolución ·
9 eclosión · 10 victoria contra salvaje. Filtro 0 = cualquiera. Las capturas y victorias en
combates de enlace, grabados o de instalaciones no cuentan.

## Tablas en ROM (`src/data/rf_quest_links.h`, sin coste de guardado)

- `sRFQuestRequirements`: requisitos extra. Por cada grupo (0–7) basta una misión reclamada; los
  grupos se suman (Y). Ejemplo: final = (A o B) y C.
- `sRFQuestBonuses`: premio compuesto al reclamar: objeto extra, dinero, flag (desbloquea diálogos
  u objetos con flag) y logro. Si algún objeto no cabe, no se entrega nada y la misión sigue lista.
- `sRFQuestObjects`: un objeto/NPC solo es visible con la misión en el rango de estados indicado.
  Se actualiza al cargar el mapa; un script que cambia el estado a mitad de escena debe añadir o
  retirar el objeto.
- `sRFQuestTiles`: casillas del mapa que cambian al alcanzar un estado (caminos, puentes). Se
  aplican después del script de carga propio del mapa.

El validador comprueba referencias, grupos, cantidades y ciclos entre prerrequisitos y requisitos.

## Avisos y guía

- Al pasar a LISTA por progreso real aparece «¡MISIÓN LISTA! nombre» cuando el jugador queda libre.
- Al entrar en el mapa de la escena de campaña seguida aparece «OBJETIVO AQUÍ: nombre».
- Los avisos de misión van antes que los de logros; un aviso interrumpido al instante (por el cartel
  del mapa) se vuelve a mostrar. Los avisos que ya no aplican se descartan.
- `Script_RF_QuestTrackedHint`: deja en `gStringVar1` el nombre y en `gStringVar4` la pista de la
  misión seguida (`VAR_RESULT` = FALSE si no hay ninguna). Pensado para Bill o el PC.

## Pendiente de usar en contenido (Fase C)

Las condiciones, contadores y tablas nuevas tienen pruebas C sobre el catálogo real; en mGBA se
comprobaron el guardado de la extensión y los dos avisos. Aún no hay misiones que usen el resto:
al crear la primera misión contada (captura, evolución, eclosión) o con tablas, añadir su
recorrido mGBA.
