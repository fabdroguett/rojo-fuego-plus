"""Exercise real story scripts, maps, journal and flash-save round-trip in mGBA.

Requires a disposable build from build_story_fixture.py. Never uses a personal save.
"""
import argparse
import json
from pathlib import Path
import re
from runtime import run as environment
from ppm2png import convert

ROOT=Path(__file__).resolve().parents[3]
parser=argparse.ArgumentParser()
parser.add_argument('--workdir',default='/work')
parser.add_argument('--case',action='append',default=[])
parser.add_argument('--output',type=Path,default=ROOT/'build_artifacts/story-campaigns/emulator')
args=parser.parse_args()
OUT=args.output.resolve();OUT.mkdir(parents=True,exist_ok=True)
work=args.workdir.rstrip('/')
data=json.loads((ROOT/'build_artifacts/story-campaigns/catalog.json').read_text(encoding='utf-8'))
actors={a['key']:a for a in data['actors']}
nm=environment('exec','-w',work,'rojo-fuego-check','arm-none-eabi-nm','pokefirered.elf',capture_output=True,text=True).stdout
symbols={p[2]:int(p[0],16) for line in nm.splitlines() if len(p:=line.split())==3}
main=symbols['gMain']+4
params=symbols['gRFStoryFixtureParams'];result=symbols['gRFStoryFixtureResult']
base='run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n'
def call(name,frames=60): return f'w32 {main:08x} {symbols["RFStoryFixture_"+name]|1:08x}\nrun {frames}\n'
def param(index,value): return f'w16 {params+index*2:08x} {value:04x}\n'
def read(index): return f'r32 {result+index*4:08x}\n'
def shot(name): return f'shot /tmp/rf-story-{name}.ppm\n'
def key(k,wait=180): return f'hold {k} 6\nrun {wait}\n'
def prepare(quest,keyname,mode=1):
    return param(0,quest)+param(1,actors[keyname]['actor'])+param(2,mode)+call('PrepareQuest')
def warp(keyname):
    # A short press toward the actor turns the player in place, as a real player would;
    # FaceActor alone is undone when an outdoor map finishes loading after it.
    a=actors[keyname];lx,ly=a['landing']
    toward='>' if lx<a['x'] else '<' if lx>a['x'] else 'D' if ly<a['y'] else 'U'
    return param(1,a['actor'])+call('WarpActor',240)+call('FaceActor',30)+f'hold {toward} 3\nrun 30\n'
def inspect(): return call('Read',1)+''.join(read(i) for i in (0,1,2,3,4,5,7,8,9,10,11,12,13,14,15))
def look(*indices): return call('Read',1)+''.join(read(i) for i in indices)
SPECIES={n:int(v) for n,v in re.findall(r'\b(SPECIES_[A-Z0-9_]+)\s*=\s*(\d+)',(ROOT/'include/constants/species.h').read_text(encoding='utf-8'))}
def wild(species,level):
    # Mewtwo (Lv. 100, Psychic first) wins with A; B clears the closing messages.
    return param(4,SPECIES[species])+param(5,level)+call('WildBattle',60)+'mash A 1800\nmash B 600\nrun 120\n'
# Batch C1: CENSO DE LA HIERBA ALTA is 414-421, LA ESCUELA DEL CAMINO is 422-429.
C1_CENSUS,C1_SCHOOL=414,422
# Batch C2 reuses reconverted generic slots: VOCES EN LA OSCURIDAD 334-341, LA RUTA DE LAS BAYAS 342-349.
C2_RESCUE,C2_BERRIES=334,342
ITEMS={n:int(v) for n,v in re.findall(r'^\s*(ITEM_[A-Z0-9_]+)\s*=\s*(\d+)',(ROOT/'include/constants/items.h').read_text(encoding='utf-8'),re.M)}
def give(item,count): return param(4,ITEMS[item])+param(5,count)+call('AddItem')
# Batch C3 hands its slots out in order; look a scene up by campaign title and row.
def c3(title,row): return next(q['id'] for q in data['quests'] if q['campaign']==title and q['scene']-min(x['scene'] for x in data['quests'] if x['campaign']==title)==row)

cases={
    'offer-cancel':(prepare(350,'alma',0)+warp('alma')+key('A')+shot('offer')+'mash B 1200\nrun 60\n'+shot('offer-cancel')+inspect(),{0:0,12:1,13:0}),
    'talk':(prepare(350,'iria')+warp('iria')+key('A')+shot('talk')+'mash A 1200\nrun 60\n'+shot('talk-complete')+inspect(),{0:3,12:1}),
    'delivery':(prepare(369,'zaira')+warp('zaira')+key('A')+shot('delivery')+'mash A 1200\nrun 60\n'+shot('delivery-complete')+inspect(),{0:3,12:1}),
    'puzzle-wrong':(prepare(356,'monan')+warp('monan')+key('A',300)+shot('puzzle')+key('B',300)+'mash B 360\nrun 60\n'+shot('puzzle-wrong')+inspect(),{0:1,12:1}),
    'puzzle':(prepare(356,'monan')+warp('monan')+key('A',300)+key('A',300)+key('B',300)+key('A',300)+'mash B 900\nrun 60\n'+shot('puzzle-complete')+inspect(),{0:3,12:1}),
    # Silva has her own four questions (yes, no, no, yes), separate from the Monan lectern.
    'puzzle-silva':(prepare(408,'silva')+warp('silva')+key('A',300)+shot('puzzle-silva')+key('A',300)+key('B',300)+key('B',300)+key('A',300)+'mash B 900\nrun 60\n'+shot('puzzle-silva-complete')+inspect(),{0:3,12:1}),
    # The race texts and its limit come from scene data; arriving in time completes it.
    'race':(prepare(407,'riel')+warp('riel')+key('A',240)+shot('race-start')+'mash B 360\nrun 60\n'+warp('lina')+key('A',240)+shot('race-arrival')+'mash A 900\nrun 60\n'+inspect(),{0:3,12:1}),
    'race-late':(prepare(407,'riel')+warp('riel')+key('A',240)+'mash B 360\nrun 2800\n'+warp('lina')+key('A',240)+shot('race-late')+'mash B 600\nrun 60\n'+inspect(),{0:1,12:1}),
    # The journal names the giver of an authored scene and refuses to accept it remotely.
    'journal-locked':(prepare(350,'alma',0)+call('Journal',120)+shot('journal-locked')+key('A',60)+shot('journal-locked-refused')+key('B',120)+inspect(),{0:0,13:0}),
    # The two-line offer is still printing after 240 frames; an A then only completes the
    # text, so wait for the YES/NO menu. The whole battle and its report need ~3600 frames.
    'battle':(prepare(360,'lio')+warp('lio')+key('A',480)+shot('battle-offer')+key('A',300)+shot('battle')+'mash A 3600\nmash B 900\nrun 120\n'+shot('battle-complete')+inspect(),{0:3}),
    'home-cubone':(prepare(373,'home_cubone',2)+warp('home_cubone')+shot('home-cubone')+key('A',240)+shot('home-cubone-dialog')+'mash B 600\nrun 60\n'+inspect(),{0:3,12:1}),
    'journal':(prepare(350,'alma')+call('Journal',120)+shot('journal')+key('>',60)+shot('journal-available')+key('>',60)+shot('journal-active')+key('>',60)+shot('journal-empty')+key('A',30)+key('B',120)+shot('journal-closed')+inspect(),{0:1,13:0}),
    'ending':(prepare(357,'alma',2)+call('CompleteStories')+call('Journal',120)+shot('journal-completed')+key('A',60)+shot('ending-page1')+key('A',60)+shot('ending-page2')+key('B',60)+key('B',120)+inspect(),{0:3,11:192,13:0}),
    # Nora (Kindle Road, y=133) must appear at her landing tile and answer; without Ponyta
    # registered the quest stays active. Also reads object count, position, map and hiding.
    'nora':(prepare(379,'nora')+warp('nora')+shot('nora')+key('A',240)+shot('nora-talk')+'mash B 600\nrun 60\n'+call('Read',1)+''.join(read(i) for i in (0,12,16,17,18,19)),{0:1,12:1,17:0x850000,19:0}),
    # Visiting the museum completes quest 0 in the field: "¡MISIÓN LISTA!" once the player is free.
    # FaceActor closes any toast, so these warp without it and capture every 60 frames.
    'ready-notice':(prepare(0,'alma')+param(1,actors['alma']['actor'])+call('WarpActor',1)+''.join('run 60\n'+shot(f'ready-notice-{i}') for i in range(8))+inspect(),{0:2}),
    # Entering the map of the tracked scene (351, Edda in Mt. Moon B2F) shows "OBJETIVO AQUÍ".
    'here-notice':(prepare(351,'edda')+param(1,actors['edda']['actor'])+call('WarpActor',1)+''.join('run 60\n'+shot(f'here-notice-{i}') for i in range(8))+inspect(),{0:1}),
    # C1 field objectives through real gameplay: seeing Pikachu in a wild battle, a real wild win,
    # the evolution scene and the egg hatch scene each move their quest to READY.
    'c1-seen':(prepare(C1_CENSUS+2,'olmo')+warp('olmo')+wild('SPECIES_PIKACHU',3)+key('A',480)+shot('c1-seen-report')+key('A',300)+'mash B 900\nrun 60\n'+look(0,12),{0:3,12:1}),
    'c1-wild-win':(prepare(C1_CENSUS+3,'olmo')+param(4,7)+call('SetProgress')+wild('SPECIES_RATTATA',2)+look(0,23),{0:2,23:8}),
    'c1-evolve':(prepare(C1_CENSUS+5,'vito')+param(4,1)+call('SetProgress')+param(4,SPECIES['SPECIES_CATERPIE'])+param(5,SPECIES['SPECIES_METAPOD'])+call('Evolve',60)+'mash A 1500\nrun 300\n'+shot('c1-evolve')+look(0,23),{0:2,23:2}),
    'c1-hatch':(prepare(C1_CENSUS+6,'nerea')+param(4,SPECIES['SPECIES_TOGEPI'])+call('Hatch',60)+'mash B 2400\nrun 120\n'+shot('c1-hatch')+look(0),{0:2}),
    # Cora joins Oak's lab only after the census is handed in; the final grants the composite reward.
    'c1-cora-hidden':(prepare(C1_CENSUS+7,'ada')+warp('ada')+param(1,actors['cora_lab']['actor'])+look(0,12),{0:1,12:0}),
    'c1-census-final':(prepare(C1_CENSUS+7,'ada')+call('ZeroMoney')+warp('ada')+key('A',480)+'mash B 900\nrun 60\n'+shot('c1-census-final')+warp('cora_lab')+shot('c1-cora-visible')+look(0,12,20,21),{0:3,12:1,20:5000,21:1}),
    # The school: Quim's battle, the decision (YES) and a later offer that follows it.
    'c1-quim':(prepare(C1_SCHOOL+2,'quim')+warp('quim')+key('A',480)+shot('c1-quim-offer')+key('A',300)+'mash A 3600\nmash B 900\nrun 120\n'+look(0),{0:3}),
    'c1-choice':(prepare(C1_SCHOOL+3,'tano')+warp('tano')+key('A',480)+shot('c1-choice-question')+key('A',300)+shot('c1-choice-ending')+'mash B 900\nrun 60\n'+look(0,22),{0:3,22:1}),
    # C2 on reconverted IDs: delivery, a talk deep in Rock Tunnel (preview screen: wait longer),
    # Mora's puzzle (no, yes, no), the forest decision and the rescue finale with Nico back home.
    'c2-delivery':(prepare(C2_RESCUE,'brisa')+give('ITEM_ESCAPE_ROPE',2)+warp('brisa')+key('A',480)+shot('c2-delivery-offer')+key('A',300)+shot('c2-delivery-confirm')+key('A',300)+'mash B 900\nrun 60\n'+shot('c2-delivery')+look(0),{0:3}),
    'c2-tunnel':(prepare(C2_RESCUE+2,'nico')+warp('nico')+'run 360\n'+key('A',480)+shot('c2-tunnel')+'mash A 900\nrun 60\n'+look(0,12),{0:3,12:1}),
    'c2-puzzle':(prepare(C2_BERRIES+3,'mora')+warp('mora')+key('A',300)+shot('c2-puzzle')+key('B',300)+key('A',300)+key('B',300)+'mash B 900\nrun 60\n'+look(0),{0:3}),
    'c2-choice':(prepare(C2_BERRIES+6,'lis')+warp('lis')+key('A',480)+key('A',300)+shot('c2-choice')+'mash B 900\nrun 60\n'+look(0,22),{0:3,22:2}),
    'c2-final':(prepare(C2_RESCUE+7,'brisa')+call('ZeroMoney')+warp('brisa')+key('A',480)+'mash B 900\nrun 60\n'+warp('nico_casa')+shot('c2-nico-home')+look(0,12,20),{0:3,12:1,20:2500}),
    # Interface: the location popup slides in after a warp to an outdoor map (Pallet Town).
    'popup':(prepare(c3('EL CARTERO DE PALETA',0),'tobi',0)+param(1,actors['tobi']['actor'])+call('WarpActor',1)+''.join('run 30\n'+shot(f'popup-{i}') for i in range(6))+''.join(f'r32 {a:08x}\n' for a in (0x0600A0E0,0x0600A100,0x0600A200,0x0600A400))+'r16 04000008\nr16 04000012\n',{}),
    # C3 (every remaining generic slot): one scene of each kind, on IDs below 334.
    'c3-delivery':(prepare(c3('EL CARTERO DE PALETA',4),'tobi')+give('ITEM_POTION',2)+warp('tobi')+key('A',480)+shot('c3-delivery-offer')+key('A',300)+key('A',300)+'mash B 900\nrun 60\n'+shot('c3-delivery')+look(0),{0:3}),
    'c3-battle':(prepare(c3('EL CARTERO DE PALETA',5),'tobi')+warp('tobi')+key('A',480)+shot('c3-battle-offer')+key('A',300)+'mash A 3600\nmash B 900\nrun 120\n'+look(0),{0:3}),
    'c3-puzzle':(prepare(c3('EL MUSEO SIN VITRINAS',3),'lazaro')+warp('lazaro')+key('A',300)+shot('c3-puzzle')+key('B',300)+key('A',300)+key('B',300)+'mash B 900\nrun 60\n'+look(0),{0:3}),
    'c3-race':(prepare(c3('LA FERIA DE CELESTE',4),'pilar')+warp('pilar')+key('A',240)+shot('c3-race-start')+'mash B 360\nrun 60\n'+warp('arturo')+key('A',240)+'mash A 900\nrun 60\n'+look(0),{0:3}),
    'c3-choice':(prepare(c3('FICHAS SIN DUEÑO',5),'dimas')+warp('dimas')+key('A',480)+key('A',300)+shot('c3-choice')+'mash B 900\nrun 60\n'+look(0,22),{0:3,22:1<<4}),
    # The seven-scene campaign closes on slot 333 with its composite money reward.
    'c3-final':(prepare(c3('RECIÉN LLEGADOS A AZAFRÁN',6),'lin')+call('ZeroMoney')+warp('lin')+key('A',480)+'mash B 900\nrun 60\n'+shot('c3-final')+look(0,20),{0:3,20:2500}),
    # A content-version 0x32 save through flash: the reconverted slot is available again.
    'migration':(call('MigrateProbe',600)+look(15),{15:1}),
    'c1-alt-offer':(prepare(C1_SCHOOL+4,'tano',0)+param(4,0)+call('SetChoice')+warp('tano')+key('A',480)+shot('c1-alt-offer')+'mash B 900\nrun 60\n'+look(0,22),{0:0,22:1}),
    # Flash round-trip of story fields and the quest extension (decisions, counters, IDs 500+),
    # then an older save whose extension magic is garbage. SaveBlock3 is 1620 of 1624 bytes.
    'save':(call('SaveRoundTrip',600)+shot('save-restored')+inspect(),{2:0x15555,3:2,4:1,5:77,8:1620,14:1,15:1}),
}

clues=''
for index in range(3):
    name=f'clue_2_{index}'
    clues+=warp(name)+key('A',240)+shot(f'clue-{index}')+'mash B 480\nrun 60\n'
cases['clues']=(prepare(352,'edda')+clues+warp('edda')+key('A',240)+'mash A 1200\nrun 60\n'+shot('clues-complete')+inspect(),{0:3,1:3})

# Check all 55 NPC/object instances in one emulator process, including conditional clues.
survey='';expected_survey={}
for a in data['actors']:
    keyname=a['key'];q=350;mode=0
    # Clue markers belong to a scene index; its quest ID comes from the catalogue (slots may be reconverted).
    if keyname.startswith('clue_'): q=data['quests'][int(keyname.split('_')[1])]['id'];mode=1
    if keyname=='home_cubone': q=373;mode=2
    if keyname=='cora_lab': q=C1_CENSUS+7;mode=2 # visible once the census is handed in
    if keyname=='nico_casa': q=C2_RESCUE+7;mode=2 # visible once the rescue report is closed
    survey+=prepare(q,keyname,mode)+warp(keyname)+shot('actor-'+keyname)+f'w32 {result+12*4:08x} 0\n'+call('Read',1)+read(12)
    # Second read for maps that finish loading later (preview screens, long interiors).
    survey+='run 360\n'+f'w32 {result+12*4:08x} 0\n'+call('Read',1)+read(12)
cases['actors']=(survey,{})

# Every legendary object appears for its active saga next to a tile the player can stand on
# (land on foot or by Surf, or water for the few sea legends). Positions come from legends.py.
def legend_survey():
    import sys
    sys.path.insert(0,str(ROOT/'tools/rojo_fuego'))
    from legends import MapGrid, behaviors, tileset_attribute_files, grid_opener
    layouts={x['id']:x for x in json.loads((ROOT/'data/layouts/layouts.json').read_text(encoding='utf-8'))['layouts'] if x}
    attrs,mb=tileset_attribute_files(),behaviors();opener=grid_opener(layouts,attrs,mb)
    groups=(ROOT/'include/constants/map_groups.h').read_text(encoding='utf-8')
    map_ids={n:int(num)|(int(group)<<8) for n,num,group in re.findall(r'(MAP_\w+)\s*=\s*\((\d+) \| \((\d+) << 8\)\)',groups)}
    quests={script:43+int(saga) for saga,script in re.findall(r'RF_QUEST_LEGEND_(\d+), (RF_EventScript_Legend_\d+)',(ROOT/'src/data/rf_legends.h').read_text(encoding='utf-8'))}
    steps,count='',0
    for path in sorted((ROOT/'data/maps').glob('*_Frlg/map.json')):
        m=json.loads(path.read_text(encoding='utf-8'))
        legends=[(i,o) for i,o in enumerate(m.get('object_events',[])) if o.get('script','').startswith('RF_EventScript_Legend_')]
        if not legends: continue
        grid=MapGrid(path.parent.name,layouts,attrs,mb)
        blocked={(o['x'],o['y']) for o in m['object_events']}
        reach=grid.reachable(blocked,opener,surf=True)
        for i,o in legends:
            around=[(o['x']+dx,o['y']+dy) for dx,dy in ((0,1),(0,-1),(1,0),(-1,0))]
            land=[t for t in around if t in reach and grid.walkable(*t)]
            x,y=(land or [t for t in around if t in reach])[0]
            steps+=param(0,quests[o['script']])+param(2,1)+call('PrepareQuest')
            steps+=param(4,map_ids[m['id']])+param(5,x)+param(6,y)+param(7,i+1)+call('WarpTo',240)
            # Clear the result first: a read that never runs (frozen game) must not reuse the last one.
            steps+=shot('legend-'+o['script'][-3:])+f'w32 {result+24*4:08x} 0\n'+call('ReadObject',1)+read(24)
            # Second read: maps with a preview screen (Rock Tunnel) finish loading later.
            steps+='run 360\n'+f'w32 {result+24*4:08x} 0\n'+call('ReadObject',1)+read(24)
            # Third read: an achievement toast can hold the frame in which a read was scheduled.
            steps+='run 600\n'+f'w32 {result+24*4:08x} 0\n'+call('ReadObject',1)+read(24)
            LEGEND_NAMES.append(f"{path.parent.name}:{o['script']}@{o['x']},{o['y']}")
            count+=1
    return steps,count
LEGEND_NAMES=[]
legend_steps,LEGEND_COUNT=legend_survey()
cases['legends']=(legend_steps,{})

report=[]
for name in args.case or list(cases):
    steps,expected=cases[name]
    for theme in ('dark','light'):
        script=base+param(3,theme=='light')+call('Setup')+steps
        script=script.replace('/tmp/rf-story-',f'/tmp/rf-story-{name}-{theme}-')
        local=OUT/f'{name}-{theme}.txt';local.write_text(script,encoding='utf-8')
        remote=f'/tmp/rf-story-test-{name}-{theme}.txt'
        environment('cp',str(local),'rojo-fuego-check:'+remote)
        r=environment('exec','-w',work,'rojo-fuego-check','/tmp/rf-mgba-harness',work+'/pokefirered.gba',remote,capture_output=True,text=True)
        (OUT/f'{name}-{theme}.log').write_text(r.stdout,encoding='utf-8')
        for line in script.splitlines():
            if line.startswith('shot '):
                source=line.split()[1];ppm=OUT/Path(source).name
                environment('cp','rojo-fuego-check:'+source,str(ppm))
                convert(str(ppm),str(ppm.with_suffix('.png')),2)
                convert(str(ppm),str(ppm.with_suffix('.native.png')),1)
        matches=re.findall(r'([0-9a-fA-F]{8}) = ([0-9a-fA-F]+)',r.stdout)
        observed={int(ad,16):int(v,16) for ad,v in matches}
        checks={i:observed.get(result+i*4) for i in expected}
        passed=checks==expected
        if name=='legends':
            # Two reads per legend: right after loading (maps with a preview screen load later) and
            # 360 frames on (a wandering wild spawn may have started a battle by then).
            reads=[int(v,16) for ad,v in matches if int(ad,16)==result+24*4]
            found=[int(1 in reads[i:i+3]) for i in range(0,len(reads),3)]
            passed=len(found)==LEGEND_COUNT and all(v==1 for v in found)
            checks={'legendsFound':sum(v==1 for v in found),'legendsExpected':LEGEND_COUNT}
            if not passed: print('missing legends:',[n for n,v in zip(LEGEND_NAMES,found) if v!=1],flush=True)
        if name=='actors':
            reads=[int(v,16) for ad,v in matches if int(ad,16)==result+12*4]
            observed_actors=[int(a==1 or b==1) for a,b in zip(reads[0::2],reads[1::2])]
            passed=len(observed_actors)==len(data['actors']) and all(v==1 for v in observed_actors)
            checks={'actorsFound':sum(v==1 for v in observed_actors),'actorsExpected':len(data['actors'])}
        entry=dict(case=name,theme=theme,passed=passed,checks=checks)
        entry['observed']={i:observed.get(result+i*4) for i in range(20) if result+i*4 in observed}
        report.append(entry)
        (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(name,theme,'PASS' if passed else 'FAIL',checks,flush=True)
        assert passed,(name,theme,checks,expected)
