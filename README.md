# Rojo Fuego Plus

ROM hack de **Pokémon Rojo Fuego** para Game Boy Advance, en español, construido sobre
[pokeemerald-expansion 1.17.1](https://github.com/rh-hideout/pokeemerald-expansion) en modo FireRed.
Conserva la historia y los mapas de Kanto y las Islas Sete, y añade las 1025 especies, mecánicas
modernas, misiones, servicios de entrenamiento y una interfaz renovada con tema oscuro.

> Este repositorio contiene **solo código fuente y recursos propios o con licencia libre**.
> No incluye ni distribuye ROMs ni partidas guardadas. Necesitas tu propia copia legal del juego.

## Qué incluye

**Pokémon y mundo**
- **Las 1025 especies obtenibles.** De día siguen los encuentros originales de Kanto; por la tarde
  y la noche aparecen las especies de las generaciones 2–9, repartidas por hábitat y nivel.
- **117 legendarios, singulares, Ultraentes y paradójicos** en 28 misiones de saga. Al aceptar una
  saga, sus Pokémon aparecen en lugares especiales del mapa ([ubicaciones](docs/LEYENDAS.md)).
- **Pokémon salvajes visibles** por el mapa (hasta seis a la vez), Pokémon que te sigue, DexNav
  rediseñado, brotes, ciclo de día y noche, y grutas ocultas diarias.
- **Selector de iniciales** con 54 opciones (generaciones 1–9 y extras) y habilidad oculta opcional
  ([detalle](docs/INICIALES.md)).

**Progresión y servicios**
- **71 misiones** en nueve capítulos, con seguimiento, recompensas y misiones por eventos.
- Logros, Omni Ring (megaevolución, movimientos Z, Dinamax y Teracristalización), guardianes
  de megapiedras y pruebas de cristales Z.
- Recordador de movimientos con preevoluciones, tutores por medalla, editor gratuito de EV/IV,
  reposición de bayas y retos diarios de la entrenadora itinerante Noa
  ([servicios](docs/SERVICIOS_ENTRENAMIENTO.md)).
- **Sistema de Claves:** cada mejora se puede desactivar de forma persistente, sin parches aparte.

**Interfaz**
- Menús nativos renovados a 240×160, tema oscuro por defecto y claro opcional
  ([guía de diseño](DESIGN.md)).
- Iconos de menú del [1-Bit Pack de Kenney](https://kenney.nl/assets/1-bit-pack) (CC0) y glifos de
  botones en los pies de pantalla.

El plan completo y su estado están en [docs/PLAN_COMPLETO.md](docs/PLAN_COMPLETO.md), y el punto
de trabajo actual en [docs/CONTINUIDAD.md](docs/CONTINUIDAD.md).

## Jugar

1. Compila la ROM (siguiente sección) o pídesela a quien la compile para ti.
2. Juega en **mGBA** (PC o Android). Empieza una **partida nueva**: el formato de guardado ha
   cambiado durante el desarrollo.
3. Para ver las misiones, abre el menú con START y pulsa R (requiere la Pokédex).

## Compilar

El proyecto usa la cadena de compilación de pokeemerald-expansion. En Linux o WSL, con las
dependencias de [INSTALL.md](INSTALL.md):

```sh
make BUILD=firered -j$(nproc)
```

El resultado es `pokefirered.gba`. Las ROMs, `build/` y los `.sav` están ignorados por Git; no
los subas.

### Herramientas del proyecto

Los datos generados se crean con scripts de `tools/rojo_fuego/` (Python 3, sin dependencias):

| Script | Genera |
|---|---|
| `encounters.py` | Tablas de tarde y noche de `src/data/wild_encounters.json` |
| `legends.py` | Sagas legendarias: scripts, mapas, misiones y [docs/LEYENDAS.md](docs/LEYENDAS.md) |
| `make_menu_icons.py` | Iconos de menú desde `graphics/rojo_fuego/icons/` |
| `content.py`, `grottos.py` | Guardianes, pruebas Z y grutas ocultas |
| `validate_quests.py` | Validación del catálogo de misiones |

Las pruebas nativas se ejecutan con `python3 -m unittest discover -s tools/rojo_fuego/tests`.
Las pruebas en emulador y su procedimiento están en
[tools/rojo_fuego/test/UI_VALIDATION.md](tools/rojo_fuego/test/UI_VALIDATION.md).

## Créditos

- Basado en [pokeemerald-expansion](https://github.com/rh-hideout/pokeemerald-expansion) de
  **RHH (Rom Hacking Hideout)** 1.17.1, que a su vez parte de
  [pret/pokeemerald](https://github.com/pret/pokeemerald) y [pret/pokefirered](https://github.com/pret/pokefirered).
  Consulta la [lista de colaboradores](CREDITS.md).
- Iconos de menú: [Kenney](https://kenney.nl), 1-Bit Pack (CC0).
- Forma arena de Castform: Mikitari, [Free Fakemon Pack](https://eeveeexpo.com/resources/1517/).
- Pokémon es una marca de Nintendo, Creatures Inc. y GAME FREAK inc. Este es un proyecto de
  aficionados sin ánimo de lucro, no afiliado a ellas.

```
Based off RHH's pokeemerald-expansion 1.17.1 https://github.com/rh-hideout/pokeemerald-expansion/
```
