"""Generate authored campaigns, reachable actors, scripts and trainer teams.

Python standard library only. Re-running preserves all unrelated objects/scripts.
"""
from collections import deque
import hashlib
import json
from pathlib import Path
import re
import textwrap
from story_content import ACTORS, ACTORS_V2, CAMPAIGNS, CAMPAIGN_SLOTS, CHOICES, CLUES, CLUE_BASE, CONTENT_VERSIONS, TRAINERS, CONTRACTS, PLACES
from legends import MapGrid, SURFABLE, behaviors, connection_seeds, tileset_attribute_files

ROOT=Path(__file__).resolve().parents[2]
FIRST=350
BASE_CONTENT_VERSION=0x32 # quest extension magic as shipped with Phase B
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


def find_spot(grid, occupied, open_grid, surf=False):
    """Check collision, elevation, reachability and that an NPC cannot seal a passage.

    surf=True also lets the player reach land by water (maps only reachable with Surf). The
    actor and its landing tile always stand on walkable land.
    """
    def passable(x,y):
        if grid.walkable(x,y): return True
        return surf and 0<=x<grid.w and 0<=y<grid.h and grid.collision(x,y)==0 and grid.behavior(x,y) in SURFABLE
    warps={(o['x'],o['y']) for o in grid.data.get('warp_events',[])}
    triggers={(o['x'],o['y']) for kind in ('coord_events','bg_events') for o in grid.data.get(kind,[])}
    traffic=scripted_traffic(grid)
    seeds=set(warps)|connection_seeds(grid,open_grid,surf)
    def reachable(blocked):
        seen=set(); queue=deque(p for p in seeds if 0<=p[0]<grid.w and 0<=p[1]<grid.h and p not in blocked)
        while queue:
            p=queue.popleft()
            if p in seen: continue
            seen.add(p)
            x,y=p
            for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)):
                if n in seen or n in blocked or not passable(*n): continue
                a,b=grid.elevation(*p),grid.elevation(*n)
                # Getting on and off the water crosses elevations; walking does not.
                shore=surf and (not grid.walkable(*p) or not grid.walkable(*n))
                if a!=b and a not in (0,15) and b not in (0,15) and not shore: continue
                queue.append(n)
        return seen
    before=reachable(occupied)
    candidates=[]
    for x,y in before:
        if not grid.walkable(x,y): continue
        if x<1 or y<1 or x>=grid.w-1 or y>=grid.h-1 or (x,y) in triggers or (x,y) in traffic: continue
        if any(abs(x-a)+abs(y-b)<2 for a,b in warps|triggers|occupied): continue
        neighbours=[n for n in ((x-1,y),(x+1,y),(x,y-1),(x,y+1)) if n in before and n not in occupied and grid.walkable(*n)]
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


def replace_block(text, name, lines):
    """Replace the generated rows between BEGIN/END markers, keeping the hand-written rest."""
    begin=re.search(rf'^([ \t]*)// BEGIN {name} \(story_campaigns.py\)\n',text,re.M)
    end=re.search(rf'^[ \t]*// END {name}\n',text,re.M)
    assert begin and end and begin.end()<=end.start(),name
    return text[:begin.end()]+''.join(begin.group(1)+line+'\n' for line in lines)+text[end.start():]


def main():
    quests=[]
    chapter_names=[]
    for chapter,(title,_,rows) in enumerate(CAMPAIGNS):
        assert 1<=len(rows)<=8,title
        chapter_names.append(title)
        slots=list(CAMPAIGN_SLOTS.get(title,[]))
        assert not slots or len(slots)==len(rows),title
        for n,row in enumerate(rows):
            # Sequential IDs from 350 unless the campaign reuses reconverted generic slots.
            qid=slots[n] if slots else FIRST+sum(1 for q in quests if q['campaign'] not in CAMPAIGN_SLOTS)
            quests.append(dict(row,chapter=9+chapter,id=qid,campaign=title,scene=len(quests)))
    ids=[q['id'] for q in quests]
    assert len(quests)==sum(len(rows) for _,_,rows in CAMPAIGNS) and len(set(ids))==len(ids) and max(ids)<650
    assert all(i<FIRST for t in CAMPAIGN_SLOTS for i in CAMPAIGN_SLOTS[t]),'slots are reconverted IDs'
    quest_count=max(max(ids),FIRST-1)+1
    campaign_start={}
    for q in quests:
        campaign_start.setdefault(q['campaign'],q['scene']) # first scene index
    actors={k:dict(v) for k,v in ACTORS.items()}
    clue_bits={}; bit=0
    # Shipped evidence bits are explicit; future campaigns must not shift this layout.
    clue_base={2:0,13:3,21:6,60:9,50:14,81:17,**CLUE_BASE}
    def add_clues(offsets):
        nonlocal bit
        for offset in offsets:
            points=CLUES[offset]
            assert quests[offset]['kind']=='clues',(offset,quests[offset]['name'])
            mask=0; bit=clue_base[offset]
            for index,(name,place,text) in enumerate(points):
                actors[f'clue_{offset}_{index}']=dict(name=name,place=place,greeting=text,gfx='OBJ_EVENT_GFX_ITEM_BALL',quest=quests[offset]['id'],bit=bit)
                mask|=1<<bit; bit+=1
            clue_bits[quests[offset]['id']]=mask
            assert bit<=32
    # Clue markers of the first eight campaigns keep their shipped place in the actor list.
    add_clues([o for o in CLUES if o<64])
    actors['home_cubone']=dict(name='Cubone del refugio',place='LavenderTown_VolunteerPokemonHouse',greeting='El pequeño del refugio se acerca por voluntad propia. Inés mantiene la puerta abierta para sus paseos.',gfx='OBJ_EVENT_GFX_SPECIES(CUBONE)',after=quests[23]['id'])
    # Later actors and clue markers come last so shipped IDs, labels and local IDs never shift.
    for key,a in ACTORS_V2.items():
        assert key not in actors,key
        actors[key]=dict(a)
    add_clues(sorted(o for o in CLUES if o>=64))
    bit=max(clue_base[o]+len(CLUES[o]) for o in CLUES)
    for a in actors.values():
        if a['gfx']=='OBJ_EVENT_GFX_HIKER': a['gfx']='OBJ_EVENT_GFX_HIKER_FRLG'
    for key in ('alma','edda','ada'): actors[key]['gfx']='OBJ_EVENT_GFX_WOMAN_1_FRLG'
    # Quest-state visibility (rf_quest_links.h) for actors that appear after a campaign step.
    visible={}
    for key,a in actors.items():
        if a.get('visible_after'):
            title,row=a['visible_after']
            visible[key]=campaign_start[title]+row
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
    folders={json.loads(path.read_text(encoding='utf-8'))['id']:path.parent.name for path in (ROOT/'data/maps').glob('*/map.json')}
    grids={}
    def open_grid(map_id):
        folder=folders.get(map_id)
        if folder is None or not (ROOT/'data/maps'/folder/'map.json').exists():
            return None
        if folder not in grids:
            grids[folder]=MapGrid(folder,layouts,attrs,mb)
        return grids[folder]
    for folder,keys in by_map.items():
        grid=MapGrid(folder,layouts,attrs,mb)
        data=grid.data
        data['object_events']=[o for o in data.get('object_events',[]) if not o.get('script','').startswith(PREFIX)]
        occupied={(o['x'],o['y']) for o in data['object_events']}
        for key in keys:
            a=actors[key]
            try:
                x,y,elevation,landing=find_spot(grid,occupied,open_grid)
                a['surf']=False
            except ValueError:
                # Only some sea maps need this; their scenes are post-game, when Surf is available.
                x,y,elevation,landing=find_spot(grid,occupied,open_grid,surf=True)
                a['surf']=True
                print(f'note: {key} in {folder} is reached by surfing')
            occupied.add((x,y))
            local=len(data['object_events'])+1
            a.update(x=x,y=y,landing=landing,local_id=local)
            data['object_events'].append(dict(local_id=f'LOCALID_RF_STORY_{actor_ids[key]}',type='object',graphics_id=a['gfx'],x=x,y=y,elevation=elevation if elevation not in (0,15) else 3,movement_type='MOVEMENT_TYPE_FACE_DOWN',movement_range_x=0,movement_range_y=0,trainer_type='TRAINER_TYPE_NONE',trainer_sight_or_berry_tree_id='0',script=PREFIX+str(actor_ids[key]),flag='0'))
            placements.append(dict(key=key,actor=actor_ids[key],map=a['map'],folder=folder,x=x,y=y,landing=landing,localId=local,surf=a['surf']))
        assert len(data['object_events'])<=64,folder
        write(Path('data/maps')/folder/'map.json',json.dumps(data,ensure_ascii=False,indent=2))
    const=['#ifndef GUARD_CONSTANTS_RF_STORY_H','#define GUARD_CONSTANTS_RF_STORY_H',f'#define RF_STORY_FIRST {FIRST}',f'#define RF_STORY_COUNT {len(quests)}',f'#define RF_STORY_ACTOR_COUNT {len(actors)}','#define RF_STORY_CHAPTER_FIRST 9','#define RF_STORY_EVENT 4','#define RF_STORY_NO_CHOICE 0xFF']
    const += [f'#define RF_STORY_CHOICE_{key.upper()} {bit}' for key,bit in CHOICES.items()]
    const += [f'#define RF_STORY_ACTOR_{key.upper()} {i}' for key,i in actor_ids.items()]
    const += [f'#define RF_QUEST_STORY_{q["scene"]:03d} {q["id"]}' for q in quests]
    content_version=max([BASE_CONTENT_VERSION,*CONTENT_VERSIONS.values()])
    const += [f'#define RF_QUEST_CONTENT_VERSION 0x{content_version:02X}']
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
    definitions+=['};']
    # Puzzle questions and race texts are scene data; the engine holds no scene-specific text.
    for i,q in enumerate(quests):
        if q['questions']:
            definitions.append(f'static const struct RFStoryQuestion sPuzzle_{i}[] = {{')
            for text,answer in q['questions']:
                assert len(textwrap.wrap(text,32))<=2,(q['name'],text)
                definitions.append(f'    {{COMPOUND_STRING({quoted(text)}), {int(answer)}}},')
            definitions.append('};')
        if q['seconds']:
            giver,target=actors[q['giver']],actors[q['target']]
            start=f"La práctica empieza ahora. Tienes {q['seconds']} segundos. Habla con {target['name']} en {PLACES[target['place']]}. Puedes volver para reintentarlo."
            late=f"Llegaste fuera de tiempo. El encargo sigue en el diario. Vuelve con {giver['name']}, en {PLACES[giver['place']]}, para empezar otra vez."
            restart=f"Vuelve al puesto de salida. La práctica dura {q['seconds']} segundos. Si guardaste y saliste del juego, puedes empezar de nuevo."
            definitions.append('static const struct RFStoryRace sRace_%d = {%d, COMPOUND_STRING(%s), COMPOUND_STRING(%s), COMPOUND_STRING(%s)};'
                               %(i,q['seconds'],quoted(start,pages=True),quoted(late,pages=True),quoted(restart,pages=True)))
    definitions+=['static const struct RFStoryScene sScenes[] = {']
    kinds={'talk':'RF_STORY_TALK','delivery':'RF_STORY_DELIVERY','clues':'RF_STORY_CLUES','battle':'RF_STORY_BATTLE','race':'RF_STORY_RACE','puzzle':'RF_STORY_PUZZLE','field':'RF_STORY_FIELD','choice':'RF_STORY_CHOICE'}
    for i,q in enumerate(quests):
        condition='QUEST_EVENT';argument='RF_STORY_EVENT';alternative=str(q['id'])
        extra=''
        if q['kind']=='delivery':
            condition='QUEST_DELIVER_ITEM';argument,alternative=q['proof'];alternative=str(alternative)
        elif q['kind']=='field':
            # Completed in the field by the quest engine; the target only receives the report.
            goal=q['goal'];condition=goal[0];argument=goal[1];alternative='0'
            if condition=='QUEST_COUNT':
                alternative=goal[2]
                extra=f'        .target = {goal[3]},\n'
        elif isinstance(q['proof'],tuple):
            c,a,b=q['proof']
            extra=f'        .objectiveCount = 2, .secondaryCondition = {c}, .secondaryArgument = {a}, .secondaryAlternative = {b},\n'
        previous='RF_QUEST_NONE' if campaign_start[q['campaign']]==i else f'RF_QUEST_STORY_{i-1:03d}'
        desc=quoted(q['objective'])
        assert len(textwrap.wrap(q['objective'],32))<=3,(i,q['objective'])
        entries += [f'    [RF_QUEST_STORY_{i:03d}] = {{',f'        .name = COMPOUND_STRING({quoted(q["name"])}),',f'        .description = COMPOUND_STRING({desc}),',f'        .prerequisite = {previous}, .requiredFlag = {q["flag"]}, .condition = {condition},',f'        .argument = {argument}, .alternative = {alternative}, .rewardItem = {q["reward"]}, .rewardCount = {q["quantity"]}, .chapter = {q["chapter"]},',extra.rstrip(),'    },']
        trainer=f'TRAINER_RF_STORY_{q["proof"]}' if q['kind']=='battle' else '0'
        puzzle=(f'sPuzzle_{i}',len(q['questions'])) if q['questions'] else ('NULL',0)
        race=f'&sRace_{i}' if q['seconds'] else 'NULL'
        def text(value, pages=True):
            return f'COMPOUND_STRING({quoted(value,pages=pages)})' if value else 'NULL'
        if q['question']:
            assert len(textwrap.wrap(q['question'],32))<=2,(q['name'],q['question'])
        choice=f"RF_STORY_CHOICE_{q['choice'].upper()}" if q['choice'] else 'RF_STORY_NO_CHOICE'
        definitions.append('    {%d, %d, %s, %s, 0x%08X, %s, %s, %s, %d, %s, %s, %s, %s, %s},'%(actor_ids[q['giver']],actor_ids[q['target']],kinds[q['kind']],trainer,clue_bits.get(q['id'],0),text(q['offer']),text(q['ending']),puzzle[0],puzzle[1],race,choice,text(q['question'],False),text(q['alt_offer']),text(q['alt_ending'])))
    # Scene <-> quest lookups: scenes may occupy reconverted slots below 350.
    definitions+=['};','static const u16 sSceneQuest[] = {']
    definitions+=[f'    {q["id"]},' for q in quests]
    definitions+=['};','static const u16 sQuestScene[RF_QUEST_COUNT] = {']
    definitions+=[f'    [{q["id"]}] = {q["scene"]+1},' for q in quests]
    definitions+=['};','static const struct RFStoryContract sContracts[] = {']
    for key,title,item,count,reward,qty,unlock in CONTRACTS:
        definitions.append(f'    {{{actor_ids[key]}, {quests[unlock]["id"]}, {item}, {count}, {reward}, {qty}, COMPOUND_STRING({quoted(title)})}},')
    definitions+=['};']
    write('src/data/rf_quests_story.h','\n'.join(entries))
    write('src/data/rf_story.h','\n'.join(definitions))
    header=ROOT/'include/constants/rf_quests.h'
    text=header.read_text(encoding='utf-8')
    text=re.sub(r'#define RF_QUEST_COUNT \d+',f'#define RF_QUEST_COUNT {quest_count}',text)
    text=re.sub(r'#define RF_QUEST_CHAPTER_COUNT \d+',f'#define RF_QUEST_CHAPTER_COUNT {9+len(CAMPAIGNS)}',text)
    write(header.relative_to(ROOT),text)
    write('src/data/rf_story_chapters.h','\n'.join(['// Generated by story_campaigns.py: journal chapters of the authored campaigns.']
                                                  +[f'    COMPOUND_STRING({quoted(name)}),' for name in chapter_names]))
    # Composite rewards and quest-state visibility live in rf_quest_links.h, between markers.
    links=(ROOT/'src/data/rf_quest_links.h').read_text(encoding='utf-8')
    bonuses=[]
    for q in quests:
        if q['bonus']:
            fields=', '.join(f'.{k} = {v}' for k,v in q['bonus'].items())
            bonuses.append(f'{{.quest = RF_QUEST_STORY_{q["scene"]:03d}, {fields}}},')
    objects=[f'{{.script = {PREFIX}{actor_ids[k]}, .quest = RF_QUEST_STORY_{scene:03d}, .minState = 3, .maxState = 3}},'
             for k,scene in visible.items()]
    # One migration row per contiguous run of reconverted IDs.
    migrations=[]
    for t in CAMPAIGN_SLOTS:
        ids_sorted=sorted(CAMPAIGN_SLOTS[t]);start=prev=ids_sorted[0]
        for i in ids_sorted[1:]+[None]:
            if i is not None and i==prev+1: prev=i;continue
            migrations.append(f'{{0x{CONTENT_VERSIONS[t]:02X}, {start}, {prev}}},')
            if i is not None: start=prev=i
    requirements=[]
    for q in quests:
        for group,members in enumerate(q['requires']):
            for title,row in members:
                requirements.append(f'{{RF_QUEST_STORY_{q["scene"]:03d}, RF_QUEST_STORY_{campaign_start[title]+row:03d}, {group}}},')
    links=replace_block(links,'STORY REQUIREMENTS',requirements)
    links=replace_block(links,'STORY MIGRATIONS',migrations)
    links=replace_block(links,'STORY EXTERNS',[f'extern const u8 {PREFIX}{actor_ids[k]}[];' for k in visible])
    links=replace_block(links,'STORY BONUSES',bonuses)
    links=replace_block(links,'STORY OBJECTS',objects)
    write('src/data/rf_quest_links.h',links)
    script=['@ Authored stories. Generated by tools/rojo_fuego/story_campaigns.py.']
    for key,a in actors.items():
        script += [f'{PREFIX}{actor_ids[key]}::','\tlock',*(['\tfaceplayer'] if a['gfx']!='OBJ_EVENT_GFX_ITEM_BALL' else []),f'\tsetvar VAR_0x8004, {actor_ids[key]}','\tcallnative Script_RFStoryPrepare','\tgoto_if_eq VAR_RESULT, 1, RFStory_Offer','\tgoto_if_eq VAR_RESULT, 2, RFStory_Confirm','\tgoto_if_eq VAR_RESULT, 3, RFStory_Puzzle','\tgoto_if_eq VAR_RESULT, 5, RFStory_Choice']
        script+=['\tgoto_if_ne VAR_RESULT, 4, RFStory_ShowText']
        for i,q in enumerate(quests):
            if q['kind']=='battle' and q['target']==key:
                script+= [f'\tgoto_if_eq VAR_0x8005, {q["id"]}, RFStory_Battle_{i}']
        script+=['\tmsgbox gStringVar4','\trelease','\tend','']
    script += ['RFStory_ShowText::','\tmsgbox gStringVar4','\tcallnative Script_RFStoryAfterText','\tgoto_if_eq VAR_0x8007, 1, RFStory_RemoveMarker','\trelease','\tend','RFStory_RemoveMarker::','\tremoveobject VAR_LAST_TALKED','\trelease','\tend','RFStory_Offer::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tgoto_if_eq VAR_RESULT, NO, RFStory_Bye','\tcallnative Script_RFStoryAccept','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Confirm::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tgoto_if_eq VAR_RESULT, NO, RFStory_Bye','\tcallnative Script_RFStoryConfirm','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Puzzle::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tcallnative Script_RFStoryPuzzle','\tgoto_if_eq VAR_RESULT, 3, RFStory_Puzzle','\tmsgbox gStringVar4','\trelease','\tend',
               'RFStory_Choice::','\tmsgbox gStringVar4, MSGBOX_YESNO','\tcallnative Script_RFStoryChoice','\tmsgbox gStringVar4','\trelease','\tend',
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
    print(f'Generated {len(quests)} authored quests, {len(actors)} actors, {bit} clues, {len(TRAINERS)} trainer teams; '
          f'{quest_count} quests in the catalogue, reconverted slots: {sorted(i for t in CAMPAIGN_SLOTS for i in CAMPAIGN_SLOTS[t])}.')


if __name__=='__main__': main()
