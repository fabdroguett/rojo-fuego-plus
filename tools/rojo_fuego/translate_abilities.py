#!/usr/bin/env python3
"""Short Spanish ability descriptions (one summary line) for the abilities of generations 1-4.

The official texts are too long for the summary's single line, so these are written for it and checked
against the normal font width (the widest English description sets the budget). Idempotent.
"""
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from translate_descriptions import CODES, width  # noqa: E402

ROOT = Path(__file__).resolve().parents[2]
NAMES = {'ABILITY_STATIC': 'Elec. Estática', 'ABILITY_VOLT_ABSORB': 'Absorbe Elec.'}
DESCRIPTIONS = {
    'NONE': 'Sin habilidad.',
    'STENCH': 'Puede amedrentar al rival.', 'DRIZZLE': 'Hace llover al entrar.',
    'SPEED_BOOST': 'Gana Velocidad cada turno.', 'STURDY': 'Evita los fulminantes.',
    'DAMP': 'Impide autodestruirse.', 'SAND_VEIL': 'Más evasión en tormentas.',
    'STATIC': 'Puede paralizar al tocarlo.', 'VOLT_ABSORB': 'Lo Eléctrico le cura PS.',
    'WATER_ABSORB': 'El Agua le cura PS.', 'CLOUD_NINE': 'Anula los efectos del clima.',
    'COMPOUND_EYES': 'Aumenta la precisión.', 'COLOR_CHANGE': 'Toma el tipo del ataque.',
    'FLASH_FIRE': 'El Fuego potencia su Fuego.', 'SHIELD_DUST': 'Bloquea efectos añadidos.',
    'SUCTION_CUPS': 'No pueden obligarlo a irse.', 'WONDER_GUARD': 'Solo le dañan si es eficaz.',
    'LEVITATE': 'Inmune a ataques Tierra.', 'EFFECT_SPORE': 'Al tocarlo puede dormir...',
    'SYNCHRONIZE': 'Contagia su problema.', 'CLEAR_BODY': 'Sus stats no bajan.',
    'NATURAL_CURE': 'Se cura al cambiarlo.', 'SERENE_GRACE': 'Más efectos añadidos.',
    'SWIFT_SWIM': 'Más Velocidad con lluvia.', 'CHLOROPHYLL': 'Más Velocidad con sol.',
    'POISON_POINT': 'Puede envenenar al tocarlo.', 'MAGNET_PULL': 'Atrapa a los tipo Acero.',
    'RAIN_DISH': 'Recupera PS con lluvia.', 'SAND_STREAM': 'Crea una tormenta de arena.',
    'PRESSURE': 'El rival gasta más PP.', 'THICK_FAT': 'Reduce daño Fuego y Hielo.',
    'EARLY_BIRD': 'Despierta antes.', 'FLAME_BODY': 'Puede quemar al tocarlo.',
    'RUN_AWAY': 'Siempre puede huir.', 'KEEN_EYE': 'Su precisión no baja.',
    'HYPER_CUTTER': 'Su Ataque no baja.', 'PICKUP': 'Puede recoger objetos.',
    'TRUANT': 'Ataca un turno sí y otro no.', 'HUSTLE': 'Más fuerza, menos puntería.',
    'CUTE_CHARM': 'Puede enamorar al tocarlo.', 'FORECAST': 'Cambia con el clima.',
    'STICKY_HOLD': 'No le quitan su objeto.', 'SHED_SKIN': 'Puede curar su problema.',
    'GUTS': 'Más Ataque con problemas.', 'MARVEL_SCALE': 'Más Defensa con problemas.',
    'LIQUID_OOZE': 'Daña a quien le roba PS.', 'OVERGROW': 'Planta más fuerte si flojea.',
    'BLAZE': 'Fuego más fuerte si flojea.', 'TORRENT': 'Agua más fuerte si flojea.',
    'SWARM': 'Bicho más fuerte si flojea.', 'ROCK_HEAD': 'No sufre daño de retroceso.',
    'DROUGHT': 'Hace sol al entrar.', 'WHITE_SMOKE': 'Sus stats no bajan.',
    'SHELL_ARMOR': 'Evita los golpes críticos.', 'AIR_LOCK': 'Anula los efectos del clima.',
    'TANGLED_FEET': 'Más evasión si se confunde.', 'MOTOR_DRIVE': 'Lo Eléctrico sube Veloc.',
    'RIVALRY': 'Fuerte ante su mismo sexo.', 'STEADFAST': 'Si retrocede, sube Veloc.',
    'GLUTTONY': 'Come bayas antes.', 'ANGER_POINT': 'Un crítico le da Ataque.',
    'UNBURDEN': 'Sin objeto, sube Velocidad.', 'HEATPROOF': 'Reduce el daño de Fuego.',
    'SIMPLE': 'Duplica cambios de stats.', 'DRY_SKIN': 'El Agua cura, el sol daña.',
    'DOWNLOAD': 'Sube ataque según el rival.', 'IRON_FIST': 'Potencia los puñetazos.',
    'POISON_HEAL': 'El veneno le cura PS.', 'ADAPTABILITY': 'Potencia los de su tipo.',
    'SKILL_LINK': 'Multigolpes al máximo.', 'HYDRATION': 'Se cura con lluvia.',
    'SOLAR_POWER': 'Sol: más fuerza, pierde PS.', 'QUICK_FEET': 'Más Veloc. con problemas.',
    'NORMALIZE': 'Sus ataques son Normal.', 'SNIPER': 'Potencia los críticos.',
    'MAGIC_GUARD': 'Solo le dañan los ataques.', 'NO_GUARD': 'Todos los ataques aciertan.',
    'STALL': 'Siempre actúa el último.', 'TECHNICIAN': 'Potencia ataques débiles.',
    'LEAF_GUARD': 'Sin problemas con sol.',
}


def main():
    path = ROOT / 'src/data/abilities.h'
    raw = path.read_bytes().decode('utf-8')
    nl = '\r\n' if '\r\n' in raw else '\n'
    s = raw.replace('\r\n', '\n')
    budget = max(width(d) for d in re.findall(r'\.description = COMPOUND_STRING\("([^"]*)"\)', s)
                 if all(c in CODES for c in d))
    done = 0
    for key, text in DESCRIPTIONS.items():
        assert all(c in CODES for c in text) and width(text) <= budget, (key, width(text), budget)
        pat = re.compile(r'(\[ABILITY_' + key + r'\]\s*=\s*\{\s*\.name = _\("[^"]*"\),\s*\.description = COMPOUND_STRING\(")[^"]*("\))')
        s, n = pat.subn(lambda m: m.group(1) + text + m.group(2), s)
        done += n
    for const, name in NAMES.items():
        s = re.sub(r'(\[' + const + r'\]\s*=\s*\{\s*\.name = _\(")[^"]*("\))', lambda m: m.group(1) + name + m.group(2), s)
    path.write_bytes(s.replace('\n', nl).encode('utf-8'))
    print('abilities:', done, 'descriptions, budget', budget, 'px')


if __name__ == '__main__':
    main()
