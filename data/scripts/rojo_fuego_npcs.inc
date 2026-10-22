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

@ Pokédex rewards: Oak's third aide in his lab hands out a prize at each milestone
RF_EventScript_DexRewards::
	lock
	faceplayer
	goto_if_unset FLAG_SYS_POKEDEX_GET, RF_EventScript_DexRewards_NoDex
	callnative Script_GetNextDexReward
	goto_if_eq VAR_RESULT, TRUE, RF_EventScript_DexRewards_Give
	goto_if_eq VAR_0x8005, 0, RF_EventScript_DexRewards_AllGiven
	msgbox RF_Text_DexRewards_Next
	release
	end

RF_EventScript_DexRewards_Give::
	msgbox RF_Text_DexRewards_Give
	giveitem VAR_0x8005, VAR_0x8006
	release
	end

RF_EventScript_DexRewards_AllGiven::
	msgbox RF_Text_DexRewards_AllGiven
	release
	end

RF_EventScript_DexRewards_NoDex::
	msgbox PalletTown_ProfessorOaksLab_Text_OakIsAuthorityOnMons
	release
	end

RF_Text_DexRewards_Next:
	.string "Llevas {STR_VAR_1} especies capturadas.\p"
	.string "Cuando llegues a {STR_VAR_2}, el\n"
	.string "PROF. OAK me pidió darte un premio.$"

RF_Text_DexRewards_Give:
	.string "¡Llevas {STR_VAR_1} especies capturadas!\n"
	.string "El PROF. OAK quiere que tengas esto.$"

RF_Text_DexRewards_AllGiven:
	.string "¡Ya te di todos los premios! El\n"
	.string "PROF. OAK está muy orgulloso.$"

@ Wonder Trade: the Poké Maniac on Two Island swaps one party Pokémon a day for a random one
RF_EventScript_WonderTrade::
	lock
	faceplayer
	goto_if_set FLAG_RF_DAILY_WONDER_TRADE, RF_EventScript_WonderTrade_Tomorrow
	msgbox RF_Text_WonderTrade_Intro, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_WonderTrade_Bye
	msgbox RF_Text_WonderTrade_WhichMon
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, RF_EventScript_WonderTrade_Bye
	callnative Script_DoWonderTrade
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_WonderTrade_Egg
	setflag FLAG_RF_DAILY_WONDER_TRADE
	playfanfare MUS_OBTAIN_ITEM
	msgbox RF_Text_WonderTrade_Received
	waitfanfare
	release
	end

RF_EventScript_WonderTrade_Tomorrow::
	msgbox RF_Text_WonderTrade_Tomorrow
	release
	end

RF_EventScript_WonderTrade_Egg::
	msgbox RF_Text_WonderTrade_Egg
	release
	end

RF_EventScript_WonderTrade_Bye::
	msgbox RF_Text_WonderTrade_Bye
	release
	end

RF_Text_WonderTrade_Intro:
	.string "¡Hago INTERCAMBIO PRODIGIO!\p"
	.string "Me das un Pokémon y te doy otro\n"
	.string "al azar, de cualquier región.\l"
	.string "Uno al día. ¿Te animas?$"

RF_Text_WonderTrade_WhichMon:
	.string "¿Qué Pokémon cambias?$"

RF_Text_WonderTrade_Received:
	.string "¡Recibiste un {STR_VAR_1}!\n"
	.string "¡Cuídalo mucho!$"

RF_Text_WonderTrade_Tomorrow:
	.string "Por hoy ya está. ¡Vuelve mañana\n"
	.string "para otro INTERCAMBIO PRODIGIO!$"

RF_Text_WonderTrade_Egg:
	.string "Los HUEVOS no se pueden cambiar.$"

RF_Text_WonderTrade_Bye:
	.string "¡Cuando quieras!$"

@ Gym Leader rematches: once the player is Champion, each Kanto Gym Leader (except Giovanni, who
@ leaves his Gym) offers a rematch with a stronger team. Before that, the Leader's own script runs.
	.macro rf_leader_rematch leader:req, trainer:req, original:req
	.global RF_EventScript_Rematch_\leader
RF_EventScript_Rematch_\leader\():
	goto_if_unset FLAG_IS_CHAMPION, \original
	lock
	faceplayer
	msgbox RF_Text_Rematch_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_Rematch_Decline
	cleartrainerflag \trainer
	trainerbattle_single \trainer, RF_Text_Rematch_Intro, RF_Text_Rematch_Defeat, RF_EventScript_Rematch_Won
	release
	end
	.endm

	rf_leader_rematch Brock, TRAINER_RF_REMATCH_BROCK, PewterCity_Gym_EventScript_Brock
	rf_leader_rematch Misty, TRAINER_RF_REMATCH_MISTY, CeruleanCity_Gym_EventScript_Misty
	rf_leader_rematch LtSurge, TRAINER_RF_REMATCH_LT_SURGE, VermilionCity_Gym_EventScript_LtSurge
	rf_leader_rematch Erika, TRAINER_RF_REMATCH_ERIKA, CeladonCity_Gym_EventScript_Erika
	rf_leader_rematch Koga, TRAINER_RF_REMATCH_KOGA, FuchsiaCity_Gym_EventScript_Koga
	rf_leader_rematch Sabrina, TRAINER_RF_REMATCH_SABRINA, SaffronCity_Gym_EventScript_Sabrina
	rf_leader_rematch Blaine, TRAINER_RF_REMATCH_BLAINE, CinnabarIsland_Gym_EventScript_Blaine

RF_EventScript_Rematch_Won::
	msgbox RF_Text_Rematch_Won
	release
	end

RF_EventScript_Rematch_Decline::
	msgbox RF_Text_Rematch_Decline
	release
	end

RF_Text_Rematch_Offer:
	.string "¡El CAMPEÓN de KANTO en persona!\p"
	.string "He entrenado con Pokémon de todas\n"
	.string "las regiones. ¿Aceptas una revancha?$"

RF_Text_Rematch_Intro:
	.string "¡Esta vez no me contendré!$"

RF_Text_Rematch_Defeat:
	.string "Increíble… Sigues siendo\n"
	.string "el más fuerte.$"

RF_Text_Rematch_Won:
	.string "Vuelve cuando quieras. ¡La próxima\n"
	.string "vez estaré todavía más preparado!$"

RF_Text_Rematch_Decline:
	.string "Aquí te espero.$"
