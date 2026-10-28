"""Generate authored campaigns, reachable actors, scripts and trainer teams.

Python standard library only. Re-running preserves all unrelated objects/scripts.
"""
from collections import deque
import hashlib
import json
from pathlib import Path
import re
import textwrap
from story_content import ACTORS, CAMPAIGNS, CLUES, TRAINERS, CONTRACTS, PLACES
from legends import MapGrid, behaviors, tileset_attribute_files

ROOT=Path(__file__).resolve().parents[2]
FIRST=350
PREFIX='RFStory_Actor_'


def quoted(text, width=32, pages=False):
    lines=textwrap.wrap(text,width=width,break_long_words=False,break_on_hyphens=False)
    if pages:
        text='\\p'.join('\\n'.join(lines[i:i+2]) for i in range(0,len(lines),2))
    else: text='\\n'.join(lines)
    return '"'+text.replace('"','\\"')+'"'


def write(path, text):
    path=ROOT/path
    data=(text.rstrip()+'\n').encode('utf-8')
    if not path.exists() or path.read_bytes()!=data:
        path.parent.mkdir(parents=True,exist_ok=True)
        path.write_bytes(data)


def scripted_traffic(grid):
    """Reserve explicit vanilla cutscene walks before adding any standing NPC."""
    text=(ROOT/'data/maps'/grid.folder/'scripts.inc').read_text(encoding='utf-8')
    labels=dict(re.findall(r'^([A-Za-z_]\w*)::?\s*\n(.*?)(?=^[A-Za-z_]\w*::?|\Z)',text,re.M|re.S))
    objects=[o for o in grid.data.get('object_events',[]) if not o.get('script','').startswith(PREFIX)]
    traffic=set()
    for who,label in re.findall(r'\bapplymovement\s+([\w]+),\s*(\w+)',text):
        steps=re.findall(r'^\s*(?:walk|walk_fast|walk_slow|run|jump|jump_2)_(up|down|left|right)(?:\s+(\d+))?\s*(?:@[^\n]*)?$',labels.get(label,''),re.M)
        if not steps: continue
        starts=[]
        for i,o in enumerate(objects):
            if who in (o.get('local_id'),str(i+1),'VAR_LAST_TALKED'):
                starts.append((o['x'],o['y']))
        if who=='LOCALID_PLAYER':
            starts=[(e['x'],e['y']) for k in ('coord_events','warp_events') for e in grid.data.get(k,[])]
            starts += [(o['x']+dx,o['y']+dy) for o in objects for dx,dy in ((1,0),(-1,0),(0,1),(0,-1))]
        for x,y in starts:
            for direction,count in steps:
                dx,dy={'left':(-1,0),'right':(1,0),'up':(0,-1),'down':(0,1)}[direction]
                for _ in range(min(int(count or 1),32)):
                    x+=dx;y+=dy
                    if 0<=x<grid.w and 0<=y<grid.h: traffic.add((x,y))
    return traffic


def find_spot(grid, occupied):
    """Check collision, elevation, reachability and that an NPC cannot seal a passage."""
    warps={(o['x'],o['y']) for o in grid.data.get('warp_events',[])}
    triggers={(o['x'],o['y']) for kind in ('coord_events','bg_events') for o in grid.data.get(kind,[])}
    traffic=scripted_traffic(grid)
    seeds=set(warps)
    for c in grid.data.get('connections') or []:
        d=c['direction']
        if d in ('up','down'): seeds.update((x,0 if d=='up' else grid.h-1) for x in range(grid.w))
        if d in ('left','right'): seeds.update((0 if d=='left' else grid.w-1,y) for y in range(grid.h))
    def reachable(blocked):
        seen=set(); queue=deque(p for p in seeds if 0<=p[0]<grid.w and 0<=p[1]<grid.h and p not in blocked)
        while queue:
            p=queue.popleft()
            if p in seen: continue
            seen.add(p)
            x,y=p
            for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                if n in seen or n in blocked or not grid.walkable(*n): continue
                a,b=grid.elevation(*p),grid.elevation(*n)
                if a!=b and a not in (0,15) and b not in (0,15): continue
                queue.append(n)
        return seen
    before=reachable(occupied)
    candidates=[]
    for x,y in before:
        if not grid.walkable(x,y): continue
        if x<1 or y<1 or x>=grid.w-1 or y>=grid.h-1 or (x,y) in triggers or (x,y) in traffic: continue
        if any(abs(x-a)+abs(y-b)<2 for a,b in warps|triggers|occupied): continue
        neighbours=[n for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)) if n in before and n not in occupied]
        if len(neighbours)<2: continue
        distance=min((abs(x-a)+abs(y-b) for a,b in occupied),default=8)
        # Prefer open tiles near entrances rather than inaccessible corners of large maps.
        entry=min((abs(x-a)+abs(y-b) for a,b in seeds),default=10)
        candidates.append((min(distance,8)*4-entry,x,y,neighbours[0]))
    if not candidates: raise ValueError(f'{grid.folder}: no safe, reachable actor position')
    for _,x,y,landing in sorted(candidates,reverse=True):
        if before-{(x,y)} <= reachable(occupied|{(x,y)}):
            return x,y,grid.elevation(x,y),landing
    raise ValueError(f'{grid.folder}: every candidate would cut a path')


def main():
    quests=[]
    for chapter,(_,_,rows) in enumerate(CAMPAIGNS):
        assert len(rows)==8
        for row in rows: quests.append(dict(row,chapter=9+chapter,id=FIRST+len(quests)))
    assert len(quests)==64
    actors={k:dict(v) for k,v in ACTORS.items()}
    for a in actors.values():
        if a['gfx']=='OBJ_EVENT_GFX_HIKER': a['gfx']='OBJ_EVENT_GFX_HIKER_FRLG'
    for key in ('alma','edda','ada'): actors[key]['gfx']='OBJ_EVENT_GFX_WOMAN_1_FRLG'
    clue_bits={}; bit=0
    # Shipped evidence bits are explicit; future campaigns must not shift this layout.
    clue_base={2:0,13:3,21:6,60:9,50:14}
    for offset,points in CLUES.items():
        assert quests[offset]['kind']=='clues',(offset,quests[offset]['name'])
        mask=0; bit=clue_base[offset]
        for index,(name,place,text) in enumerate(points):
            actors[f'clue_{offset}_{index}']=dict(name=name,place=place,greeting=text,gfx='OBJ_EVENT_GFX_ITEM_BALL',quest=FIRST+offset,bit=bit)
            mask|=1<<bit; bit+=1
        clue_bits[FIRST+offset]=mask
    assert bit<=32
    actors['home_cubone']=dict(name='Cubone del refugio',place='LavenderTown_VolunteerPokemonHouse',greeting='El pequeño del refugio se acerca por voluntad propia. Inés mantiene la puerta abierta para sus paseos.',gfx='OBJ_EVENT_GFX_SPECIES(CUBONE)',after=FIRST+23)
    actor_ids={k:i for i,k in enumerate(actors)}
    layouts={x['id']:x for x in json.loads((ROOT/'data/layouts/layouts.json').read_text(encoding='utf-8'))['layouts'] if x}
    attrs,mb=tileset_attribute_files(),behaviors()
    by_map={}
    for key,a in actors.items():
        folder=a['place']+'_Frlg'
        path=ROOT/'data/maps'/folder/'map.json'
        assert path.exists(),folder
        a['map']=json.loads(path.read_text(encoding='utf-8'))['id']
        by_map.setdefault(folder,[]).append(key)
    placements=[]
    for path in (ROOT/'data/maps').glob('*_Frlg/map.json'):
        if path.parent.name in by_map: continue
        data=json.loads(path.read_text(encoding='utf-8'))
        kept=[o for o in data.get('object_events',[]) if not o.get('script','').startswith(PREFIX)]
        if len(kept)!=len(data.get('object_events',[])):
            data['object_events']=kept
            write(path.relative_to(ROOT),json.dumps(data,ensure_ascii=False,indent=2))
    for folder,keys in by_map.items():
        grid=MapGrid(folder,layouts,attrs,mb)
        data=grid.data
        data['object_events']=[o for o in data.get('object_events',[]) if not o.get('script','').startswith(PREFIX)]
        occupied={(o['x'],o['y']) for o in data['object_events']}
        for key in keys:
            a=actors[key]
            x,y,elevation,landing=find_spot(grid,occupied)
            occupied.add((x,y))
            local=len(data['object_events'])+1
            a.update(x=x,y=y,landing=landing,local_id=local)
            data['object_events'].append(dict(local_id=f'LOCALID_RF_STORY_{actor_ids[key]}',type='object',graphics_id=a['gfx'],x=x,y=y,elevation=elevation if elevation not in (0,15) else 3,movement_type='MOVEMENT_TYPE_FACE_DOWN',movement_range_x=0,movement_range_y=0,trainer_type='TRAINER_TYPE_NONE',trainer_sight_or_berry_tree_id='0',script=PREFIX+str(actor_ids[key]),flag='0'))
            placements.append(dict(key=key,actor=actor_ids[key],map=a['map'],folder=folder,x=x,y=y,landing=landing,localId=local))
        assert len(data['object_events'])<=64,folder
        write(Path('data/maps')/folder/'map.json',json.dumps(data,ensure_ascii=False,indent=2))
    const=['#ifndef GUARD_CONSTANTS_RF_STORY_H','#define GUARD_CONSTANTS_RF_STORY_H',f'#define RF_STORY_FIRST {FIRST}',f'#define RF_STORY_COUNT {len(quests)}',f'#define RF_STORY_ACTOR_COUNT {len(actors)}','#define RF_STORY_CHAPTER_FIRST 9','#define RF_STORY_EVENT 4']
    const += [f'#define RF_STORY_ACTOR_{key.upper()} {i}' for key,i in actor_ids.items()]
    const += [f'#define RF_QUEST_STORY_{i:03d} {FIRST+i}' for i in range(len(quests))]
    const+=['#endif']
    write('include/constants/rf_story.h','\n'.join(const))
    entries=['// Generated by story_campaigns.py; append-only stable quest slots.']
    definitions=['// Generated authored scene definitions.']
    definitions += [f'extern const u8 {PREFIX}{i}[];' for i in range(len(actors))]
    definitions+=['static const struct RFStoryActor sActors[] = {']
    for key,a in actors.items():
        hint=quoted(a['name']+'. '+PLACES[a['place']]+'.')
        assert len(textwrap.wrap(a['name']+'. '+PLACES[a['place']]+'.',32))<=3
        definitions.append('    {%d, %s, %s, %d, %d, %d, %s, %s},'%(actor_ids[key],a['map'],PREFIX+str(actor_ids[key]),a.get('quest',65535),a.get('bit',255),a.get('after',65535),f'COMPOUND_STRING({quoted(a["greeting"],pages=True)})',f'COMPOUND_STRING({hint})'))
    definitions+=['};','static const struct RFStoryScene sScenes[] = {']
    kinds={'talk':'RF_STORY_TALK','delivery':'RF_STORY_DELIVERY','clues':'RF_STORY_CLUES','battle':'RF_STORY_BATTLE','race':'RF_STORY_RACE','puzzle':'RF_STORY_PUZZLE'}
    for i,q in enumerate(quests):
        condition='QUEST_EVENT';argument='RF_STORY_EVENT';alternative=str(q['id'])
        extra=''
        if q['kind']=='delivery':
            condition='QUEST_DELIVER_ITEM';argument,alternative=q['proof'];alternative=str(alternative)
        elif isinstance(q['proof'],tuple):
            c,a,b=q['proof']
            extra=f'        .objectiveCount = 2, .secondaryCondition = {c}, .secondaryArgument = {a}, .secondaryAlternative = {b},\n'
        previous='RF_QUEST_NONE' if i%8==0 else f'RF_QUEST_STORY_{i-1:03d}'
        desc=quoted(q['objective'])
        assert len(textwrap.wrap(q['objective'],32))<=3,(i,q['objective'])
        entries += [f'    [RF_QUEST_STORY_{i:03d}] = {{',f'        .name = COMPOUND_STRING({quoted(q["name"])}),',f'        .description = COMPOUND_STRING({desc}),',f'        .prerequisite = {previous}, .requiredFlag = {q["flag"]}, .condition = {condition},',f'        .argument = {argument}, .alternative = {alternative}, .rewardItem = {q["reward"]}, .rewardCount = {q["quantity"]}, .chapter = {q["chapter"]},',extra.rstrip(),'    },']
        trainer=f'TRAINER_RF_STORY_{q["proof"]}' if q['kind']=='battle' else '0'
        definitions.append('    {%d, %d, %s, %s, 0x%08X, %s, %s},'%(actor_ids[q['giver']],actor_ids[q['target']],kinds[q['kind']],trainer,clue_bits.get(q['id'],0),f'COMPOUND_STRING({quoted(q["offer"],pages=True)})',f'COMPOUND_STRING({quoted(q["ending"],pages=True)})'))
    definitions+=['};','static const struct RFStoryContract sContracts[] = {']
    for key,title,item,count,reward,qty,unlock in CONTRACTS:
        definitions.append(f'    {{{actor_ids[key]}, {FIRST+unlock}, {item}, {count}, {reward}, {qty}, COMPOUND_STRING({quoted(title)})}},')
    definitions+=['};']
    write('src/data/rf_quests_story.h','\n'.join(entries))
    write('src/data/rf_story.h','\n'.join(definitions))
    header=ROOT/'include/constants/rf_quests.h'
    text=header.read_text(encoding='utf-8')
    text=re.sub(r'#define RF_QUEST_COUNT \d+',f'#define RF_QUEST_COUNT {FIRST+len(quests)}',text)
    text=re.sub(r'#define RF_QUEST_CHAPTER_COUNT \d+','#define RF_QUEST_CHAPTER_COUNT 17',text)
    write(header.relative_to(ROOT),text)
    script=['@ Authored stories. Generated by tools/rojo_fuego/story_campaigns.py.']
    for key,a in actors.items():
        script += [f'{PREFIX}{actor_ids[key]}::','\tlock',*(['\tfaceplayer'] if a['gfx']!='OBJ_EVENT_GFX_ITEM_BALL' else []),f'\tsetvar VAR_0x8004, {actor_ids[key]}','\tcallnative Script_RFStoryPrepare','\tgoto_if_eq VAR_RESULT, 1, RFStory_Offer','\tgoto_if_eq VAR_RESULT, 2, RFStory_Confirm','\tgoto_if_eq VAR_RESULT, 3, RFStory_Puzzle']
        script+=['\tgoto_if_ne VAR_RESULT, 4, RFStory_ShowText']
        for i,q in enumerate(quests):
            if q['kind']=='battle' and q['target']==key:
                script+= [f'\tgoto_if_eq VAR_0x8005, {q["id"]}, RFStory_Battle_{i}']
        script+=['\tmsgbox gStringVar4','\trelease','\tend','']
    script += ['RFStory_ShowText::','\tmsgbox gStringVar4','\tcallnative Script_RFStoryAfterText','\tgoto_if_eq VAR_0x8007, 1, RFStory_RemoveMarker','\trelease','\tend','RFStory_RemoveMarker::','\tremoveobject VAR_LAST_TALKED','\trelease','\tend','RFStory_Offer::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tgoto_if_eq VAR_RESULT, NO, RFStory_Bye','\tcallnative Script_RFStoryAccept','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Confirm::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tgoto_if_eq VAR_RESULT, NO, RFStory_Bye','\tcallnative Script_RFStoryConfirm','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Puzzle::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tcallnative Script_RFStoryPuzzle','\tgoto_if_eq VAR_RESULT, 3, RFStory_Puzzle','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Bye::','\trelease','\tend']
    for i,q in enumerate(quests):
        if q['kind']=='battle':
            script += [f'RFStory_Battle_{i}::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tgoto_if_eq VAR_RESULT, NO, RFStory_Bye',f'\ttrainerbattle_single TRAINER_RF_STORY_{q["proof"]}, RFStory_BattleIntro, RFStory_BattleLost',f'\tsetvar VAR_0x8005, {q["id"]}','\tcallnative Script_RFStoryBattleWon','\tmsgbox gStringVar4','\trelease','\tend']
    script += ['RFStory_BattleIntro::','\t.string "Una práctica, sin apuestas.\\nAprendamos de la respuesta.$"','RFStory_BattleLost::','\t.string "Tengo que revisar mis notas.\\nGracias por el combate.$"']
    write('data/scripts/rojo_fuego_story.inc','\n'.join(script))
    opponents=ROOT/'include/constants/opponents_frlg.h'
    source=opponents.read_text(encoding='utf-8')
    source=re.sub(r'// BEGIN RF STORY.*?// END RF STORY\n','',source,flags=re.S)
    block='// BEGIN RF STORY\n'+'\n'.join(f'#define TRAINER_RF_STORY_{i} {639+i}' for i in range(len(TRAINERS)))+'\n// END RF STORY\n'
    source=source.replace('#define TRAINERS_COUNT_FRLG',block+'#define TRAINERS_COUNT_FRLG')
    source=re.sub(r'(#define TRAINERS_COUNT_FRLG\s+)\d+',lambda m:m[1]+str(639+len(TRAINERS)),source)
    write(opponents.relative_to(ROOT),source)
    path=ROOT/'src/data/trainers_frlg.party';source=path.read_text(encoding='utf-8')
    source=re.sub(r'\n(?:// BEGIN RF STORY|/\* BEGIN RF STORY \*/).*?(?:// END RF STORY|/\* END RF STORY \*/)\n?','\n',source,flags=re.S)
    teams=['/* BEGIN RF STORY */']
    for i,(name,level,gender,pic,species) in enumerate(TRAINERS):
        teams += [f'=== TRAINER_RF_STORY_{i} ===',f'Name: {name}','Class: Pkmn Trainer 1',f'Pic: {pic}',f'Gender: {gender}','Music: Serious','AI: Check Bad Move / Try To Faint / Check Viability','']
        for s in species: teams += [s,f'Level: {level}','IVs: 15 HP / 15 Atk / 15 Def / 15 SpA / 15 SpD / 15 Spe','']
    teams+=['/* END RF STORY */']
    write(path.relative_to(ROOT),source.rstrip()+'\n\n'+'\n'.join(teams))
    out=ROOT/'build_artifacts/story-campaigns';out.mkdir(parents=True,exist_ok=True)
    write('build_artifacts/story-campaigns/catalog.json',json.dumps(dict(quests=quests,actors=placements,clueBits=clue_bits),ensure_ascii=False,indent=2))
    fixture=['// Generated actor positions for disposable emulator fixtures only.']
    fixture += [f'extern const u8 {PREFIX}{i}[];' for i in range(len(actors))]
    fixture+=['static const struct RFStoryFixtureActor sFixtureActors[] = {']
    for key,a in actors.items():
        fixture.append('    {%s,%d,%d,%d,%d,%d,%s},'%(a['map'],a['x'],a['y'],a['landing'][0],a['landing'][1],a['local_id'],PREFIX+str(actor_ids[key])))
    fixture+=['};']
    write('tools/rojo_fuego/test/story_fixture_actors.h','\n'.join(fixture))
    print(f'Generated {len(quests)} authored quests, {len(actors)} actors, {bit} clues, {len(TRAINERS)} trainer teams; IDs 350–413.')


if __name__=='__main__': main()
