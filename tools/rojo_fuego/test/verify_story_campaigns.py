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
    if keyname.startswith('clue_'): q=350+int(keyname.split('_')[1]);mode=1
    if keyname=='home_cubone': q=373;mode=2
    survey+=prepare(q,keyname,mode)+warp(keyname)+shot('actor-'+keyname)+call('Read',1)+read(12)
cases['actors']=(survey,{})

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
        if name=='actors':
            observed_actors=[int(v,16) for ad,v in matches if int(ad,16)==result+12*4]
            passed=len(observed_actors)==55 and all(v==1 for v in observed_actors)
            checks={'actorsFound':sum(v==1 for v in observed_actors),'actorsExpected':55}
        entry=dict(case=name,theme=theme,passed=passed,checks=checks)
        entry['observed']={i:observed.get(result+i*4) for i in range(20) if result+i*4 in observed}
        report.append(entry)
        (OUT/'report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
        print(name,theme,'PASS' if passed else 'FAIL',checks,flush=True)
        assert passed,(name,theme,checks,expected)
