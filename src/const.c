/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 * Original SMAUG 1.4a written by Thoric (Derek Snider) with Altrag,        *
 * Blodkai, Haus, Narn, Scryn, Swordbearer, Tricops, Gorog, Rennard,        *
 * Grishnakh, Fireblade, and Nivek.                                         *
 *                                                                          *
 * Original MERC 2.1 code by Hatchet, Furey, and Kahn.                      *
 *                                                                          *
 * Original DikuMUD code by: Hans Staerfeldt, Katja Nyboe, Tom Madsen,      *
 * Michael Seifert, and Sebastian Hammer.                                   *
 * ------------------------------------------------------------------------ *
 *			     Mud constants module			    *
 ****************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "./Headers/mud.h"

char *const npc_race[MAX_NPC_RACE] = {
   "badger", "crane", "dragon", "lion", "naga", "phoenix",
   "kappa", "kitsune", "otoroshi", "jubokko", "baku", "tengu", "nue", "hainu", "tanuki",
   "basan", "ashalan", "fudoshi", "gaki", "henemuri", "ikiryo", "kenku", "kumo", "mokumokuren",
   "naga", "ningyo", "nue", "nukarumi", "obake", "orochi", "podling", "shiyokai", "takesasu",
   "tanuki", "ubume", "uragiri", "yokai", "yorei", "zokujin", "shiryo", "merido", "sakkaku",
   "scorpion", "unicorn", "r3", "r4", "r5", "r6", "r7", "r8", "r9",
   "s1", "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",
   "ghoul", "shadow", "skeleton", "worm", "spider", "beast", "canine", "feline", "rat",
   "ox", "tiger", "rabbit", "dragon", "snake", "horse", "sheep", "monkey", "rooster",
   "dog", "pig", "cat", "ant", "rat", "bat"
};
char *const npc_Class[MAX_NPC_CLASS] = {
   "kensei", "wu jen", "bushi", "fighter", "shugenja", "iaijutsu", "genshin",
   "kishi", "sohei", "mystic", "wizard", "savage", "blacksmith", "questmaster",
   "tattooartist", "weaponsmith", "armorsmith", "merchant", "trainer", "baker", "butcher",
   "princess", "prince", "emporer", "emporess" "pc1", "pc2", "pc3",
   "pc4", "pc5", "pc6", "pc7", "pc8", "pc9", "pc10",
};

/*
 * Attribute bonus tables.
 */
const struct str_app_type str_app[26] = {
   {-5, -4, 0, 0},   /* 0  */
   {-5, -4, 3, 1},   /* 1  */
   {-3, -2, 3, 2},
   {-3, -1, 10, 3},  /* 3  */
   {-2, -1, 25, 4},
   {-2, -1, 55, 5},  /* 5  */
   {-1, 0, 80, 6},
   {-1, 0, 90, 7},
   {0, 0, 100, 8},
   {0, 0, 100, 9},
   {0, 0, 115, 10},  /* 10  */
   {0, 0, 115, 11},
   {0, 0, 140, 12},
   {0, 0, 140, 13},  /* 13  */
   {0, 1, 170, 14},
   {1, 1, 170, 15},  /* 15  */
   {1, 2, 195, 16},
   {2, 3, 220, 22},
   {2, 4, 250, 25},  /* 18  */
   {3, 5, 400, 30},
   {3, 6, 500, 35},  /* 20  */
   {4, 7, 600, 40},
   {5, 7, 700, 45},
   {6, 8, 800, 50},
   {8, 10, 900, 55},
   {10, 12, 999, 60} /* 25   */
};
const struct int_app_type int_app[26] = {
   {3},  /*  0 */
   {5},  /*  1 */
   {7},
   {8},  /*  3 */
   {9},
   {10}, /*  5 */
   {11},
   {12},
   {13},
   {15},
   {17}, /* 10 */
   {19},
   {22},
   {25},
   {28},
   {31}, /* 15 */
   {34},
   {37},
   {40}, /* 18 */
   {44},
   {49}, /* 20 */
   {55},
   {60},
   {70},
   {85},
   {99}  /* 25 */
};
const struct wis_app_type wis_app[26] = {
   {0},  /*  0 */
   {0},  /*  1 */
   {0},
   {0},  /*  3 */
   {0},
   {0},  /*  5 */
   {0},
   {0},
   {0},
   {0},
   {0},  /* 10 */
   {4},
   {4},
   {5},
   {5},
   {6},  /* 15 */
   {6},
   {7},
   {7},  /* 18 */
   {8},
   {8},  /* 20 */
   {9},
   {9},
   {9},
   {10},
   {10}  /* 25 */
};
const struct dex_app_type dex_app[26] = {
   {60}, /* 0 */
   {50}, /* 1 */
   {50},
   {40},
   {30},
   {20}, /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0},  /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10},   /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50},   /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120}   /* 25 */
};
const struct con_app_type con_app[26] = {
   {-4, 20},   /*  0 */
   {-3, 25},   /*  1 */
   {-2, 30},
   {-2, 35},   /*  3 */
   {-1, 40},
   {-1, 45},   /*  5 */
   {-1, 50},
   {0, 55},
   {0, 60},
   {0, 65},
   {0, 70}, /* 10 */
   {0, 75},
   {0, 80},
   {0, 85},
   {0, 88},
   {1, 90}, /* 15 */
   {2, 95},
   {2, 97},
   {3, 99}, /* 18 */
   {3, 99},
   {4, 99}, /* 20 */
   {4, 99},
   {5, 99},
   {6, 99},
   {7, 99},
   {8, 99}  /* 25 */
};
const struct cha_app_type cha_app[26] = {
   {-60},   /* 0 */
   {-50},   /* 1 */
   {-50},
   {-40},
   {-30},
   {-20},   /* 5 */
   {-10},
   {-5},
   {-1},
   {0},
   {0},  /* 10 */
   {0},
   {0},
   {0},
   {1},
   {5},  /* 15 */
   {10},
   {20},
   {30},
   {40},
   {50}, /* 20 */
   {60},
   {70},
   {80},
   {90},
   {99}  /* 25 */
};

/* Have to fix this up - not exactly sure how it works (Scryn) */
const struct lck_app_type lck_app[26] = {
   {60}, /* 0 */
   {50}, /* 1 */
   {50},
   {40},
   {30},
   {20}, /* 5 */
   {10},
   {0},
   {0},
   {0},
   {0},  /* 10 */
   {0},
   {0},
   {0},
   {0},
   {-10},   /* 15 */
   {-15},
   {-20},
   {-30},
   {-40},
   {-50},   /* 20 */
   {-60},
   {-75},
   {-90},
   {-105},
   {-120}   /* 25 */
};

/*
 * Liquid properties.
 * Used in #OBJECT section of area file.
 */
const struct liq_type liq_table[LIQ_MAX] = {
   {"water", "clear", {0, 1, 10}},  /*  0 */
   {"beer", "amber", {3, 2, 5}},
   {"wine", "rose", {5, 2, 5}},
   {"ale", "brown", {2, 2, 5}},
   {"dark ale", "dark", {1, 2, 5}},
   {"whisky", "golden", {6, 1, 4}}, /*  5 */
   {"lemonade", "pink", {0, 1, 8}},
   {"firebreather", "boiling", {10, 0, 0}},
   {"local specialty", "everclear", {3, 3, 3}},
   {"slime mold juice", "green", {0, 4, -8}},
   {"milk", "white", {0, 3, 6}}, /* 10 */
   {"tea", "tan", {0, 1, 6}},
   {"coffee", "black", {0, 1, 6}},
   {"blood", "red", {0, 2, -1}},
   {"salt water", "clear", {0, 1, -2}},
   {"cola", "cherry", {0, 1, 5}},   /* 15 */
   {"mead", "honey color", {4, 2, 5}}, /* 16 */
   {"grog", "thick brown", {3, 2, 5}}  /* 17 */
};

/* removed "pea" and added chop, spear, smash - Grimm */
/* Removed duplication in damage types - Samson 1-9-00 */
char *attack_table[MAX_DAM_TYPE] = {
   "hit", "slash", "stab", "hack", "crush", "lash", "pierce", "thrust", "slice"
};
char *attack_table_plural[MAX_DAM_TYPE] = {
   "hits", "slashes", "stabs", "hacks", "crushes", "lashes", "pierces", "thrusts", "slices"
};
char *weapon_skills[WEP_MAX] = {
   "Barehand", "Sword", "Dagger", "Whip", "Talon", "Hammer", "Archery", "Blowgun",
   "Sling", "Axe", "Spear", "Staff", "Polearm"
};
char *projectiles[PROJ_MAX] = {
   "Bolt", "Arrow", "Dart", "Stone"
};
char *s_blade_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char *p_blade_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char *s_blunt_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char *p_blunt_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char *s_generic_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char *p_generic_messages[24] = {
   "&gmisses&D", "&gbarely scratchs&D", "&gscratchs&D", "&gnicks&D",
   "&G- cuts -&D", "&G- hits -&D", "&G- tears -&D", "&G- rips -&D",
   "&O= gashs =&D", "&O= lacerates =&D", "&O= hacks =&D", "&O= mauls =&D",
   "&Y* rends *&D", "&Y* decimates *&D", "&Y* mangles *&D", "&Y* devastates *&D",
   "&r** cleaves **&D", "&r** butchers **&D", "&r** disembowels **&D", "&r** disfigures **&D",
   "&R*** GUTS ***&D", "&R*** EVISCERATES ***&D", "&R*** SLAUGHTERS ***&D", "&R*** ANNIHILATES ***&D"
};
char **const s_message_table[MAX_DAM_TYPE] = {
   s_generic_messages,  /* hit */
   s_blade_messages, /* slash */
   s_blade_messages, /* stab */
   s_blade_messages, /* hack */
   s_blunt_messages, /* crush */
   s_blunt_messages, /* lash */
   s_blade_messages, /* pierce */
   s_blade_messages, /* thrust */
   s_blade_messages, /* slice */
   s_blade_messages, /* MAGIC */
};
char **const p_message_table[MAX_DAM_TYPE] = {
   p_generic_messages,  /* hit */
   p_blade_messages, /* slash */
   p_blade_messages, /* stab */
   p_blade_messages, /* hack */
   p_blunt_messages, /* crush */
   p_blunt_messages, /* lash */
   p_blade_messages, /* pierce */
   p_blade_messages, /* thrust */
   p_blade_messages, /* slice */
   p_blade_messages, /* MAGIC */
};

/* Weather constants - FB */
char *const temp_settings[MAX_CLIMATE] = {
   "cold",
   "cool",
   "normal",
   "warm",
   "hot",
};
char *const precip_settings[MAX_CLIMATE] = {
   "arid",
   "dry",
   "normal",
   "damp",
   "wet",
};
char *const wind_settings[MAX_CLIMATE] = {
   "still",
   "calm",
   "normal",
   "breezy",
   "windy",
};
char *const preciptemp_msg[6][6] = {
   /*
    * precip = 0 
    */
   {
    "Frigid temperatures settle over the land",
    "It is bitterly cold",
    "The weather is crisp and dry",
    "A comfortable warmth sets in",
    "A dry heat warms the land",
    "Seething heat bakes the land"},
   /*
    * precip = 1 
    */
   {
    "A few flurries drift from the high clouds",
    "Frozen drops of rain fall from the sky",
    "An occasional raindrop falls to the ground",
    "Mild drops of rain seep from the clouds",
    "It is very warm, and the sky is overcast",
    "High humidity intensifies the seering heat"},
   /*
    * precip = 2 
    */
   {
    "A brief snow squall dusts the earth",
    "A light flurry dusts the ground",
    "Light snow drifts down from the heavens",
    "A light drizzle mars an otherwise perfect day",
    "A few drops of rain fall to the warm ground",
    "A light rain falls through the sweltering sky"},
   /*
    * precip = 3 
    */
   {
    "Snowfall covers the frigid earth",
    "Light snow falls to the ground",
    "A brief shower moistens the crisp air",
    "A pleasant rain falls from the heavens",
    "The warm air is heavy with rain",
    "A refreshing shower eases the oppresive heat"},
   /*
    * precip = 4 
    */
   {
    "Sleet falls in sheets through the frosty air",
    "Snow falls quickly, piling upon the cold earth",
    "Rain pelts the ground on this crisp day",
    "Rain drums the ground rythmically",
    "A warm rain drums the ground loudly",
    "Tropical rain showers pelt the seering ground"},
   /*
    * precip = 5 
    */
   {
    "A downpour of frozen rain covers the land in ice",
    "A blizzard blankets everything in pristine white",
    "Torrents of rain fall from a cool sky",
    "A drenching downpour obscures the temperate day",
    "Warm rain pours from the sky",
    "A torrent of rain soaks the heated earth"}
};
char *const windtemp_msg[6][6] = {
   /*
    * wind = 0 
    */
   {
    "The frigid air is completely still",
    "A cold temperature hangs over the area",
    "The crisp air is eerily calm",
    "The warm air is still",
    "No wind makes the day uncomfortably warm",
    "The stagnant heat is sweltering"},
   /*
    * wind = 1 
    */
   {
    "A light breeze makes the frigid air seem colder",
    "A stirring of the air intensifies the cold",
    "A touch of wind makes the day cool",
    "It is a temperate day, with a slight breeze",
    "It is very warm, the air stirs slightly",
    "A faint breeze stirs the feverish air"},
   /*
    * wind = 2 
    */
   {
    "A breeze gives the frigid air bite",
    "A breeze swirls the cold air",
    "A lively breeze cools the area",
    "It is a temperate day, with a pleasant breeze",
    "Very warm breezes buffet the area",
    "A breeze ciculates the sweltering air"},
   /*
    * wind = 3 
    */
   {
    "Stiff gusts add cold to the frigid air",
    "The cold air is agitated by gusts of wind",
    "Wind blows in from the north, cooling the area",
    "Gusty winds mix the temperate air",
    "Brief gusts of wind punctuate the warm day",
    "Wind attempts to cut the sweltering heat"},
   /*
    * wind = 4 
    */
   {
    "The frigid air whirls in gusts of wind",
    "A strong, cold wind blows in from the north",
    "Strong wind makes the cool air nip",
    "It is a pleasant day, with gusty winds",
    "Warm, gusty winds move through the area",
    "Blustering winds punctuate the seering heat"},
   /*
    * wind = 5 
    */
   {
    "A frigid gale sets bones shivering",
    "Howling gusts of wind cut the cold air",
    "An angry wind whips the air into a frenzy",
    "Fierce winds tear through the tepid air",
    "Gale-like winds whip up the warm air",
    "Monsoon winds tear the feverish air"}
};
char *const precip_msg[3] = {
   "there is not a cloud in the sky",
   "pristine white clouds are in the sky",
   "thick, grey clouds mask the sun"
};
char *const wind_msg[6] = {
   "there is not a breath of wind in the air",
   "a slight breeze stirs the air",
   "a breeze wafts through the area",
   "brief gusts of wind punctuate the air",
   "angry gusts of wind blow",
   "howling winds whip the air into a frenzy"
};


