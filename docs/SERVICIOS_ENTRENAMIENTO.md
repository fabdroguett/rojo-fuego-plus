# Servicios de entrenamiento

Implementación del 23-10-2026, paquete SERVICIOS de PLAN_COMPLETO.md.

## Editor EV/IV

El entrenador de la quinta planta de los grandes almacenes de Azulona ofrece ahora
**Usar chapas**, **Editar EV**, **Editar IV reales** y **Salir**. El servicio de chapas
existente conserva sus costes y efecto; el editor es un servicio opcional gratuito,
seleccionado explícitamente. Trabaja sobre Pokémon del equipo; rechaza huevos, huevos
malos, ranuras vacías, selección cancelada y uso durante un combate.

Después de elegir Pokémon y estadística muestra el valor real y el máximo permitido.
Permite máximo, cero, sumar/restar 1, sumar/restar 10 y cambiar de estadística. Antes
de aplicar muestra el valor propuesto y pide confirmación. Cancelar o previsualizar
no cambia el Pokémon ni consume objetos.

- EV: máximo configurado por estadística (252 con la configuración actual) y 510 total.
  El máximo disponible descuenta los EV de las otras cinco estadísticas. No redistribuye
  ni elimina EV de otras estadísticas para permitir una subida.
- IV: valores reales 0–31. Cambian el dato genético, no solo su valor efectivo; al aplicar
  retira únicamente la marca de entrenamiento extremo de la estadística elegida. Los
  restantes IV, marcas de entrenamiento extremo, naturaleza y objetos se conservan.
- Tras un cambio se recalculan las estadísticas mediante CalculateMonStats del motor.
- No se agregan campos persistentes. Los valores se guardan en los datos normales del Pokémon.

Código: `src/rf_training.c`, `include/rf_training.h` y el script del NPC en
`data/scripts/rojo_fuego_npcs.inc`. El contrato del script usa 8004 para ranura, 8005 para
estadística, 8006 para modo, 800A para acción y 8007 para previsualización. Los límites
se verifican de nuevo al aplicar; la confirmación no es la única protección.

## Recordador

La ficha ya permitía recordar movimientos de nivel, huevo, MT disponibles y tutor.
Se habilitó P_PRE_EVO_MOVES para incluir movimientos de preevoluciones hasta el nivel
actual, sin habilitar anticipadamente todos los movimientos de nivel ni MT que no se poseen.
Los cuatro generadores deduplican y acotan las listas a 254 entradas más Cancelar, evitando
desbordamientos y manteniendo el contador u8 del menú.

## Consumibles equipados

La base ya repone consumibles que no son bayas según sus reglas modernas. La nueva clave
**Reponer bayas**, activa por defecto y desactivable, amplía esa comodidad tras entrenadores:
solo repone la baya original usada por el jugador y únicamente si el hueco quedó vacío.
No devuelve bayas robadas o retiradas sin consumo, no sobrescribe objetos de reemplazo y
no se aplica a salvajes, enlace ni combates grabados. No cambia sus efectos durante el combate.

## Tutores por gimnasio

Las estatuas de los ocho gimnasios ofrecen entrenamiento temático al obtener su medalla.
Se conserva el diálogo original y después se ofrece enseñar el movimiento, gratis y sin
límite de usos, a un Pokémon compatible del equipo o del PC. Los métodos anteriores para
aprender movimientos siguen disponibles; no es un desbloqueo exclusivo del movimiento.

| Medalla | Movimiento |
|---|---|
| Brock | Avalancha |
| Misty | Hidropulso |
| Lt. Surge | Puño Trueno |
| Erika | Bomba Germen |
| Koga | Lanza Mugre |
| Sabrina | Cabezazo Zen |
| Blaine | Onda Ígnea |
| Giovanni | Tierra Viva |

La disponibilidad depende de las flags de medalla existentes, sin nuevos campos o flags
de premio único. La macro compartida `rf_gym_tutor` se reconoce en `make_tutors.py`, para
que los generadores de compatibilidad y del recordador incluyan realmente esos movimientos.
Se respetan las compatibilidades de especies del motor, no se fuerza su aprendizaje.

## Evidencia

- 18 pruebas automáticas (C con ASan/UBSan y descubrimiento de tutores en Python): límites EV/IV, acciones, previsualización inmutable,
  cancelación/entradas inválidas, marca de entrenamiento extremo, listas de movimientos,
  objetos consumidos/robados y regresiones de los demás sistemas.
- mGBA, fixture de entrenamiento: cancelar un máximo de EV preserva datos; aplicar el
  máximo con 504 EV previos añade solo 6; editar el IV de PS a cero elimina su marca de
  entrenamiento extremo sin cambiar EV ni especie. Lecturas esperadas: 01 / 0F / 0F.
- mGBA, fixture de progresión: aprender un movimiento con Vileplume; consumir y reponer
  una Baya Zidra con la clave activa y dejarla consumida con la clave apagada.
- Capturas y scripts en `build_artifacts/training/` y `build_artifacts/progression/`, ignorados.
- mGBA: los ocho tutores enseñan su movimiento a un Mew sintético compatible; sin medalla
  el tutor permanece bloqueado. Nueve casos pasan; capturas en `build_artifacts/gym-tutors/`.

Reproducir el editor: sincronizar fuentes y herramientas a `/work`, ejecutar en el contenedor
`bash tools/rojo_fuego/test/build_training_fixture.sh` y desde Windows
`python tools/rojo_fuego/test/verify_training.py`. Las fases individuales sirven para depurar
el mismo ELF; nunca reutilizar sus estados después de reenlazar. Antes de una entrega retirar
la fuente temporal y el ELF, compilar con flags normales y comprobar ausencia de fixtures.
Para los ocho tutores usar la misma fixture y ejecutar `verify_gym_tutors.py`.
