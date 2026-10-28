"""Execute the production quest and scene engines together, not an imitation of them."""
from pathlib import Path
import json
import re
import shutil
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[3]
sys.path.insert(0,str(ROOT/'tools/rojo_fuego'))
from story_content import CAMPAIGNS, CLUES, CONTRACTS


class StoryCampaignTests(unittest.TestCase):
    def test_catalog_scripts_and_persistent_ids(self):
        rows=[q for _,_,campaign in CAMPAIGNS for q in campaign]
        self.assertEqual(len(rows),64)
        self.assertEqual(len(set(q['name'] for q in rows)),64)
        self.assertEqual(sum(len(points) for points in CLUES.values()),17)
        data=json.loads((ROOT/'build_artifacts/story-campaigns/catalog.json').read_text(encoding='utf-8')) if (ROOT/'build_artifacts/story-campaigns/catalog.json').exists() else None
        # The source integration itself remains testable when ignored build artifacts are absent.
        actors={}
        for path in (ROOT/'data/maps').glob('*_Frlg/map.json'):
            m=json.loads(path.read_text(encoding='utf-8'))
            for o in m.get('object_events',[]):
                if o.get('script','').startswith('RFStory_Actor_'):
                    self.assertTrue(o['local_id'].startswith('LOCALID_RF_STORY_'))
                    self.assertNotIn(o['script'],actors)
                    actors[o['script']]=path
        script=(ROOT/'data/scripts/rojo_fuego_story.inc').read_text(encoding='utf-8')
        for label in actors: self.assertIn(label+'::',script)
        self.assertEqual(len(actors),55)
        self.assertIn('goto_if_ne VAR_RESULT, 4, RFStory_ShowText',script)
        for battle in re.findall(r'RFStory_Battle_\d+::(.*?)(?=\nRFStory_|\Z)',script,re.S):
            self.assertLess(battle.index('MSGBOX_YESNO'),battle.index('trainerbattle_single'))
        constants=(ROOT/'include/constants/rf_quests.h').read_text()
        self.assertIn('#define RF_QUEST_MUSEUM 0',constants)
        self.assertIn('#define RF_QUEST_EXTRA_FIRST 71',constants)
        self.assertIn('#define RF_QUEST_CAPACITY 500',constants)

    def test_all_campaigns_migration_clues_rewards_contracts_and_race(self):
        def expand(path):
            s=path.read_text(encoding='utf-8')
            def sub(m):
                p=path.parent/m[1]
                return expand(p) if p.exists() and p.suffix=='.h' else m[0]
            return re.sub(r'^#include "([^\"]+)"',sub,s,flags=re.M)
        quest=(ROOT/'src/rf_quests.c').read_text(encoding='utf-8').replace('#include "data/rf_quests.h"',expand(ROOT/'src/data/rf_quests.h'))
        story=(ROOT/'src/rf_story.c').read_text(encoding='utf-8').replace('#include "data/rf_story.h"',(ROOT/'src/data/rf_story.h').read_text(encoding='utf-8'))
        source=re.sub(r'^#include .*$', '',quest+'\n'+story,flags=re.M)
        # The ROM preprocessor emits game-encoded strings. Reproduce its control bytes
        # while leaving ordinary letters readable for a host-compiler test.
        source=source.replace('\\p','\\373').replace('\\n','\\376')
        species=sorted(set(re.findall(r'\bSPECIES_[A-Z0-9_]+',source))-{'SPECIES_NONE'})
        items=sorted(set(re.findall(r'\bITEM_[A-Z0-9_]+',source))-{'ITEM_NONE'})
        maps=sorted(set(re.findall(r'\bMAP_[A-Z0-9_]+',source))-{'MAP_GROUP','MAP_NUM'})
        flags=sorted(set(re.findall(r'\bFLAG_[A-Z0-9_]+',source))-{'FLAG_GET_CAUGHT'})
        trainers=sorted(set(re.findall(r'\bTRAINER_[A-Z0-9_]+',source))-{'TRAINER_BATTLE_PARAM'})
        prefix=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8; typedef int8_t s8; typedef uint16_t u16; typedef int16_t s16;
typedef uint32_t u32; typedef int32_t s32; typedef int32_t bool32; typedef uint8_t bool8;
#define EWRAM_DATA
#define TRUE 1
#define FALSE 0
#define IS_FRLG 1
#define PARTY_SIZE 6
#define B_TRAINER_PLAYER 0
#define ARRAY_COUNT(a) (sizeof(a)/sizeof((a)[0]))
#define STATIC_ASSERT(x,n) _Static_assert(x,#n)
#define COMPOUND_STRING(x) ((const u8 *)(x "\377"))
#define MAP_GROUP(m) ((m)>>8)
#define MAP_NUM(m) ((m)&255)
#define GET_BASE_SPECIES_ID(x) (x)
#define EOS 255
#define CHAR_NEWLINE 254
#define CHAR_PROMPT_CLEAR 251
#define ACH_BYTES 40
#define STR_CONV_MODE_LEFT_ALIGN 0
#define min(a,b) ((a)<(b)?(a):(b))
#define B_OUTCOME_WON 1
#define BATTLE_TYPE_TRAINER 1
#define BATTLE_TYPE_LINK 2
#define BATTLE_TYPE_RECORDED 4
#define BATTLE_TYPE_FRONTIER 8
'''
        for name in ('rf_quests.h','rf_quests_extra.h','rf_story.h'):
            prefix+=(ROOT/'include/constants'/name).read_text(encoding='utf-8')+'\n'
        for filename in ('rf_quests.h','rf_story.h'):
            prefix+=re.sub(r'^#include .*$', '',(ROOT/'include'/filename).read_text(encoding='utf-8'),flags=re.M)+'\n'
        prefix+='enum Species {SPECIES_NONE, '+','.join(species)+'};\n'
        prefix+='enum Item {ITEM_NONE, '+','.join(items)+'};\n'
        prefix+='\n'.join(f'#define {n} {1000+i}' for i,n in enumerate(maps))+'\n'
        prefix+='\n'.join(f'#define {n} {1+i}' for i,n in enumerate(flags))+'\n'
        prefix+='\n'.join(f'#define {n} {100+i}' for i,n in enumerate(trainers))+'\n'
        prefix+=r'''
#define FLAG_GET_CAUGHT 1
enum {MON_DATA_SPECIES,MON_DATA_IS_EGG};
struct Pokemon {u16 species; bool8 egg;};
struct Save3 {
    u8 old[64],achievementsUnlocked[40],achievementsClaimed[40]; u32 rfQuestSaveMagic; u8 rfQuestStates[RF_QUEST_STATE_BYTES]; u16 rfQuestTracked;
    u32 rfStorySaveMagic,rfStoryClues; u16 rfStoryContractDay; u8 rfStoryContractReady,rfStoryContractClaimed;
};
static struct Save3 save;
static struct Save3 *gSaveBlock3Ptr=&save;
static struct {struct {u8 mapGroup,mapNum;} location;} world;
static typeof(world) *gSaveBlock1Ptr=&world;
static struct Pokemon gParties[1][6];
static struct {u32 vblankCounter1;} gMain;
static struct {s16 days;} gLocalTime;
static struct {u16 opponentA;} trainer;
#define TRAINER_BATTLE_PARAM trainer
static u32 gBattleOutcome,gBattleTypeFlags;
static u16 gSpecialVar_0x8004,gSpecialVar_0x8005,gSpecialVar_0x8006,gSpecialVar_0x8007,gSpecialVar_Result;
static u8 gStringVar4[1000];
static bool32 room=TRUE;
static bool32 legendProof=FALSE;
static bool8 flags[1024],caught[2048];
static u16 bag[2048],removed[2048];
#define ACH_RF_STORY_FIRST 318
#define ACH_RF_STORY_ALL 319
static void Achievement_Unlock(u32 id) {assert(id==318||id==319);save.achievementsUnlocked[id/8]|=1u<<(id%8);}
static bool32 FlagGet(u16 f) {assert(f<1024);return flags[f];}
static u32 GetMonData(struct Pokemon *m,u32 f) {return f==MON_DATA_IS_EGG?m->egg:m->species;}
static u32 SpeciesToNationalPokedexNum(u32 s) {return s;}
static u8 GetSetPokedexFlag(u32 s,u8 action) {(void)action;assert(s<2048);return caught[s];}
static bool32 RFLegends_IsSagaCaught(u32 saga) {(void)saga;return legendProof;}
static bool32 CheckBagHasItem(u32 item,u32 count) {assert(item<2048);return bag[item]>=count;}
static bool32 AddBagItem(u32 item,u32 count) {assert(item<2048);if(!room)return FALSE;bag[item]+=count;return TRUE;}
static bool32 RemoveBagItem(u32 item,u32 count) {if(!CheckBagHasItem(item,count))return FALSE;bag[item]-=count;removed[item]+=count;return TRUE;}
static u8 *StringCopy(u8 *out,const u8 *s) {while(*s!=EOS)*out++=*s++;*out=EOS;return out;}
static u32 StringLength(const u8 *s) {u32 n=0;while(s[n]!=EOS){assert(n<999);n++;}return n;}
static u8 *StringAppend(u8 *out,const u8 *s) {return StringCopy(out+StringLength(out),s);}
static u8 *CopyItemName(u32 item,u8 *out) {(void)item;return StringCopy(out,COMPOUND_STRING("Material"));}
static u8 *ConvertIntToDecimalStringN(u8 *out,u32 value,u32 mode,u32 length) {
    char n[16];(void)mode;(void)length;snprintf(n,sizeof n,"%u",value);
    for(u32 i=0;n[i];i++)*out++=n[i];*out=EOS;return out;
}
'''
        prefix+='\n'.join(f'const u8 RFStory_Actor_{i}[]={{0}};' for i in range(55))+'\n'
        harness=r'''
static void Reset(void) {
    memset(&save,0,sizeof save);memset(&world,0,sizeof world);
    memset(flags,1,sizeof flags);memset(caught,0,sizeof caught);
    memset(bag,0,sizeof bag);memset(removed,0,sizeof removed);memset(gParties,0,sizeof gParties);
    room=TRUE;legendProof=FALSE;sRaceQuest=0;sPuzzleStep=0;gMain.vblankCounter1=100;
    RFQuests_GetState(0);RFStory_EnsureSave();
}
static void At(u32 actor) {
    world.location.mapGroup=MAP_GROUP(sActors[actor].map);
    world.location.mapNum=MAP_NUM(sActors[actor].map);
    gSpecialVar_0x8004=actor;
}
static void Proof(u32 id) {
    const struct RFQuest *q=&sRFQuests[id];
    if(q->objectiveCount==2) {
        if(q->secondaryCondition==QUEST_PARTY_EITHER) gParties[0][0].species=q->secondaryArgument;
        if(q->secondaryCondition==QUEST_DEX_CAUGHT) caught[q->secondaryArgument]=1;
        if(q->secondaryCondition==QUEST_COLLECT_ITEM) bag[q->secondaryArgument]=9;
        if(q->secondaryCondition==QUEST_LEGENDS) legendProof=TRUE;
    }
}
int main(void) {
    Reset();
    SetQuestState(0,RF_QUEST_CLAIMED);SetQuestState(70,RF_QUEST_READY);SetQuestState(349,RF_QUEST_ACTIVE);
    u8 old[RF_QUEST_STATE_BYTES];memcpy(old,save.rfQuestStates,sizeof old);
    save.rfStorySaveMagic=0;save.rfStoryClues=0xA5A5A5A5;save.rfStoryContractClaimed=255;
    save.achievementsUnlocked[39]=255;save.achievementsClaimed[39]=255;
    RFStory_EnsureSave();assert(save.rfStoryClues==0 && save.rfStoryContractClaimed==0);
    assert(memcmp(old,save.rfQuestStates,sizeof old)==0);
    assert(save.achievementsUnlocked[39]==0x3F && save.achievementsClaimed[39]==0x3F);
    assert(RFQuests_GetState(0)==RF_QUEST_CLAIMED && RFQuests_GetState(349)==RF_QUEST_ACTIVE);
    // A conversation cannot bypass the secondary objective or a locked scene.
    Reset();assert(RFQuests_Accept(350));At(sScenes[0].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(350)==RF_QUEST_CLAIMED);
    assert(RFQuests_Accept(351));At(sScenes[1].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(351)==RF_QUEST_ACTIVE);
    Proof(351);Script_RFStoryPrepare();assert(RFQuests_GetState(351)==RF_QUEST_CLAIMED);
    // Every authored campaign reaches its finale through the production scene handlers.
    Reset();
    for(u32 id=350;id<414;id++) {
        const struct RFStoryScene *s=&sScenes[id-350];
        assert(RFQuests_Accept(id));Proof(id);
        if(s->kind==RF_STORY_CLUES) {
            for(u32 a=0;a<55;a++) if(sActors[a].clueQuest==id && sActors[a].clueBit<32) {
                At(a);assert(RFStory_RecordClue(a));
                struct Save3 persisted=save;save=persisted;RFStory_EnsureSave();
                assert(save.rfStoryClues&(1u<<sActors[a].clueBit));
            }
        }
        if(s->kind==RF_STORY_RACE) {
            At(s->giver);Script_RFStoryPrepare();assert(gSpecialVar_0x8007==2);
            assert(sRaceQuest==0); // Reading instructions never starts the timer.
            gMain.vblankCounter1+=2000;Script_RFStoryAfterText();assert(sRaceQuest==id+1);
            gMain.vblankCounter1+=100;
        }
        if(s->kind==RF_STORY_DELIVERY) bag[sRFQuests[id].argument]=sRFQuests[id].alternative;
        At(s->target);Script_RFStoryPrepare();
        if(s->kind==RF_STORY_DELIVERY) {
            bag[sRFQuests[id].argument]=sRFQuests[id].alternative;
            u32 before=removed[sRFQuests[id].argument];Script_RFStoryConfirm();
            assert(removed[sRFQuests[id].argument]==before+sRFQuests[id].alternative);
        }
        if(s->kind==RF_STORY_PUZZLE) {
            assert(gSpecialVar_Result==3 && s->questions!=NULL && s->questionCount>0 && s->race==NULL);
            // A wrong answer restarts only the puzzle; the quest stays active.
            gSpecialVar_Result=!s->questions[0].answer;Script_RFStoryPuzzle();
            assert(RFQuests_GetState(id)==RF_QUEST_ACTIVE && sPuzzleStep==0);
            At(s->target);Script_RFStoryPrepare();assert(gSpecialVar_Result==3);
            for(u32 step=0;step<s->questionCount;step++) {
                assert(RFQuests_GetState(id)==RF_QUEST_ACTIVE);
                gSpecialVar_Result=s->questions[step].answer;Script_RFStoryPuzzle();
            }
        }
        else assert(s->questions==NULL && s->questionCount==0);
        assert((s->kind==RF_STORY_RACE)==(s->race!=NULL));
        assert(RFStory_GiverHint(id)==sActors[s->giver].hint);
        if(s->kind==RF_STORY_BATTLE) {
            assert(gSpecialVar_Result==4);
            gBattleOutcome=2;gBattleTypeFlags=BATTLE_TYPE_TRAINER;TRAINER_BATTLE_PARAM.opponentA=s->trainer;
            Script_RFStoryBattleWon();assert(RFQuests_GetState(id)==RF_QUEST_ACTIVE);
            gBattleOutcome=B_OUTCOME_WON;gBattleTypeFlags|=BATTLE_TYPE_LINK;
            Script_RFStoryBattleWon();assert(RFQuests_GetState(id)==RF_QUEST_ACTIVE);
            gBattleTypeFlags=BATTLE_TYPE_TRAINER;Script_RFStoryBattleWon();
        }
        assert(RFQuests_GetState(id)==RF_QUEST_CLAIMED);
        u32 reward=bag[sRFQuests[id].rewardItem];assert(!RFQuests_Claim(id));
        assert(bag[sRFQuests[id].rewardItem]==reward);
        u8 page[100];assert(RFStory_ReadEnding(id,0,page,sizeof page));
        assert(!RFStory_ReadEnding(id,99,page,sizeof page));
        assert(RFStory_MatchesFilter(id,RF_QUEST_FILTER_CLAIMED));
        assert(!RFStory_MatchesFilter(id,RF_QUEST_FILTER_ACTIVE));
        assert(RFStory_Difficulty(id)>=1 && RFStory_Difficulty(id)<=3);
    }
    assert(!RFStory_MatchesFilter(999,0));
    assert(RFStory_GiverHint(0)==NULL && RFStory_GiverHint(349)==NULL && RFStory_GiverHint(414)==NULL);
    // Puzzle scenes no longer share one hard-coded question set.
    {
        const struct RFStoryScene *p[2];u32 n=0;
        for(u32 i=0;i<RF_STORY_COUNT;i++) if(sScenes[i].kind==RF_STORY_PUZZLE) p[n++]=&sScenes[i];
        assert(n==2 && p[0]->questions!=p[1]->questions && p[0]->questionCount!=p[1]->questionCount);
    }
    assert((save.achievementsUnlocked[39]&0xC0)==0xC0);
    // A guide shared by independent campaigns cannot lock a report behind another plot.
    Reset();SetQuestState(402,RF_QUEST_CLAIMED);assert(RFQuests_Accept(403));
    SetQuestState(409,RF_QUEST_CLAIMED);assert(RFQuests_Accept(410));
    save.rfStoryClues=sScenes[60].clues;At(sScenes[60].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(410)==RF_QUEST_CLAIMED && RFQuests_GetState(403)==RF_QUEST_ACTIVE);
    // A full bag keeps a narrative reward ready, without granting it twice.
    Reset();assert(RFQuests_Accept(350));room=FALSE;At(sScenes[0].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(350)==RF_QUEST_READY);assert(!RFQuests_Claim(350));
    room=TRUE;assert(RFQuests_Claim(350));assert(!RFQuests_Claim(350));
    // Failed races and interrupted sessions can be restarted; they never fail the quest.
    Reset();SetQuestState(406,RF_QUEST_CLAIMED);assert(RFQuests_Accept(407));
    At(sScenes[57].target);Script_RFStoryPrepare();assert(RFQuests_GetState(407)==RF_QUEST_ACTIVE);
    // Arriving during a race takes priority over another pending reward at the timekeeper.
    At(sScenes[57].giver);Script_RFStoryPrepare();Script_RFStoryAfterText();
    SetQuestState(389,RF_QUEST_READY);At(sScenes[57].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(407)==RF_QUEST_CLAIMED && RFQuests_GetState(389)==RF_QUEST_READY);
    // Arriving in the last allowed frame still wins.
    Reset();SetQuestState(406,RF_QUEST_CLAIMED);assert(RFQuests_Accept(407));
    At(sScenes[57].giver);Script_RFStoryPrepare();Script_RFStoryAfterText();
    gMain.vblankCounter1+=sScenes[57].race->seconds*60;At(sScenes[57].target);Script_RFStoryPrepare();
    assert(RFQuests_GetState(407)==RF_QUEST_CLAIMED);
    Reset();SetQuestState(406,RF_QUEST_CLAIMED);assert(RFQuests_Accept(407));
    At(sScenes[57].giver);Script_RFStoryPrepare();Script_RFStoryAfterText();gMain.vblankCounter1+=sScenes[57].race->seconds*60+1;
    At(sScenes[57].target);Script_RFStoryPrepare();assert(RFQuests_GetState(407)==RF_QUEST_ACTIVE);
    // Contracts are separate from one-time mission count, persist pending deliveries,
    // renew forwards only, and never take the supplied item a second time.
    Reset();
    for(u32 c=0;c<3;c++) {
        Reset();
        const struct RFStoryContract *k=&sContracts[c];SetQuestState(k->unlockQuest,RF_QUEST_CLAIMED);
        bag[k->item]=9;room=FALSE;
        assert(RFStory_DeliverContract(c,10)==1);u32 used=removed[k->item];
        struct Save3 persisted=save;save=persisted;
        assert(RFStory_DeliverContract(c,11)==1 && removed[k->item]==used);
        room=TRUE;assert(RFStory_DeliverContract(c,11)==2);
        assert(RFStory_DeliverContract(c,11)==3 && removed[k->item]==used);
        assert(RFStory_DeliverContract(c,9)==3);
        assert(RFStory_DeliverContract(c,12)==2 && removed[k->item]==used+k->quantity);
    }
    return 0;
}
'''
        compiler=shutil.which('cc');self.assertIsNotNone(compiler)
        with tempfile.TemporaryDirectory() as d:
            c=Path(d)/'story.c';exe=Path(d)/'story'
            c.write_text(prefix+'\n'+source+'\n'+harness,encoding='utf-8')
            r=subprocess.run([compiler,'-std=gnu11','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fsanitize=address,undefined','-g',str(c),'-o',str(exe)],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stderr[-9000:])
            r=subprocess.run([str(exe)],capture_output=True,text=True)
            self.assertEqual(r.returncode,0,r.stdout+r.stderr)


if __name__=='__main__': unittest.main()
