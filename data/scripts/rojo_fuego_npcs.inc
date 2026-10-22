@ Rojo Fuego+ NPC scripts written by hand (FRLG only, see data/event_scripts.s)

@ Hyper Training: the Sailor on Celadon Department Store 5F maxes out IVs for Bottle Caps
RF_EventScript_HyperTrainer::
	lock
	faceplayer
	msgbox RF_Text_HyperTrainer_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_HyperTrainer_Bye
	checkitem ITEM_BOTTLE_CAP
	goto_if_eq VAR_RESULT, TRUE, RF_EventScript_HyperTrainer_ChooseMon
	checkitem ITEM_GOLD_BOTTLE_CAP
	goto_if_eq VAR_RESULT, TRUE, RF_EventScript_HyperTrainer_ChooseMon
	msgbox RF_Text_HyperTrainer_NoCaps
	release
	end

RF_EventScript_HyperTrainer_ChooseMon::
	msgbox RF_Text_HyperTrainer_WhichMon
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, RF_EventScript_HyperTrainer_Bye
	message RF_Text_HyperTrainer_WhichStat
	waitmessage
	dynmultichoice 17, 1, FALSE, 7, 0, DYN_MULTICHOICE_CB_NONE, RF_Text_Stat_HP, RF_Text_Stat_Atk, RF_Text_Stat_Def, RF_Text_Stat_Speed, RF_Text_Stat_SpAtk, RF_Text_Stat_SpDef, RF_Text_Stat_All
	closemessage
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, RF_EventScript_HyperTrainer_Bye
	copyvar VAR_0x8005, VAR_RESULT
	goto_if_eq VAR_0x8005, 6, RF_EventScript_HyperTrainer_All  @ 6 = NUM_STATS, "Todas"
	checkitem ITEM_BOTTLE_CAP
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_HyperTrainer_NeedSilver
	callnative Script_HyperTrainChosenMon
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_HyperTrainer_Nothing
	removeitem ITEM_BOTTLE_CAP
	goto RF_EventScript_HyperTrainer_Done

RF_EventScript_HyperTrainer_All::
	checkitem ITEM_GOLD_BOTTLE_CAP
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_HyperTrainer_NeedGold
	callnative Script_HyperTrainChosenMon
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_HyperTrainer_Nothing
	removeitem ITEM_GOLD_BOTTLE_CAP
	goto RF_EventScript_HyperTrainer_Done

RF_EventScript_HyperTrainer_Done::
	playfanfare MUS_LEVEL_UP
	msgbox RF_Text_HyperTrainer_Done
	waitfanfare
	release
	end

RF_EventScript_HyperTrainer_Nothing::
	msgbox RF_Text_HyperTrainer_Nothing
	release
	end

RF_EventScript_HyperTrainer_NeedSilver::
	msgbox RF_Text_HyperTrainer_NeedSilver
	release
	end

RF_EventScript_HyperTrainer_NeedGold::
	msgbox RF_Text_HyperTrainer_NeedGold
	release
	end

RF_EventScript_HyperTrainer_Bye::
	msgbox RF_Text_HyperTrainer_Bye
	release
	end

RF_Text_HyperTrainer_Intro:
	.string "¡Hago ENTRENAMIENTO EXTREMO!\p"
	.string "Por una CHAPA PLATEADA llevo un\n"
	.string "IV al máximo; por una DORADA, todos.\p"
	.string "¿Quieres entrenar a un Pokémon?$"

RF_Text_HyperTrainer_NoCaps:
	.string "No tienes ninguna CHAPA. Se venden\n"
	.string "aquí mismo, junto a las vitaminas.$"

RF_Text_HyperTrainer_WhichMon:
	.string "¿Qué Pokémon entreno?$"

RF_Text_HyperTrainer_WhichStat:
	.string "¿Qué estadística mejoro?$"

RF_Text_HyperTrainer_Done:
	.string "¡Listo! Tu Pokémon ha superado\n"
	.string "sus límites.$"

RF_Text_HyperTrainer_Nothing:
	.string "Eso ya está al máximo.\n"
	.string "No te cobro nada.$"

RF_Text_HyperTrainer_NeedSilver:
	.string "Para eso necesito una\n"
	.string "CHAPA PLATEADA.$"

RF_Text_HyperTrainer_NeedGold:
	.string "Para entrenar todo a la vez\n"
	.string "necesito una CHAPA DORADA.$"

RF_Text_HyperTrainer_Bye:
	.string "¡Vuelve cuando quieras!$"

RF_Text_Stat_HP:
	.string "PS$"
RF_Text_Stat_Atk:
	.string "Ataque$"
RF_Text_Stat_Def:
	.string "Defensa$"
RF_Text_Stat_Speed:
	.string "Velocidad$"
RF_Text_Stat_SpAtk:
	.string "At. Esp.$"
RF_Text_Stat_SpDef:
	.string "Def. Esp.$"
RF_Text_Stat_All:
	.string "Todas$"
