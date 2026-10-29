#!/usr/bin/env python3
"""Spanish text for the battle messages players see in almost every battle (Pokémon Ultimate Kanto).

Exact English literals in src/battle_message.c (and a few other sources) are replaced, keeping every
control code. Idempotent: a literal that is no longer present is reported and skipped.
Name prefixes: the engine prepends "The wild "/"The opposing " to names, so Spanish messages are worded
to read naturally after "El "/"el ".
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

BATTLE = {
    # Name prefixes and stat names.
    '_("The wild ")': '_("El ")', '_("The opposing ")': '_("El ")',
    '_("the wild ")': '_("el ")', '_("the opposing ")': '_("el ")',
    '_("The opposing")': '_("El")', '_("the opposing")': '_("el")',
    'sText_Attack[] = _("Attack")': 'sText_Attack[] = _("Ataque")',
    'sText_Defense[] = _("Defense")': 'sText_Defense[] = _("Defensa")',
    'sText_Speed[] = _("Speed")': 'sText_Speed[] = _("Velocidad")',
    'sText_SpAttack[] = _("Sp. Atk")': 'sText_SpAttack[] = _("At. Esp.")',
    'sText_SpDefense[] = _("Sp. Def")': 'sText_SpDefense[] = _("Def. Esp.")',
    'sText_HP[] = _("HP")': 'sText_HP[] = _("PS")',
    'sText_Accuracy[] = _("accuracy")': 'sText_Accuracy[] = _("Precisión")',
    'sText_Evasiveness[] = _("evasiveness")': 'sText_Evasiveness[] = _("Evasión")',
    'gText_StatSharply[] = _(" sharply")': 'gText_StatSharply[] = _(" mucho")',
    'gText_drastically[] = _(" drastically")': 'gText_drastically[] = _(" muchísimo")',
    'gText_severely[] = _("severely ")': 'gText_severely[] = _(" muchísimo")',
    'gText_StatRose[] = _("rose!")': 'gText_StatRose[] = _("subió!")',
    'gText_StatFell[] = _("fell!")': 'gText_StatFell[] = _("bajó!")',
    '"{B_DEF_NAME_WITH_PREFIX}\'s {B_BUFF1} rose{B_BUFF2}!"': '"¡{B_BUFF1} de {B_DEF_NAME_WITH_PREFIX2} subió{B_BUFF2}!"',
    # Encounters and send-outs.
    '"You encountered a wild {B_OPPONENT_MON1_NAME}!\\p"': '"¡Un {B_OPPONENT_MON1_NAME} salvaje!\\p"',
    '"You encountered a wild {B_OPPONENT_MON1_NAME}!{PAUSE 127}"': '"¡Un {B_OPPONENT_MON1_NAME} salvaje!{PAUSE 127}"',
    '"Oh! A wild {B_OPPONENT_MON1_NAME} and {B_OPPONENT_MON2_NAME} appeared!\\p"':
        '"¡Aparecieron {B_OPPONENT_MON1_NAME} y {B_OPPONENT_MON2_NAME} salvajes!\\p"',
    '"{B_OPPONENT_MON1_NAME} appeared!\\p"': '"¡Apareció {B_OPPONENT_MON1_NAME}!\\p"',
    '"The GHOST appeared!\\p"': '"¡Apareció el FANTASMA!\\p"',
    '"The GHOST appeared!\\pDarn!\\nThe GHOST can\'t be ID\'d!\\p"': '"¡Apareció el FANTASMA!\\p¡Vaya!\\nNo se puede identificar.\\p"',
    '"You are challenged by {B_TRAINER1_NAME_WITH_CLASS}!\\p"': '"¡{B_TRAINER1_NAME_WITH_CLASS} te desafía!\\p"',
    '"{B_TRAINER1_NAME_WITH_CLASS} sent out {B_OPPONENT_MON1_NAME}!"': '"¡{B_TRAINER1_NAME_WITH_CLASS} sacó a {B_OPPONENT_MON1_NAME}!"',
    '"{B_TRAINER1_NAME_WITH_CLASS} sent out {B_BUFF1}!"': '"¡{B_TRAINER1_NAME_WITH_CLASS} sacó a {B_BUFF1}!"',
    '"Go! {B_PLAYER_MON1_NAME}!"': '"¡Adelante, {B_PLAYER_MON1_NAME}!"',
    '"Go! {B_BUFF1}!"': '"¡Adelante, {B_BUFF1}!"',
    '"Go! {B_PLAYER_MON1_NAME} and {B_PLAYER_MON2_NAME}!"': '"¡Adelante, {B_PLAYER_MON1_NAME} y {B_PLAYER_MON2_NAME}!"',
    '"You\'re in charge, {B_BUFF1}!"': '"¡Te toca, {B_BUFF1}!"',
    '"Go for it, {B_BUFF1}!"': '"¡A por ellos, {B_BUFF1}!"',
    '"Be careful, {B_PLAYER_MON1_NAME}!"': '"¡Ten cuidado, {B_PLAYER_MON1_NAME}!"',
    '"Your opponent\'s weak! Get \'em, {B_BUFF1}!"': '"¡Está débil! ¡Remátalo, {B_BUFF1}!"',
    '"{B_BUFF1}, that\'s enough! Come back!"': '"¡{B_BUFF1}, ya basta! ¡Vuelve!"',
    '"{B_BUFF1}, come back!"': '"¡{B_BUFF1}, vuelve!"',
    '"OK, {B_BUFF1}! Come back!"': '"¡Bien, {B_BUFF1}! ¡Vuelve!"',
    '"Good job, {B_BUFF1}! Come back!"': '"¡Buen trabajo, {B_BUFF1}! ¡Vuelve!"',
    '"{B_TRAINER1_NAME_WITH_CLASS} withdrew {B_BUFF1}!"': '"¡{B_TRAINER1_NAME_WITH_CLASS} retiró a {B_BUFF1}!"',
    '"You defeated {B_TRAINER1_NAME_WITH_CLASS}!\\p"': '"¡Has vencido a {B_TRAINER1_NAME_WITH_CLASS}!\\p"',
    # Moves and effectiveness.
    '"{B_ATK_NAME_WITH_PREFIX} used {B_BUFF3}!"': '"¡{B_ATK_NAME_WITH_PREFIX} usó {B_BUFF3}!"',
    '"It\'s super effective!"': '"¡Es muy eficaz!"',
    '"It\'s not very effective…"': '"No es muy eficaz…"',
    '"It doesn\'t affect {B_DEF_NAME_WITH_PREFIX2}…"': '"No afecta a {B_DEF_NAME_WITH_PREFIX2}…"',
    '"It doesn\'t affect {B_SCR_NAME_WITH_PREFIX2}…"': '"No afecta a {B_SCR_NAME_WITH_PREFIX2}…"',
    '"A critical hit!"': '"¡Un golpe crítico!"',
    '"It\'s super effective on {B_DEF_NAME_WITH_PREFIX2}!"': '"¡Es muy eficaz contra {B_DEF_NAME_WITH_PREFIX2}!"',
    '"It\'s not very effective on {B_DEF_NAME_WITH_PREFIX2}."': '"No es muy eficaz contra {B_DEF_NAME_WITH_PREFIX2}."',
    '"A critical hit on {B_DEF_NAME_WITH_PREFIX2}!"': '"¡Golpe crítico a {B_DEF_NAME_WITH_PREFIX2}!"',
    '"But it failed!"': '"¡Pero falló!"',
    '"{B_SCR_NAME_WITH_PREFIX}\'s {B_BUFF1} rose{B_BUFF2}!"': '"¡{B_BUFF1} de {B_SCR_NAME_WITH_PREFIX2} subió{B_BUFF2}!"',
    '"{B_SCR_NAME_WITH_PREFIX}\'s {B_BUFF1} {B_BUFF2}fell!"': '"¡{B_BUFF1} de {B_SCR_NAME_WITH_PREFIX2} bajó{B_BUFF2}!"',
    # Status.
    '"{B_EFF_NAME_WITH_PREFIX} fell asleep!"': '"¡{B_EFF_NAME_WITH_PREFIX} se durmió!"',
    '"{B_EFF_NAME_WITH_PREFIX} was poisoned!"': '"¡{B_EFF_NAME_WITH_PREFIX} fue envenenado!"',
    '"{B_ATK_NAME_WITH_PREFIX} was hurt by its poisoning!"': '"¡El veneno resta PS a {B_ATK_NAME_WITH_PREFIX2}!"',
    '"{B_EFF_NAME_WITH_PREFIX} was burned!"': '"¡{B_EFF_NAME_WITH_PREFIX} se quemó!"',
    '"{B_ATK_NAME_WITH_PREFIX} was hurt by its burn!"': '"¡La quemadura resta PS a {B_ATK_NAME_WITH_PREFIX2}!"',
    '"{B_EFF_NAME_WITH_PREFIX} was frozen solid!"': '"¡{B_EFF_NAME_WITH_PREFIX} se congeló!"',
    '"{B_EFF_NAME_WITH_PREFIX} is paralyzed, so it may be unable to move!"':
        '"¡{B_EFF_NAME_WITH_PREFIX} está paralizado! Quizá no pueda moverse."',
    '"{B_ATK_NAME_WITH_PREFIX} is confused!"': '"¡{B_ATK_NAME_WITH_PREFIX} está confuso!"',
    # Fainting, experience and moves learned.
    '"{B_SCR_NAME_WITH_PREFIX} fainted!\\p"': '"¡{B_SCR_NAME_WITH_PREFIX} se debilitó!\\p"',
    '"{B_BUFF1} gained{B_BUFF2} {B_BUFF3} Exp. Points!\\p"': '"¡{B_BUFF1} ganó{B_BUFF2} {B_BUFF3} puntos de experiencia!\\p"',
    '"{B_BUFF1} grew to Lv. {B_BUFF2}!{WAIT_SE}\\p"': '"¡{B_BUFF1} subió al nivel {B_BUFF2}!{WAIT_SE}\\p"',
    '"{B_BUFF1} learned {B_BUFF2}!{WAIT_SE}\\p"': '"¡{B_BUFF1} aprendió {B_BUFF2}!{WAIT_SE}\\p"',
    '"{B_BUFF1} wants to learn the move {B_BUFF2}.\\p"': '"{B_BUFF1} quiere aprender {B_BUFF2}.\\p"',
    '"However, {B_BUFF1} already knows four moves.\\p"': '"Pero {B_BUFF1} ya conoce cuatro movimientos.\\p"',
    '"Should another move be forgotten and replaced with {B_BUFF2}?"': '"¿Olvidar un movimiento para aprender {B_BUFF2}?"',
    '"{B_BUFF1} forgot {B_BUFF2}…\\p"': '"{B_BUFF1} olvidó {B_BUFF2}…\\p"',
    '"{PAUSE 32}Do you want to give up on having {B_BUFF1} learn {B_BUFF2}?"': '"{PAUSE 32}¿Renunciar a que {B_BUFF1} aprenda {B_BUFF2}?"',
    '"{B_BUFF1} did not learn {B_BUFF2}.\\p"': '"{B_BUFF1} no aprendió {B_BUFF2}.\\p"',
    '"{B_ATK_NAME_WITH_PREFIX} learned {B_BUFF1}!"': '"¡{B_ATK_NAME_WITH_PREFIX} aprendió {B_BUFF1}!"',
    '"One…{PAUSE 10}two…{PAUSE 10}and…{PAUSE 10}{PAUSE 20}{PLAY_SE SE_BALL_BOUNCE_1}ta-da!\\p"':
        '"Un, dos…{PAUSE 10} y…{PAUSE 10}{PAUSE 20}{PLAY_SE SE_BALL_BOUNCE_1} ¡tachán!\\p"',
    '"The rest of your team gained Exp. Points thanks to the Exp. Share!\\p"':
        '"¡El resto del equipo ganó experiencia gracias al Repartir Exp.!\\p"',
    # Money, escape, capture, defeat.
    '"You got ¥{B_BUFF1} for winning!\\p"': '"¡Has ganado ¥{B_BUFF1}!\\p"',
    '"You have no more Pokémon that can fight!\\p"': '"¡No te quedan Pokémon que puedan luchar!\\p"',
    '"You panicked and dropped ¥{B_BUFF1}…"': '"Con el susto se te cayeron ¥{B_BUFF1}…"',
    '"You gave ¥{B_BUFF1} to the winner…"': '"Le diste ¥{B_BUFF1} al ganador…"',
    '"You were overwhelmed by your defeat!"': '"La derrota te ha dejado sin aliento…"',
    '"{PLAY_SE SE_FLEE}The wild {B_BUFF1} fled!"': '"{PLAY_SE SE_FLEE}¡El {B_BUFF1} salvaje huyó!"',
    '"You can\'t escape!\\p"': '"¡No puedes huir!\\p"',
    '"You couldn\'t get away!\\p"': '"¡No has podido escapar!\\p"',
    '"Gotcha! {B_DEF_NAME} was caught!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\\p"': '"¡Ya está! ¡{B_DEF_NAME} atrapado!{WAIT_SE}{PLAY_BGM MUS_CAUGHT}\\p"',
    '"You used {B_LAST_ITEM}!"': '"¡Has usado {B_LAST_ITEM}!"',
    '"{B_ATK_TRAINER_NAME_WITH_CLASS} used {B_LAST_ITEM}!"': '"¡{B_ATK_TRAINER_NAME_WITH_CLASS} usó {B_LAST_ITEM}!"',
    '"{B_SCR_NAME_WITH_PREFIX} restored its health using its {B_LAST_ITEM}!"': '"¡{B_SCR_NAME_WITH_PREFIX} recuperó salud con su {B_LAST_ITEM}!"',
}


def apply(rel, table):
    path = ROOT / rel
    raw = path.read_bytes().decode('utf-8')
    nl = '\r\n' if '\r\n' in raw else '\n'
    s = raw.replace('\r\n', '\n')
    done = 0
    missing = []
    for a, b in table.items():
        if a in s:
            s = s.replace(a, b)
            done += 1
        elif b not in s:
            missing.append(a[:60])
    path.write_bytes(s.replace('\n', nl).encode('utf-8'))
    print(f'{rel}: {done} replaced, {len(missing)} not found')
    for m in missing:
        print('  missing:', m)


if __name__ == '__main__':
    apply('src/battle_message.c', BATTLE)
