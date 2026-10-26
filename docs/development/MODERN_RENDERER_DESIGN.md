# Diseño preliminar de runtime moderno

## Estado

Documento de viabilidad futura, fuera de la cola activa. La instrucción más reciente prioriza solo funcionalidad; no iniciar UI, renderer ni port hasta que se solicite.

## Requisitos

- Conservar el arte GBA con escala entera y filtro nearest-neighbor como modo pixel-perfect.
- Permitir paquetes HD opcionales mapeados por IDs estables, fondos con más detalle, iluminación, sombras, clima, partículas, shaders y animaciones suaves.
- Separar lógica de juego, presentación, audio, entrada, reloj y persistencia.
- Soportar ventanas panorámicas, resoluciones múltiples, teclado, mando y táctil, con layout adaptado sin cambiar reglas de juego.
- Medir Android y PC; evitar asumir que más resolución equivale a más consumo aceptable.

## Comparación inicial

| Opción | Ventaja | Coste/riesgo para este código C | Mejor prueba |
|---|---|---|---|
| Godot 4 | Motor/editor completo, escenas, animación, UI y shaders; GDExtension permite bibliotecas nativas. | Llevar C decompilado requiere puente C/C++ y abstraer reloj, recursos y estado. Flujo de assets sería nuevo. | Vertical slice 2D con un mapa, HUD, control y wrapper de una función determinista. |
| SDL3 | Biblioteca C multiplataforma para audio, teclado, joystick, gráficos; encaja con capa runtime propia. | Es de bajo nivel; hay que crear editor, escenas, UI, pipeline de assets y herramientas. | Renderer de mapa/tiles con input/audio manteniendo C y paso fijo. |
| raylib | API C sencilla, audio/render incluidos; atractiva para prototipos y herramientas. | Menos estructura de motor; será necesario construir sistemas y pipeline propio; evaluar backend móvil y escala de proyecto. | Prototipo de combate/mapa pequeño con assets GBA. |

Fuentes oficiales revisadas: [Godot docs](https://docs.godotengine.org/en/stable/), [GDExtension](https://docs.godotengine.org/en/stable/engine_details/engine_api/gdextension/index.html), [SDL3](https://wiki.libsdl.org/SDL3/FrontPage), [raylib](https://github.com/raysan5/raylib). SDL3 describe acceso multiplataforma de bajo nivel a audio, entrada y hardware gráfico; raylib es una biblioteca multimedia C; Godot es un motor 2D/3D con extensión nativa. Verificar versiones/licencias al iniciar prototipo.

## Recomendación provisional

Probar Godot primero si la prioridad es editor, UI escalable y mejoras visuales. Probar SDL3 si la prioridad es reutilizar C y tener control del runtime. raylib es útil para medir rápidamente el coste de un render mínimo. No elegir definitivamente hasta fijar plataformas objetivo, fidelidad y plan de reutilización de lógica/datos.

## Entregas por etapas

1. Inventario de datos/recursos exportables y dependencias hardware.
2. Pruebas canónicas de datos, combate, eventos y guardados.
3. Renderer de una habitación con tilemap y escalado entero.
4. Control/input y audio con paso de simulación fijo.
5. Paquete HD opcional y comparación visual/performance.
6. Portabilidad PC/Android y verificación de saves.
