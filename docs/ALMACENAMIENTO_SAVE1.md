# SAVE-1: capacidad y estrategia de almacenamiento

Medido el 23-10-2026 sobre el ELF FireRed compilado con GCC ARM del contenedor local.
La fixture desechable `tools/rojo_fuego/test/progression_fixture.c` exporta `gRFSaveLayout`;
`verify_progression.py` lee sus valores en mGBA y escribe `build_artifacts/progression/save-layout.json`.
No se usan valores aproximados de comentarios de estructuras.

| Estructura | Bytes usados | Capacidad serializada | Libres |
|---|---:|---:|---:|
| SaveBlock1 | 15756 | 15872 | 116 |
| SaveBlock2 | 3884 | 3968 | 84 |
| SaveBlock3, incluido premio pendiente de Noa | 1532 | 1624 | 92 |
| PokemonStorage | 34144 | 35712 | 1568 |

Cada BoxPokemon ocupa 80 bytes y cada Pokemon de equipo 100. Una caja adicional de 30
Pokémon requiere al menos 2400 bytes, más nombre y wallpaper. Por tanto, no cabe otra caja
en los nueve sectores de almacenamiento actuales. Los huecos de otros bloques tampoco
alcanzan para una caja completa. Aumentar el límite de ROM no cambia estos límites.

Offsets reales de PokemonStorage: cajas 4, nombres 33604, wallpapers 33730 y fusiones 33744.
El almacenamiento mantiene cuatro Pokémon de fusión. Cambiar el tamaño del array de cajas
desplazaría nombres, wallpapers y fusiones de partidas anteriores.

## Decisión de arquitectura para el prototipo SAVE-2

Separar acceso lógico a cajas de su almacenamiento físico antes de aumentar el número.
Conservar la representación de las 14 cajas existentes y migrar mediante una versión explícita.
No habilitar cajas adicionales hasta tener escritura y recuperación verificadas.

El primer prototipo de ampliación será un banco adicional con **una caja**, con copias
alternas y checksum en los sectores 30/31. Esta decisión implica reemplazar los usos de
Trainer Hill/Recorded Battle de esos sectores en esa modalidad de guardado; no se puede
reservarlos a la vez para ambas funciones. Primero inventariar todos sus lectores/escritores,
incluido borrado tras Salón de la Fama, guardado por enlace y recuperación de errores.

No convertir ese prototipo en formato de entrega hasta verificar compatibilidad con las
funciones que ya usa FireRed. Si algún uso activo impide reservar esos sectores, el prototipo
debe permanecer aislado y se continúa con una representación comprimida/versionada.

Un banco debe contener magic, versión, contador de guardado asociado, checksum, nombre,
wallpaper y Pokémon. La escritura debe preservar una copia válida y asociar la extensión al
slot principal correcto; cargar un slot de respaldo no puede recuperar Pokémon de otro contador.
Los fallos de lectura deben detectarse, nunca presentarse como una caja vacía y sobrescribirse.

Este prototipo sirve para probar migración y acceso por bancos; **no fija la capacidad final**.
Ampliaciones mayores necesitan compresión con capacidad garantizada o una nueva estrategia
de persistencia. No prometer más plazas contando con que las cajas estén parcialmente vacías.

## Próximas verificaciones de SAVE-2

- [ ] Enrutar accesos directos a cajas/nombres/wallpapers por funciones comunes y comprobar límites.
- [ ] Inventariar sectores 30/31, formatos actuales y todos los modos de guardado.
- [ ] Implementar el banco en una prueba aislada; dejar formato de entrega anterior intacto.
- [ ] Probar datos válidos, checksums dañados, rollback de slot, interrupción de escritura,
  cambios de clave de cifrado, fusiones y cajas llenas.
- [ ] Determinar capacidad final y cerrar compatibilidad antes de habilitar nuevas cajas.

Los nuevos eventos de misiones no agregan campos persistentes: usan los dos bits ya reservados
para cada misión. Noa agrega ocho bytes al final de SaveBlock3, con magic de inicialización
independiente; no se cambian offsets de campos anteriores.

La mochila de FireRed mantiene sus 30 espacios originales en SaveBlock1 y 30 adicionales
en la extensión previa de SaveBlock3. La capacidad nueva añade 20 espacios (84 bytes con
magic) al final de SaveBlock1, usando la holgura medida de ese bloque; no inserta campos ni
desplaza offsets existentes. Cada extensión tiene magic propio y se inicializa sin borrar
los objetos de la otra. Una aserción de compilación limita SaveBlock1 a los 15872 bytes
serializados, y otra mantiene SaveBlock3 bajo 1624 bytes. Los espacios adicionales se
validan en `tools/rojo_fuego/tests/test_bag.py`.
