@ Rojo Fuego+ NPC scripts written by hand (FRLG only, see data/event_scripts.s)

@ Hyper Training: the Sailor on Celadon Department Store 5F maxes out IVs for Bottle Caps
RF_EventScript_HyperTrainer::
	lock
	faceplayer
	dynmultichoice 14, 1, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, RF_Text_Training_Caps, RF_Text_Training_EV, RF_Text_Training_IV, RF_Text_Training_Exit
	goto_if_eq VAR_RESULT, 1, RF_EventScript_TrainingEV
	goto_if_eq VAR_RESULT, 2, RF_EventScript_TrainingIV
	goto_if_ne VAR_RESULT, 0, RF_EventScript_HyperTrainer_Bye
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

RF_EventScript_TrainingEV:
	setvar VAR_0x8006, 0
	goto RF_EventScript_TrainingChooseMon
RF_EventScript_TrainingIV:
	setvar VAR_0x8006, 1
RF_EventScript_TrainingChooseMon:
	msgbox RF_Text_HyperTrainer_WhichMon
	special ChoosePartyMon
	goto_if_ge VAR_0x8004, PARTY_SIZE, RF_EventScript_HyperTrainer_Bye
	callnative Script_RF_TrainingCanEdit
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_TrainingInvalid
RF_EventScript_TrainingChooseStat:
	message RF_Text_HyperTrainer_WhichStat
	waitmessage
	dynmultichoice 17, 1, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, RF_Text_Stat_HP, RF_Text_Stat_Atk, RF_Text_Stat_Def, RF_Text_Stat_Speed, RF_Text_Stat_SpAtk, RF_Text_Stat_SpDef
	closemessage
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, RF_EventScript_HyperTrainer_Bye
	copyvar VAR_0x8005, VAR_RESULT
RF_EventScript_TrainingRead:
	callnative Script_RF_TrainingRead
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_TrainingInvalid
	buffernumberstring STR_VAR_1, VAR_0x8007
	buffernumberstring STR_VAR_2, VAR_0x8008
	buffernumberstring STR_VAR_3, VAR_0x800B
	call_if_eq VAR_0x8006, 0, RF_EventScript_TrainingShowEV
	call_if_eq VAR_0x8006, 1, RF_EventScript_TrainingShowIV
	closemessage
	dynmultichoice 14, 1, FALSE, 7, 0, DYN_MULTICHOICE_CB_NONE, RF_Text_Training_Max, RF_Text_Training_Zero, RF_Text_Training_PlusOne, RF_Text_Training_MinusOne, RF_Text_Training_PlusTen, RF_Text_Training_MinusTen, RF_Text_Training_OtherStat
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, RF_EventScript_HyperTrainer_Bye
	goto_if_eq VAR_RESULT, 6, RF_EventScript_TrainingChooseStat
	copyvar VAR_0x800A, VAR_RESULT
	callnative Script_RF_TrainingPreview
	goto_if_eq VAR_RESULT, FALSE, RF_EventScript_TrainingInvalid
	buffernumberstring STR_VAR_1, VAR_0x8007
	msgbox RF_Text_Training_Confirm, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_TrainingRead
	callnative Script_RF_TrainingEdit
	call_if_eq VAR_RESULT, FALSE, RF_EventScript_TrainingNoChange
	goto RF_EventScript_TrainingRead
RF_EventScript_TrainingShowEV:
	msgbox RF_Text_Training_EVValue
	return
RF_EventScript_TrainingShowIV:
	msgbox RF_Text_Training_IVValue
	return
RF_EventScript_TrainingNoChange:
	msgbox RF_Text_Training_NoChange
	return
RF_EventScript_TrainingInvalid:
	msgbox RF_Text_Training_Invalid
	release
	end

RF_Text_Training_Caps:
	.string "Usar chapas$"
RF_Text_Training_EV:
	.string "Editar EV$"
RF_Text_Training_IV:
	.string "Editar IV reales$"
RF_Text_Training_Exit:
	.string "Salir$"
RF_Text_Training_Max:
	.string "Máximo permitido$"
RF_Text_Training_Zero:
	.string "Dejar en cero$"
RF_Text_Training_PlusOne:
	.string "Sumar 1$"
RF_Text_Training_MinusOne:
	.string "Restar 1$"
RF_Text_Training_PlusTen:
	.string "Sumar 10$"
RF_Text_Training_MinusTen:
	.string "Restar 10$"
RF_Text_Training_OtherStat:
	.string "Otra estadística$"
RF_Text_Training_EVValue:
	.string "EV actual: {STR_VAR_1}. Máximo: {STR_VAR_2}.\n"
	.string "Total: {STR_VAR_3}/510. Servicio gratuito.$"
RF_Text_Training_IVValue:
	.string "IV real: {STR_VAR_1}. Máximo: {STR_VAR_2}.\n"
	.string "Este editor cambia el IV real.\p"
	.string "Al aplicarlo, quita el entrenamiento\n"
	.string "extremo de esa estadística.$"
RF_Text_Training_Confirm:
	.string "¿Dejar esta estadística en {STR_VAR_1}?\n"
	.string "No se consumen objetos.$"
RF_Text_Training_NoChange:
	.string "No hay cambios posibles.\n"
	.string "Revisa el valor y el límite de EV.$"
RF_Text_Training_Invalid:
	.string "No puedo editar este Pokémon.\n"
	.string "Los HUEVOS no pueden entrenar aquí.$"

@ Each gym statue keeps an unlimited, free thematic tutor after its badge is earned.
@ Badge flags already persist; no new save fields or once-only reward flags are needed.
	.macro rf_gym_tutor index:req, badge:req, move:req
	.global RF_EventScript_GymTutor_\index
RF_EventScript_GymTutor_\index\():
	goto_if_unset \badge, RF_EventScript_GymTutor_Return
	buffermovename STR_VAR_1, \move
	move_tutor \move, RF_Text_GymTutor_Offer, RF_Text_GymTutor_WhichMon, RF_EventScript_GymTutor_Return, RF_EventScript_GymTutor_Taught
	return
	.endm

	rf_gym_tutor 0, FLAG_BADGE01_GET, MOVE_ROCK_SLIDE
	rf_gym_tutor 1, FLAG_BADGE02_GET, MOVE_WATER_PULSE
	rf_gym_tutor 2, FLAG_BADGE03_GET, MOVE_THUNDER_PUNCH
	rf_gym_tutor 3, FLAG_BADGE04_GET, MOVE_SEED_BOMB
	rf_gym_tutor 4, FLAG_BADGE05_GET, MOVE_GUNK_SHOT
	rf_gym_tutor 5, FLAG_BADGE06_GET, MOVE_ZEN_HEADBUTT
	rf_gym_tutor 6, FLAG_BADGE07_GET, MOVE_HEAT_WAVE
	rf_gym_tutor 7, FLAG_BADGE08_GET, MOVE_EARTH_POWER

RF_EventScript_GymTutor_Taught:
	msgbox RF_Text_GymTutor_Taught
RF_EventScript_GymTutor_Return:
	return

RF_Text_GymTutor_Offer:
	.string "Tu medalla abre este entrenamiento.\n"
	.string "Puedo enseñar {STR_VAR_1}.\p"
	.string "Es gratis y puedes volver siempre.\n"
	.string "¿Quieres elegir un Pokémon?$"
RF_Text_GymTutor_WhichMon:
	.string "Elige un Pokémon compatible\n"
	.string "del equipo o del PC.$"
RF_Text_GymTutor_Taught:
	.string "¡Entrenamiento completado!\n"
	.string "Vuelve cuando quieras.$"

@ Pokédex rewards: Oak's third aide in his lab hands out a prize at each milestone
RF_EventScript_QuestDesk::
	setvar VAR_0x8004, RF_QUEST_EVENT_OAK_REPORT
	setvar VAR_0x8005, 0
	callnative Script_RF_QuestNotify
	call_if_ne VAR_RESULT, 0, RF_EventScript_QuestDesk_Report
	setvar VAR_0x8004, RF_QUEST_FIELD_SUPPLIES
	callnative Script_RF_QuestGetState
	goto_if_ne VAR_RESULT, 1, RF_EventScript_QuestDesk_Return
	msgbox RF_Text_QuestDesk_Delivery, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_QuestDesk_Return
	setvar VAR_0x8004, RF_QUEST_FIELD_SUPPLIES
	callnative Script_RF_QuestDeliver
	goto_if_eq VAR_RESULT, 1, RF_EventScript_QuestDesk_Delivered
	goto_if_eq VAR_RESULT, 2, RF_EventScript_QuestDesk_Missing
RF_EventScript_QuestDesk_Return:
	return

RF_EventScript_QuestDesk_Report:
	msgbox RF_Text_QuestDesk_Report
	return
RF_EventScript_QuestDesk_Delivered:
	msgbox RF_Text_QuestDesk_Delivered
	return
RF_EventScript_QuestDesk_Missing:
	msgbox RF_Text_QuestDesk_Missing
	return

RF_Text_QuestDesk_Report:
	.string "¡He anotado tu informe!\n"
	.string "Reclama el premio en el diario.$"
RF_Text_QuestDesk_Delivery:
	.string "Prepararemos botiquines de ruta.\n"
	.string "¿Me entregas 2 POCIONES?\p"
	.string "Se retirarán de tu mochila.$"
RF_Text_QuestDesk_Delivered:
	.string "¡Gracias por las 2 POCIONES!\n"
	.string "Tu premio te espera en el diario.$"
RF_Text_QuestDesk_Missing:
	.string "Necesitas 2 POCIONES en la mochila.\n"
	.string "Vuelve cuando las tengas.$"

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

@ Title Defense (from Pokémon SoulGold): once the player is Champion, a League run sometimes ends
@ with a challenger who wants the title. Half the time one of the Kanto Gym Leaders shows up with
@ their rematch team, right after the Champion battle.
	.macro rf_title_challenger leader:req, trainer:req, name:req
RF_EventScript_TitleDefense_\leader\():
	bufferstring STR_VAR_1, RF_Text_TitleDefense_Name_\leader
	msgbox RF_Text_TitleDefense_Arrives
	closemessage
	cleartrainerflag \trainer
	trainerbattle_no_intro \trainer, RF_Text_TitleDefense_Defeat
	cleartrainerflag \trainer
	goto RF_EventScript_TitleDefense_Won
RF_Text_TitleDefense_Name_\leader\():
	.string "\name$"
	.endm

	.global RF_EventScript_TitleDefense
RF_EventScript_TitleDefense::
	random 2
	goto_if_ne VAR_RESULT, 0, RF_EventScript_TitleDefense_None
	random 7
	goto_if_eq VAR_RESULT, 0, RF_EventScript_TitleDefense_Brock
	goto_if_eq VAR_RESULT, 1, RF_EventScript_TitleDefense_Misty
	goto_if_eq VAR_RESULT, 2, RF_EventScript_TitleDefense_LtSurge
	goto_if_eq VAR_RESULT, 3, RF_EventScript_TitleDefense_Erika
	goto_if_eq VAR_RESULT, 4, RF_EventScript_TitleDefense_Koga
	goto_if_eq VAR_RESULT, 5, RF_EventScript_TitleDefense_Sabrina
	goto RF_EventScript_TitleDefense_Blaine

RF_EventScript_TitleDefense_None::
	return

	rf_title_challenger Brock, TRAINER_RF_REMATCH_BROCK, "BROCK"
	rf_title_challenger Misty, TRAINER_RF_REMATCH_MISTY, "MISTY"
	rf_title_challenger LtSurge, TRAINER_RF_REMATCH_LT_SURGE, "LT. SURGE"
	rf_title_challenger Erika, TRAINER_RF_REMATCH_ERIKA, "ERIKA"
	rf_title_challenger Koga, TRAINER_RF_REMATCH_KOGA, "KOGA"
	rf_title_challenger Sabrina, TRAINER_RF_REMATCH_SABRINA, "SABRINA"
	rf_title_challenger Blaine, TRAINER_RF_REMATCH_BLAINE, "BLAINE"

RF_EventScript_TitleDefense_Won::
	msgbox RF_Text_TitleDefense_Won
	giveitem ITEM_BOTTLE_CAP
	return

RF_Text_TitleDefense_Arrives:
	.string "¡Alguien entra corriendo en la sala!\p"
	.string "{STR_VAR_1}: ¡Has vencido a la Liga,\n"
	.string "pero el título de Campeón será mío!\p"
	.string "¡Defiende tu título!$"

RF_Text_TitleDefense_Defeat:
	.string "Sigues siendo el Campeón…\n"
	.string "¡Volveré más fuerte!$"

RF_Text_TitleDefense_Won:
	.string "¡Has defendido tu título de Campeón!\n"
	.string "El retador te deja un recuerdo.$"

@ Traveling challenger (daily challenges, from Crystal Clear): Noa waits in a different Pokémon
@ Center every day (src/rf_traveler.c, tools/rojo_fuego/traveler.py) and battles once a day under
@ that day's rules. Her team and the prize follow the player's progress.
	.macro rf_traveler_battle tier:req, single:req, double:req
RF_EventScript_Traveler_Single\tier\():
	cleartrainerflag \single
	trainerbattle_no_intro \single, RF_Text_Traveler_Defeat
	cleartrainerflag \single
	goto RF_EventScript_Traveler_Won
RF_EventScript_Traveler_Double\tier\():
	cleartrainerflag \double
	trainerbattle_no_intro \double, RF_Text_Traveler_Defeat
	cleartrainerflag \double
	goto RF_EventScript_Traveler_Won
	.endm

	.global RF_EventScript_Traveler
RF_EventScript_Traveler::
	lock
	faceplayer
	callnative Script_RF_TravelerSetup
	callnative Script_RF_TravelerClaimPrize
	goto_if_eq VAR_RESULT, 1, RF_EventScript_Traveler_PrizeReceived
	goto_if_eq VAR_RESULT, 2, RF_EventScript_Traveler_PrizeFull
	goto_if_set FLAG_RF_DAILY_TRAVELER, RF_EventScript_Traveler_Done
	msgbox RF_Text_Traveler_Intro
	msgbox RF_Text_Traveler_Rules
	call_if_eq VAR_0x8007, TRUE, RF_EventScript_Traveler_ExplainInverse
	msgbox RF_Text_Traveler_Offer, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, RF_EventScript_Traveler_Decline
	goto_if_eq VAR_0x8006, FALSE, RF_EventScript_Traveler_NotEnough
	msgbox RF_Text_Traveler_Start
	closemessage
	callnative Script_RF_TravelerApplyRules
	goto_if_eq VAR_0x8005, TRUE, RF_EventScript_Traveler_Double
	switch VAR_0x8004
	case 0, RF_EventScript_Traveler_Single0
	case 1, RF_EventScript_Traveler_Single1
	case 2, RF_EventScript_Traveler_Single2
	goto RF_EventScript_Traveler_Single3

RF_EventScript_Traveler_Double:
	switch VAR_0x8004
	case 0, RF_EventScript_Traveler_Double0
	case 1, RF_EventScript_Traveler_Double1
	case 2, RF_EventScript_Traveler_Double2
	goto RF_EventScript_Traveler_Double3

	rf_traveler_battle 0, TRAINER_RF_TRAVELER_0, TRAINER_RF_TRAVELER_DOUBLE_0
	rf_traveler_battle 1, TRAINER_RF_TRAVELER_1, TRAINER_RF_TRAVELER_DOUBLE_1
	rf_traveler_battle 2, TRAINER_RF_TRAVELER_2, TRAINER_RF_TRAVELER_DOUBLE_2
	rf_traveler_battle 3, TRAINER_RF_TRAVELER_3, TRAINER_RF_TRAVELER_DOUBLE_3

RF_EventScript_Traveler_Won:
	callnative Script_RF_TravelerClearRules
	callnative Script_RF_TravelerRecordWin
	msgbox RF_Text_Traveler_Won
	callnative Script_RF_TravelerClaimPrize
	goto_if_eq VAR_RESULT, 2, RF_EventScript_Traveler_PrizeFull
	goto_if_ne VAR_RESULT, 1, RF_EventScript_Traveler_Bye
RF_EventScript_Traveler_PrizeReceived:
	bufferitemname STR_VAR_1, VAR_0x8000
	buffernumberstring STR_VAR_3, VAR_0x8001
	playfanfare MUS_OBTAIN_ITEM
	msgbox RF_Text_Traveler_PrizeReceived
	waitfanfare
RF_EventScript_Traveler_Bye:
	msgbox RF_Text_Traveler_Tomorrow
	release
	end

RF_EventScript_Traveler_PrizeFull:
	msgbox RF_Text_Traveler_PrizeFull
	release
	end

RF_Text_Traveler_PrizeReceived:
	.string "¡Recibiste {STR_VAR_1} x{STR_VAR_3}!$"

RF_Text_Traveler_PrizeFull:
	.string "No cabe el premio en tu mochila.\n"
	.string "Te lo guardaré hasta que vuelvas.\p"
	.string "Haz espacio y habla conmigo\n"
	.string "en la ciudad donde esté ese día.$"

RF_EventScript_Traveler_ExplainInverse:
	msgbox RF_Text_Traveler_Inverse
	return

RF_EventScript_Traveler_Done:
	msgbox RF_Text_Traveler_Tomorrow
	release
	end

RF_EventScript_Traveler_Decline:
	msgbox RF_Text_Traveler_Decline
	release
	end

RF_EventScript_Traveler_NotEnough:
	msgbox RF_Text_Traveler_NotEnough
	release
	end

RF_Text_Traveler_Intro:
	.string "Soy NOA. Viajo por Kanto y cada día\n"
	.string "reto a alguien en un Centro distinto.$"

RF_Text_Traveler_Rules:
	.string "Las reglas de hoy son:\n"
	.string "{STR_VAR_1}.$"

RF_Text_Traveler_Inverse:
	.string "En la inversa, los tipos van al revés:\n"
	.string "lo muy eficaz pasa a ser poco eficaz.\p"
	.string "Y los ataques sin efecto ahora\n"
	.string "son muy eficaces.$"

RF_Text_Traveler_Offer:
	.string "¿Aceptas el desafío?$"

RF_Text_Traveler_Start:
	.string "¡Muy bien! A ver cómo te adaptas.$"

RF_Text_Traveler_Defeat:
	.string "¡Te adaptaste a la perfección!$"

RF_Text_Traveler_Won:
	.string "Gran combate. Toma, te lo has ganado.$"

RF_Text_Traveler_Tomorrow:
	.string "Mañana estaré en el Centro Pokémon\n"
	.string "de {STR_VAR_2}. ¡Búscame allí!$"

RF_Text_Traveler_Decline:
	.string "Vaya… Estaré aquí hasta mañana\n"
	.string "por si cambias de opinión.$"

RF_Text_Traveler_NotEnough:
	.string "Para un combate doble necesitas\n"
	.string "dos Pokémon que puedan luchar.$"
