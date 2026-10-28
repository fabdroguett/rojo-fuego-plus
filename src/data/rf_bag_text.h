// Bag-only Spanish display text; item IDs, effects and the source catalog stay intact.
static const struct RFBagText
{
    enum Item item;
    const u8 *name;
    const u8 *description;
} sRFBagText[] = {
    {ITEM_POTION, COMPOUND_STRING("Poción"), COMPOUND_STRING("Restaura 20 PS\nde un Pokémon.")},
#if I_HEALTH_RECOVERY >= GEN_7
    {ITEM_SUPER_POTION, COMPOUND_STRING("Superpoción"), COMPOUND_STRING("Restaura 60 PS\nde un Pokémon.")},
    {ITEM_HYPER_POTION, COMPOUND_STRING("Hiperpoción"), COMPOUND_STRING("Restaura 120 PS\nde un Pokémon.")},
#else
    {ITEM_SUPER_POTION, COMPOUND_STRING("Superpoción"), COMPOUND_STRING("Restaura 50 PS\nde un Pokémon.")},
    {ITEM_HYPER_POTION, COMPOUND_STRING("Hiperpoción"), COMPOUND_STRING("Restaura 200 PS\nde un Pokémon.")},
#endif
    {ITEM_MAX_POTION, COMPOUND_STRING("Poción Máxima"), COMPOUND_STRING("Restaura todos\nlos PS de un\nPokémon.")},
    {ITEM_FULL_RESTORE, COMPOUND_STRING("Restaurar Todo"), COMPOUND_STRING("Restaura los PS\ny cura problemas\nde estado.")},
    {ITEM_REVIVE, COMPOUND_STRING("Revivir"), COMPOUND_STRING("Reanima con la\nmitad de sus PS a\nun Pokémon.")},
    {ITEM_MAX_REVIVE, COMPOUND_STRING("Revivir Máximo"), COMPOUND_STRING("Reanima con todos\nsus PS a un\nPokémon.")},
    {ITEM_ANTIDOTE, COMPOUND_STRING("Antídoto"), COMPOUND_STRING("Cura a un Pokémon\nenvenenado.")},
    {ITEM_PARALYZE_HEAL, COMPOUND_STRING("Antiparalizador"), COMPOUND_STRING("Cura a un Pokémon\nparalizado.")},
    {ITEM_AWAKENING, COMPOUND_STRING("Despertar"), COMPOUND_STRING("Despierta a un\nPokémon dormido.")},
    {ITEM_BURN_HEAL, COMPOUND_STRING("Antiquemar"), COMPOUND_STRING("Cura las quemaduras\nde un Pokémon.")},
    {ITEM_ICE_HEAL, COMPOUND_STRING("Antihielo"), COMPOUND_STRING("Descongela a un\nPokémon.")},
    {ITEM_FULL_HEAL, COMPOUND_STRING("Cura Total"), COMPOUND_STRING("Cura problemas de\nestado de un\nPokémon.")},
    {ITEM_RARE_CANDY, COMPOUND_STRING("Caramelo Raro"), COMPOUND_STRING("Sube un nivel\na un Pokémon.")},
    {ITEM_X_ATTACK, COMPOUND_STRING("Ataque X"), COMPOUND_STRING("Aumenta el Ataque\ndurante un\ncombate.")},
    {ITEM_FIRE_STONE, COMPOUND_STRING("Piedra Fuego"), COMPOUND_STRING("Hace evolucionar\na ciertas especies\nde Pokémon.")},
    {ITEM_LEFTOVERS, COMPOUND_STRING("Restos"), COMPOUND_STRING("Restaura PS poco\na poco al llevarlo\nequipado.")},
    {ITEM_NUGGET, COMPOUND_STRING("Pepita"), COMPOUND_STRING("Pepita de oro que\nse vende a un\nprecio elevado.")},
    {ITEM_ESCAPE_ROPE, COMPOUND_STRING("Cuerda Huida"), COMPOUND_STRING("Permite salir de\ncuevas y\nmazmorras.")},
    {ITEM_POKE_BALL, COMPOUND_STRING("Poké Ball"), COMPOUND_STRING("Sirve para atrapar\nPokémon salvajes.")},
    {ITEM_GREAT_BALL, COMPOUND_STRING("Super Ball"), COMPOUND_STRING("Mayor eficacia que\nuna Poké Ball para\natrapar Pokémon.")},
    {ITEM_ULTRA_BALL, COMPOUND_STRING("Ultra Ball"), COMPOUND_STRING("Mayor eficacia que\nuna Super Ball para\natrapar Pokémon.")},
    {ITEM_ORAN_BERRY, COMPOUND_STRING("Baya Aranja"), COMPOUND_STRING("Restaura 10 PS de\nun Pokémon.")},
    {ITEM_TOWN_MAP, COMPOUND_STRING("Mapa"), COMPOUND_STRING("Permite consultar\nel mapa de\nla región.")},
    {ITEM_BICYCLE, COMPOUND_STRING("Bicicleta"), COMPOUND_STRING("Permite viajar\nmás rápido por\nla región.")},
    {ITEM_ABILITY_CAPSULE, COMPOUND_STRING("Cápsula Habilidad"), COMPOUND_STRING("Permite cambiar\nentre habilidades\nnormales.")},
    {ITEM_STRAWBERRY_SWEET, COMPOUND_STRING("Confite Fresa"), COMPOUND_STRING("Dulce de fresa\npara que lo lleve\nun Milcery.")},
    {ITEM_GALARICA_WREATH, COMPOUND_STRING("Corona Galanuez"), COMPOUND_STRING("Corona trenzada\npara un Slowpoke\nde Galar.")},
    {ITEM_PEWTER_CRUNCHIES, COMPOUND_STRING("Dulce de Plateada"), COMPOUND_STRING("Dulce de Plateada.\nCura problemas\nde estado.")},
    {ITEM_ADAMANT_CRYSTAL, COMPOUND_STRING("Gran Diamansfera"), COMPOUND_STRING("Una esfera que\ncontiene el poder\nde Dialga.")},
};
