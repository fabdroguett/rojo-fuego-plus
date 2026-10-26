# Desarrollo directo con Ubuntu WSL

Ubuntu 24.04 (`Ubuntu-24.04`), usuario `dev`. Docker Desktop puede permanecer cerrado.
Esta distribución trabaja sin systemd: no necesita servicios de servidor para compilar.
El repositorio de Windows es la fuente de trabajo; `/work` es una copia compilable en el
disco Linux. No editar ambas copias: los cambios se hacen en Windows y se sincronizan con
los siguientes comandos desde la raíz del repositorio, en PowerShell:

```powershell
./tools/rojo_fuego/wsl.ps1 sync
./tools/rojo_fuego/wsl.ps1 check
./tools/rojo_fuego/wsl.ps1 test
./tools/rojo_fuego/wsl.ps1 build
./tools/rojo_fuego/wsl.ps1 shell
./tools/rojo_fuego/wsl.ps1 stop
```

`check` compila y enlaza el ELF sin generar ni modificar ROMs. `test` valida el catálogo y
ejecuta la regresión nativa con sanitizadores. `build` genera la ROM, rechaza fixtures y
copia el resultado a `build_artifacts/wsl/pokefirered.gba`, respaldando la copia anterior.
No reemplaza automáticamente la ROM del Escritorio.

La sincronización empaqueta las fuentes nuevas/modificadas en Windows y las extrae de una
vez en Linux. Usa fechas y tamaños del manifiesto de `/work` y también elimina fuentes que
se hayan quitado del repositorio. Excluye archivos ignorados por Git, ROMs y guardados.
Se requiere Python en Windows, ya disponible en este equipo. `/work` es una copia de
compilación: los cambios manuales en fuentes de esa copia deben trasladarse a Windows.

La compilación usa **dos trabajos** por defecto. `-Jobs 1` reduce el paralelismo y
`-Jobs 4` lo aumenta. No se cambia el límite global de RAM de WSL ni otras distribuciones.
`stop` termina solo Ubuntu; libera sus procesos y su memoria de trabajo.
Si otro proyecto está trabajando en la misma distribución, terminarla también cierra sus
procesos. Durante esta preparación se detectó trabajo de otro proyecto y se dejó encendida.

Los scripts de emulador en `tools/rojo_fuego/test/` usan Ubuntu por defecto desde Windows:

```powershell
python tools/rojo_fuego/test/verify_release.py
```

La verificación de entrega comprueba cabecera, ausencia de fixtures y arranque hasta el mapa,
con guardado en memoria. Para pruebas de interfaz/progresión, preparar primero la fixture
correspondiente según `tools/rojo_fuego/test/UI_VALIDATION.md`. El ejecutor mGBA se recompila
en `/tmp/rf-mgba-harness` en cada sincronización: `/tmp` no es almacenamiento persistente.
`RF_TEST_BACKEND=docker` permite usar el entorno antiguo; `RF_WSL_DISTRO` cambia la distribución
para los verificadores Python, y `-Distro` para el comando PowerShell.

## Reinstalar las dependencias

Después de instalar Ubuntu 24.04 en WSL, ejecutar como root el script
`tools/rojo_fuego/setup_wsl.sh` usando su ruta `/mnt/c/...`. Instala GCC ARM, newlib,
compiladores nativos, Python, Git, rsync, PNG y mGBA; crea `/work` y el usuario `dev`.
Terminar y volver a iniciar la distribución para aplicar el usuario predeterminado.
No requiere Docker ni modifica guardados del jugador.

## Verificación del entorno, 26-10-2026

- Ubuntu 24.04.5 LTS / WSL 2, usuario `dev`; GCC ARM 13.2.1, mGBA 0.10.2.
- Las 18 pruebas nativas pasan, incluidas las que usan ASan/UBSan.
- El validador comprueba las 71 misiones del catálogo actual.
- El adaptador WSL ejecuta comandos en `/work` y copia archivos en ambas direcciones,
  incluidos nombres con espacios, sin Docker.
- Sincronización incremental comprobada con alta y eliminación de una fuente temporal;
  la pasada sin modificaciones empaqueta cero archivos.
- `check` compila y enlaza el ELF; `build` genera y copia la ROM local, sin fixtures.
- El verificador de entrega confirma 32 MiB, código BPRE, checksum de cabecera y arranque
  hasta `CB2_Overworld` en mGBA con guardado en memoria.
- ROM de comprobación: `build_artifacts/wsl/pokefirered.gba`, SHA-256
  `d01d7e344af176686091e465ba730c41f2f9c0f9a892fdb8986663a17fc438dc`.
  Informe/capturas: `build_artifacts/releases/20261026-093902/`. La ROM del Escritorio se conserva.
- Enlace: EWRAM 230624 B, IWRAM 28632 B, ROM enlazada 27151308 B.
- La compilación inicial no tiene errores ni advertencias de C/enlace. Libpng emite
  209 avisos `bKGD: invalid index` y tres `iCCP: known incorrect sRGB profile` en recursos
  existentes. Quedan para la revisión de metadatos de arte; no se afirma una compilación
  completamente libre de avisos. Registro: `build_artifacts/wsl/code-check.log`.
