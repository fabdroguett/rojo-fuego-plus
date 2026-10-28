"""Run disposable real-menu fixtures in mGBA, preserving scripts and native screenshots."""
import argparse
import json
from pathlib import Path
import re
import subprocess
import sys
from runtime import run as environment

ROOT = Path(__file__).resolve().parents[3]
p = argparse.ArgumentParser()
p.add_argument('--workdir', default='/work')
p.add_argument('--case', action='append', default=[])
p.add_argument('--output', type=Path, default=ROOT/'build_artifacts/ui/kanto-art')
a = p.parse_args()
OUT=a.output.resolve()
OUT.mkdir(parents=True,exist_ok=True)
work=a.workdir.rstrip('/')
nm=environment('exec','-w',work,'rojo-fuego-check','arm-none-eabi-nm','pokefirered.elf',capture_output=True,text=True).stdout
symbols={s[2]:int(s[0],16) for line in nm.splitlines() if len(s:=line.split())==3}
symbols['gMainCallback2']=symbols['gMain']+4
base='run 600\nmash T 30\nrun 180\nmash A 3900\nmash B 240\nrun 60\n'
def cb(name): return f'w32 {symbols["gMainCallback2"]:08x} {symbols[name]|1:08x}\n'
def read(name): return f'r32 {symbols[name]:08x}\n'
def shot(name): return f'shot /tmp/rf-kanto-{name}.ppm\n'
def key(k,n=60): return f'hold {k} 3\nrun {n}\n'
def inspect(): return cb('RFMainMenuFixture_Inspect')+'run 1\n'+read('gRFMainMenuFixtureWindow')

cases={}
for theme in ('Dark','Light'):
    fixture='RFMainMenuFixture_'+theme
    cases[theme.lower()]=(fixture,shot(theme.lower())+key('B',90)+shot(theme.lower()+'-closed')+inspect(),{'gRFMainMenuFixtureWindow':255})
    cases['font-'+theme.lower()]=(fixture,cb('RFMainMenuFixture_Font')+'run 60\n'+shot('font-'+theme.lower()),{})
    cases['grid-'+theme.lower()]=(fixture,''.join(shot(f'grid-{theme.lower()}-{i}')+key('>',60) for i in range(9))+read('sStartMenuCursorPos'),{'sStartMenuCursorPos':0})
cases['zero']=('RFMainMenuFixture_Zero',shot('money-zero'),{})
cases['max']=('RFMainMenuFixture_Max',shot('money-max'),{})
cases['locked']=('RFMainMenuFixture_Locked',shot('locked')+key('R')+shot('locked-r')+inspect(),{})
cases['shortcut']=('RFMainMenuFixture_Dark',key('R',150)+shot('shortcut-missions')+key('B',120)+shot('shortcut-return'),{})
# Action order comes from BuildNormalStartMenu; do not invent disabled entries.
actions=['pokedex','dexnav','equipo','mochila','perfil','guardar','ajustes','claves','misiones']
callbacks=['StartMenuPokedexCallback','HandleStartMenuInput','StartMenuPokemonCallback',
           'StartMenuBagCallback','StartMenuPlayerNameCallback','SaveCallback',
           'StartMenuOptionCallback','StartMenuKeySystemCallback','StartMenuQuestsCallback']
for i,(name,callback) in enumerate(zip(actions,callbacks)):
    for theme in ('Dark','Light'):
        suffix=name+('-light' if theme=='Light' else '')
        cases['action-'+suffix]=('RFMainMenuFixture_'+theme,key('>',30)*i+key('A',180)+shot('action-'+suffix)
                              +read('gMenuCallback')+key('B',180)+shot('return-'+suffix)+read('gMainCallback2'),
                              {'gMenuCallback':symbols[callback]|1,'gMainCallback2':symbols['CB2_Overworld']|1})
if 'RFMainMenuFixture_Outdoor' in symbols:
    cases['dexnav-available']=('RFMainMenuFixture_Outdoor',key('>')+key('A',180)+shot('dexnav-available')
                               +read('gMenuCallback')+key('B',180)+shot('dexnav-return')+read('gMainCallback2'),
                               {'gMenuCallback':symbols['StartMenuDexNavCallback']|1,'gMainCallback2':symbols['CB2_Overworld']|1})

report=[]
for name in a.case or list(cases):
    fixture,steps,expected=cases[name]
    script=base+cb(fixture)+'run 180\n'+steps
    local=OUT/(name+'.txt')
    local.write_text(script,encoding='utf-8')
    remote='/tmp/rf-kanto-'+name+'.txt'
    environment('cp',str(local),'rojo-fuego-check:'+remote)
    result=environment('exec','-w',work,'rojo-fuego-check','/tmp/rf-mgba-harness',work+'/pokefirered.gba',remote,capture_output=True,text=True)
    (OUT/(name+'.log')).write_text(result.stdout,encoding='utf-8')
    for line in script.splitlines():
        if line.startswith('shot '):
            src=line.split()[1]
            ppm=OUT/Path(src).name
            environment('cp','rojo-fuego-check:'+src,str(ppm))
            subprocess.run([sys.executable,str(Path(__file__).with_name('ppm2png.py')),str(ppm)],check=True)
    values={int(ad,16):int(v,16) for ad,v in re.findall(r'([0-9a-fA-F]{8}) = ([0-9a-fA-F]+)',result.stdout)}
    checks={s:values.get(symbols[s]) for s in expected}
    if 'sStartMenuCursorPos' in checks and checks['sStartMenuCursorPos'] is not None:
        checks['sStartMenuCursorPos'] &=255
    passed=checks==expected
    report.append({'case':name,'assertionsPassed':passed,'checks':checks,'visualReviewRequired':True})
    (OUT/'menu-report.json').write_text(json.dumps(report,indent=2)+'\n',encoding='utf-8')
    print(name,('PASS' if expected else 'CAPTURED') if passed else 'FAIL',checks,flush=True)
    assert passed,(name,checks,expected)
