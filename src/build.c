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
 *		       Online Building and Editing Module		    *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
extern int top_affect;
extern int top_reset;
extern int top_ed;
extern bool fBootDb;
REL_DATA *first_relation = NULL;
REL_DATA *last_relation = NULL;
/*
 * Exit Pull/push types
 * (water, air, earth, fire)
 */
char *const ex_pmisc[] = { "undefined", "vortex", "vacuum", "slip", "ice", "mysterious"
                         };
char *const ex_pwater[] = { "current", "wave", "whirlpool", "geyser"
                          };
char *const ex_pair[] = { "wind", "storm", "coldwind", "breeze"
                        };
char *const ex_pearth[] = { "landslide", "sinkhole", "quicksand", "earthquake"
                          };
char *const ex_pfire[] = { "lava", "hotair"
                         };
char *const ex_flags[] =
{
	"isdoor", "closed", "locked", "secret", "swim", "pickproof", "fly", "climb",
	"dig", "eatkey", "nopassdoor", "hidden", "passage", "portal", "r1", "r2",
	"can_climb", "can_enter", "can_leave", "auto", "noflee", "searchable",
	"bashed", "bashproof", "nomob", "window", "can_look", "isbolt", "bolted", "overland"
};
char *const sec_flags[] =
{
	"inside", "city", "field", "forest", "hills", "mountain", "water_swim",
	"water_noswim", "underwater", "air", "desert", "river", "oceanfloor",
	"underground", "jungle", "swamp", "tundra", "ice", "ocean", "lava", "shore",
	"r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16"
};
char *const r_flags[] =
{
	"dark", "death", "nomob", "indoors", "lawful", "neutral", "chaotic",
	"nomagic", "tunnel", "private", "safe", "solitary", "petshop", "norecall",
	"donation", "nodropall", "silence", "logspeech", "nodrop", "clanstoreroom",
	"nosummon", "noastral", "teleport", "teledesc", "nofloor", "nosupplicate",
	"arena", "nomissile", "map", "nobuy", "prototype", "dnd", "locker", "nomap"
};

char *const o_flags[] =
{
	"glow", "hum", "dark", "loyal", "evil", "invis", "magic", "nodrop", "bless",
	"antigood", "antievil", "antineutral", "noremove", "inventory",
	"organic", "metal", "donation", "clanobject", "clancorpse", "hidden", "poisoned",
	"covering", "deathrot", "buried", "prototype", "nolocate", "groundrot", "lootable",
	"onmap", "quest", "lodged", "refined", "unique", "elite", "antideath", "antilife",
	"antidecay", "antigrowth"
};
char *const container_flags[] =
{
	"closeable", "pickproof", "closed", "locked", "eatkey", "r1", "r2", "r3",
	"r4", "r5", "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
	"r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23", "r24", "r25", "r26",
	"r27"
};
char *const mag_flags[] =
{
	"returning", "backstabber", "bane", "loyal", "haste", "drain",
	"lightning_blade"
};
char *const w_flags[] =
{
	"take", "head", "body", "back", "arms", "hands", "wield", "shield", "hold",
	"legs", "feet", "light", "lodge_rib", "lodge_arm", "lodge_leg"
};
char *const item_w_flags[] =
{
	"take", "head", "body", "back", "arms", "hands", "wield", "shield", "hold",
	"legs", "feet", "light", "lodge_rib", "lodge_arm", "lodge_leg"
};
char *const wear_locs[] =
{
	"light", "take", "head", "body", "back", "arms", "hands", "wield", "shield", "hold",
	"legs", "feet"
};
char *const area_flags[] =
{
	"nopkill", "freekill", "noteleport", "spelllimit", "r4", "r5", "r6", "r7", "r8",
	"r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17",
	"r18", "r19", "r20", "r21", "r22", "r23", "r24",
	"r25", "r26", "r27", "r28", "r29", "r30", "r31"
};
char *const o_types[] =
{
	"none", "light", "scroll", "wand", "staff", "weapon", "_fireweapon", "_missile",
	"treasure", "armor", "potion", "_worn", "furniture", "trash", "_oldtrap",
	"container", "_note", "drinkcon", "key", "food", "money", "pen", "boat",
	"corpse", "corpse_pc", "fountain", "pill", "blood", "bloodstain",
	"scraps", "pipe", "herbcon", "herb", "incense", "fire", "book", "switch",
	"lever", "pullchain", "button", "dial", "rune", "runepouch", "match", "trap",
	"map", "portal", "paper", "tinder", "lockpick", "spike", "disease", "oil",
	"fuel", "_empty1", "_empty2", "missileweapon", "projectile", "quiver", "shovel",
	"salve", "cook", "keyring", "odor", "dye", "sewkit", "handmade", "ore", "mix", "questtoken",
	"fishing_pole", "fishing_bait", "chance"
};
char *const a_types[] =
{
	"none", "strength", "dexterity", "intelligence", "wisdom", "constitution",
	"sex", "Class", "level", "age", "height", "weight", "mana", "hit", "move",
	"gold", "experience", "armor", "hitroll", "damroll", "save_poison", "save_rod",
	"save_para", "save_breath", "save_spell", "charisma", "affected", "resistant",
	"immune", "susceptible", "weaponspell", "luck", "backstab", "pick", "track",
	"steal", "sneak", "hide", "palm", "detrap", "dodge", "peek", "scan", "gouge",
	"search", "mount", "disarm", "kick", "parry", "bash", "stun", "punch", "climb",
	"grip", "scribe", "brew", "wearspell", "removespell", "stripsn", "remove",
	"dig", "full", "thirst", "drunk", "blood", "cook", "recurringspell", "contagious",
	"xaffected", "odor", "roomflag", "sectortype", "roomlight", "televnum", "teledelay"
};
char *const a_flags[] =
{
	"blind", "invisible", "detect_evil", "detect_invis", "detect_magic",
	"detect_hidden", "hold", "sanctuary", "faerie_fire", "infrared", "curse",
	"_flaming", "poison", "protect", "_paralysis", "sneak", "hide", "sleep",
	"charm", "flying", "pass_door", "floating", "truesight", "detect_traps",
	"scrying", "fireshield", "shockshield", "r1", "iceshield", "possess",
	"berserk", "aqua_breath", "recurringspell", "contagious", "acidmist",
	"venomshield"
};
char *const act_flags[] =
{
	"npc", "sentinel", "scavenger", "onmap", "tattooartist", "aggressive", "stayarea",
	"wimpy", "pet", "notused", "practice", "immortal", "deadly", "polyself",
	"meta_aggr", "guardian", "running", "nowander", "mountable", "mounted",
	"scholar", "secretive", "hardhat", "mobinvis", "noassist", "autonomous",
	"pacifist", "noattack", "annoying", "statshield", "prototype", "banker", "pktoggle",
	"questmaster", "noquest", "travelmage", "healer", "forge", "restring", "train"
};
char *const pc_flags[] =
{
	"r1", "deadly", "unauthed", "norecall", "nointro", "gag", "retired", "guest",
	"nosummon", "pager", "notitled", "groupwho", "diagnose", "highgag", "watch",
	"nstart", "dnd", "idle", "flags", "sector", "aname", "helper", "coder", "builder", "r18",
	"r19", "r20", "r21", "r22", "r23", "r24", "r25"
};
char *const plr_flags[] =
{
	"npc", "boughtpet", "shovedrag", "autoexits", "autoloot", "autosac", "blank",
	"outcast", "brief", "combine", "prompt", "telnet_ga", "holylight",
	"wizinvis", "roomvnum", "silence", "noemote", "attacker", "notell", "log",
	"deny", "freeze", "thief", "killer", "litterbug", "ansi", "rip", "nice",
	"flee", "autogold", "automap", "afk", "invisprompt", "onmap", "mapedit",
	"compass", "questor", "mip", "exempt"
};

char *const cmd_flags[] =
{
	"possessed", "polymorphed", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
	"r9", "r10", "r11", "r12", "r13", "r14", "r15", "r16", "r17", "r18", "r19",
	"r20", "r21", "r22", "r23", "r24", "r25", "r26", "r27", "r28", "r29", "r30"
};
char *const ris_flags[] =
{
	"fire", "cold", "electricity", "energy", "blunt", "pierce", "slash", "acid",
	"poison", "drain", "sleep", "charm", "hold", "nonmagic", "plus1", "plus2",
	"plus3", "plus4", "plus5", "plus6", "magic", "paralysis", "lash", "hack", "r3",
	"r4", "r5", "r6", "r7", "r8", "r9", "r10"
};
char *const trig_flags[] =
{
	"up", "unlock", "lock", "d_north", "d_south", "d_east", "d_west", "d_up",
	"d_down", "door", "container", "open", "close", "passage", "oload", "mload",
	"teleport", "teleportall", "teleportplus", "death", "cast", "fakeblade",
	"rand4", "rand6", "trapdoor", "anotherroom", "usedial", "absolutevnum",
	"showroomdesc", "autoreturn", "r2", "r3"
};
char *const part_flags[] =
{
	"head", "heart", "arms", "legs", "guts", "brains", "hands", "feet", "fingers",
	"ear", "eye", "long_tongue", "eyestalks", "tentacles", "fins", "wings",
	"tail", "scales", "tusks", "horns", "claws", "feathers", "forlegs", "paws",
	"hooves", "beak", "sharpscales", "haunches", "fangs", "r1", "r2", "r3"
};
char *const attack_flags[] =
{
	"bite", "claws", "tail", "sting", "punch", "kick", "trip", "bash", "stun",
	"gouge", "backstab", "feed", "drain", "firebreath", "frostbreath",
	"acidbreath", "lightnbreath", "gasbreath", "poison", "nastypoison", "gaze",
	"blindness", "causeserious", "earthquake", "causecritical", "curse",
	"flamestrike", "harm", "fireball", "colorspray", "weaken", "r1"
};
char *const defense_flags[] =
{
	"parry", "dodge", "heal", "curelight", "cureserious", "curecritical",
	"dispelmagic", "dispelevil", "sanctuary", "fireshield", "shockshield",
	"shield", "bless", "stoneskin", "teleport", "monsum1", "monsum2", "monsum3",
	"monsum4", "disarm", "iceshield", "grip", "truesight", "r4", "r5", "r6", "r7",
	"r8", "r9", "r10", "r11", "r12", "acidmist", "venomshield"
};

/*
 * Note: I put them all in one big set of flags since almost all of these
 * can be shared between mobs, objs and rooms for the exception of
 * bribe and hitprcnt, which will probably only be used on mobs.
 * ie: drop -- for an object, it would be triggered when that object is
 * dropped; -- for a room, it would be triggered when anything is dropped
 *          -- for a mob, it would be triggered when anything is dropped
 *
 * Something to consider: some of these triggers can be grouped together,
 * and differentiated by different arguments... for example:
 *  hour and time, rand and randiw, speech and speechiw
 *
 */
char *const mprog_flags[] =
{
	"act", "speech", "rand", "fight", "death", "hitprcnt", "entry", "greet",
	"allgreet", "give", "bribe", "hour", "time", "wear", "remove", "sac",
	"look", "exa", "zap", "get", "drop", "damage", "repair", "randiw",
	"speechiw", "pull", "push", "sleep", "rest", "leave", "script", "use"
};

/* Strlen_color by Rusty, useful for skipping over colors */
/* Fixed and edited by Xerves -- 8/29/99 */
int strlen_color ( char *argument )
{
	char *str;
	int i, length;
	str = argument;
	if ( argument == NULL )
		return 0;
	for ( length = i = 0; i < strlen ( argument ); ++i )
	{
		if ( ( str[i] != '&' ) && ( str[i] != '^' ) )
			++length;
		if ( ( str[i] == '&' ) || ( str[i] == '^' ) )
		{
			if ( ( str[i] == '&' ) && ( str[i + 1] == '&' ) )
				length = 2 + length;
			else if ( ( str[i] == '^' ) && ( str[i + 1] == '^' ) )
				length = 2 + length;
			else
				--length;
		}
	}
	return length;
}
char *flag_string ( int bitvector, char *const flagarray[] )
{
	static char buf[MAX_STRING_LENGTH];
	int x;
	buf[0] = STRING_NULL;
	for ( x = 0; x < 32; x++ )
		if ( IS_SET ( bitvector, 1 << x ) )
		{
			strcat ( buf, flagarray[x] );
			/*
			 * don't catenate a blank if the last char is blank  --Gorog
			 */
			if ( buf[0] != STRING_NULL && ' ' != buf[strlen ( buf ) - 1] )
				strcat ( buf, " " );
		}
	if ( ( x = strlen ( buf ) ) > 0 )
		buf[--x] = STRING_NULL;
	return buf;
}
char *ext_flag_string ( EXT_BV * bitvector, char *const flagarray[] )
{
	static char buf[MAX_STRING_LENGTH];
	int x;
	buf[0] = STRING_NULL;
	for ( x = 0; x < MAX_BITS; x++ )
		if ( xIS_SET ( *bitvector, x ) )
		{
			strcat ( buf, flagarray[x] );
			strcat ( buf, " " );
		}
	if ( ( x = strlen ( buf ) ) > 0 )
		buf[--x] = STRING_NULL;
	return buf;
}

bool can_rmodify ( CHAR_DATA * ch, ROOM_INDEX_DATA * room )
{
	int vnum = room->vnum;
	AREA_DATA *pArea;
	if ( IS_NPC ( ch ) )
		return FALSE;
	if ( IS_PLR_FLAG ( ch, PLR_ONMAP ) )
	{
		send_to_char ( "You cannot use rset from the overland maps.\r\n", ch );
		return FALSE;
	}
	if ( get_trust ( ch ) >= sysdata.level_modify_proto )
		return TRUE;
	if ( !xIS_SET ( room->room_flags, ROOM_PROTOTYPE ) )
	{
		send_to_char ( "You cannot modify this room.\r\n", ch );
		return FALSE;
	}
	if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
	{
		send_to_char ( "You must have an assigned area to modify this room.\r\n", ch );
		return FALSE;
	}
	if ( vnum >= pArea->low_r_vnum && vnum <= pArea->hi_r_vnum )
		return TRUE;
	send_to_char ( "That room is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_omodify ( CHAR_DATA * ch, OBJ_DATA * obj )
{
	int vnum = obj->pIndexData->vnum;
	AREA_DATA *pArea;
	if ( IS_NPC ( ch ) )
		return FALSE;
	if ( get_trust ( ch ) >= sysdata.level_modify_proto )
		return TRUE;
	if ( !IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char ( "You cannot modify this object.\r\n", ch );
		return FALSE;
	}
	if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
	{
		send_to_char ( "You must have an assigned area to modify this object.\r\n", ch );
		return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum )
		return TRUE;
	send_to_char ( "That object is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_oedit ( CHAR_DATA * ch, OBJ_INDEX_DATA * obj )
{
	int vnum = obj->vnum;
	AREA_DATA *pArea;
	if ( IS_NPC ( ch ) )
		return FALSE;
	if ( get_trust ( ch ) >= LEVEL_GOD )
		return TRUE;
	if ( !IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char ( "You cannot modify this object.\r\n", ch );
		return FALSE;
	}
	if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
	{
		send_to_char ( "You must have an assigned area to modify this object.\r\n", ch );
		return FALSE;
	}
	if ( vnum >= pArea->low_o_vnum && vnum <= pArea->hi_o_vnum )
		return TRUE;
	send_to_char ( "That object is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_mmodify ( CHAR_DATA * ch, CHAR_DATA * mob )
{
	int vnum;
	AREA_DATA *pArea;
	if ( mob == ch )
		return TRUE;
	if ( !IS_NPC ( mob ) )
	{
		if ( get_trust ( ch ) >= sysdata.level_modify_proto && get_trust ( ch ) > get_trust ( mob ) )
			return TRUE;
		else
			send_to_char ( "You can't do that.\r\n", ch );
		return FALSE;
	}
	vnum = mob->pIndexData->vnum;
	if ( IS_NPC ( ch ) )
		return FALSE;
	if ( get_trust ( ch ) >= sysdata.level_modify_proto )
		return TRUE;
	if ( !xIS_SET ( mob->act, ACT_PROTOTYPE ) )
	{
		send_to_char ( "You cannot modify this mobile.\r\n", ch );
		return FALSE;
	}
	if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
	{
		send_to_char ( "You must have an assigned area to modify this mobile.\r\n", ch );
		return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum )
		return TRUE;
	send_to_char ( "That mobile is not in your allocated range.\r\n", ch );
	return FALSE;
}

bool can_medit ( CHAR_DATA * ch, MOB_INDEX_DATA * mob )
{
	int vnum = mob->vnum;
	AREA_DATA *pArea;
	if ( IS_NPC ( ch ) )
		return FALSE;
	if ( get_trust ( ch ) >= LEVEL_GOD )
		return TRUE;
	if ( !xIS_SET ( mob->act, ACT_PROTOTYPE ) )
	{
		send_to_char ( "You cannot modify this mobile.\r\n", ch );
		return FALSE;
	}
	if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
	{
		send_to_char ( "You must have an assigned area to modify this mobile.\r\n", ch );
		return FALSE;
	}
	if ( vnum >= pArea->low_m_vnum && vnum <= pArea->hi_m_vnum )
		return TRUE;
	send_to_char ( "That mobile is not in your allocated range.\r\n", ch );
	return FALSE;
}
int get_otype ( char *type )
{
	int x;
	for ( x = 0; x < ( sizeof ( o_types ) / sizeof ( o_types[0] ) ); x++ )
		if ( !str_cmp ( type, o_types[x] ) )
			return x;
	return -1;
}
int get_aflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( a_flags ) / sizeof ( a_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, a_flags[x] ) )
			return x;
	return -1;
}

int get_atype ( char *type )
{
	int x;
	for ( x = 0; x < ( sizeof ( a_types ) / sizeof ( a_types[0] ) ); x++ )
		if ( !str_cmp ( type, a_types[x] ) )
			return x;
	return -1;
}
int get_npc_race ( char *type )
{
	int x;
	for ( x = 0; x < MAX_NPC_RACE; x++ )
		if ( !str_cmp ( type, npc_race[x] ) )
			return x;
	return -1;
}
int get_pc_Class ( char *Class )
{
	int x;
	for ( x = 0; x < MAX_PC_CLASS; x++ )
		if ( !str_cmp ( Class_table[x]->who_name, Class ) )
			return x;
	return -1;
}
int get_pc_race ( char *type )
{
	int i;
	for ( i = 0; i < MAX_PC_RACE; i++ )
		if ( !str_cmp ( type, race_table[i]->race_name ) )
			return i;
	return -1;
}
int get_wearloc ( char *type )
{
	int x;
	for ( x = 0; x < ( sizeof ( wear_locs ) / sizeof ( wear_locs[0] ) ); x++ )
		if ( !str_cmp ( type, wear_locs[x] ) )
			return x;
	return -1;
}
int get_secflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( sec_flags ) / sizeof ( sec_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, sec_flags[x] ) )
			return x;
	return -1;
}
int get_exflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( ex_flags ) / sizeof ( ex_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, ex_flags[x] ) )
			return x;
	return -1;
}
int get_pulltype ( char *type )
{
	int x;
	if ( !str_cmp ( type, "none" ) || !str_cmp ( type, "clear" ) )
		return 0;
	for ( x = 0; x < ( sizeof ( ex_pmisc ) / sizeof ( ex_pmisc[0] ) ); x++ )
		if ( !str_cmp ( type, ex_pmisc[x] ) )
			return x;
	for ( x = 0; x < ( sizeof ( ex_pwater ) / sizeof ( ex_pwater[0] ) ); x++ )
		if ( !str_cmp ( type, ex_pwater[x] ) )
			return x + PT_WATER;
	for ( x = 0; x < ( sizeof ( ex_pair ) / sizeof ( ex_pair[0] ) ); x++ )
		if ( !str_cmp ( type, ex_pair[x] ) )
			return x + PT_AIR;
	for ( x = 0; x < ( sizeof ( ex_pearth ) / sizeof ( ex_pearth[0] ) ); x++ )
		if ( !str_cmp ( type, ex_pearth[x] ) )
			return x + PT_EARTH;
	for ( x = 0; x < ( sizeof ( ex_pfire ) / sizeof ( ex_pfire[0] ) ); x++ )
		if ( !str_cmp ( type, ex_pfire[x] ) )
			return x + PT_FIRE;
	return -1;
}
int get_rflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( r_flags ) / sizeof ( r_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, r_flags[x] ) )
			return x;
	return -1;
}
int get_mpflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( mprog_flags ) / sizeof ( mprog_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, mprog_flags[x] ) )
			return x;
	return -1;
}
int get_oflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( o_flags ) / sizeof ( o_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, o_flags[x] ) )
			return x;
	return -1;
}
int get_areaflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( area_flags ) / sizeof ( area_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, area_flags[x] ) )
			return x;
	return -1;
}
int get_wflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( w_flags ) / sizeof ( w_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, w_flags[x] ) )
			return x;
	return -1;
}
int get_actflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( act_flags ) / sizeof ( act_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, act_flags[x] ) )
			return x;
	return -1;
}
int get_pcflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( pc_flags ) / sizeof ( pc_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, pc_flags[x] ) )
			return x;
	return -1;
}
int get_plrflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( plr_flags ) / sizeof ( plr_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, plr_flags[x] ) )
			return x;
	return -1;
}
int get_risflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( ris_flags ) / sizeof ( ris_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, ris_flags[x] ) )
			return x;
	return -1;
}

/*
 * For use with cedit --Shaddai
 */
int get_cmdflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( cmd_flags ) / sizeof ( cmd_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, cmd_flags[x] ) )
			return x;
	return -1;
}
int get_trigflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( trig_flags ) / sizeof ( trig_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, trig_flags[x] ) )
			return x;
	return -1;
}
int get_partflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( part_flags ) / sizeof ( part_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, part_flags[x] ) )
			return x;
	return -1;
}
int get_attackflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( attack_flags ) / sizeof ( attack_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, attack_flags[x] ) )
			return x;
	return -1;
}
int get_defenseflag ( char *flag )
{
	int x;
	for ( x = 0; x < ( sizeof ( defense_flags ) / sizeof ( defense_flags[0] ) ); x++ )
		if ( !str_cmp ( flag, defense_flags[x] ) )
			return x;
	return -1;
}
int get_langflag ( char *flag )
{
	int x;
	for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
		if ( !str_cmp ( flag, lang_names[x] ) )
			return lang_array[x];
	return LANG_UNKNOWN;
}
int get_langnum ( char *flag )
{
	int x;
	for ( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
		if ( !str_cmp ( flag, lang_names[x] ) )
			return x;
	return -1;
}

/*
 * Remove carriage returns from a line
 */
char *strip_cr ( char *str )
{
	static char newstr[MAX_STRING_LENGTH];
	int i, j;
	if ( !str || str[0] == STRING_NULL )
	{
		// bug( "%s: NULL string!", __FUNCTION__ );
		return "";
	}
	for ( i = j = 0; str[i] != STRING_NULL; i++ )
		if ( str[i] != '\r' )
		{
			newstr[j++] = str[i];
		}
	newstr[j] = STRING_NULL;
	return newstr;
}

/*
 * Removes the tildes from a line, except if it's the last character.
 */
void smush_tilde ( char *str )
{
	int len;
	char last;
	char *strptr;
	strptr = str;
	len = strlen ( str );
	if ( len )
		last = strptr[len - 1];
	else
		last = STRING_NULL;
	for ( ; *str != STRING_NULL; str++ )
	{
		if ( *str == '~' )
			*str = '-';
	}
	if ( len )
		strptr[len - 1] = last;
	return;
}
void start_editing ( CHAR_DATA * ch, char *data )
{
	EDITOR_DATA *edit;
	short lines, size, lpos;
	char c;
	if ( !ch->desc )
	{
		bug ( "Fatal: start_editing: no desc", 0 );
		return;
	}
	if ( ch->substate == SUB_RESTRICTED )
		bug ( "NOT GOOD: start_editing: ch->substate == SUB_RESTRICTED", 0 );
	set_char_color ( AT_GREEN, ch );
	send_to_char ( "Begin entering your text now (/? = help /s = save /c = clear /l = list)\r\n", ch );
	send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
	if ( ch->editor )
		stop_editing ( ch );
	CREATE ( edit, EDITOR_DATA, 1 );
	edit->numlines = 0;
	edit->on_line = 0;
	edit->size = 0;
	size = 0;
	lpos = 0;
	lines = 0;
	if ( !data )
		bug ( "editor: data is NULL!\r\n", 0 );
	else
		for ( ;; )
		{
			c = data[size++];
			if ( c == STRING_NULL )
			{
				edit->line[lines][lpos] = STRING_NULL;
				break;
			}
			else if ( c == '\r' )
				;
			else if ( c == '\n' || lpos > 78 )
			{
				edit->line[lines][lpos] = STRING_NULL;
				++lines;
				lpos = 0;
			}
			else
				edit->line[lines][lpos++] = c;
			if ( lines >= 49 || size > 4096 )
			{
				edit->line[lines][lpos] = STRING_NULL;
				break;
			}
		}
	if ( lpos > 0 && lpos < 78 && lines < 49 )
	{
		edit->line[lines][lpos] = '~';
		edit->line[lines][lpos + 1] = STRING_NULL;
		++lines;
		lpos = 0;
	}
	edit->numlines = lines;
	edit->size = size;
	edit->on_line = lines;
	ch->editor = edit;
	ch->desc->connected = CON_EDITING;
}

char *copy_buffer_nohash ( CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char tmp[100];
	short x, len;
	if ( !ch )
	{
		bug ( "%s", "copy_buffer_nohash: null ch" );
		return str_dup ( "" );
	}
	if ( !ch->editor )
	{
		bug ( "%s", "copy_buffer_nohash: null editor" );
		return str_dup ( "" );
	}
	buf[0] = STRING_NULL;
	for ( x = 0; x < ch->editor->numlines; x++ )
	{
		strcpy ( tmp, ch->editor->line[x] );
		len = strlen ( tmp );
		if ( tmp[len - 1] == '~' )
			tmp[len - 1] = STRING_NULL;
		else
			strcat ( tmp, "\r\n" );
		smash_tilde ( tmp );
		strcat ( buf, tmp );
	}
	return str_dup ( buf );
}

char *copy_buffer ( CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char tmp[100];
	short x, len;
	if ( !ch )
	{
		bug ( "copy_buffer: null ch", 0 );
		return STRALLOC ( "" );
	}
	if ( !ch->editor )
	{
		bug ( "copy_buffer: null editor", 0 );
		return STRALLOC ( "" );
	}
	buf[0] = STRING_NULL;
	for ( x = 0; x < ch->editor->numlines; x++ )
	{
		strcpy ( tmp, ch->editor->line[x] );
		len = strlen ( tmp );
		if ( tmp[len - 1] == '~' )
			tmp[len - 1] = STRING_NULL;
		else
			strcat ( tmp, "\r\n" );
		/*
		 * This is wrong
		 * smush_tilde(tmp);
		 */
		smash_tilde ( tmp );
		strcat ( buf, tmp );
	}
	return STRALLOC ( buf );
}

void stop_editing ( CHAR_DATA * ch )
{
	set_char_color ( AT_PLAIN, ch );
	DISPOSE ( ch->editor );
	ch->editor = NULL;
	send_to_char ( "Done.\r\n", ch );
	ch->dest_buf = NULL;
	ch->spare_ptr = NULL;
	ch->substate = SUB_NONE;
	if ( !ch->desc )
	{
		bug ( "Fatal: stop_editing: no desc", 0 );
		return;
	}
	ch->desc->connected = CON_PLAYING;
}
void goto_char ( CHAR_DATA * ch, CHAR_DATA * wch, char *argument )
{
	ROOM_INDEX_DATA *location, *in_room;
	set_char_color ( AT_IMMORT, ch );
	location = wch->in_room;
	if ( is_ignoring ( wch, ch ) )
	{
		send_to_char ( "No such location.\r\n", ch );
		return;
	}
	if ( room_is_private ( location ) )
	{
		if ( get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char ( "That room is private right now.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "Overriding private flag!\r\n", ch );
		}
	}
	in_room = ch->in_room;
	if ( ch->fighting )
		stop_fighting ( ch, TRUE );
	/*
	 * Modified bamfout processing by Altrag, installed by Samson 12-10-97
	 */
	if ( ch->pcdata && ch->pcdata->bamfout[0] )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfout, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n vanishes suddenly into thin air.", ch, NULL, NULL, TO_CANSEE );
	ch->regoto = ch->in_room->vnum;
	leave_map ( ch, wch, location );
	/*
	 * Modified bamfin processing by Altrag, installed by Samson 12-10-97
	 */
	if ( ch->pcdata && ch->pcdata->bamfin[0] )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfin, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n appears suddenly out of thin air.", ch, NULL, NULL, TO_CANSEE );
	return;
}
void goto_obj ( CHAR_DATA * ch, OBJ_DATA * obj, char *argument )
{
	ROOM_INDEX_DATA *location;
	set_char_color ( AT_IMMORT, ch );
	location = obj->in_room;
	if ( room_is_private ( location ) )
	{
		if ( get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char ( "That room is private right now.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "Overriding private flag!\r\n", ch );
		}
	}
	if ( ch->fighting )
		stop_fighting ( ch, TRUE );
	/*
	 * Modified bamfout processing by Altrag, installed by Samson 12-10-97
	 */
	if ( ch->pcdata && ch->pcdata->bamfout[0] )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfout, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n vanishes suddenly into thin air.", ch, NULL, NULL, TO_CANSEE );
	ch->regoto = ch->in_room->vnum;
	leave_map ( ch, NULL, location );
	/*
	 * Modified bamfin processing by Altrag, installed by Samson 12-10-97
	 */
	if ( ch->pcdata && ch->pcdata->bamfin[0] )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfin, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n appears suddenly out of thin air.", ch, NULL, NULL, TO_CANSEE );
	return;
}
void do_goto ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location, *in_room;
	CHAR_DATA *wch;
	OBJ_DATA *obj;
	int vnum;
	argument = one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "&wUsage: goto <vnum>\r\n", ch );
		send_to_char ( "&wUsage: goto <char_name>\r\n", ch );
		send_to_char ( "&wUsage: goto map <mapname> <X> <Y>\r\n", ch );
		return;
	}
	/*
	 * Begin Overland Map additions
	 */
	if ( !str_cmp ( arg, "map" ) )
	{
		char arg1[MAX_INPUT_LENGTH];
		char arg2[MAX_INPUT_LENGTH];
		int x, y;
		int map = -1;
		argument = one_argument ( argument, arg1 );
		argument = one_argument ( argument, arg2 );
		if ( arg1[0] == STRING_NULL )
		{
			send_to_char ( "&wGoto which map??\r\n", ch );
			send_to_char ( "&wUsage: goto map <mapname> <X> <Y>\r\n", ch );
			send_to_char ( "&wMap Names: sangang       nyemo      jaili \r\n", ch );
			send_to_char ( "&w           yakongza      xianza     bangoin \r\n", ch );
			send_to_char ( "&w           konjo         zogang     tangmai  \r\n", ch );
			return;
		}

		if ( !str_cmp ( arg1, "sangang" ) )
			map = ACON_C1;
		else if ( !str_cmp ( arg1, "nyemo" ) )
			map = ACON_C2;
		else if ( !str_cmp ( arg1, "jaili" ) )
			map = ACON_C3;
		else if ( !str_cmp ( arg1, "yakongza" ) )
			map = ACON_C4;
		else if ( !str_cmp ( arg1, "xianza" ) )
			map = ACON_C5;
		else if ( !str_cmp ( arg1, "bangoin" ) )
			map = ACON_C6;
		else if ( !str_cmp ( arg1, "konjo" ) )
			map = ACON_C7;
		else if ( !str_cmp ( arg1, "zogang" ) )
			map = ACON_C8;
		else if ( !str_cmp ( arg1, "tangmai" ) )
			map = ACON_C9;

		if ( map == -1 )
		{
			ch_printf ( ch, "There isn't a map for '%s'.\r\n", arg1 );
			return;
		}
		if ( arg2[0] == STRING_NULL || argument[0] == STRING_NULL )
		{
			send_to_char ( "&wUsage: goto map <mapname> <X> <Y>\r\n", ch );
			send_to_char ( "&wMap Names:  sangang       nyemo      jaili \r\n", ch );
			send_to_char ( "&w           yakongza      xianza     bangoin \r\n", ch );
			send_to_char ( "&w           konjo         zogang     tangmai  \r\n", ch );
			return;
		}
		x = atoi ( arg2 );
		y = atoi ( argument );
		if ( x < 0 || x >= MAX_X )
		{
			ch_printf ( ch, "Valid x coordinates are 0 to %d.\r\n", MAX_X - 1 );
			return;
		}
		if ( y < 0 || y >= MAX_Y )
		{
			ch_printf ( ch, "Valid y coordinates are 0 to %d.\r\n", MAX_Y - 1 );
			return;
		}
		enter_map ( ch, x, y, map );
		return;
	}
	/*
	 * End of Overland Map additions
	 */
	if ( !is_number ( arg ) )
	{
		if ( ( wch = get_char_world ( ch, arg ) ) != NULL && wch->in_room != NULL )
		{
			goto_char ( ch, wch, arg );
			return;
		}
		if ( ( obj = get_obj_world ( ch, arg ) ) != NULL )
		{
			goto_obj ( ch, obj, arg );
			return;
		}
	}
	if ( ( location = find_location ( ch, arg ) ) == NULL )
	{
		if ( vnum < 0 || get_room_index ( vnum ) )
		{
			send_to_char ( "That vnum is not in existance...\r\n", ch );
			return;
		}
		if ( get_trust ( ch ) < LEVEL_CREATOR || vnum < 1 || IS_NPC ( ch ) || !ch->pcdata->area )
		{
			send_to_char ( "Unable to create that room, try room_dig <direction> instead.\r\n", ch );
			return;
		}
		if ( vnum < 1 || vnum > MAX_VNUM )
		{
			ch_printf ( ch, "Invalid vnum. Allowable range is 1 to %d\r\n", MAX_VNUM );
			return;
		}
		location = make_room ( vnum, ch->pcdata->area );
		if ( !location )
		{
			bug ( "%s", "Goto: make_room failed" );
			return;
		}
		location->area = ch->pcdata->area;
		send_to_char ( "&WWaving your hand, you form order from swirling chaos,\r\nand step into a new reality...\r\n", ch );
	}
	if ( room_is_private ( location ) )
	{
		if ( ch->level < sysdata.level_override_private )
		{
			send_to_char ( "That room is private right now.\r\n", ch );
			return;
		}
		else
			send_to_char ( "Overriding private flag!\r\n", ch );
	}
	in_room = ch->in_room;
	if ( ch->fighting )
		stop_fighting ( ch, TRUE );
	/*
	 * Modified bamfout processing by Altrag, installed by Samson 12-10-97
	 */
	if ( ch->pcdata && ch->pcdata->bamfout && ch->pcdata->bamfout[0] != STRING_NULL )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfout, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n vanishes suddenly into thin air.", ch, NULL, NULL, TO_CANSEE );
	/*
	 * It's assumed that if you've come this far, it's a room vnum you entered
	 */
	leave_map ( ch, NULL, location );
	if ( ch->pcdata && ch->pcdata->bamfin && ch->pcdata->bamfin[0] != STRING_NULL )
		act ( AT_IMMORT, "$T", ch, NULL, bamf_print ( ch->pcdata->bamfin, ch ), TO_CANSEE );
	else
		act ( AT_IMMORT, "$n appears suddenly out of thin air.", ch, NULL, NULL, TO_CANSEE );
	return;
}
void do_mset ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	int num, size, plus;
	int v2;
	char char1, char2;
	CHAR_DATA *victim;
	int value;
	int minattr, maxattr;
	bool lockvictim;
	char *origarg = argument;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't mset\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_MOB_DESC:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_mset: sub_mob_desc: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			victim = ch->dest_buf;
			if ( char_died ( victim ) )
			{
				send_to_char ( "Your victim died!\r\n", ch );
				stop_editing ( ch );
				return;
			}
			STRFREE ( victim->description );
			victim->description = copy_buffer ( ch );
			if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			{
				STRFREE ( victim->pIndexData->description );
				victim->pIndexData->description = QUICKLINK ( victim->description );
			}
			stop_editing ( ch );
			ch->substate = ch->tempnum;
			return;
	}
	victim = NULL;
	lockvictim = FALSE;
	smash_tilde ( argument );
	if ( ch->substate == SUB_REPEATCMD )
	{
		victim = ch->dest_buf;
		if ( !victim )
		{
			send_to_char ( "Your victim died!\r\n", ch );
			argument = "done";
		}
		if ( argument[0] == STRING_NULL || !str_cmp ( argument, " " ) || !str_cmp ( argument, "stat" ) )
		{
			if ( victim )
				do_mstat ( ch, victim->name );
			else
				send_to_char ( "No victim selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if ( !str_cmp ( argument, "done" ) || !str_cmp ( argument, "off" ) )
		{
			if ( ch->dest_buf )
				RelDestroy ( relMSET_ON, ch, ch->dest_buf );
			send_to_char ( "Mset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if ( ch->pcdata && ch->pcdata->subprompt )
			{
				STRFREE ( ch->pcdata->subprompt );
				ch->pcdata->subprompt = NULL;
			}
			return;
		}
	}
	if ( victim )
	{
		lockvictim = TRUE;
		strcpy ( arg1, victim->name );
		argument = one_argument ( argument, arg2 );
		strcpy ( arg3, argument );
	}
	else
	{
		lockvictim = FALSE;
		argument = one_argument ( argument, arg1 );
		argument = one_argument ( argument, arg2 );
		strcpy ( arg3, argument );
	}
	if ( !str_cmp ( arg1, "on" ) )
	{
		send_to_char ( "Syntax: mset <victim|vnum> on.\r\n", ch );
		return;
	}
	if ( arg1[0] == STRING_NULL || ( arg2[0] == STRING_NULL && ch->substate != SUB_REPEATCMD ) || !str_cmp ( arg1, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
		{
			if ( victim )
				send_to_char ( "Syntax: <field>  <value>\r\n", ch );
			else
				send_to_char ( "Syntax: <victim> <field>  <value>\r\n", ch );
		}
		else
			send_to_char ( "Syntax: mset <victim> <field>  <value>\r\n\r\n", ch );
		send_to_char ( "  NPC fields being one of:		\r\n", ch );
		send_to_char ( "  name short long description level race Class align	\r\n", ch );
		send_to_char ( "  damplus hitplus hitroll damroll act alignment pos defpos sex	\r\n", ch );
		send_to_char ( "  height weight speaks speaking  (see LANGUAGES)\r\n", ch );
		send_to_char ( "  immune resistant susceptible (see RIS)\r\n\r\n", ch );
		send_to_char ( "  PC fields being one of:		\r\n", ch );
		send_to_char ( "  str int wis dex con cha lck	\r\n", ch );
		send_to_char ( "  gold hp mana move practice	\r\n", ch );
		send_to_char ( "  hitroll damroll armor affected level\r\n", ch );
		send_to_char ( "  thirst drunk fullflags		\r\n", ch );
		send_to_char ( "  save_poision save_mental save_physical save_weapon (see SAVINGTHROWS)\r\n", ch );
		send_to_char ( "  quest qp qpa favor deity	\r\n", ch );
		send_to_char ( "  To toggle area flag: aloaded	\r\n", ch );
		send_to_char ( "  To toggle pkill flag: pkill	\r\n", ch );
		return;
	}
	if ( !victim && get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( victim = get_char_room ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "They aren't here.\r\n", ch );
			return;
		}
	}
	else if ( !victim )
	{
		if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}
	if ( get_trust ( ch ) < get_trust ( victim ) && !IS_NPC ( victim ) )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		ch->dest_buf = NULL;
		return;
	}
	if ( get_trust ( ch ) < LEVEL_GREATER && IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_STATSHIELD ) )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		ch->dest_buf = NULL;
		return;
	}
	if ( lockvictim )
		ch->dest_buf = victim;
	if ( IS_NPC ( victim ) )
	{
		minattr = 1;
		maxattr = 25;
	}
	else
	{
		minattr = 3;
		maxattr = 25;
	}
	if ( !str_cmp ( arg2, "on" ) )
	{
		CHECK_SUBRESTRICTED ( ch );
		ch_printf ( ch, "Mset mode on. (Editing %s).\r\n", victim->name );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = victim;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE ( ch->pcdata->subprompt );
			if ( IS_NPC ( victim ) )
				sprintf ( buf, "<&CMset &W#%d&w> %%i", victim->pIndexData->vnum );
			else
				sprintf ( buf, "<&CMset &W%s&w> %%i", victim->name );
			ch->pcdata->subprompt = STRALLOC ( buf );
		}
		RelCreate ( relMSET_ON, ch, victim );
		return;
	}
	value = is_number ( arg3 ) ? atoi ( arg3 ) : -1;
	if ( atoi ( arg3 ) < -1 && value == -1 )
		value = atoi ( arg3 );
	if ( !str_cmp ( arg2, "str" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Strength range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_str = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_str = value;
		return;
	}
	if ( !str_cmp ( arg2, "int" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Intelligence range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_int = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_int = value;
		return;
	}
	if ( !str_cmp ( arg2, "wis" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Wisdom range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_wis = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_wis = value;
		return;
	}
	if ( !str_cmp ( arg2, "dex" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Dexterity range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_dex = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_dex = value;
		return;
	}
	if ( !str_cmp ( arg2, "con" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Constitution range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_con = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_con = value;
		return;
	}
	if ( !str_cmp ( arg2, "cha" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Charisma range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_cha = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_cha = value;
		return;
	}
	if ( !str_cmp ( arg2, "lck" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < minattr || value > maxattr )
		{
			ch_printf ( ch, "Luck range is %d to %d.\r\n", minattr, maxattr );
			return;
		}
		victim->perm_lck = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->perm_lck = value;
		return;
	}
	if ( !str_cmp ( arg2, "save_poison" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 20 )
		{
			send_to_char ( "Saving throw range is 0 to 20.\r\n", ch );
			return;
		}
		victim->saving_poison_death = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_poison_death = value;
		return;
	}
	if ( !str_cmp ( arg2, "save_mental" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 20 )
		{
			send_to_char ( "Saving throw range is 0 to 20.\r\n", ch );
			return;
		}
		victim->saving_mental = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_mental = value;
		return;
	}
	if ( !str_cmp ( arg2, "save_physical" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 20 )
		{
			send_to_char ( "Saving throw range is 0 to 20.\r\n", ch );
			return;
		}
		victim->saving_physical = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_physical = value;
		return;
	}
	if ( !str_cmp ( arg2, "save_weapons" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 20 )
		{
			send_to_char ( "Saving throw range is 0 to 20.\r\n", ch );
			return;
		}
		victim->saving_weapons = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->saving_weapons = value;
		return;
	}
	if ( !str_cmp ( arg2, "sex" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 2 )
		{
			send_to_char ( "Sex range is 0 to 2.\r\n", ch );
			return;
		}
		victim->sex = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->sex = value;
		return;
	}
	if ( !str_cmp ( arg2, "Class" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( IS_NPC ( victim ) )
		{
			if ( value >= MAX_NPC_CLASS || value < 0 )
			{
				ch_printf ( ch, "NPC Class range is 0 to %d.\n", MAX_NPC_CLASS - 1 );
				return;
			}
			victim->Class = value;
			if ( xIS_SET ( victim->act, ACT_PROTOTYPE ) )
				victim->pIndexData->Class = value;
			return;
		}
		if ( value < 0 || value >= MAX_CLASS )
		{
			ch_printf ( ch, "Class range is 0 to %d.\n", MAX_CLASS );
			return;
		}
		victim->Class = value;
		return;
	}
	if ( !str_cmp ( arg2, "race" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( IS_NPC ( victim ) )
			value = get_npc_race ( arg3 );
		else
			value = get_pc_race ( arg3 );
		if ( value < 0 )
			value = atoi ( arg3 );
		if ( !IS_NPC ( victim ) && ( value < 0 || value >= MAX_RACE ) )
		{
			ch_printf ( ch, "Race range is 0 to %d.\n", MAX_RACE - 1 );
			return;
		}
		if ( IS_NPC ( victim ) && ( value < 0 || value >= MAX_NPC_RACE ) )
		{
			ch_printf ( ch, "Race range is 0 to %d.\n", MAX_NPC_RACE - 1 );
			return;
		}
		victim->race = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->race = value;
		return;
	}
	if ( !str_cmp ( arg2, "armor" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < -1000 || value > 300 )
		{
			send_to_char ( "AC range is -300 to 300.\r\n", ch );
			return;
		}
		victim->armor = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->ac = value;
		return;
	}
	if ( !str_cmp ( arg2, "level" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Not on PC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > LEVEL_AVATAR + 50 )
		{
			ch_printf ( ch, "Level range is 0 to %d.\r\n", LEVEL_AVATAR + 50 );
			return;
		}
		victim->level = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->level = value;
		return;
	}
	if ( !str_cmp ( arg2, "numattacks" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Not on PC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 20 )
		{
			send_to_char ( "Attacks range is 0 to 20.\r\n", ch );
			return;
		}
		victim->numattacks = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->numattacks = value;
		return;
	}
	if ( !str_cmp ( arg2, "gold" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		victim->gold = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->gold = value;
		return;
	}
	if ( !str_cmp ( arg2, "hitroll" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		victim->hitroll = URANGE ( 0, value, 85 );
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitroll = victim->hitroll;
		return;
	}
	if ( !str_cmp ( arg2, "damroll" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		victim->damroll = URANGE ( 0, value, 65 );
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damroll = victim->damroll;
		return;
	}
	if ( !str_cmp ( arg2, "hp" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 1 || value > 32700 )
		{
			send_to_char ( "Hp range is 1 to 32,700 hit points.\r\n", ch );
			return;
		}
		victim->max_hit = value;
		return;
	}
	if ( !str_cmp ( arg2, "mana" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char ( "Mana range is 0 to 30,000 mana points.\r\n", ch );
			return;
		}
		victim->max_mana = value;
		return;
	}
	if ( !str_cmp ( arg2, "move" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char ( "Move range is 0 to 30,000 move points.\r\n", ch );
			return;
		}
		victim->max_move = value;
		return;
	}
	if ( !str_cmp ( arg2, "practice" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char ( "Practice range is 0 to 100 sessions.\r\n", ch );
			return;
		}
		victim->practice = value;
		return;
	}
	if ( !str_cmp ( arg2, "align" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < -1000 || value > 1000 )
		{
			send_to_char ( "Alignment range is -1000 to 1000.\r\n", ch );
			return;
		}
		victim->alignment = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->alignment = value;
		return;
	}
	if ( !str_cmp ( arg2, "password" ) )
	{
		char *pwdnew;
		char *p;
		if ( get_trust ( ch ) < LEVEL_SUB_IMPLEM )
		{
			send_to_char ( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Mobs don't have passwords.\r\n", ch );
			return;
		}
		if ( strlen ( arg3 ) < 5 )
		{
			send_to_char ( "New password must be at least five characters long.\r\n", ch );
			return;
		}
		if ( arg3[0] == '!' )
		{
			send_to_char ( "New password cannot begin with the '!' character.", ch );
			return;
		}
		/*
		 * No tilde allowed because of player file format.
		 */
		pwdnew = sha256_crypt ( arg3 );
		for ( p = pwdnew; *p != STRING_NULL; p++ )
		{
			if ( *p == '~' )
			{
				send_to_char ( "New password not acceptable, try again.\r\n", ch );
				return;
			}
		}
		DISPOSE ( victim->pcdata->pwd );
		victim->pcdata->pwd = str_dup ( pwdnew );
		if ( IS_SET ( sysdata.save_flags, SV_PASSCHG ) )
			save_char_obj ( victim );
		send_to_char ( "Ok.\r\n", ch );
		ch_printf ( victim, "Your password has been changed by %s.\r\n", ch->name );
		return;
	}
	if ( !str_cmp ( arg2, "rank" ) )
	{
		if ( get_trust ( ch ) < LEVEL_GOD )
		{
			send_to_char ( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		smash_tilde ( argument );
		DISPOSE ( victim->pcdata->rank );
		if ( !argument || argument[0] == STRING_NULL || !str_cmp ( argument, "none" ) )
			victim->pcdata->rank = str_dup ( "" );
		else
			victim->pcdata->rank = str_dup ( argument );
		send_to_char ( "Ok.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "quest" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 500 )
		{
			send_to_char ( "The current quest range is 0 to 500.\r\n", ch );
			return;
		}
		victim->pcdata->quest_number = value;
		return;
	}
	if ( !str_cmp ( arg2, "qpa" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		victim->pcdata->quest_accum = value;
		return;
	}
	if ( !str_cmp ( arg2, "qp" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 5000 )
		{
			send_to_char ( "The current quest point range is 0 to 5000.\r\n", ch );
			return;
		}
		victim->pcdata->quest_curr = value;
		return;
	}
	if ( !str_cmp ( arg2, "favor" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < -2500 || value > 2500 )
		{
			send_to_char ( "Range is from -2500 to 2500.\r\n", ch );
			return;
		}
		victim->pcdata->favor = value;
		return;
	}
	if ( !str_cmp ( arg2, "name" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Not on PC's.\r\n", ch );
			return;
		}
		if ( arg3[0] == STRING_NULL )
		{
			send_to_char ( "Names can not be set to an empty string.\r\n", ch );
			return;
		}
		STRFREE ( victim->name );
		victim->name = STRALLOC ( arg3 );
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE ( victim->pIndexData->player_name );
			victim->pIndexData->player_name = QUICKLINK ( victim->name );
		}
		return;
	}
	if ( !str_cmp ( arg2, "minsnoop" ) )
	{
		if ( get_trust ( ch ) < LEVEL_SUB_IMPLEM )
		{
			send_to_char ( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( victim->pcdata )
		{
			victim->pcdata->min_snoop = value;
			return;
		}
	}
	if ( !str_cmp ( arg2, "clan" ) )
	{
		CLAN_DATA *clan;
		if ( get_trust ( ch ) < LEVEL_GOD )
		{
			send_to_char ( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( arg3[0] == STRING_NULL )
		{
			/*
			 * Crash bug fix, oops guess I should have caught this one :)
			 * * But it was early in the morning :P --Shaddai
			 */
			if ( victim->pcdata->clan == NULL )
				return;
			/*
			 * Added a check on immortals so immortals don't take up
			 * * any membership space. --Shaddai
			 */
			if ( !IS_IMMORTAL ( victim ) )
			{
				--victim->pcdata->clan->members;
				save_clan ( victim->pcdata->clan );
			}
			STRFREE ( victim->pcdata->clan_name );
			victim->pcdata->clan_name = STRALLOC ( "" );
			victim->pcdata->clan = NULL;
			return;
		}
		clan = get_clan ( arg3 );
		if ( !clan )
		{
			send_to_char ( "No such clan.\r\n", ch );
			return;
		}
		if ( victim->pcdata->clan != NULL && !IS_IMMORTAL ( victim ) )
		{
			--victim->pcdata->clan->members;
			save_clan ( victim->pcdata->clan );
		}
		STRFREE ( victim->pcdata->clan_name );
		victim->pcdata->clan_name = QUICKLINK ( clan->name );
		victim->pcdata->clan = clan;
		if ( !IS_IMMORTAL ( victim ) )
		{
			++victim->pcdata->clan->members;
			save_clan ( victim->pcdata->clan );
		}
		return;
	}
	if ( !str_cmp ( arg2, "deity" ) )
	{
		DEITY_DATA *deity;
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( arg3[0] == STRING_NULL )
		{
			STRFREE ( victim->pcdata->deity_name );
			victim->pcdata->deity_name = STRALLOC ( "" );
			victim->pcdata->deity = NULL;
			send_to_char ( "Deity removed.\r\n", ch );
			return;
		}
		deity = get_deity ( arg3 );
		if ( !deity )
		{
			send_to_char ( "No such deity.\r\n", ch );
			return;
		}
		STRFREE ( victim->pcdata->deity_name );
		victim->pcdata->deity_name = QUICKLINK ( deity->name );
		victim->pcdata->deity = deity;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "short" ) )
	{
		STRFREE ( victim->short_descr );
		victim->short_descr = STRALLOC ( arg3 );
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE ( victim->pIndexData->short_descr );
			victim->pIndexData->short_descr = QUICKLINK ( victim->short_descr );
		}
		return;
	}
	if ( !str_cmp ( arg2, "long" ) )
	{
		STRFREE ( victim->long_descr );
		strcpy ( buf, arg3 );
		strcat ( buf, "\r\n" );
		victim->long_descr = STRALLOC ( buf );
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
		{
			STRFREE ( victim->pIndexData->long_descr );
			victim->pIndexData->long_descr = QUICKLINK ( victim->long_descr );
		}
		return;
	}
	if ( !str_cmp ( arg2, "description" ) )
	{
		if ( arg3[0] )
		{
			STRFREE ( victim->description );
			victim->description = STRALLOC ( arg3 );
			if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			{
				STRFREE ( victim->pIndexData->description );
				victim->pIndexData->description = QUICKLINK ( victim->description );
			}
			return;
		}
		CHECK_SUBRESTRICTED ( ch );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_MOB_DESC;
		ch->dest_buf = victim;
		start_editing ( ch, victim->description );
		return;
	}
	if ( !str_cmp ( arg2, "title" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		set_title ( victim, arg3 );
		return;
	}
	if ( !str_cmp ( arg2, "spec" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Not on PC's.\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg3, "none" ) )
		{
			victim->spec_fun = NULL;
			send_to_char ( "Special function removed.\r\n", ch );
			if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
				victim->pIndexData->spec_fun = victim->spec_fun;
			return;
		}
		if ( ( victim->spec_fun = spec_lookup ( arg3 ) ) == 0 )
		{
			send_to_char ( "No such spec fun.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->spec_fun = victim->spec_fun;
		return;
	}
	if ( !str_cmp ( arg2, "flags" ) )
	{
		bool pcflag;
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char ( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			pcflag = FALSE;
			argument = one_argument ( argument, arg3 );
			value = IS_NPC ( victim ) ? get_actflag ( arg3 ) : get_plrflag ( arg3 );
			if ( !IS_NPC ( victim ) && ( value < 0 || value > MAX_BITS ) )
			{
				pcflag = TRUE;
				value = get_pcflag ( arg3 );
			}
			if ( value < 0 || value > MAX_BITS )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( IS_NPC ( victim ) && value == ACT_PROTOTYPE && get_trust ( ch ) < 35 && !is_name ( "protoflag", ch->pcdata->bestowments ) )
					send_to_char ( "You cannot change the prototype flag.\r\n", ch );
				else if ( IS_NPC ( victim ) && value == ACT_IS_NPC )
					send_to_char ( "If that could be changed, it would cause many problems.\r\n", ch );
				else
				{
					if ( pcflag )
						TOGGLE_BIT ( victim->pcdata->flags, 1 << value );
					else
					{
						xTOGGLE_BIT ( victim->act, value );
						/*
						 * NPC check added by Gorog
						 */
						if ( IS_NPC ( victim ) && value == ACT_PROTOTYPE )
							victim->pIndexData->act = victim->act;
					}
				}
			}
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->act = victim->act;
		return;
	}
	if ( !str_cmp ( arg2, "affected" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's flags.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> affected <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_aflag ( arg3 );
			if ( value < 0 || value > MAX_BITS )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				xTOGGLE_BIT ( victim->affected_by, value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->affected_by = victim->affected_by;
		return;
	}
	/*
	 * save some more finger-leather for setting RIS stuff
	 */
	if ( !str_cmp ( arg2, "r" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "i" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s immune %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "s" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "ri" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s immune %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "rs" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "is" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s immune %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "ris" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's ris.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sprintf ( outbuf, "%s resistant %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s immune %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s susceptible %s", arg1, arg3 );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "resistant" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's resistancies.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> resistant <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_risflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( victim->resistant, 1 << value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->resistant = victim->resistant;
		return;
	}
	if ( !str_cmp ( arg2, "immune" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's immunities.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> immune <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_risflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( victim->immune, 1 << value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->immune = victim->immune;
		return;
	}
	if ( !str_cmp ( arg2, "susceptible" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's susceptibilities.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> susceptible <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_risflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( victim->susceptible, 1 << value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->susceptible = victim->susceptible;
		return;
	}
	if ( !str_cmp ( arg2, "part" ) )
	{
		if ( !IS_NPC ( victim ) && get_trust ( ch ) < LEVEL_NEOPHYTE )
		{
			send_to_char ( "You can only modify a mobile's parts.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> part <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_partflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( victim->xflags, 1 << value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->xflags = victim->xflags;
		return;
	}
	if ( !str_cmp ( arg2, "attack" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "You can only modify a mobile's attacks.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> attack <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_attackflag ( arg3 );
			if ( value < 0 || value > MAX_BITS )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				xTOGGLE_BIT ( victim->attacks, value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->attacks = victim->attacks;
		return;
	}
	if ( !str_cmp ( arg2, "defense" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "You can only modify a mobile's defenses.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> defense <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_defenseflag ( arg3 );
			if ( value < 0 || value > MAX_BITS )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				xTOGGLE_BIT ( victim->defenses, value );
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defenses = victim->defenses;
		return;
	}
	if ( !str_cmp ( arg2, "pos" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > POS_STANDING )
		{
			ch_printf ( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->position = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->position = victim->position;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "defpos" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > POS_STANDING )
		{
			ch_printf ( ch, "Position range is 0 to %d.\r\n", POS_STANDING );
			return;
		}
		victim->defposition = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->defposition = victim->defposition;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	/*
	 * save some finger-leather
	 */
	if ( !str_cmp ( arg2, "hitdie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sscanf ( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
		sprintf ( outbuf, "%s hitnumdie %d", arg1, num );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s hitsizedie %d", arg1, size );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s hitplus %d", arg1, plus );
		do_mset ( ch, outbuf );
		return;
	}
	/*
	 * save some more finger-leather
	 */
	if ( !str_cmp ( arg2, "damdie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		sscanf ( arg3, "%d %c %d %c %d", &num, &char1, &size, &char2, &plus );
		sprintf ( outbuf, "%s damnumdie %d", arg1, num );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s damsizedie %d", arg1, size );
		do_mset ( ch, outbuf );
		sprintf ( outbuf, "%s damplus %d", arg1, plus );
		do_mset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "hitnumdie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char ( "Number of hitpoint dice range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitnodice = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hitsizedie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char ( "Hitpoint dice size range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitsizedice = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hitplus" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 30000 )
		{
			send_to_char ( "Hitpoint bonus range is 0 to 30000.\r\n", ch );
			return;
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->hitplus = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "damnumdie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char ( "Number of damage dice range is 0 to 100.\r\n", ch );
			return;
		}
		victim->barenumdie = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damnodice = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "damsizedie" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 100 )
		{
			send_to_char ( "Damage dice size range is 0 to 100.\r\n", ch );
			return;
		}
		victim->baresizedie = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damsizedice = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "damplus" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Mobiles only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( value < 0 || value > 1000 )
		{
			send_to_char ( "Damage bonus range is 0 to 1000.\r\n", ch );
			return;
		}
		victim->damplus = value;
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->damplus = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "aloaded" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Player Characters only.\r\n", ch );
			return;
		}
		/*
		 * Make sure they have an area assigned -Druid
		 */
		if ( !victim->pcdata->area )
		{
			send_to_char ( "Player does not have an area assigned to them.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !IS_SET ( victim->pcdata->area->status, AREA_LOADED ) )
		{
			SET_BIT ( victim->pcdata->area->status, AREA_LOADED );
			send_to_char ( "Your area set to LOADED!\r\n", victim );
			if ( ch != victim )
				send_to_char ( "Area set to LOADED!\r\n", ch );
			return;
		}
		else
		{
			REMOVE_BIT ( victim->pcdata->area->status, AREA_LOADED );
			send_to_char ( "Your area set to NOT-LOADED!\r\n", victim );
			if ( ch != victim )
				send_to_char ( "Area set to NON-LOADED!\r\n", ch );
			return;
		}
	}
	if ( !str_cmp ( arg2, "pkill" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Player Characters only.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
		{
			send_to_char ( "You can't do that.\r\n", ch );
			return;
		}
		if ( IS_SET ( victim->pcdata->flags, PCFLAG_DEADLY ) )
		{
			REMOVE_BIT ( victim->pcdata->flags, PCFLAG_DEADLY );
			xSET_BIT ( victim->act, PLR_NICE );
			send_to_char ( "You are now a NON-PKILL player.\r\n", victim );
			if ( ch != victim )
				send_to_char ( "That player is now non-pkill.\r\n", ch );
		}
		else
		{
			SET_BIT ( victim->pcdata->flags, PCFLAG_DEADLY );
			xREMOVE_BIT ( victim->act, PLR_NICE );
			send_to_char ( "You are now a PKILL player.\r\n", victim );
			if ( ch != victim )
				send_to_char ( "That player is now pkill.\r\n", ch );
		}
		if ( victim->pcdata->clan && !IS_IMMORTAL ( victim ) )
		{
			if ( victim->speaking & LANG_CLAN )
				victim->speaking = LANG_COMMON;
			REMOVE_BIT ( victim->speaks, LANG_CLAN );
			--victim->pcdata->clan->members;
			if ( !str_cmp ( victim->name, victim->pcdata->clan->leader ) )
			{
				STRFREE ( victim->pcdata->clan->leader );
				victim->pcdata->clan->leader = STRALLOC ( "" );
			}
			if ( !str_cmp ( victim->name, victim->pcdata->clan->number1 ) )
			{
				STRFREE ( victim->pcdata->clan->number1 );
				victim->pcdata->clan->number1 = STRALLOC ( "" );
			}
			if ( !str_cmp ( victim->name, victim->pcdata->clan->number2 ) )
			{
				STRFREE ( victim->pcdata->clan->number2 );
				victim->pcdata->clan->number2 = STRALLOC ( "" );
			}
			save_clan ( victim->pcdata->clan );
			STRFREE ( victim->pcdata->clan_name );
			victim->pcdata->clan_name = STRALLOC ( "" );
			victim->pcdata->clan = NULL;
		}
		save_char_obj ( victim );
		return;
	}
	if ( !str_cmp ( arg2, "speaks" ) )
	{
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> speaks <language> [language] ...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_langflag ( arg3 );
			if ( value == LANG_UNKNOWN )
				ch_printf ( ch, "Unknown language: %s\r\n", arg3 );
			else if ( !IS_NPC ( victim ) )
			{
				int valid_langs = LANG_COMMON | LANG_CLAN | LANG_UNKNOWN;
				if ( ! ( value &= valid_langs ) )
				{
					ch_printf ( ch, "Players may not know %s.\r\n", arg3 );
					continue;
				}
			}
			v2 = get_langnum ( arg3 );
			if ( v2 == -1 )
				ch_printf ( ch, "Unknown language: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( victim->speaks, 1 << v2 );
		}
		if ( !IS_NPC ( victim ) )
		{
			REMOVE_BIT ( victim->speaks, race_table[victim->race]->language );
			if ( !knows_language ( victim, victim->speaking, victim ) )
				victim->speaking = race_table[victim->race]->language;
		}
		else if ( xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->speaks = victim->speaks;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "nextquest" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 30 )
		{
			send_to_char ( "The current quest range is 0 to 30.\r\n", ch );
			return;
		}
		victim->pcdata->nextquest = value;
		return;
	}
	if ( !str_cmp ( arg2, "countdown" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 1 || value > 30 )
		{
			send_to_char ( "The current quest range is 1 to 30.\r\n", ch );
			return;
		}
		victim->pcdata->countdown = value;
		return;
	}
	if ( !str_cmp ( arg2, "questobj" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 2000000 )
		{
			send_to_char ( "The current object target range is 0 to 2000000.\r\n", ch );
			return;
		}
		victim->pcdata->questobj = value;
		victim->pcdata->questmob = 0;
		return;
	}
	if ( !str_cmp ( arg2, "questmob" ) )
	{
		if ( IS_NPC ( victim ) )
		{
			send_to_char ( "Not on NPC's.\r\n", ch );
			return;
		}
		if ( value < 0 || value > 2000000 )
		{
			send_to_char ( "The current mob target range is 0 to 2000000.\r\n", ch );
			return;
		}
		victim->pcdata->questmob = value;
		victim->pcdata->questobj = 0;
		return;
	}
	if ( !str_cmp ( arg2, "speaking" ) )
	{
		if ( !IS_NPC ( victim ) )
		{
			send_to_char ( "Players must choose the language they speak themselves.\r\n", ch );
			return;
		}
		if ( !can_mmodify ( ch, victim ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: mset <victim> speaking <language> [language]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_langflag ( arg3 );
			if ( value == LANG_UNKNOWN )
				ch_printf ( ch, "Unknown language: %s\r\n", arg3 );
			else
			{
				v2 = get_langnum ( arg3 );
				if ( v2 == -1 )
					ch_printf ( ch, "Unknown language: %s\r\n", arg3 );
				else
					TOGGLE_BIT ( victim->speaking, 1 << v2 );
			}
		}
		if ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
			victim->pIndexData->speaking = victim->speaking;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret ( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_mset;
	}
	else
		do_mset ( ch, "" );
	return;
}
void do_oset ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	char outbuf[MAX_STRING_LENGTH];
	OBJ_DATA *obj, *tmpobj;
	EXTRA_DESCR_DATA *ed;
	bool lockobj;
	char *origarg = argument;
	int value, tmp;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't oset\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_OBJ_EXTRA:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_oset: sub_obj_extra: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			/*
			 * hopefully the object didn't get extracted...
			 * if you're REALLY paranoid, you could always go through
			 * the object and index-object lists, searching through the
			 * extra_descr lists for a matching pointer...
			 */
			ed = ch->dest_buf;
			STRFREE ( ed->description );
			ed->description = copy_buffer ( ch );
			tmpobj = ch->spare_ptr;
			stop_editing ( ch );
			ch->dest_buf = tmpobj;
			ch->substate = ch->tempnum;
			return;
		case SUB_OBJ_LONG:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_oset: sub_obj_long: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			obj = ch->dest_buf;
			if ( obj && obj_extracted ( obj ) )
			{
				send_to_char ( "Your object was extracted!\r\n", ch );
				stop_editing ( ch );
				return;
			}
			STRFREE ( obj->description );
			obj->description = copy_buffer ( ch );
			if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			{
				if ( can_omodify ( ch, obj ) )
				{
					STRFREE ( obj->pIndexData->description );
					obj->pIndexData->description = QUICKLINK ( obj->description );
				}
			}
			tmpobj = ch->spare_ptr;
			stop_editing ( ch );
			ch->substate = ch->tempnum;
			ch->dest_buf = tmpobj;
			return;
	}
	obj = NULL;
	smash_tilde ( argument );
	if ( ch->substate == SUB_REPEATCMD )
	{
		obj = ch->dest_buf;
		if ( !obj )
		{
			send_to_char ( "Your object was extracted!\r\n", ch );
			argument = "done";
		}
		if ( argument[0] == STRING_NULL || !str_cmp ( argument, " " ) || !str_cmp ( argument, "stat" ) )
		{
			if ( obj )
				do_ostat ( ch, obj->name );
			else
				send_to_char ( "No object selected.  Type '?' for help.\r\n", ch );
			return;
		}
		if ( !str_cmp ( argument, "done" ) || !str_cmp ( argument, "off" ) )
		{
			if ( ch->dest_buf )
				RelDestroy ( relOSET_ON, ch, ch->dest_buf );
			send_to_char ( "Oset mode off.\r\n", ch );
			ch->substate = SUB_NONE;
			ch->dest_buf = NULL;
			if ( ch->pcdata && ch->pcdata->subprompt )
			{
				STRFREE ( ch->pcdata->subprompt );
				ch->pcdata->subprompt = NULL;
			}
			return;
		}
	}
	if ( obj )
	{
		lockobj = TRUE;
		strcpy ( arg1, obj->name );
		argument = one_argument ( argument, arg2 );
		strcpy ( arg3, argument );
	}
	else
	{
		lockobj = FALSE;
		argument = one_argument ( argument, arg1 );
		argument = one_argument ( argument, arg2 );
		strcpy ( arg3, argument );
	}
	if ( !str_cmp ( arg1, "on" ) )
	{
		send_to_char ( "Syntax: oset <object|vnum> on.\r\n", ch );
		return;
	}
	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL || !str_cmp ( arg1, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
		{
			if ( obj )
				send_to_char ( "Syntax: <field>  <value>\r\n", ch );
			else
				send_to_char ( "Syntax: <object> <field>  <value>\r\n", ch );
		}
		else
			send_to_char ( "Syntax: oset <object> <field>  <value>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Field being one of:\r\n", ch );
		send_to_char ( "  flags wear level weight cost rent timer\r\n", ch );
		send_to_char ( "  name short long ed rmed actiondesc\r\n", ch );
		send_to_char ( "  type value0 value1 value2 value3 value4 value5\r\n", ch );
		send_to_char ( "  affect rmaffect layers\r\n", ch );
		send_to_char ( "For weapons:             For armor:\r\n", ch );
		send_to_char ( "  weapontype condition     ac condition\r\n", ch );
		send_to_char ( "For scrolls, potions and pills:\r\n", ch );
		send_to_char ( "  slevel spell1 spell2 spell3\r\n", ch );
		send_to_char ( "For wands and staves:\r\n", ch );
		send_to_char ( "  slevel spell maxcharges charges\r\n", ch );
		send_to_char ( "For containers:          For levers and switches:\r\n", ch );
		send_to_char ( "  cflags key capacity      tflags\r\n", ch );
		return;
	}
	if ( !obj && get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( obj = get_obj_here ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "You can't find that here.\r\n", ch );
			return;
		}
	}
	else if ( !obj )
	{
		if ( ( obj = get_obj_world ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "There is nothing like that in all the realms.\r\n", ch );
			return;
		}
	}
	if ( lockobj )
		ch->dest_buf = obj;
	separate_obj ( obj );
	value = atoi ( arg3 );
	if ( !str_cmp ( arg2, "on" ) )
	{
		CHECK_SUBRESTRICTED ( ch );
		ch_printf ( ch, "Oset mode on. (Editing '%s' vnum %d).\r\n", obj->name, obj->pIndexData->vnum );
		ch->substate = SUB_REPEATCMD;
		ch->dest_buf = obj;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE ( ch->pcdata->subprompt );
			sprintf ( buf, "<&COset &W#%d&w> %%i", obj->pIndexData->vnum );
			ch->pcdata->subprompt = STRALLOC ( buf );
		}
		RelCreate ( relOSET_ON, ch, obj );
		return;
	}
	if ( !str_cmp ( arg2, "name" ) )
	{
		bool proto = FALSE;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			proto = TRUE;
		if ( proto && !can_omodify ( ch, obj ) )
			return;
		STRFREE ( obj->name );
		obj->name = STRALLOC ( arg3 );
		if ( proto )
		{
			STRFREE ( obj->pIndexData->name );
			obj->pIndexData->name = QUICKLINK ( obj->name );
		}
		return;
	}
	if ( !str_cmp ( arg2, "short" ) )
	{
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		{
			if ( !can_omodify ( ch, obj ) )
				return;
			STRFREE ( obj->short_descr );
			obj->short_descr = STRALLOC ( arg3 );
			STRFREE ( obj->pIndexData->short_descr );
			obj->pIndexData->short_descr = QUICKLINK ( obj->short_descr );
		}
		else
			/*
			 * Feature added by Narn, Apr/96
			 * * If the item is not proto, add the word 'rename' to the keywords
			 * * if it is not already there.
			 */
		{
			STRFREE ( obj->short_descr );
			obj->short_descr = STRALLOC ( arg3 );
			if ( str_infix ( "rename", obj->name ) )
			{
				sprintf ( buf, "%s %s", obj->name, "rename" );
				STRFREE ( obj->name );
				obj->name = STRALLOC ( buf );
			}
		}
		return;
	}
	if ( !str_cmp ( arg2, "long" ) )
	{
		if ( arg3[0] )
		{
			if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			{
				if ( !can_omodify ( ch, obj ) )
					return;
				STRFREE ( obj->description );
				obj->description = STRALLOC ( arg3 );
				STRFREE ( obj->pIndexData->description );
				obj->pIndexData->description = QUICKLINK ( obj->description );
				return;
			}
			STRFREE ( obj->description );
			obj->description = STRALLOC ( arg3 );
			return;
		}
		CHECK_SUBRESTRICTED ( ch );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_LONG;
		ch->dest_buf = obj;
		start_editing ( ch, obj->description );
		return;
	}
	if ( !str_cmp ( arg2, "ed" ) )
	{
		if ( arg3[0] == STRING_NULL )
		{
			send_to_char ( "Syntax: oset <object> ed <keywords>\r\n", ch );
			return;
		}
		CHECK_SUBRESTRICTED ( ch );
		if ( obj->timer )
		{
			send_to_char ( "It's not safe to edit an extra description on an object with a timer.\r\nTurn it off first.\r\n", ch );
			return;
		}
		if ( obj->item_type == ITEM_PAPER && get_trust ( ch ) < LEVEL_IMPLEMENTOR )
		{
			send_to_char ( "You can not add an extra description to a note paper at the moment.\r\n", ch );
			return;
		}
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			ed = SetOExtraProto ( obj->pIndexData, arg3 );
		else
			ed = SetOExtra ( obj, arg3 );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		if ( lockobj )
			ch->spare_ptr = obj;
		else
			ch->spare_ptr = NULL;
		ch->substate = SUB_OBJ_EXTRA;
		ch->dest_buf = ed;
		start_editing ( ch, ed->description );
		return;
	}
	if ( !str_cmp ( arg2, "rmed" ) )
	{
		if ( arg3[0] == STRING_NULL )
		{
			send_to_char ( "Syntax: oset <object> rmed <keywords>\r\n", ch );
			return;
		}
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		{
			if ( DelOExtraProto ( obj->pIndexData, arg3 ) )
				send_to_char ( "Deleted.\r\n", ch );
			else
				send_to_char ( "Not found.\r\n", ch );
			return;
		}
		if ( DelOExtra ( obj, arg3 ) )
			send_to_char ( "Deleted.\r\n", ch );
		else
			send_to_char ( "Not found.\r\n", ch );
		return;
	}
	if ( get_trust ( ch ) < LEVEL_DEMI )
	{
		send_to_char ( "You can only oset the name, short and long right now.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "value0" ) || !str_cmp ( arg2, "v0" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[0] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[0] = value;
		return;
	}
	if ( !str_cmp ( arg2, "value1" ) || !str_cmp ( arg2, "v1" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[1] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[1] = value;
		return;
	}
	if ( !str_cmp ( arg2, "value2" ) || !str_cmp ( arg2, "v2" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[2] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		{
			obj->pIndexData->value[2] = value;
			if ( obj->item_type == ITEM_WEAPON && value != 0 )
				obj->value[2] = obj->pIndexData->value[1] * obj->pIndexData->value[2];
		}
		return;
	}
	if ( !str_cmp ( arg2, "value3" ) || !str_cmp ( arg2, "v3" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[3] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[3] = value;
		return;
	}
	if ( !str_cmp ( arg2, "value4" ) || !str_cmp ( arg2, "v4" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[4] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[4] = value;
		return;
	}
	if ( !str_cmp ( arg2, "value5" ) || !str_cmp ( arg2, "v5" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[5] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[5] = value;
		return;
	}
	if ( !str_cmp ( arg2, "type" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: oset <object> type <type>\r\n", ch );
			return;
		}
		value = get_otype ( argument );
		if ( value < 1 )
		{
			ch_printf ( ch, "Unknown type: %s\r\n", arg3 );
			return;
		}
		obj->item_type = ( short ) value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->item_type = obj->item_type;
		return;
	}
	if ( !str_cmp ( arg2, "flags" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: oset <object> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_oflag ( arg3 );
			if ( value < 0 || value > MAX_BITS )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( value == ITEM_PROTOTYPE && get_trust ( ch ) < 35 && !is_name ( "protoflag", ch->pcdata->bestowments ) )
					send_to_char ( "You cannot change the prototype flag.\r\n", ch );
				else
				{
					xTOGGLE_BIT ( obj->extra_flags, value );
					if ( value == ITEM_PROTOTYPE )
						obj->pIndexData->extra_flags = obj->extra_flags;
				}
			}
		}
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->extra_flags = obj->extra_flags;
		return;
	}
	if ( !str_cmp ( arg2, "wear" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: oset <object> wear <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_wflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
				TOGGLE_BIT ( obj->wear_flags, 1 << value );
		}
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->wear_flags = obj->wear_flags;
		return;
	}
	if ( !str_cmp ( arg2, "level" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->level = value;
		return;
	}
	if ( !str_cmp ( arg2, "weight" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->weight = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->weight = value;
		return;
	}
	if ( !str_cmp ( arg2, "cost" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->cost = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->cost = value;
		return;
	}
	if ( !str_cmp ( arg2, "rent" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->rent = value;
		else
			send_to_char ( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "layers" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->layers = value;
		else
			send_to_char ( "Item must have prototype flag to set this value.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "timer" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->timer = value;
		return;
	}
	if ( !str_cmp ( arg2, "actiondesc" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( strstr ( arg3, "%n" ) || strstr ( arg3, "%d" ) || strstr ( arg3, "%l" ) )
		{
			send_to_char ( "Illegal characters!\r\n", ch );
			return;
		}
		STRFREE ( obj->action_desc );
		obj->action_desc = STRALLOC ( arg3 );
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		{
			STRFREE ( obj->pIndexData->action_desc );
			obj->pIndexData->action_desc = QUICKLINK ( obj->action_desc );
		}
		return;
	}
	/*
	 * Crash fix and name support by Shaddai
	 */
	if ( !str_cmp ( arg2, "affect" ) )
	{
		AFFECT_DATA *paf;
		short loc;
		int bitv;
		if ( !can_omodify ( ch, obj ) )
			return;
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL || !argument || argument[0] == 0 )
		{
			send_to_char ( "Usage: oset <object> affect <field> <value>\r\n", ch );
			return;
		}
		loc = get_atype ( arg2 );
		if ( loc < 1 )
		{
			ch_printf ( ch, "Unknown field: %s\r\n", arg2 );
			return;
		}
		if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
		{
			bitv = 0;
			while ( argument[0] != STRING_NULL )
			{
				argument = one_argument ( argument, arg3 );
				if ( loc == APPLY_AFFECT )
					value = get_aflag ( arg3 );
				else
					value = get_risflag ( arg3 );
				if ( value < 0 || value > 31 )
					ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
				else
					SET_BIT ( bitv, 1 << value );
			}
			if ( !bitv )
				return;
			value = bitv;
		}
		else
		{
			one_argument ( argument, arg3 );
			if ( loc == APPLY_WEARSPELL && !is_number ( arg3 ) )
			{
				value = bsearch_skill_exact ( arg3, gsn_first_spell, gsn_first_skill - 1 );
				if ( value == -1 )
				{
					/*
					 * printf("%s\r\n", arg3);
					 */
					send_to_char ( "Unknown spell name.\r\n", ch );
					return;
				}
			}
			else
				value = atoi ( arg3 );
		}
		CREATE ( paf, AFFECT_DATA, 1 );
		paf->type = -1;
		paf->duration = -1;
		paf->location = loc;
		paf->modifier = value;
		xCLEAR_BITS ( paf->bitvector );
		paf->next = NULL;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			LINK ( paf, obj->pIndexData->first_affect, obj->pIndexData->last_affect, next, prev );
		else
			LINK ( paf, obj->first_affect, obj->last_affect, next, prev );
		++top_affect;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "rmaffect" ) )
	{
		AFFECT_DATA *paf;
		short loc, count;
		if ( !can_omodify ( ch, obj ) )
			return;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: oset <object> rmaffect <affect#>\r\n", ch );
			return;
		}
		loc = atoi ( argument );
		if ( loc < 1 )
		{
			send_to_char ( "Invalid number.\r\n", ch );
			return;
		}
		count = 0;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		{
			OBJ_INDEX_DATA *pObjIndex;
			pObjIndex = obj->pIndexData;
			for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
			{
				if ( ++count == loc )
				{
					UNLINK ( paf, pObjIndex->first_affect, pObjIndex->last_affect, next, prev );
					DISPOSE ( paf );
					send_to_char ( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
			send_to_char ( "Not found.\r\n", ch );
			return;
		}
		else
		{
			for ( paf = obj->first_affect; paf; paf = paf->next )
			{
				if ( ++count == loc )
				{
					UNLINK ( paf, obj->first_affect, obj->last_affect, next, prev );
					DISPOSE ( paf );
					send_to_char ( "Removed.\r\n", ch );
					--top_affect;
					return;
				}
			}
			send_to_char ( "Not found.\r\n", ch );
			return;
		}
	}
	/*
	 * save some finger-leather
	 */
	if ( !str_cmp ( arg2, "ris" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect resistant %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		sprintf ( outbuf, "%s affect immune %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		sprintf ( outbuf, "%s affect susceptible %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "r" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect resistant %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "i" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect immune %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "s" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect susceptible %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "ri" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect resistant %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		sprintf ( outbuf, "%s affect immune %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "rs" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect resistant %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		sprintf ( outbuf, "%s affect susceptible %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	if ( !str_cmp ( arg2, "is" ) )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		sprintf ( outbuf, "%s affect immune %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		sprintf ( outbuf, "%s affect susceptible %s", arg1, arg3 );
		do_oset ( ch, outbuf );
		return;
	}
	/*
	 * Make it easier to set special object values by name than number
	 *                 -Thoric
	 */
	tmp = -1;
	switch ( obj->item_type )
	{
		case ITEM_PROJECTILE:
			if ( !str_cmp ( arg2, "missiletype" ) )
			{
				unsigned int x;
				value = -1;
				for ( x = 0; x < sizeof ( projectiles ) / sizeof ( projectiles[0] ); x++ )
					if ( !str_cmp ( arg3, projectiles[x] ) )
						value = x;
				if ( value < 0 )
				{
					send_to_char ( "Unknown projectile type.\r\n", ch );
					return;
				}
				tmp = 4;
				break;
			}
			if ( !str_cmp ( arg2, "damtype" ) )
			{
				unsigned int x;
				value = -1;
				for ( x = 0; x < sizeof ( attack_table ) / sizeof ( attack_table[0] ); x++ )
					if ( !str_cmp ( arg3, attack_table[x] ) )
						value = x;
				if ( value < 0 )
				{
					send_to_char ( "Unknown damage type.\r\n", ch );
					return;
				}
				tmp = 3;
				break;
			}
		case ITEM_WEAPON:
			if ( !str_cmp ( arg2, "weapontype" ) )
			{
				unsigned int x;
				value = -1;
				for ( x = 0; x < sizeof ( weapon_skills ) / sizeof ( weapon_skills[0] ); x++ )
					if ( !str_cmp ( arg3, weapon_skills[x] ) )
						value = x;
				if ( value < 0 )
				{
					send_to_char ( "Unknown weapon type.\r\n", ch );
					return;
				}
				tmp = 4;
				break;
			}
			if ( !str_cmp ( arg2, "damtype" ) )
			{
				unsigned int x;
				value = -1;
				for ( x = 0; x < sizeof ( attack_table ) / sizeof ( attack_table[0] ); x++ )
					if ( !str_cmp ( arg3, attack_table[x] ) )
						value = x;
				if ( value < 0 )
				{
					send_to_char ( "Unknown damage type.\r\n", ch );
					return;
				}
				tmp = 3;
				break;
			}
			if ( !str_cmp ( arg2, "condition" ) )
				tmp = 0;
			break;
		case ITEM_ARMOR:
			if ( !str_cmp ( arg2, "condition" ) )
				tmp = 3;
			if ( !str_cmp ( arg2, "ac" ) )
				tmp = 1;
			break;
		case ITEM_SALVE:
			if ( !str_cmp ( arg2, "slevel" ) )
				tmp = 0;
			if ( !str_cmp ( arg2, "maxdoses" ) )
				tmp = 1;
			if ( !str_cmp ( arg2, "doses" ) )
				tmp = 2;
			if ( !str_cmp ( arg2, "delay" ) )
				tmp = 3;
			if ( !str_cmp ( arg2, "spell1" ) )
				tmp = 4;
			if ( !str_cmp ( arg2, "spell2" ) )
				tmp = 5;
			if ( tmp >= 4 && tmp <= 5 )
				value = skill_lookup ( arg3 );
			break;
		case ITEM_SCROLL:
		case ITEM_POTION:
		case ITEM_PILL:
			if ( !str_cmp ( arg2, "slevel" ) )
				tmp = 0;
			if ( !str_cmp ( arg2, "spell1" ) )
				tmp = 1;
			if ( !str_cmp ( arg2, "spell2" ) )
				tmp = 2;
			if ( !str_cmp ( arg2, "spell3" ) )
				tmp = 3;
			if ( tmp >= 1 && tmp <= 3 )
				value = skill_lookup ( arg3 );
			break;
		case ITEM_STAFF:
		case ITEM_WAND:
			if ( !str_cmp ( arg2, "slevel" ) )
				tmp = 0;
			if ( !str_cmp ( arg2, "spell" ) )
			{
				tmp = 3;
				value = skill_lookup ( arg3 );
			}
			if ( !str_cmp ( arg2, "maxcharges" ) )
				tmp = 1;
			if ( !str_cmp ( arg2, "charges" ) )
				tmp = 2;
			break;
		case ITEM_CONTAINER:
			if ( !str_cmp ( arg2, "capacity" ) )
				tmp = 0;
			if ( !str_cmp ( arg2, "cflags" ) )
				tmp = 1;
			if ( !str_cmp ( arg2, "key" ) )
				tmp = 2;
			break;
		case ITEM_SWITCH:
		case ITEM_LEVER:
		case ITEM_PULLCHAIN:
		case ITEM_BUTTON:
			if ( !str_cmp ( arg2, "tflags" ) )
			{
				tmp = 0;
				value = get_trigflag ( arg3 );
			}
			break;
	}
	if ( tmp >= 0 && tmp <= 3 )
	{
		if ( !can_omodify ( ch, obj ) )
			return;
		obj->value[tmp] = value;
		if ( IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
			obj->pIndexData->value[tmp] = value;
		return;
	}
	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret ( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_oset;
	}
	else
		do_oset ( ch, "" );
	return;
}

/*
 * Returns value 0 - 9 based on directional text.
 */
int get_dir ( char *txt )
{
	int edir;
	char c1, c2;
	if ( !str_cmp ( txt, "northeast" ) )
		return DIR_NORTHEAST;
	if ( !str_cmp ( txt, "northwest" ) )
		return DIR_NORTHWEST;
	if ( !str_cmp ( txt, "southeast" ) )
		return DIR_SOUTHEAST;
	if ( !str_cmp ( txt, "southwest" ) )
		return DIR_SOUTHWEST;
	if ( !str_cmp ( txt, "somewhere" ) )
		return 10;
	c1 = txt[0];
	if ( c1 == STRING_NULL )
		return 0;
	c2 = txt[1];
	edir = 0;
	switch ( c1 )
	{
		case 'n':
			switch ( c2 )
			{
				default:
					edir = 0;
					break;   /* north */
				case 'e':
					edir = 6;
					break;   /* ne   */
				case 'w':
					edir = 7;
					break;   /* nw   */
			}
			break;
		case '0':
			edir = 0;
			break;   /* north */
		case 'e':
		case '1':
			edir = 1;
			break;   /* east  */
		case 's':
			switch ( c2 )
			{
				default:
					edir = 2;
					break;   /* south */
				case 'e':
					edir = 8;
					break;   /* se   */
				case 'w':
					edir = 9;
					break;   /* sw   */
			}
			break;
		case '2':
			edir = 2;
			break;   /* south */
		case 'w':
		case '3':
			edir = 3;
			break;   /* west  */
		case 'u':
		case '4':
			edir = 4;
			break;   /* up    */
		case 'd':
		case '5':
			edir = 5;
			break;   /* down  */
		case '6':
			edir = 6;
			break;   /* ne    */
		case '7':
			edir = 7;
			break;   /* nw    */
		case '8':
			edir = 8;
			break;   /* se    */
		case '9':
			edir = 9;
			break;   /* sw    */
		case '?':
			edir = 10;
			break;   /* somewhere */
	}
	return edir;
}
void do_rset ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	ROOM_INDEX_DATA *location, *tmp;
	EXTRA_DESCR_DATA *ed;
	char dir = 0;
	EXIT_DATA *xit, *texit;
	int value;
	int edir = 0, ekey, evnum;
	char *origarg = argument;
	set_char_color ( AT_PLAIN, ch );
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor.\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_ROOM_DESC:
			location = ch->dest_buf;
			if ( !location )
			{
				bug ( "rset: sub_room_desc: NULL ch->dest_buf", 0 );
				location = ch->in_room;
			}
			STRFREE ( location->description );
			location->description = copy_buffer ( ch );
			stop_editing ( ch );
			ch->substate = ch->tempnum;
			return;
		case SUB_ROOM_EXTRA:
			ed = ch->dest_buf;
			if ( !ed )
			{
				bug ( "rset: sub_room_extra: NULL ch->dest_buf", 0 );
				stop_editing ( ch );
				return;
			}
			STRFREE ( ed->description );
			ed->description = copy_buffer ( ch );
			stop_editing ( ch );
			ch->substate = ch->tempnum;
			return;
	}
	location = ch->in_room;
	smash_tilde ( argument );
	argument = one_argument ( argument, arg );
	if ( ch->substate == SUB_REPEATCMD )
	{
		if ( arg[0] == STRING_NULL )
		{
			do_rstat ( ch, "" );
			return;
		}
		if ( !str_cmp ( arg, "done" ) || !str_cmp ( arg, "off" ) )
		{
			send_to_char ( "Redit mode off.\r\n", ch );
			if ( ch->pcdata && ch->pcdata->subprompt )
			{
				STRFREE ( ch->pcdata->subprompt );
				ch->pcdata->subprompt = NULL;
			}
			ch->substate = SUB_NONE;
			return;
		}
	}
	if ( arg[0] == STRING_NULL || !str_cmp ( arg, "?" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
			send_to_char ( "Syntax: <field> value\r\n", ch );
		else
			send_to_char ( "Syntax: rset <field> value\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Field being one of:\r\n", ch );
		send_to_char ( "  name desc ed rmed\r\n", ch );
		send_to_char ( "  exit bexit exdesc excoord exflags exname exkey\r\n", ch );
		send_to_char ( "  flags sector teledelay televnum tunnel\r\n", ch );
		send_to_char ( "  rlist exdistance pulltype pull push\r\n", ch );
		return;
	}
	if ( !can_rmodify ( ch, location ) )
		return;
	if ( !str_cmp ( arg, "on" ) )
	{
		CHECK_SUBRESTRICTED ( ch );
		send_to_char ( "Redit mode on.\r\n", ch );
		ch->substate = SUB_REPEATCMD;
		if ( ch->pcdata )
		{
			if ( ch->pcdata->subprompt )
				STRFREE ( ch->pcdata->subprompt );
			ch->pcdata->subprompt = STRALLOC ( "<&CRedit &W#%r&w> %i" );
		}
		return;
	}
	if ( !str_cmp ( arg, "name" ) )
	{
		if ( argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the room name.  A very brief single line room description.\r\n", ch );
			send_to_char ( "Usage: rset name <Room summary>\r\n", ch );
			return;
		}
		STRFREE ( location->name );
		location->name = STRALLOC ( argument );
		return;
	}
	if ( !str_cmp ( arg, "desc" ) )
	{
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_DESC;
		ch->dest_buf = location;
		start_editing ( ch, location->description );
		return;
	}
	if ( !str_cmp ( arg, "tunnel" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the maximum characters allowed in the room at one time. (0 = unlimited).\r\n", ch );
			send_to_char ( "Usage: rset tunnel <value>\r\n", ch );
			return;
		}
		location->tunnel = URANGE ( 0, atoi ( argument ), 1000 );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	/*
	 * Crash fix and name support by Shaddai
	 */
	if ( !str_cmp ( arg, "affect" ) )
	{
		AFFECT_DATA *paf;
		short loc;
		int bitv;
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL || !argument || argument[0] == 0 )
		{
			send_to_char ( "Usage: rset affect <field> <value>\r\n", ch );
			return;
		}
		loc = get_atype ( arg2 );
		if ( loc < 1 )
		{
			ch_printf ( ch, "Unknown field: %s\r\n", arg2 );
			return;
		}
		if ( loc >= APPLY_AFFECT && loc < APPLY_WEAPONSPELL )
		{
			bitv = 0;
			while ( argument[0] != STRING_NULL )
			{
				argument = one_argument ( argument, arg3 );
				if ( loc == APPLY_AFFECT )
					value = get_aflag ( arg3 );
				else
					value = get_risflag ( arg3 );
				if ( value < 0 || value > 31 )
					ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
				else
					SET_BIT ( bitv, 1 << value );
			}
			if ( !bitv )
				return;
			value = bitv;
		}
		else
		{
			one_argument ( argument, arg3 );
			if ( loc == APPLY_WEARSPELL && !is_number ( arg3 ) )
			{
				value = bsearch_skill_exact ( arg3, gsn_first_spell, gsn_first_skill - 1 );
				if ( value == -1 )
				{
					/*
					 * printf("%s\r\n", arg3);
					 */
					send_to_char ( "Unknown spell name.\r\n", ch );
					return;
				}
			}
			else
				value = atoi ( arg3 );
		}
		CREATE ( paf, AFFECT_DATA, 1 );
		paf->type = -1;
		paf->duration = -1;
		paf->location = loc;
		paf->modifier = value;
		xCLEAR_BITS ( paf->bitvector );
		paf->next = NULL;
		LINK ( paf, location->first_affect, location->last_affect, next, prev );
		++top_affect;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "rmaffect" ) )
	{
		AFFECT_DATA *paf;
		short loc, count;
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: rset rmaffect <affect#>\r\n", ch );
			return;
		}
		loc = atoi ( argument );
		if ( loc < 1 )
		{
			send_to_char ( "Invalid number.\r\n", ch );
			return;
		}
		count = 0;
		for ( paf = location->first_affect; paf; paf = paf->next )
		{
			if ( ++count == loc )
			{
				UNLINK ( paf, location->first_affect, location->last_affect, next, prev );
				DISPOSE ( paf );
				send_to_char ( "Removed.\r\n", ch );
				--top_affect;
				return;
			}
		}
		send_to_char ( "Not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "ed" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Create an extra description.\r\n", ch );
			send_to_char ( "You must supply keyword(s).\r\n", ch );
			return;
		}
		CHECK_SUBRESTRICTED ( ch );
		ed = SetRExtra ( location, argument );
		if ( ch->substate == SUB_REPEATCMD )
			ch->tempnum = SUB_REPEATCMD;
		else
			ch->tempnum = SUB_NONE;
		ch->substate = SUB_ROOM_EXTRA;
		ch->dest_buf = ed;
		start_editing ( ch, ed->description );
		return;
	}
	if ( !str_cmp ( arg, "rmed" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Remove an extra description.\r\n", ch );
			send_to_char ( "You must supply keyword(s).\r\n", ch );
			return;
		}
		if ( DelRExtra ( location, argument ) )
			send_to_char ( "Deleted.\r\n", ch );
		else
			send_to_char ( "Not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "rlist" ) )
	{
		RESET_DATA *pReset;
		char *rbuf;
		short num;
		if ( !location->first_reset )
		{
			send_to_char ( "This room has no resets to list.\r\n", ch );
			return;
		}
		num = 0;
		for ( pReset = location->first_reset; pReset; pReset = pReset->next )
		{
			++num;
			if ( ! ( rbuf = sprint_reset ( pReset, &num ) ) )
				continue;
			send_to_char ( rbuf, ch );
		}
		return;
	}
	if ( !str_cmp ( arg, "flags" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Toggle the room flags.\r\n", ch );
			send_to_char ( "Usage: rset flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg2 );
			value = get_rflag ( arg2 );
			if ( value < 0 || value > 40 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg2 );
			else
			{
				if ( 1 << value == ROOM_PROTOTYPE && get_trust ( ch ) < 35 )
					send_to_char ( "You cannot change the prototype flag.\r\n", ch );
				else
					xTOGGLE_BIT ( location->room_flags, value );
			}
		}
		return;
	}
	if ( !str_cmp ( arg, "teledelay" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the delay of the teleport. (0 = off).\r\n", ch );
			send_to_char ( "Usage: rset teledelay <value>\r\n", ch );
			return;
		}
		location->tele_delay = atoi ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "televnum" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the vnum of the room to teleport to.\r\n", ch );
			send_to_char ( "Usage: rset televnum <vnum>\r\n", ch );
			return;
		}
		location->tele_vnum = atoi ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "sector" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the sector type.\r\n", ch );
			send_to_char ( "Usage: rset sector <value>\r\n", ch );
			return;
		}
		location->sector_type = atoi ( argument );
		if ( location->sector_type < 0 || location->sector_type >= SECT_MAX )
		{
			location->sector_type = 1;
			send_to_char ( "Out of range.\r\n", ch );
		}
		else
			send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "exkey" ) )
	{
		argument = one_argument ( argument, arg2 );
		argument = one_argument ( argument, arg3 );
		if ( arg2[0] == STRING_NULL || arg3[0] == STRING_NULL )
		{
			send_to_char ( "Usage: rset exkey <dir> <key vnum>\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		value = atoi ( arg3 );
		if ( !xit )
		{
			send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
			return;
		}
		xit->key = value;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "excoord" ) )
	{
		int x, y;
		argument = one_argument ( argument, arg2 );
		argument = one_argument ( argument, arg3 );
		if ( arg2[0] == STRING_NULL || arg3[0] == STRING_NULL || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: rset excoord <dir> <X> <Y>\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		x = atoi ( arg3 );
		y = atoi ( argument );
		if ( x < 0 || x >= MAX_X )
		{
			ch_printf ( ch, "Valid X coordinates are 0 to %d.\r\n", MAX_X - 1 );
			return;
		}
		if ( y < 0 || y >= MAX_Y )
		{
			ch_printf ( ch, "Valid Y coordinates are 0 to %d.\r\n", MAX_Y - 1 );
			return;
		}
		if ( !xit )
		{
			send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
			return;
		}
		xit->x = x;
		xit->y = y;
		send_to_char ( "Exit coordinates set.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "exname" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Change or clear exit keywords.\r\n", ch );
			send_to_char ( "Usage: rset exname <dir> [keywords]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( !xit )
		{
			send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
			return;
		}
		STRFREE ( xit->keyword );
		xit->keyword = STRALLOC ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "exflags" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Toggle or display exit flags.\r\n", ch );
			send_to_char ( "Usage: rset exflags <dir> <flag> [flag]...\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( !xit )
		{
			send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
			return;
		}
		if ( argument[0] == STRING_NULL )
		{
			sprintf ( buf, "Flags for exit direction: %d  Keywords: %s  Key: %d\r\n[ ", xit->vdir, xit->keyword, xit->key );
			for ( value = 0; value <= MAX_EXFLAG; value++ )
			{
				if ( IS_SET ( xit->exit_info, 1 << value ) )
				{
					strcat ( buf, ex_flags[value] );
					strcat ( buf, " " );
				}
			}
			strcat ( buf, "]\r\n" );
			send_to_char ( buf, ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg2 );
			value = get_exflag ( arg2 );
			if ( value < 0 || value > MAX_EXFLAG )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg2 );
			else
				TOGGLE_BIT ( xit->exit_info, 1 << value );
		}
		return;
	}
	if ( !str_cmp ( arg, "ex_flags" ) )
	{
		argument = one_argument ( argument, arg2 );
		value = get_exflag ( arg2 );
		if ( value < 0 )
		{
			send_to_char ( "Bad exit flag. \r\n", ch );
			return;
		}
		if ( ( xit = get_exit ( location, edir ) ) == NULL )
		{
			sprintf ( buf, "exit %c 1", dir );
			do_rset ( ch, buf );
			xit = get_exit ( location, edir );
		}
		TOGGLE_BIT ( xit->exit_info, 1 << value );
		return;
	}
	if ( !str_cmp ( arg, "ex_to_room" ) )
	{
		argument = one_argument ( argument, arg2 );
		evnum = atoi ( arg2 );
		if ( evnum < 1 || evnum > 2147483647 )
		{
			send_to_char ( "Invalid room number.\r\n", ch );
			return;
		}
		if ( ( tmp = get_room_index ( evnum ) ) == NULL )
		{
			send_to_char ( "Non-existant room.\r\n", ch );
			return;
		}
		if ( ( xit = get_exit ( location, edir ) ) == NULL )
		{
			sprintf ( buf, "exit %c 1", dir );
			do_rset ( ch, buf );
			xit = get_exit ( location, edir );
		}
		xit->vnum = evnum;
		return;
	}
	if ( !str_cmp ( arg, "ex_key" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( ( xit = get_exit ( location, edir ) ) == NULL )
		{
			sprintf ( buf, "exit %c 1", dir );
			do_rset ( ch, buf );
			xit = get_exit ( location, edir );
		}
		xit->key = atoi ( arg2 );
		return;
	}
	if ( !str_cmp ( arg, "ex_exdesc" ) )
	{
		if ( ( xit = get_exit ( location, edir ) ) == NULL )
		{
			sprintf ( buf, "exit %c 1", dir );
			do_rset ( ch, buf );
		}
		sprintf ( buf, "exdesc %c %s", dir, argument );
		do_rset ( ch, buf );
		return;
	}
	if ( !str_cmp ( arg, "ex_keywords" ) ) /* not called yet */
	{
		if ( ( xit = get_exit ( location, edir ) ) == NULL )
		{
			sprintf ( buf, "exit %c 1", dir );
			do_rset ( ch, buf );
			if ( ( xit = get_exit ( location, edir ) ) == NULL )
				return;
		}
		sprintf ( buf, "%s %s", xit->keyword, argument );
		STRFREE ( xit->keyword );
		xit->keyword = STRALLOC ( buf );
		return;
	}
	if ( !str_cmp ( arg, "exit" ) )
	{
		bool addexit, numnotdir;
		argument = one_argument ( argument, arg2 );
		argument = one_argument ( argument, arg3 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Create, change or remove an exit.\r\n", ch );
			send_to_char ( "Usage: rset exit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		addexit = numnotdir = FALSE;
		switch ( arg2[0] )
		{
			default:
				edir = get_dir ( arg2 );
				break;
			case '+':
				edir = get_dir ( arg2 + 1 );
				addexit = TRUE;
				break;
			case '#':
				edir = atoi ( arg2 + 1 );
				numnotdir = TRUE;
				break;
		}
		if ( arg3[0] == STRING_NULL )
			evnum = 0;
		else
			evnum = atoi ( arg3 );
		if ( numnotdir )
		{
			if ( ( xit = get_exit_num ( location, edir ) ) != NULL )
				edir = xit->vdir;
		}
		else
			xit = get_exit ( location, edir );
		if ( !evnum )
		{
			if ( xit )
			{
				extract_exit ( location, xit );
				send_to_char ( "Exit removed.\r\n", ch );
				return;
			}
			send_to_char ( "No exit in that direction.\r\n", ch );
			return;
		}
		if ( evnum < 1 || evnum > 2147483647 )
		{
			send_to_char ( "Invalid room number.\r\n", ch );
			return;
		}
		if ( ( tmp = get_room_index ( evnum ) ) == NULL )
		{
			send_to_char ( "Non-existant room.\r\n", ch );
			return;
		}
		if ( addexit || !xit )
		{
			if ( numnotdir )
			{
				send_to_char ( "Cannot add an exit by number, sorry.\r\n", ch );
				return;
			}
			if ( addexit && xit && get_exit_to ( location, edir, tmp->vnum ) )
			{
				send_to_char ( "There is already an exit in that direction leading to that location.\r\n", ch );
				return;
			}
			xit = make_exit ( location, tmp, edir );
			xit->keyword = STRALLOC ( "" );
			xit->description = STRALLOC ( "" );
			xit->key = -1;
			xit->exit_info = 0;
			act ( AT_IMMORT, "$n reveals a hidden passage!", ch, NULL, NULL, TO_ROOM );
		}
		else
			act ( AT_IMMORT, "Something is different...", ch, NULL, NULL, TO_ROOM );
		if ( xit->to_room != tmp )
		{
			xit->to_room = tmp;
			xit->vnum = evnum;
			texit = get_exit_to ( xit->to_room, rev_dir[edir], location->vnum );
			if ( texit )
			{
				texit->rexit = xit;
				xit->rexit = texit;
			}
		}
		argument = one_argument ( argument, arg3 );
		if ( arg3[0] != STRING_NULL )
			xit->exit_info = atoi ( arg3 );
		if ( argument && argument[0] != STRING_NULL )
		{
			one_argument ( argument, arg3 );
			ekey = atoi ( arg3 );
			if ( ekey != 0 || arg3[0] == '0' )
			{
				argument = one_argument ( argument, arg3 );
				xit->key = ekey;
			}
			if ( argument && argument[0] != STRING_NULL )
			{
				STRFREE ( xit->keyword );
				xit->keyword = STRALLOC ( argument );
			}
		}
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	/*
	 * Twisted and evil, but works          -Thoric
	 * Makes an exit, and the reverse in one shot.
	 */
	if ( !str_cmp ( arg, "bexit" ) )
	{
		EXIT_DATA *nxit, *rxit;
		char tmpcmd[MAX_INPUT_LENGTH];
		ROOM_INDEX_DATA *tmploc;
		int vnum, exnum;
		char rvnum[MAX_INPUT_LENGTH];
		bool numnotdir;
		argument = one_argument ( argument, arg2 );
		argument = one_argument ( argument, arg3 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Create, change or remove a two-way exit.\r\n", ch );
			send_to_char ( "Usage: rset bexit <dir> [room] [flags] [key] [keywords]\r\n", ch );
			return;
		}
		numnotdir = FALSE;
		switch ( arg2[0] )
		{
			default:
				edir = get_dir ( arg2 );
				break;
			case '#':
				numnotdir = TRUE;
				edir = atoi ( arg2 + 1 );
				break;
			case '+':
				edir = get_dir ( arg2 + 1 );
				break;
		}
		tmploc = location;
		exnum = edir;
		if ( numnotdir )
		{
			if ( ( nxit = get_exit_num ( tmploc, edir ) ) != NULL )
				edir = nxit->vdir;
		}
		else
			nxit = get_exit ( tmploc, edir );
		rxit = NULL;
		vnum = 0;
		rvnum[0] = STRING_NULL;
		if ( nxit )
		{
			vnum = nxit->vnum;
			if ( arg3[0] != STRING_NULL )
				sprintf ( rvnum, "%d", tmploc->vnum );
			if ( nxit->to_room )
				rxit = get_exit ( nxit->to_room, rev_dir[edir] );
			else
				rxit = NULL;
		}
		sprintf ( tmpcmd, "exit %s %s %s", arg2, arg3, argument );
		do_rset ( ch, tmpcmd );
		if ( numnotdir )
			nxit = get_exit_num ( tmploc, exnum );
		else
			nxit = get_exit ( tmploc, edir );
		if ( !rxit && nxit )
		{
			vnum = nxit->vnum;
			if ( arg3[0] != STRING_NULL )
				sprintf ( rvnum, "%d", tmploc->vnum );
			if ( nxit->to_room )
				rxit = get_exit ( nxit->to_room, rev_dir[edir] );
			else
				rxit = NULL;
		}
		if ( vnum )
		{
			sprintf ( tmpcmd, "%d rset exit %d %s %s", vnum, rev_dir[edir], rvnum, argument );
			do_at ( ch, tmpcmd );
		}
		return;
	}
	if ( !str_cmp ( arg, "pulltype" ) || !str_cmp ( arg, "pushtype" ) )
	{
		int pt;
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			ch_printf ( ch, "Set the %s between this room, and the destination room.\r\n", arg );
			ch_printf ( ch, "Usage: rset %s <dir> <type>\r\n", arg );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( xit )
		{
			if ( ( pt = get_pulltype ( argument ) ) == -1 )
				ch_printf ( ch, "Unknown pulltype: %s.  (See help PULLTYPES)\r\n", argument );
			else
			{
				xit->pulltype = pt;
				send_to_char ( "Done.\r\n", ch );
				return;
			}
		}
		send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "pull" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Set the 'pull' between this room, and the destination room.\r\n", ch );
			send_to_char ( "Usage: rset pull <dir> <force (0 to 100)>\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( xit )
		{
			xit->pull = URANGE ( -100, atoi ( argument ), 100 );
			send_to_char ( "Done.\r\n", ch );
			return;
		}
		send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "push" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Set the 'push' away from the destination room in the opposite direction.\r\n", ch );
			send_to_char ( "Usage: rset push <dir> <force (0 to 100)>\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( xit )
		{
			xit->pull = URANGE ( -100, - ( atoi ( argument ) ), 100 );
			send_to_char ( "Done.\r\n", ch );
			return;
		}
		send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "exdistance" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Set the distance (in rooms) between this room, and the destination room.\r\n", ch );
			send_to_char ( "Usage: rset exdistance <dir> [distance]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( xit )
		{
			xit->distance = URANGE ( 1, atoi ( argument ), 50 );
			send_to_char ( "Done.\r\n", ch );
			return;
		}
		send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg, "exdesc" ) )
	{
		argument = one_argument ( argument, arg2 );
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Create or clear a description for an exit.\r\n", ch );
			send_to_char ( "Usage: rset exdesc <dir> [description]\r\n", ch );
			return;
		}
		if ( arg2[0] == '#' )
		{
			edir = atoi ( arg2 + 1 );
			xit = get_exit_num ( location, edir );
		}
		else
		{
			edir = get_dir ( arg2 );
			xit = get_exit ( location, edir );
		}
		if ( xit )
		{
			STRFREE ( xit->description );
			if ( !argument || argument[0] == STRING_NULL )
				xit->description = STRALLOC ( "" );
			else
			{
				sprintf ( buf, "%s\r\n", argument );
				xit->description = STRALLOC ( buf );
			}
			send_to_char ( "Done.\r\n", ch );
			return;
		}
		send_to_char ( "No exit in that direction.  Use 'rset exit ...' first.\r\n", ch );
		return;
	}
	/*
	 * Generate usage message.
	 */
	if ( ch->substate == SUB_REPEATCMD )
	{
		ch->substate = SUB_RESTRICTED;
		interpret ( ch, origarg );
		ch->substate = SUB_REPEATCMD;
		ch->last_cmd = do_rset;
	}
	else
		do_rset ( ch, "" );
	return;
}
void do_ocreate ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int vnum, cvnum;
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mobiles cannot create.\r\n", ch );
		return;
	}
	argument = one_argument ( argument, arg );
	vnum = is_number ( arg ) ? atoi ( arg ) : -1;
	if ( vnum == -1 || !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Usage:  ocreate <vnum> [copy vnum] <item name>\r\n", ch );
		return;
	}
	if ( vnum < 1 || vnum > 2147483647 )
	{
		send_to_char ( "Vnum out of range.\r\n", ch );
		return;
	}
	one_argument ( argument, arg2 );
	cvnum = atoi ( arg2 );
	if ( cvnum != 0 )
		argument = one_argument ( argument, arg2 );
	if ( cvnum < 1 )
		cvnum = 0;
	if ( get_obj_index ( vnum ) )
	{
		send_to_char ( "An object with that number already exists.\r\n", ch );
		return;
	}
	if ( IS_NPC ( ch ) )
		return;
	if ( get_trust ( ch ) < LEVEL_NEOPHYTE )
	{
		AREA_DATA *pArea;
		if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
		{
			send_to_char ( "You must have an assigned area to create objects.\r\n", ch );
			return;
		}
		if ( vnum < pArea->low_o_vnum || vnum > pArea->hi_o_vnum )
		{
			send_to_char ( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}
	pObjIndex = make_object ( vnum, cvnum, argument );
	if ( !pObjIndex )
	{
		send_to_char ( "Error.\r\n", ch );
		log_string ( "do_ocreate: make_object failed." );
		return;
	}
	obj = create_object ( pObjIndex, get_trust ( ch ) );
	obj_to_char ( obj, ch );
	act ( AT_IMMORT, "$n makes arcane gestures, and opens $s hands to reveal $p!", ch, obj, NULL, TO_ROOM );
	ch_printf_color ( ch,
	                  "&YYou make arcane gestures, and open your hands to reveal %s!\r\nObjVnum:  &W%d   &YKeywords:  &W%s\r\n",
	                  pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name );
}
void do_mcreate ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *mob;
	int vnum, cvnum, level;
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mobiles cannot create.\r\n", ch );
		return;
	}
	argument = one_argument ( argument, arg );
	vnum = is_number ( arg ) ? atoi ( arg ) : -1;
	if ( vnum == -1 || !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Usage:  mcreate <vnum> <level> [cvnum] <mobile name>\r\n", ch );
		return;
	}
	if ( vnum < 1 || vnum > MAX_VNUM )
	{
		send_to_char ( "Vnum out of range.\r\n", ch );
		return;
	}
	one_argument ( argument, arg2 );
	level = atoi ( arg2 );
	if ( level != 0 )
		argument = one_argument ( argument, arg2 );
	if ( level < 1 )
		level = 1;
	one_argument ( argument, arg3 );
	cvnum = atoi ( arg3 );
	if ( cvnum != 0 )
		argument = one_argument ( argument, arg3 );
	if ( cvnum < 1 )
		cvnum = 0;
	if ( get_mob_index ( vnum ) )
	{
		send_to_char ( "A mobile with that number already exists.\r\n", ch );
		return;
	}
	if ( IS_NPC ( ch ) )
		return;
	if ( get_trust ( ch ) < LEVEL_NEOPHYTE )
	{
		AREA_DATA *pArea;
		if ( !ch->pcdata || ! ( pArea = ch->pcdata->area ) )
		{
			send_to_char ( "You must have an assigned area to create mobiles.\r\n", ch );
			return;
		}
		if ( vnum < pArea->low_m_vnum || vnum > pArea->hi_m_vnum )
		{
			send_to_char ( "That number is not in your allocated range.\r\n", ch );
			return;
		}
	}
	pMobIndex = make_mobile ( vnum, cvnum, argument, level );
	if ( !pMobIndex )
	{
		send_to_char ( "Error.\r\n", ch );
		log_string ( "do_mcreate: make_mobile failed." );
		return;
	}
	mob = create_mobile ( pMobIndex );
	char_to_room ( mob, ch->in_room );
	/*
	 * If you create one on the map, make sure it gets placed properly - Samson 8-21-99
	 */
	fix_maps ( ch, mob );
	act ( AT_IMMORT, "$n waves $s arms about, and $N appears at $s command!", ch, NULL, mob, TO_ROOM );
	ch_printf_color ( ch,
	                  "&YYou wave your arms about, and %s appears at your command!\r\nMobVnum:  &W%d   &YKeywords:  &W%s\r\n",
	                  pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name );
}

/*
 * Simple but nice and handy line editor.			-Thoric
 */
void edit_buffer ( CHAR_DATA * ch, char *argument )
{
	DESCRIPTOR_DATA *d;
	EDITOR_DATA *edit;
	char cmd[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	short x, line, max_buf_lines;
	bool save;
	if ( ( d = ch->desc ) == NULL )
	{
		send_to_char ( "You have no descriptor.\r\n", ch );
		return;
	}
	if ( d->connected != CON_EDITING )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		bug ( "Edit_buffer: d->connected != CON_EDITING", 0 );
		return;
	}
	if ( ch->substate <= SUB_PAUSE )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		bug ( "Edit_buffer: illegal ch->substate (%d)", ch->substate );
		d->connected = CON_PLAYING;
		return;
	}
	if ( !ch->editor )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		bug ( "Edit_buffer: null editor", 0 );
		d->connected = CON_PLAYING;
		return;
	}
	edit = ch->editor;
	save = FALSE;
	max_buf_lines = 100;
	if ( ch->substate == SUB_MPROG_EDIT || ch->substate == SUB_HELP_EDIT )
		max_buf_lines = 100;
	if ( argument[0] == '/' || argument[0] == '\\' )
	{
		one_argument ( argument, cmd );
		if ( !str_cmp ( cmd + 1, "?" ) )
		{
			send_to_char ( "Editing commands\r\n", ch );
			send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
			send_to_char ( "/l              list buffer\r\n", ch );
			send_to_char ( "/f <format>     format text in buffer\r\n", ch );
			send_to_char ( "/c              clear buffer\r\n", ch );
			send_to_char ( "/d [line]       delete line\r\n", ch );
			send_to_char ( "/g <line>       goto line\r\n", ch );
			send_to_char ( "/i <line>       insert line\r\n", ch );
			send_to_char ( "/r <old> <new>  global replace\r\n", ch );
			send_to_char ( "/a              abort editing\r\n", ch );
			send_to_char ( "/s              save buffer\r\n", ch );
			if ( get_trust ( ch ) > LEVEL_IMMORTAL )
				send_to_char ( "/! <command>    execute command (do not use another editing command)\r\n", ch );
			send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
			return;
		}
		if ( !str_cmp ( cmd + 1, "c" ) )
		{
			memset ( edit, STRING_NULL, sizeof ( EDITOR_DATA ) );
			edit->numlines = 0;
			edit->on_line = 0;
			send_to_char ( "Buffer cleared.\r\n> ", ch );
			return;
		}
		if ( !str_cmp ( cmd + 1, "r" ) )
		{
			char word1[MAX_INPUT_LENGTH];
			char word2[MAX_INPUT_LENGTH];
			char *sptr, *wptr, *lwptr;
			int count, wordln, word2ln, lineln;
			sptr = one_argument ( argument, word1 );
			sptr = one_argument ( sptr, word1 );
			sptr = one_argument ( sptr, word2 );
			if ( word1[0] == STRING_NULL || word2[0] == STRING_NULL )
			{
				send_to_char ( "Need word to replace, and replacement.\r\n> ", ch );
				return;
			}
			if ( strcmp ( word1, word2 ) == 0 )
			{
				send_to_char ( "Done.\r\n> ", ch );
				return;
			}
			count = 0;
			wordln = strlen ( word1 );
			word2ln = strlen ( word2 );
			ch_printf ( ch, "Replacing all occurrences of %s with %s...\r\n", word1, word2 );
			for ( x = 0; x < edit->numlines; x++ )
			{
				lwptr = edit->line[x];
				while ( ( wptr = strstr ( lwptr, word1 ) ) != NULL )
				{
					++count;
					lineln = sprintf ( buf, "%s%s", word2, wptr + wordln );
					if ( lineln + wptr - edit->line[x] > 79 )
						buf[lineln] = STRING_NULL;
					strcpy ( wptr, buf );
					lwptr = wptr + word2ln;
				}
			}
			ch_printf ( ch, "Found and replaced %d occurrence(s).\r\n> ", count );
			return;
		}
		if ( !str_cmp ( cmd + 1, "i" ) )
		{
			if ( edit->numlines >= max_buf_lines )
				send_to_char ( "Buffer is full.\r\n> ", ch );
			else
			{
				if ( argument[2] == ' ' )
					line = atoi ( argument + 2 ) - 1;
				else
					line = edit->on_line;
				if ( line < 0 )
					line = edit->on_line;
				if ( line < 0 || line > edit->numlines )
					send_to_char ( "Out of range.\r\n> ", ch );
				else
				{
					for ( x = ++edit->numlines; x > line; x-- )
						strcpy ( edit->line[x], edit->line[x - 1] );
					strcpy ( edit->line[line], "" );
					send_to_char ( "Line inserted.\r\n> ", ch );
				}
			}
			return;
		}
		/*
		 * added format command - shogar
		 */
		/*
		 * This has been redone to be more efficient, and to make format
		 * start at beginning of buffer, not whatever line you happened
		 * to be on, at the time.
		 */
		if ( !str_cmp ( cmd + 1, "f" ) )
		{
			char temp_buf[MAX_STRING_LENGTH + max_buf_lines];
			int x1, ep, old_p, end_mark;
			int p = 0;
			pager_printf ( ch, "Reformating...\r\n" );
			for ( x1 = 0; x1 < edit->numlines; x1++ )
			{
				strcpy ( temp_buf + p, edit->line[x1] );
				p += strlen ( edit->line[x1] );
				temp_buf[p] = ' ';
				p++;
			}
			temp_buf[p] = STRING_NULL;
			end_mark = p;
			p = 79;
			old_p = 0;
			edit->on_line = 0;
			edit->numlines = 0;
			while ( old_p < end_mark )
			{
				while ( temp_buf[p] != ' ' && p > old_p )
					p--;
				if ( p == old_p )
					p += 79;
				if ( p > end_mark )
					p = end_mark;
				ep = 0;
				for ( x1 = old_p; x1 < p; x1++ )
				{
					edit->line[edit->on_line][ep] = temp_buf[x1];
					ep++;
				}
				edit->line[edit->on_line][ep] = STRING_NULL;
				edit->on_line++;
				edit->numlines++;
				old_p = p + 1;
				p += 79;
			}
			pager_printf ( ch, "Reformating done.\r\n" );
			return;
		}
		if ( !str_cmp ( cmd + 1, "d" ) )
		{
			if ( edit->numlines == 0 )
				send_to_char ( "Buffer is empty.\r\n", ch );
			else
			{
				if ( argument[2] == ' ' )
					line = atoi ( argument + 2 ) - 1;
				else
					line = edit->on_line;
				if ( line < 0 )
					line = edit->on_line;
				if ( line < 0 || line > edit->numlines )
					send_to_char ( "Out of range.\r\n", ch );
				else
				{
					if ( line == 0 && edit->numlines == 1 )
					{
						memset ( edit, STRING_NULL, sizeof ( EDITOR_DATA ) );
						edit->numlines = 0;
						edit->on_line = 0;
						send_to_char ( "Line deleted.\r\n", ch );
						return;
					}
					for ( x = line; x < ( edit->numlines - 1 ); x++ )
						strcpy ( edit->line[x], edit->line[x + 1] );
					strcpy ( edit->line[edit->numlines--], "" );
					if ( edit->on_line > edit->numlines )
						edit->on_line = edit->numlines;
					send_to_char ( "Line deleted.\r\n", ch );
				}
			}
			return;
		}
		if ( !str_cmp ( cmd + 1, "g" ) )
		{
			if ( edit->numlines == 0 )
				send_to_char ( "Buffer is empty.\r\n", ch );
			else
			{
				if ( argument[2] == ' ' )
					line = atoi ( argument + 2 ) - 1;
				else
				{
					send_to_char ( "Goto what line?\r\n", ch );
					return;
				}
				if ( line < 0 )
					line = edit->on_line;
				if ( line < 0 || line > edit->numlines )
					send_to_char ( "Out of range.\r\n", ch );
				else
				{
					edit->on_line = line;
					ch_printf ( ch, "(On line %d)\r\n", line + 1 );
				}
			}
			return;
		}
		if ( !str_cmp ( cmd + 1, "l" ) )
		{
			if ( edit->numlines == 0 )
				send_to_char ( "Buffer is empty.\r\n", ch );
			else
			{
				for ( x = 0; x < edit->numlines; x++ )
					ch_printf ( ch, "%d %s\r\n", x + 1, edit->line[x] );
				send_to_char ( "--------------------------------------------------------------------------------\r\n> ", ch );
			}
			return;
		}
		if ( !str_cmp ( cmd + 1, "a" ) )
		{
			send_to_char ( "\r\nAborting... ", ch );
			stop_editing ( ch );
			return;
		}
		if ( get_trust ( ch ) > LEVEL_IMMORTAL && !str_cmp ( cmd + 1, "!" ) )
		{
			DO_FUN *last_cmd;
			int substate = ch->substate;
			last_cmd = ch->last_cmd;
			ch->substate = SUB_RESTRICTED;
			interpret ( ch, argument + 3 );
			ch->substate = substate;
			ch->last_cmd = last_cmd;
			set_char_color ( AT_GREEN, ch );
			send_to_char ( "\r\n", ch );
			return;
		}
		if ( !str_cmp ( cmd + 1, "s" ) )
		{
			d->connected = CON_PLAYING;
			if ( !ch->last_cmd )
				return;
			( *ch->last_cmd ) ( ch, "" );
			return;
		}
	}
	if ( edit->size + strlen ( argument ) + 1 >= MAX_STRING_LENGTH - 1 )
		send_to_char ( "You buffer is full.\r\n", ch );
	else
	{
		if ( strlen_color ( argument ) > 79 )
		{
			char arg[MAX_STRING_LENGTH];
			buf[0] = STRING_NULL;
			while ( argument[0] != STRING_NULL )
			{
				argument = one_argument ( argument, arg );
				if ( strlen_color ( arg ) > 79 ) /* A single word more than 80 long? Skip! */
					continue;
				if ( ( strlen_color ( buf ) + strlen_color ( arg ) + 1 ) <= 79 )
					sprintf ( buf, "%s%s%s", buf, buf[0] == STRING_NULL ? "" : " ", arg );
				else  /* Ok end this line and move onto the next */
				{
					if ( ch->substate == SUB_MPROG_EDIT )
						strcat ( buf, "+" ); // Add this in so concatenates properly.
					strcpy ( edit->line[edit->on_line++], buf );
					if ( edit->on_line > edit->numlines )
						edit->numlines++;
					buf[0] = STRING_NULL;   /* Resets the buffer to empty so can start over again. */
					strcat ( buf, arg ); /* Adds the word that was too much, to buffer and continues processing */
				}
				if ( edit->numlines > max_buf_lines )
				{
					edit->numlines = max_buf_lines;
					send_to_char ( "You've run out of room in the editing buffer.\r\n", ch );
					save = TRUE;
				}
			}
			strcpy ( edit->line[edit->on_line++], buf );
			if ( edit->on_line > edit->numlines )
				edit->numlines++;
			buf[0] = STRING_NULL;   /* Resets the buffer to empty so can start over again. */
			if ( edit->numlines > max_buf_lines )
			{
				edit->numlines = max_buf_lines;
				send_to_char ( "You've run out of room in the editing buffer.\r\n", ch );
				save = TRUE;
			}
		}
		else
			strcpy ( buf, argument );
		if ( buf[0] != STRING_NULL )
			strcpy ( edit->line[edit->on_line++], buf );
		if ( edit->on_line > edit->numlines )
			edit->numlines++;
		if ( edit->numlines > max_buf_lines )
		{
			edit->numlines = max_buf_lines;
			send_to_char ( "Buffer full.\r\n", ch );
			save = TRUE;
		}
	}
	if ( save )
	{
		d->connected = CON_PLAYING;
		if ( !ch->last_cmd )
			return;
		( *ch->last_cmd ) ( ch, "" );
		return;
	}
	send_to_char ( "> ", ch );
}

void assign_area ( CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char taf[1024];
	AREA_DATA *tarea, *tmp;
	bool created = FALSE;
	if ( IS_NPC ( ch ) )
		return;
	if ( get_trust ( ch ) > LEVEL_IMMORTAL && ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
	{
		tarea = ch->pcdata->area;
		sprintf ( taf, "%s.are", capitalize ( ch->name ) );
		if ( !tarea )
		{
			for ( tmp = first_build; tmp; tmp = tmp->next )
				if ( !str_cmp ( taf, tmp->filename ) )
				{
					tarea = tmp;
					break;
				}
		}
		if ( !tarea )
		{
			sprintf ( buf, "Creating area entry for %s", ch->name );
			log_string_plus ( buf, LOG_NORMAL, ch->level );
			CREATE ( tarea, AREA_DATA, 1 );
			LINK ( tarea, first_build, last_build, next, prev );
			tarea->first_room = tarea->last_room = NULL;
			sprintf ( buf, "{PROTO} %s", ch->name );
			tarea->name = str_dup ( buf );
			tarea->filename = str_dup ( taf );
			sprintf ( buf2, "%s", ch->name );
			tarea->author = STRALLOC ( buf2 );
			tarea->age = 0;
			tarea->nplayer = 0;
			CREATE ( tarea->weather, WEATHER_DATA, 1 );  /* FB */
			tarea->weather->temp = 0;
			tarea->weather->precip = 0;
			tarea->weather->wind = 0;
			tarea->weather->temp_vector = 0;
			tarea->weather->precip_vector = 0;
			tarea->weather->wind_vector = 0;
			tarea->weather->climate_temp = 2;
			tarea->weather->climate_precip = 2;
			tarea->weather->climate_wind = 2;
			tarea->weather->first_neighbor = NULL;
			tarea->weather->last_neighbor = NULL;
			tarea->weather->echo = NULL;
			tarea->weather->echo_color = AT_GREY;
			created = TRUE;
		}
		else
		{
			sprintf ( buf, "Updating area entry for %s", ch->name );
			log_string_plus ( buf, LOG_NORMAL, ch->level );
		}
		tarea->low_r_vnum = ch->pcdata->r_range_lo;
		tarea->low_o_vnum = ch->pcdata->o_range_lo;
		tarea->low_m_vnum = ch->pcdata->m_range_lo;
		tarea->hi_r_vnum = ch->pcdata->r_range_hi;
		tarea->hi_o_vnum = ch->pcdata->o_range_hi;
		tarea->hi_m_vnum = ch->pcdata->m_range_hi;
		ch->pcdata->area = tarea;
		if ( created )
			sort_area ( tarea, TRUE );
	}
}
void do_aassign ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	AREA_DATA *tarea, *tmp;
	set_char_color ( AT_IMMORT, ch );
	if ( IS_NPC ( ch ) )
		return;
	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: aassign <filename.are>\r\n", ch );
		return;
	}
	if ( !str_cmp ( "none", argument ) || !str_cmp ( "null", argument ) || !str_cmp ( "clear", argument ) )
	{
		ch->pcdata->area = NULL;
		assign_area ( ch );
		if ( !ch->pcdata->area )
			send_to_char ( "Area pointer cleared.\r\n", ch );
		else
			send_to_char ( "Originally assigned area restored.\r\n", ch );
		return;
	}
	sprintf ( buf, "%s", argument );
	tarea = NULL;
	/*
	 * if ( get_trust(ch) >= sysdata.level_modify_proto )
	 */
	if ( get_trust ( ch ) >= LEVEL_NEOPHYTE || ( is_name ( buf, ch->pcdata->bestowments ) && get_trust ( ch ) >= sysdata.level_modify_proto ) )
		for ( tmp = first_area; tmp; tmp = tmp->next )
			if ( !str_cmp ( buf, tmp->filename ) )
			{
				tarea = tmp;
				break;
			}
	if ( !tarea )
		for ( tmp = first_build; tmp; tmp = tmp->next )
			if ( !str_cmp ( buf, tmp->filename ) )
			{
				/*
				 * if ( get_trust(ch) >= sysdata.level_modify_proto
				 */
				if ( get_trust ( ch ) >= LEVEL_NEOPHYTE || is_name ( tmp->filename, ch->pcdata->bestowments ) )
				{
					tarea = tmp;
					break;
				}
				else
				{
					send_to_char ( "You do not have permission to use that area.\r\n", ch );
					return;
				}
			}
	if ( !tarea )
	{
		if ( get_trust ( ch ) >= LEVEL_NEOPHYTE )
			send_to_char ( "No such area.  Use 'zones'.\r\n", ch );
		else
			send_to_char ( "No such area.  Use 'newzones'.\r\n", ch );
		return;
	}
	ch->pcdata->area = tarea;
	ch_printf ( ch, "Assigning you: %s\r\n", tarea->name );
	return;
}

EXTRA_DESCR_DATA *SetRExtra ( ROOM_INDEX_DATA * room, char *keywords )
{
	EXTRA_DESCR_DATA *ed;
	for ( ed = room->first_extradesc; ed; ed = ed->next )
	{
		if ( is_name ( keywords, ed->keyword ) )
			break;
	}
	if ( !ed )
	{
		CREATE ( ed, EXTRA_DESCR_DATA, 1 );
		LINK ( ed, room->first_extradesc, room->last_extradesc, next, prev );
		ed->keyword = STRALLOC ( keywords );
		ed->description = STRALLOC ( "" );
		top_ed++;
	}
	return ed;
}

bool DelRExtra ( ROOM_INDEX_DATA * room, char *keywords )
{
	EXTRA_DESCR_DATA *rmed;
	for ( rmed = room->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name ( keywords, rmed->keyword ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK ( rmed, room->first_extradesc, room->last_extradesc, next, prev );
	STRFREE ( rmed->keyword );
	STRFREE ( rmed->description );
	DISPOSE ( rmed );
	top_ed--;
	return TRUE;
}

EXTRA_DESCR_DATA *SetOExtra ( OBJ_DATA * obj, char *keywords )
{
	EXTRA_DESCR_DATA *ed;
	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if ( is_name ( keywords, ed->keyword ) )
			break;
	}
	if ( !ed )
	{
		CREATE ( ed, EXTRA_DESCR_DATA, 1 );
		LINK ( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword = STRALLOC ( keywords );
		ed->description = STRALLOC ( "" );
		top_ed++;
	}
	return ed;
}

bool DelOExtra ( OBJ_DATA * obj, char *keywords )
{
	EXTRA_DESCR_DATA *rmed;
	for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name ( keywords, rmed->keyword ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK ( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	STRFREE ( rmed->keyword );
	STRFREE ( rmed->description );
	DISPOSE ( rmed );
	top_ed--;
	return TRUE;
}

EXTRA_DESCR_DATA *SetOExtraProto ( OBJ_INDEX_DATA * obj, char *keywords )
{
	EXTRA_DESCR_DATA *ed;
	for ( ed = obj->first_extradesc; ed; ed = ed->next )
	{
		if ( is_name ( keywords, ed->keyword ) )
			break;
	}
	if ( !ed )
	{
		CREATE ( ed, EXTRA_DESCR_DATA, 1 );
		LINK ( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
		ed->keyword = STRALLOC ( keywords );
		ed->description = STRALLOC ( "" );
		top_ed++;
	}
	return ed;
}

bool DelOExtraProto ( OBJ_INDEX_DATA * obj, char *keywords )
{
	EXTRA_DESCR_DATA *rmed;
	for ( rmed = obj->first_extradesc; rmed; rmed = rmed->next )
	{
		if ( is_name ( keywords, rmed->keyword ) )
			break;
	}
	if ( !rmed )
		return FALSE;
	UNLINK ( rmed, obj->first_extradesc, obj->last_extradesc, next, prev );
	STRFREE ( rmed->keyword );
	STRFREE ( rmed->description );
	DISPOSE ( rmed );
	top_ed--;
	return TRUE;
}
void fold_area ( AREA_DATA * tarea, char *filename, bool install )
{
	RESET_DATA *pReset, *tReset, *gReset;
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	MPROG_DATA *mprog;
	EXIT_DATA *xit;
	EXTRA_DESCR_DATA *ed;
	AFFECT_DATA *paf;
	SHOP_DATA *pShop;
	REPAIR_DATA *pRepair;
	NEIGHBOR_DATA *neigh;
	char buf[MAX_STRING_LENGTH];
	FILE *fpout;
	int vnum;
	int val0, val1, val2, val3, val4, val5;
	sprintf ( buf, "Saving %s...", tarea->filename );
	log_string_plus ( buf, LOG_NORMAL, LEVEL_GREATER );
	sprintf ( buf, "%s.bak", filename );
	rename ( filename, buf );
	fclose ( fpReserve );
	if ( ( fpout = fopen ( filename, "w" ) ) == NULL )
	{
		bug ( "fold_area: fopen", 0 );
		perror ( filename );
		fpReserve = fopen ( NULL_FILE, "r" );
		return;
	}
	fprintf ( fpout, "#AREA        %s~\n", tarea->name );
	fprintf ( fpout, "#VERSION            %d\n", AREA_VERSION_WRITE );
	fprintf ( fpout, "#AUTHOR             %s~\n", tarea->author );
	fprintf ( fpout, "#RANGES             %d %d %d %d\n", tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
	fprintf ( fpout, "#SPELLLIMIT         %d\n", tarea->spelllimit );
	fprintf ( fpout, "#RESETMSG           %s~\n", tarea->resetmsg );
	fprintf ( fpout, "#RESETFREQ          %d\n", tarea->reset_frequency );
	fprintf ( fpout, "#FLAGS              %d\n", tarea->flags );
	fprintf ( fpout, "#ECONOMY            %d %d\n", tarea->high_economy, tarea->low_economy );
	fprintf ( fpout, "#CONTINENT          %s~\n", continents[tarea->continent] );
	/*
	 * Climate info - FB
	 */
	fprintf ( fpout, "#CLIMATE            %d %d %d\n", tarea->weather->climate_temp, tarea->weather->climate_precip, tarea->weather->climate_wind );
	/*
	 * neighboring weather systems - FB
	 */
	for ( neigh = tarea->weather->first_neighbor; neigh; neigh = neigh->next )
		fprintf ( fpout, "#NEIGHBOR           %s~\n", neigh->name );
	/*
	 * save mobiles
	 */
	fprintf ( fpout, "\n\n#MOBILES\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( ( pMobIndex = get_mob_index ( vnum ) ) == NULL )
			continue;
		if ( install )
			xREMOVE_BIT ( pMobIndex->act, ACT_PROTOTYPE );
		fprintf ( fpout, "#  %d\n", pMobIndex->vnum );
		fprintf ( fpout, "%s~\n", pMobIndex->player_name );
		fprintf ( fpout, "%s~\n", pMobIndex->short_descr );
		fprintf ( fpout, "%s~\n", strip_cr ( pMobIndex->long_descr ) );
		fprintf ( fpout, "%s~\n\n", strip_cr ( pMobIndex->description ) );
		fprintf ( fpout, "%d %d %d\n", pMobIndex->level, pMobIndex->race, pMobIndex->Class );
		fprintf ( fpout, "%d %d %d %d\n", pMobIndex->hitplus, pMobIndex->damplus, pMobIndex->hitroll, pMobIndex->damroll );
		fprintf ( fpout, "%s %d %d %d %d\n", print_bitvector ( &pMobIndex->act ), pMobIndex->alignment, pMobIndex->position, pMobIndex->defposition, pMobIndex->sex );
		fprintf ( fpout, "%d %d %d %d %d\n", pMobIndex->height, pMobIndex->weight, pMobIndex->speaks, pMobIndex->speaking, pMobIndex->xflags );
		fprintf ( fpout, "%d %d %d\n", pMobIndex->immune, pMobIndex->resistant, pMobIndex->susceptible );
		if ( pMobIndex->mudprogs )
		{
			for ( mprog = pMobIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf ( fpout, "> %s %s~\n%s~\n", mprog_type_to_name ( mprog->type ), mprog->arglist, strip_cr ( mprog->comlist ) );
			fprintf ( fpout, "|\n\n" );
		}
	}
	fprintf ( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_m_vnum )
		tarea->hi_m_vnum = vnum - 1;
	/*
	 * save objects
	 */
	fprintf ( fpout, "#OBJECTS\n" );
	for ( vnum = tarea->low_o_vnum; vnum <= tarea->hi_o_vnum; vnum++ )
	{
		if ( ( pObjIndex = get_obj_index ( vnum ) ) == NULL )
			continue;
		if ( install )
			xREMOVE_BIT ( pObjIndex->extra_flags, ITEM_PROTOTYPE );
		fprintf ( fpout, "#%d\n", vnum );
		fprintf ( fpout, "%s~\n", pObjIndex->name );
		fprintf ( fpout, "%s~\n", pObjIndex->short_descr );
		fprintf ( fpout, "%s~\n", pObjIndex->description );
		fprintf ( fpout, "%s~\n", pObjIndex->action_desc );
		if ( pObjIndex->layers )
			fprintf ( fpout, "%d %s %d %d\n", pObjIndex->item_type, print_bitvector ( &pObjIndex->extra_flags ), pObjIndex->wear_flags, pObjIndex->layers );
		else
			fprintf ( fpout, "%d %s %d\n", pObjIndex->item_type, print_bitvector ( &pObjIndex->extra_flags ), pObjIndex->wear_flags );
		val0 = pObjIndex->value[0];
		val1 = pObjIndex->value[1];
		val2 = pObjIndex->value[2];
		val3 = pObjIndex->value[3];
		val4 = pObjIndex->value[4];
		val5 = pObjIndex->value[5];
		switch ( pObjIndex->item_type )
		{
			case ITEM_PILL:
			case ITEM_POTION:
			case ITEM_SCROLL:
				if ( IS_VALID_SN ( val1 ) )
				{
					val1 = HAS_SPELL_INDEX;
				}
				if ( IS_VALID_SN ( val2 ) )
				{
					val2 = HAS_SPELL_INDEX;
				}
				if ( IS_VALID_SN ( val3 ) )
				{
					val3 = HAS_SPELL_INDEX;
				}
				break;
			case ITEM_STAFF:
			case ITEM_WAND:
				if ( IS_VALID_SN ( val3 ) )
				{
					val3 = HAS_SPELL_INDEX;
				}
				break;
			case ITEM_SALVE:
				if ( IS_VALID_SN ( val4 ) )
				{
					val4 = HAS_SPELL_INDEX;
				}
				if ( IS_VALID_SN ( val5 ) )
				{
					val5 = HAS_SPELL_INDEX;
				}
				break;
		}
		if ( val4 || val5 )
			fprintf ( fpout, "%d %d %d %d %d %d\n", val0, val1, val2, val3, val4, val5 );
		else
			fprintf ( fpout, "%d %d %d %d\n", val0, val1, val2, val3 );
		fprintf ( fpout, "%d %d %d\n", pObjIndex->weight, pObjIndex->cost, pObjIndex->rent ? pObjIndex->rent : ( int ) ( pObjIndex->cost / 10 ) );
		if ( AREA_VERSION_WRITE > 0 )
			switch ( pObjIndex->item_type )
			{
				case ITEM_PILL:
				case ITEM_POTION:
				case ITEM_SCROLL:
					fprintf ( fpout, "'%s' '%s' '%s'\n",
					          IS_VALID_SN ( pObjIndex->value[1] ) ?
					          skill_table[pObjIndex->value[1]]->name : "NONE",
					          IS_VALID_SN ( pObjIndex->value[2] ) ?
					          skill_table[pObjIndex->value[2]]->name : "NONE", IS_VALID_SN ( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
					break;
				case ITEM_STAFF:
				case ITEM_WAND:
					fprintf ( fpout, "'%s'\n", IS_VALID_SN ( pObjIndex->value[3] ) ? skill_table[pObjIndex->value[3]]->name : "NONE" );
					break;
				case ITEM_SALVE:
					fprintf ( fpout, "'%s' '%s'\n",
					          IS_VALID_SN ( pObjIndex->value[4] ) ?
					          skill_table[pObjIndex->value[4]]->name : "NONE", IS_VALID_SN ( pObjIndex->value[5] ) ? skill_table[pObjIndex->value[5]]->name : "NONE" );
					break;
			}
		for ( ed = pObjIndex->first_extradesc; ed; ed = ed->next )
			fprintf ( fpout, "E\n%s~\n%s~\n", ed->keyword, strip_cr ( ed->description ) );
		for ( paf = pObjIndex->first_affect; paf; paf = paf->next )
			fprintf ( fpout, "A\n%d %d\n", paf->location,
			          ( ( paf->location == APPLY_WEAPONSPELL
			              || paf->location == APPLY_WEARSPELL
			              || paf->location == APPLY_REMOVESPELL
			              || paf->location == APPLY_STRIPSN
			              || paf->location == APPLY_RECURRINGSPELL ) && IS_VALID_SN ( paf->modifier ) ) );
		if ( pObjIndex->mudprogs )
		{
			for ( mprog = pObjIndex->mudprogs; mprog; mprog = mprog->next )
				fprintf ( fpout, "> %s %s~\n%s~\n", mprog_type_to_name ( mprog->type ), mprog->arglist, strip_cr ( mprog->comlist ) );
			fprintf ( fpout, "|\n" );
		}
	}
	fprintf ( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_o_vnum )
		tarea->hi_o_vnum = vnum - 1;
	/*
	 * save rooms
	 */
	fprintf ( fpout, "#ROOMS\n" );
	for ( vnum = tarea->low_r_vnum; vnum <= tarea->hi_r_vnum; vnum++ )
	{
		if ( ( room = get_room_index ( vnum ) ) == NULL )
			continue;
		if ( install )
		{
			CHAR_DATA *victim, *vnext;
			OBJ_DATA *obj, *obj_next;
			/*
			 * remove prototype flag from room
			 */
			xREMOVE_BIT ( room->room_flags, ROOM_PROTOTYPE );
			/*
			 * purge room of (prototyped) mobiles
			 */
			for ( victim = room->first_person; victim; victim = vnext )
			{
				vnext = victim->next_in_room;
				if ( IS_NPC ( victim ) )
					extract_char ( victim, TRUE );
			}
			/*
			 * purge room of (prototyped) objects
			 */
			for ( obj = room->first_content; obj; obj = obj_next )
			{
				obj_next = obj->next_content;
				extract_obj ( obj );
			}
		}
		fprintf ( fpout, "#%d\n", vnum );
		fprintf ( fpout, "%s~\n", room->name );
		fprintf ( fpout, "%s~\n", strip_cr ( room->description ) );
		if ( ( room->tele_delay > 0 && room->tele_vnum > 0 ) || room->tunnel > 0 )
		{
			fprintf ( fpout, "0 %s ", print_bitvector ( &room->room_flags ) );
			fprintf ( fpout, "%d %d %d %d ", room->sector_type, room->tele_delay, room->tele_vnum, room->tunnel );
		}
		else
			fprintf ( fpout, "0 %s ", print_bitvector ( &room->room_flags ) );
		fprintf ( fpout, "%d ", room->sector_type );
		for ( xit = room->first_exit; xit; xit = xit->next )
		{
			if ( IS_SET ( xit->exit_info, EX_PORTAL ) ) /* don't fold portals */
				continue;
			fprintf ( fpout, "\nD%d\n", xit->vdir );
			fprintf ( fpout, "%s~\n", strip_cr ( xit->description ) );
			fprintf ( fpout, "%s~\n", strip_cr ( xit->keyword ) );
			if ( xit->distance > 1 || xit->pull )
				fprintf ( fpout, "%d %d %d %d %d %d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance, xit->x, xit->y, xit->pulltype, xit->pull );
			else
				fprintf ( fpout, "%d %d %d %d %d %d\n", xit->exit_info & ~EX_BASHED, xit->key, xit->vnum, xit->distance, xit->x, xit->y );
		}
		for ( pReset = room->first_reset; pReset; pReset = pReset->next )
		{
			switch ( pReset->command ) /* extra arg1 arg2 arg3 */
			{
				default:
				case '*':
					break;
				case 'm':
				case 'M':
				case 'o':
				case 'O':
					fprintf ( fpout, "R %c %d %d %d %d\n", UPPER ( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
					for ( tReset = pReset->first_reset; tReset; tReset = tReset->next_reset )
					{
						switch ( tReset->command )
						{
							case 'p':
							case 'P':
							case 'e':
							case 'E':
								fprintf ( fpout, "  R %c %d %d %d %d\n", UPPER ( tReset->command ), tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
								if ( tReset->first_reset )
								{
									for ( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
									{
										if ( gReset->command != 'p' && gReset->command != 'P' )
											continue;
										fprintf ( fpout, "    R %c %d %d %d %d\n", UPPER ( gReset->command ), gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
									}
								}
								break;
							case 'g':
							case 'G':
								fprintf ( fpout, "  R %c %d %d %d\n", UPPER ( tReset->command ), tReset->extra, tReset->arg1, tReset->arg2 );
								if ( tReset->first_reset )
								{
									for ( gReset = tReset->first_reset; gReset; gReset = gReset->next_reset )
									{
										if ( gReset->command != 'p' && gReset->command != 'P' )
											continue;
										fprintf ( fpout, "    R %c %d %d %d %d\n", UPPER ( gReset->command ), gReset->extra, gReset->arg1, gReset->arg2, gReset->arg3 );
									}
								}
								break;
							case 't':
							case 'T':
							case 'h':
							case 'H':
								fprintf ( fpout, "  R %c %d %d %d %d\n", UPPER ( tReset->command ), tReset->extra, tReset->arg1, tReset->arg2, tReset->arg3 );
								break;
						}
					}
					break;
				case 'd':
				case 'D':
				case 't':
				case 'T':
				case 'h':
				case 'H':
					fprintf ( fpout, "R %c %d %d %d %d\n", UPPER ( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2, pReset->arg3 );
					break;
				case 'r':
				case 'R':
					fprintf ( fpout, "R %c %d %d %d\n", UPPER ( pReset->command ), pReset->extra, pReset->arg1, pReset->arg2 );
					break;
			}
		}
		for ( ed = room->first_extradesc; ed; ed = ed->next )
			fprintf ( fpout, "\nE\n%s~\n%s~\n", ed->keyword, strip_cr ( ed->description ) );
		if ( room->mudprogs )
		{
			for ( mprog = room->mudprogs; mprog; mprog = mprog->next )
				fprintf ( fpout, "\n> %s %s~\n%s~\n", mprog_type_to_name ( mprog->type ), mprog->arglist, strip_cr ( mprog->comlist ) );
			fprintf ( fpout, "|\n" );
		}
		fprintf ( fpout, "\nS\n" );
	}
	fprintf ( fpout, "#0\n\n\n" );
	if ( install && vnum < tarea->hi_r_vnum )
		tarea->hi_r_vnum = vnum - 1;
	/*
	 * save shops
	 */
	fprintf ( fpout, "#SHOPS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( ( pMobIndex = get_mob_index ( vnum ) ) == NULL )
			continue;
		if ( ( pShop = pMobIndex->pShop ) == NULL )
			continue;
		fprintf ( fpout, " %d   %2d %2d %2d %2d %2d   %3d %3d",
		          pShop->keeper, pShop->buy_type[0], pShop->buy_type[1], pShop->buy_type[2], pShop->buy_type[3], pShop->buy_type[4], pShop->profit_buy, pShop->profit_sell );
		fprintf ( fpout, "        %2d %2d    ; %s\n", pShop->open_hour, pShop->close_hour, pMobIndex->short_descr );
	}
	fprintf ( fpout, "0\n\n\n" );
	/*
	 * save repair shops
	 */
	fprintf ( fpout, "#REPAIRS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( ( pMobIndex = get_mob_index ( vnum ) ) == NULL )
			continue;
		if ( ( pRepair = pMobIndex->rShop ) == NULL )
			continue;
		fprintf ( fpout, " %d   %2d %2d %2d         %3d %3d",
		          pRepair->keeper, pRepair->fix_type[0], pRepair->fix_type[1], pRepair->fix_type[2], pRepair->profit_fix, pRepair->shop_type );
		fprintf ( fpout, "        %2d %2d    ; %s\n", pRepair->open_hour, pRepair->close_hour, pMobIndex->short_descr );
	}
	fprintf ( fpout, "0\n\n\n" );
	/*
	 * save specials
	 */
	fprintf ( fpout, "#SPECIALS\n" );
	for ( vnum = tarea->low_m_vnum; vnum <= tarea->hi_m_vnum; vnum++ )
	{
		if ( ( pMobIndex = get_mob_index ( vnum ) ) == NULL )
			continue;
		if ( !pMobIndex->spec_fun )
			continue;
		fprintf ( fpout, "M  %d %s\n", pMobIndex->vnum, lookup_spec ( pMobIndex->spec_fun ) );
	}
	fprintf ( fpout, "S\n\n\n" );
	/*
	 * END
	 */
	fprintf ( fpout, "#$\n" );
	fclose ( fpout );
	fpReserve = fopen ( NULL_FILE, "r" );
	return;
}
void do_savearea ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	char filename[256];
	set_char_color ( AT_IMMORT, ch );
	if ( IS_NPC ( ch ) || get_trust ( ch ) < LEVEL_CREATOR || !ch->pcdata || ( argument[0] == STRING_NULL && !ch->pcdata->area ) )
	{
		send_to_char ( "You don't have an assigned area to save.\r\n", ch );
		return;
	}
	if ( argument[0] == STRING_NULL )
		tarea = ch->pcdata->area;
	else
	{
		bool found;
		if ( get_trust ( ch ) < LEVEL_GOD )
		{
			send_to_char ( "You can only save your own area.\r\n", ch );
			return;
		}
		for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
			if ( !str_cmp ( tarea->filename, argument ) )
			{
				found = TRUE;
				break;
			}
		if ( !found )
		{
			send_to_char ( "Area not found.\r\n", ch );
			return;
		}
	}
	if ( !tarea )
	{
		send_to_char ( "No area to save.\r\n", ch );
		return;
	}
	/*
	 * Ensure not wiping out their area with save before load - Scryn 8/11
	 */
	if ( !IS_SET ( tarea->status, AREA_LOADED ) )
	{
		send_to_char ( "Your area is not loaded!\r\n", ch );
		return;
	}
	sprintf ( filename, "%s%s", BUILD_DIR, tarea->filename );
	send_to_char ( "Saving area...\r\n", ch );
	fold_area ( tarea, filename, FALSE );
	set_char_color ( AT_IMMORT, ch );
	send_to_char ( "Done.\r\n", ch );
}
void do_loadarea ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	char filename[256];
	int tmp;
	set_char_color ( AT_IMMORT, ch );
	if ( IS_NPC ( ch ) || get_trust ( ch ) < LEVEL_CREATOR || !ch->pcdata || ( argument[0] == STRING_NULL && !ch->pcdata->area ) )
	{
		send_to_char ( "You don't have an assigned area to load.\r\n", ch );
		return;
	}
	if ( argument[0] == STRING_NULL )
		tarea = ch->pcdata->area;
	else
	{
		bool found;
		if ( get_trust ( ch ) < LEVEL_GOD )
		{
			send_to_char ( "You can only load your own area.\r\n", ch );
			return;
		}
		for ( found = FALSE, tarea = first_build; tarea; tarea = tarea->next )
			if ( !str_cmp ( tarea->filename, argument ) )
			{
				found = TRUE;
				break;
			}
		if ( !found )
		{
			send_to_char ( "Area not found.\r\n", ch );
			return;
		}
	}
	if ( !tarea )
	{
		send_to_char ( "No area to load.\r\n", ch );
		return;
	}
	/*
	 * Stops char from loading when already loaded - Scryn 8/11
	 */
	if ( IS_SET ( tarea->status, AREA_LOADED ) )
	{
		send_to_char ( "Your area is already loaded.\r\n", ch );
		return;
	}
	sprintf ( filename, "%s%s", BUILD_DIR, tarea->filename );
	send_to_char ( "Loading...\r\n", ch );
	load_area_file ( tarea, filename );
	send_to_char ( "Linking exits...\r\n", ch );
	fix_area_exits ( tarea );
	if ( tarea->first_room )
	{
		tmp = tarea->nplayer;
		tarea->nplayer = 0;
		send_to_char ( "Resetting area...\r\n", ch );
		reset_area ( tarea );
		tarea->nplayer = tmp;
	}
	send_to_char ( "Done.\r\n", ch );
}

/*
 * Dangerous command.  Can be used to install an area that was either:
 *   (a) already installed but removed from area.lst
 *   (b) designed offline
 * The mud will likely crash if:
 *   (a) this area is already loaded
 *   (b) it contains vnums that exist
 *   (c) the area has errors
 *
 * NOTE: Use of this command is not recommended.		-Thoric
 */
void do_unfoldarea ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_IMMORT, ch );
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Unfold what?\r\n", ch );
		return;
	}
	fBootDb = TRUE;
	load_area_file ( last_area, argument );
	fBootDb = FALSE;
	return;
}
void do_foldarea ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	set_char_color ( AT_IMMORT, ch );
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Fold what?\r\n", ch );
		return;
	}
	for ( tarea = first_area; tarea; tarea = tarea->next )
	{
		if ( !str_cmp ( tarea->filename, argument ) )
		{
			send_to_char ( "Folding area...\r\n", ch );
			fold_area ( tarea, tarea->filename, FALSE );
			set_char_color ( AT_IMMORT, ch );
			send_to_char ( "Done.\r\n", ch );
			return;
		}
	}
	send_to_char ( "No such area exists.\r\n", ch );
	return;
}
extern int top_area;
void write_area_list ( void )
{
	AREA_DATA *tarea;
	FILE *fpout;
	fpout = fopen ( AREA_LIST, "w" );
	if ( !fpout )
	{
		bug ( "FATAL: cannot open area.lst for writing!\r\n", 0 );
		return;
	}
	fprintf ( fpout, "help.are\n" );
	for ( tarea = first_area; tarea; tarea = tarea->next )
		fprintf ( fpout, "%s\n", tarea->filename );
	fprintf ( fpout, "$\n" );
	fclose ( fpout );
}

/*
 * A complicated to use command as it currently exists.		-Thoric
 * Once area->author and area->name are cleaned up... it will be easier
 */
void do_installarea ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int num;
	DESCRIPTOR_DATA *d;
	set_char_color ( AT_IMMORT, ch );
	argument = one_argument ( argument, arg );
	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: installarea <filename> [Area title]\r\n", ch );
		return;
	}
	for ( tarea = first_build; tarea; tarea = tarea->next )
	{
		if ( !str_cmp ( tarea->filename, arg ) )
		{
			if ( argument && argument[0] != STRING_NULL )
			{
				DISPOSE ( tarea->name );
				tarea->name = str_dup ( argument );
			}
			/*
			 * Fold area with install flag -- auto-removes prototype flags
			 */
			send_to_char ( "Saving and installing file...\r\n", ch );
			fold_area ( tarea, tarea->filename, TRUE );
			/*
			 * Remove from prototype area list
			 */
			UNLINK ( tarea, first_build, last_build, next, prev );
			/*
			 * Add to real area list
			 */
			LINK ( tarea, first_area, last_area, next, prev );
			/*
			 * Remove it from the prototype sort list. BUGFIX: Samson 4-15-03
			 */
			UNLINK ( tarea, first_bsort, last_bsort, next_sort, prev_sort );
			/*
			 * Sort the area into it's proper sort list. BUGFIX: Samson 4-15-03
			 */
			sort_area ( tarea, FALSE );
			/*
			 * Fix up author if online
			 */
			for ( d = first_descriptor; d; d = d->next )
				if ( d->character && d->character->pcdata && d->character->pcdata->area == tarea )
				{
					/*
					 * remove area from author
					 */
					d->character->pcdata->area = NULL;
					/*
					 * clear out author vnums
					 */
					d->character->pcdata->r_range_lo = 0;
					d->character->pcdata->r_range_hi = 0;
					d->character->pcdata->o_range_lo = 0;
					d->character->pcdata->o_range_hi = 0;
					d->character->pcdata->m_range_lo = 0;
					d->character->pcdata->m_range_hi = 0;
				}
			top_area++;
			send_to_char ( "Writing area.lst...\r\n", ch );
			write_area_list( );
			send_to_char ( "Resetting new area.\r\n", ch );
			num = tarea->nplayer;
			tarea->nplayer = 0;
			reset_area ( tarea );
			tarea->nplayer = num;
			send_to_char ( "Renaming author's building file.\r\n", ch );
			sprintf ( buf, "%s%s.installed", BUILD_DIR, tarea->filename );
			sprintf ( arg, "%s%s", BUILD_DIR, tarea->filename );
			rename ( arg, buf );
			send_to_char ( "Done.\r\n", ch );
			return;
		}
	}
	send_to_char ( "No such area exists.\r\n", ch );
	return;
}
void do_astat ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	bool proto, found;
	int pdeaths = 0, pkills = 0, mdeaths = 0, mkills = 0;
	found = FALSE;
	proto = FALSE;
	set_char_color ( AT_PLAIN, ch );
	if ( !str_cmp ( "summary", argument ) )
	{
		for ( tarea = first_area; tarea; tarea = tarea->next )
		{
			pdeaths += tarea->pdeaths;
			mdeaths += tarea->mdeaths;
			pkills += tarea->pkills;
			mkills += tarea->mkills;
		}
		ch_printf_color ( ch, "&WTotal pdeaths:      &w%d\r\n", pdeaths );
		ch_printf_color ( ch, "&WTotal pkills:       &w%d\r\n", pkills );
		ch_printf_color ( ch, "&WTotal mdeaths:      &w%d\r\n", mdeaths );
		ch_printf_color ( ch, "&WTotal mkills:       &w%d\r\n", mkills );
		return;
	}
	for ( tarea = first_area; tarea; tarea = tarea->next )
		if ( !str_cmp ( tarea->filename, argument ) )
		{
			found = TRUE;
			break;
		}
	if ( !found )
		for ( tarea = first_build; tarea; tarea = tarea->next )
			if ( !str_cmp ( tarea->filename, argument ) )
			{
				found = TRUE;
				proto = TRUE;
				break;
			}
	if ( !found )
	{
		if ( argument && argument[0] != STRING_NULL )
		{
			send_to_char ( "Area not found.  Check 'zones'.\r\n", ch );
			return;
		}
		else
		{
			tarea = ch->in_room->area;
		}
	}
	ch_printf_color ( ch, "\r\n&wName:     &W%s\r\n&wFilename: &W%-20s  &wPrototype: &W%s\r\n&wAuthor:   &W%s\r\n",
	                  tarea->name, tarea->filename, proto ? "yes" : "no", tarea->author );
	ch_printf_color ( ch, "&wAge: &W%-3d  &wCurrent number of players: &W%-3d  &wMax players: &W%d\r\n", tarea->age, tarea->nplayer, tarea->max_players );
	if ( !proto )
	{
		if ( tarea->high_economy )
			ch_printf_color ( ch, "&wArea economy: &W%d &wbillion and &W%d gold coins.\r\n", tarea->high_economy, tarea->low_economy );
		else
			ch_printf_color ( ch, "&wArea economy: &W%d &wgold coins.\r\n", tarea->low_economy );
		ch_printf_color ( ch, "&wGold Looted:  &W%d\r\n", tarea->gold_looted );
		ch_printf_color ( ch, "&wMdeaths: &W%d   &wMkills: &W%d   &wPdeaths: &W%d   &wPkills: &W%d   &wIllegalPK: &W%d\r\n",
		                  tarea->mdeaths, tarea->mkills, tarea->pdeaths, tarea->pkills, tarea->illegal_pk );
	}
	ch_printf_color ( ch, "&wlow_room: &W%5d    &whi_room: &W%5d\r\n", tarea->low_r_vnum, tarea->hi_r_vnum );
	ch_printf_color ( ch, "&wlow_obj : &W%5d    &whi_obj : &W%5d\r\n", tarea->low_o_vnum, tarea->hi_o_vnum );
	ch_printf_color ( ch, "&wlow_mob : &W%5d    &whi_mob : &W%5d\r\n", tarea->low_m_vnum, tarea->hi_m_vnum );
	ch_printf_color ( ch, "&wsoft range: &W%d - %d    &whard range: &W%d - %d\r\n", tarea->low_soft_range, tarea->hi_soft_range, tarea->low_hard_range, tarea->hi_hard_range );
	ch_printf_color ( ch, "&wArea flags: &W%s\r\n", flag_string ( tarea->flags, area_flags ) );
	ch_printf_color ( ch, "&wResetmsg: &W%s\r\n", tarea->resetmsg ? tarea->resetmsg : "(default)" ); /* Rennard */
	ch_printf_color ( ch, "&wReset frequency: &W%d &wminutes.\r\n", tarea->reset_frequency ? tarea->reset_frequency : 15 );
	ch_printf_color ( ch, "&wContinent: &W%s\r\n", continents[tarea->continent] );
}
void do_aset ( CHAR_DATA * ch, char *argument )
{
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	bool proto, found;
	int vnum, value;
	set_char_color ( AT_IMMORT, ch );
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	vnum = atoi ( argument );
	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Usage: aset <area filename> <field> <value>\r\n", ch );
		send_to_char ( "\r\nField being one of:\r\n", ch );
		send_to_char ( "  low_room hi_room low_obj hi_obj low_mob hi_mob\r\n", ch );
		send_to_char ( "  name filename low_soft hi_soft low_hard hi_hard\r\n", ch );
		send_to_char ( "  author resetmsg resetfreq flags\r\n", ch );
		return;
	}
	found = FALSE;
	proto = FALSE;
	for ( tarea = first_area; tarea; tarea = tarea->next )
		if ( !str_cmp ( tarea->filename, arg1 ) )
		{
			found = TRUE;
			break;
		}
	if ( !found )
		for ( tarea = first_build; tarea; tarea = tarea->next )
			if ( !str_cmp ( tarea->filename, arg1 ) )
			{
				found = TRUE;
				proto = TRUE;
				break;
			}
	if ( !found )
	{
		send_to_char ( "Area not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "name" ) )
	{
		DISPOSE ( tarea->name );
		tarea->name = str_dup ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "filename" ) )
	{
		DISPOSE ( tarea->filename );
		tarea->filename = str_dup ( argument );
		write_area_list( );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "continent" ) )
	{
		/*
		 * Area continent editing - Samson 8-8-98
		 */
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Set the area's continent.\r\n", ch );
			send_to_char ( "Usage: aset continent <name>\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg2 );
		value = get_continent ( arg2 );
		if ( value < 0 || value > ACON_MAX )
		{
			tarea->continent = 0;
			send_to_char ( "Invalid area continent, set to 'alsherok' by default.\r\n", ch );
		}
		else
		{
			tarea->continent = value;
			ch_printf ( ch, "Area continent set to %s.\r\n", arg2 );
		}
		return;
	}
	if ( !str_cmp ( arg2, "low_economy" ) )
	{
		tarea->low_economy = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "high_economy" ) )
	{
		tarea->high_economy = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "low_room" ) )
	{
		tarea->low_r_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hi_room" ) )
	{
		tarea->hi_r_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "low_obj" ) )
	{
		tarea->low_o_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hi_obj" ) )
	{
		tarea->hi_o_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "low_mob" ) )
	{
		tarea->low_m_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hi_mob" ) )
	{
		tarea->hi_m_vnum = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "low_soft" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char ( "That is not an acceptable value.\r\n", ch );
			return;
		}
		tarea->low_soft_range = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hi_soft" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char ( "That is not an acceptable value.\r\n", ch );
			return;
		}
		tarea->hi_soft_range = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "low_hard" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char ( "That is not an acceptable value.\r\n", ch );
			return;
		}
		tarea->low_hard_range = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "hi_hard" ) )
	{
		if ( vnum < 0 || vnum > MAX_LEVEL )
		{
			send_to_char ( "That is not an acceptable value.\r\n", ch );
			return;
		}
		tarea->hi_hard_range = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "author" ) )
	{
		STRFREE ( tarea->author );
		tarea->author = STRALLOC ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "resetmsg" ) )
	{
		if ( tarea->resetmsg )
			DISPOSE ( tarea->resetmsg );
		if ( str_cmp ( argument, "clear" ) )
			tarea->resetmsg = str_dup ( argument );
		send_to_char ( "Done.\r\n", ch );
		return;
	}  /* Rennard */
	if ( !str_cmp ( arg2, "resetfreq" ) )
	{
		tarea->reset_frequency = vnum;
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "flags" ) )
	{
		if ( !argument || argument[0] == STRING_NULL )
		{
			send_to_char ( "Usage: aset <filename> flags <flag> [flag]...\r\n", ch );
			return;
		}
		while ( argument[0] != STRING_NULL )
		{
			argument = one_argument ( argument, arg3 );
			value = get_areaflag ( arg3 );
			if ( value < 0 || value > 31 )
				ch_printf ( ch, "Unknown flag: %s\r\n", arg3 );
			else
			{
				if ( IS_SET ( tarea->flags, 1 << value ) )
					REMOVE_BIT ( tarea->flags, 1 << value );
				else
					SET_BIT ( tarea->flags, 1 << value );
			}
		}
		return;
	}
	do_aset ( ch, "" );
	return;
}
void do_rlist ( CHAR_DATA * ch, char *argument )
{
	ROOM_INDEX_DATA *room;
	int vnum;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	AREA_DATA *tarea;
	int lrange;
	int trange;
	set_pager_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) || get_trust ( ch ) < LEVEL_CREATOR || !ch->pcdata || ( !ch->pcdata->area && get_trust ( ch ) < LEVEL_GREATER ) )
	{
		send_to_char_color ( "&YYou don't have an assigned area.\r\n", ch );
		return;
	}
	tarea = ch->pcdata->area;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	if ( arg1[0] != STRING_NULL && !is_number ( arg1 ) )
		return;
	if ( arg2[0] != STRING_NULL && !is_number ( arg2 ) )
		return;
	/*
	 * I know the above code could be combined into 3 lines, but we write code
	 * once and then read it many times. It is so much easier to read without
	 * trying to figure out what opening parenthesis belongs with what closing
	 * one.  -- Gorog
	 */
	/*
	 * Bah! The following code uses atoi(arg1) and atoi(arg2) without even
	 * checking they are numeric or even exist. I put a fix for this above.
	 * It also uses is_number(arg1) without checking if arg1 may be null.
	 * Caused a crash when the command was uses with a single alpha arg.
	 * -- Gorog
	 */
	if ( tarea )
	{
		if ( arg1[0] == STRING_NULL ) /* cleaned a big scary mess */
			lrange = tarea->low_r_vnum;   /* here.     -Thoric */
		else
			lrange = atoi ( arg1 );
		if ( arg2[0] == STRING_NULL )
			trange = tarea->hi_r_vnum;
		else
			trange = atoi ( arg2 );
		if ( ( lrange < tarea->low_r_vnum || trange > tarea->hi_r_vnum ) && get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char_color ( "&YThat is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number ( arg1 ) ? atoi ( arg1 ) : 1 );
		trange = ( is_number ( arg2 ) ? atoi ( arg2 ) : 1 );
	}
	for ( vnum = lrange; vnum <= trange; vnum++ )
	{
		if ( ( room = get_room_index ( vnum ) ) == NULL )
			continue;
		pager_printf ( ch, "%5d) %s\r\n", vnum, room->name );
	}
	return;
}
void do_olist ( CHAR_DATA * ch, char *argument )
{
	OBJ_INDEX_DATA *obj;
	int vnum;
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lrange;
	int trange;
	/*
	 * Greater+ can list out of assigned range - Tri (mlist/rlist as well)
	 */
	set_pager_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) || get_trust ( ch ) < LEVEL_CREATOR || !ch->pcdata || ( !ch->pcdata->area && get_trust ( ch ) < LEVEL_GREATER ) )
	{
		send_to_char_color ( "&YYou don't have an assigned area.\r\n", ch );
		return;
	}
	tarea = ch->pcdata->area;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	if ( tarea )
	{
		if ( arg1[0] == STRING_NULL ) /* cleaned a big scary mess */
			lrange = tarea->low_o_vnum;   /* here.     -Thoric */
		else
			lrange = atoi ( arg1 );
		if ( arg2[0] == STRING_NULL )
			trange = tarea->hi_o_vnum;
		else
			trange = atoi ( arg2 );
		if ( ( lrange < tarea->low_o_vnum || trange > tarea->hi_o_vnum ) && get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char_color ( "&YThat is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number ( arg1 ) ? atoi ( arg1 ) : 1 );
		trange = ( is_number ( arg2 ) ? atoi ( arg2 ) : 3 );
	}
	for ( vnum = lrange; vnum <= trange; vnum++ )
	{
		if ( ( obj = get_obj_index ( vnum ) ) == NULL )
			continue;
		pager_printf ( ch, "%5d) %-20s (%s)\r\n", vnum, obj->name, obj->short_descr );
	}
	return;
}
void do_mlist ( CHAR_DATA * ch, char *argument )
{
	MOB_INDEX_DATA *mob;
	int vnum;
	AREA_DATA *tarea;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lrange;
	int trange;
	set_pager_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) || get_trust ( ch ) < LEVEL_CREATOR || !ch->pcdata || ( !ch->pcdata->area && get_trust ( ch ) < LEVEL_GREATER ) )
	{
		send_to_char_color ( "&YYou don't have an assigned area.\r\n", ch );
		return;
	}
	tarea = ch->pcdata->area;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	if ( tarea )
	{
		if ( arg1[0] == STRING_NULL ) /* cleaned a big scary mess */
			lrange = tarea->low_m_vnum;   /* here.     -Thoric */
		else
			lrange = atoi ( arg1 );
		if ( arg2[0] == STRING_NULL )
			trange = tarea->hi_m_vnum;
		else
			trange = atoi ( arg2 );
		if ( ( lrange < tarea->low_m_vnum || trange > tarea->hi_m_vnum ) && get_trust ( ch ) < LEVEL_GREATER )
		{
			send_to_char_color ( "&YThat is out of your vnum range.\r\n", ch );
			return;
		}
	}
	else
	{
		lrange = ( is_number ( arg1 ) ? atoi ( arg1 ) : 1 );
		trange = ( is_number ( arg2 ) ? atoi ( arg2 ) : 1 );
	}
	for ( vnum = lrange; vnum <= trange; vnum++ )
	{
		if ( ( mob = get_mob_index ( vnum ) ) == NULL )
			continue;
		pager_printf ( ch, "%5d) %-20s '%s'\r\n", vnum, mob->player_name, mob->short_descr );
	}
}
void mpedit ( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument )
{
	if ( mptype != -1 )
	{
		mprg->type = mptype;
		if ( mprg->arglist )
			STRFREE ( mprg->arglist );
		mprg->arglist = STRALLOC ( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if ( !mprg->comlist )
		mprg->comlist = STRALLOC ( "" );
	start_editing ( ch, mprg->comlist );
	return;
}

/*
 * Mobprogram editing - cumbersome				-Thoric
 */
void do_mpedit ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
	int value, mptype = -1, cnt;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't mpedit\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_mpedit: sub_mprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog = ch->dest_buf;
			if ( mprog->comlist )
				STRFREE ( mprog->comlist );
			mprog->comlist = copy_buffer ( ch );
			stop_editing ( ch );
			return;
	}
	smash_tilde ( argument );
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	value = atoi ( arg3 );
	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: mpedit <victim> <command> [number] <program> <value>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Command being one of:\r\n", ch );
		send_to_char ( "  add delete insert edit list\r\n", ch );
		send_to_char ( "Program being one of:\r\n", ch );
		send_to_char ( "  act speech rand fight hitprcnt greet allgreet\r\n", ch );
		send_to_char ( "  entry give bribe death time hour script\r\n", ch );
		return;
	}
	if ( get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( victim = get_char_room ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "They aren't here.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "No one like that in all the realms.\r\n", ch );
			return;
		}
	}
	if ( get_trust ( ch ) < victim->level || !IS_NPC ( victim ) )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		return;
	}
	if ( get_trust ( ch ) < LEVEL_GREATER && IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_STATSHIELD ) )
	{
		set_pager_color ( AT_IMMORT, ch );
		send_to_pager ( "Their godly glow prevents you from getting close enough.\r\n", ch );
		return;
	}
	if ( !can_mmodify ( ch, victim ) )
		return;
	if ( !xIS_SET ( victim->act, ACT_PROTOTYPE ) )
	{
		send_to_char ( "A mobile must have a prototype flag to be mpset.\r\n", ch );
		return;
	}
	mprog = victim->pIndexData->mudprogs;
	set_char_color ( AT_GREEN, ch );
	if ( !str_cmp ( arg2, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char ( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			if ( strcmp ( "full", arg3 ) )
			{
				for ( mprg = mprog; mprg; mprg = mprg->next )
				{
					ch_printf ( ch, "%d>%s %s\r\n", ++cnt, mprog_type_to_name ( mprg->type ), mprg->arglist );
				}
				return;
			}
			else
			{
				for ( mprg = mprog; mprg; mprg = mprg->next )
				{
					ch_printf ( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name ( mprg->type ), mprg->arglist, mprg->comlist );
				}
				return;
			}
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				ch_printf ( ch, "%d>%s %s\r\n%s\r\n", cnt, mprog_type_to_name ( mprg->type ), mprg->arglist, mprg->comlist );
				break;
			}
		}
		if ( !mprg )
			send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		if ( arg4[0] != STRING_NULL )
		{
			mptype = get_mpflag ( arg4 );
			if ( mptype == -1 )
			{
				send_to_char ( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit ( ch, mprg, mptype, argument );
				xCLEAR_BITS ( victim->pIndexData->progtypes );
				for ( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT ( victim->pIndexData->progtypes, mprg->type );
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "delete" ) )
	{
		int num;
		bool found;
		if ( !mprog )
		{
			send_to_char ( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( mprg->type == mptype )
				num++;
		if ( value == 1 )
		{
			mprg_next = victim->pIndexData->mudprogs;
			victim->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		if ( mprg_next )
		{
			STRFREE ( mprg_next->arglist );
			STRFREE ( mprg_next->comlist );
			DISPOSE ( mprg_next );
			if ( num <= 1 )
				xREMOVE_BIT ( victim->pIndexData->progtypes, mptype );
			send_to_char ( "Program removed.\r\n", ch );
		}
		return;
	}
	if ( !str_cmp ( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "That mobile has no mob programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		mptype = get_mpflag ( arg4 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE ( mprg, MPROG_DATA, 1 );
			xSET_BIT ( victim->pIndexData->progtypes, mptype );
			mpedit ( ch, mprg, mptype, argument );
			mprg->next = mprog;
			victim->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE ( mprg_next, MPROG_DATA, 1 );
				xSET_BIT ( victim->pIndexData->progtypes, mptype );
				mpedit ( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "add" ) )
	{
		mptype = get_mpflag ( arg3 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog != NULL )
			for ( ; mprog->next; mprog = mprog->next )
				;
		CREATE ( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next = mprg;
		else
			victim->pIndexData->mudprogs = mprg;
		xSET_BIT ( victim->pIndexData->progtypes, mptype );
		mpedit ( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}
	do_mpedit ( ch, "" );
}
void do_opedit ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
	int value, mptype = -1, cnt;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't opedit\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog = ch->dest_buf;
			if ( mprog->comlist )
				STRFREE ( mprog->comlist );
			mprog->comlist = copy_buffer ( ch );
			stop_editing ( ch );
			return;
	}
	smash_tilde ( argument );
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	value = atoi ( arg3 );
	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: opedit <object> <command> [number] <program> <value>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Command being one of:\r\n", ch );
		send_to_char ( "  add delete insert edit list\r\n", ch );
		send_to_char ( "Program being one of:\r\n", ch );
		send_to_char ( "  act speech rand wear remove sac zap get\r\n", ch );
		send_to_char ( "  drop damage repair greet exa use\r\n", ch );
		send_to_char ( "  pull push (for levers,pullchains,buttons)\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Object should be in your inventory to edit.\r\n", ch );
		return;
	}
	if ( get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( obj = get_obj_carry ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "You aren't carrying that.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( obj = get_obj_world ( ch, arg1 ) ) == NULL )
		{
			send_to_char ( "Nothing like that in all the realms.\r\n", ch );
			return;
		}
	}
	if ( !can_omodify ( ch, obj ) )
		return;
	if ( !IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
	{
		send_to_char ( "An object must have a prototype flag to be opset.\r\n", ch );
		return;
	}
	mprog = obj->pIndexData->mudprogs;
	set_char_color ( AT_GREEN, ch );
	if ( !str_cmp ( arg2, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char ( "That object has no obj programs.\r\n", ch );
			return;
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf ( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name ( mprg->type ), mprg->arglist, mprg->comlist );
		return;
	}
	if ( !str_cmp ( arg2, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		if ( arg4[0] != STRING_NULL )
		{
			mptype = get_mpflag ( arg4 );
			if ( mptype == -1 )
			{
				send_to_char ( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit ( ch, mprg, mptype, argument );
				xCLEAR_BITS ( obj->pIndexData->progtypes );
				for ( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT ( obj->pIndexData->progtypes, mprg->type );
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "delete" ) )
	{
		int num;
		bool found;
		if ( !mprog )
		{
			send_to_char ( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( mprg->type == mptype )
				num++;
		if ( value == 1 )
		{
			mprg_next = obj->pIndexData->mudprogs;
			obj->pIndexData->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		if ( mprg_next )
		{
			STRFREE ( mprg_next->arglist );
			STRFREE ( mprg_next->comlist );
			DISPOSE ( mprg_next );
			if ( num <= 1 )
				xREMOVE_BIT ( obj->pIndexData->progtypes, mptype );
			send_to_char ( "Program removed.\r\n", ch );
		}
		return;
	}
	if ( !str_cmp ( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "That object has no obj programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg4 );
		mptype = get_mpflag ( arg4 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE ( mprg, MPROG_DATA, 1 );
			xSET_BIT ( obj->pIndexData->progtypes, mptype );
			mpedit ( ch, mprg, mptype, argument );
			mprg->next = mprog;
			obj->pIndexData->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE ( mprg_next, MPROG_DATA, 1 );
				xSET_BIT ( obj->pIndexData->progtypes, mptype );
				mpedit ( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg2, "add" ) )
	{
		mptype = get_mpflag ( arg3 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog != NULL )
			for ( ; mprog->next; mprog = mprog->next )
				;
		CREATE ( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next = mprg;
		else
			obj->pIndexData->mudprogs = mprg;
		xSET_BIT ( obj->pIndexData->progtypes, mptype );
		mpedit ( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}
	do_opedit ( ch, "" );
}

/*
 * RoomProg Support
 */
void rpedit ( CHAR_DATA * ch, MPROG_DATA * mprg, int mptype, char *argument )
{
	if ( mptype != -1 )
	{
		mprg->type = mptype;
		if ( mprg->arglist )
			STRFREE ( mprg->arglist );
		mprg->arglist = STRALLOC ( argument );
	}
	ch->substate = SUB_MPROG_EDIT;
	ch->dest_buf = mprg;
	if ( !mprg->comlist )
		mprg->comlist = STRALLOC ( "" );
	start_editing ( ch, mprg->comlist );
	return;
}
void do_rpedit ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	MPROG_DATA *mprog, *mprg, *mprg_next = NULL;
	int value, mptype = -1, cnt;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't rpedit\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	switch ( ch->substate )
	{
		default:
			break;
		case SUB_MPROG_EDIT:
			if ( !ch->dest_buf )
			{
				send_to_char ( "Fatal error: report to Thoric.\r\n", ch );
				bug ( "do_opedit: sub_oprog_edit: NULL ch->dest_buf", 0 );
				ch->substate = SUB_NONE;
				return;
			}
			mprog = ch->dest_buf;
			if ( mprog->comlist )
				STRFREE ( mprog->comlist );
			mprog->comlist = copy_buffer ( ch );
			stop_editing ( ch );
			return;
	}
	smash_tilde ( argument );
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	value = atoi ( arg2 );
	/*
	 * argument = one_argument( argument, arg3 );
	 */
	if ( arg1[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: rpedit <command> [number] <program> <value>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Command being one of:\r\n", ch );
		send_to_char ( "  add delete insert edit list\r\n", ch );
		send_to_char ( "Program being one of:\r\n", ch );
		send_to_char ( "  act speech rand sleep rest rfight enter\r\n", ch );
		send_to_char ( "  leave death\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "You should be standing in room you wish to edit.\r\n", ch );
		return;
	}
	if ( !can_rmodify ( ch, ch->in_room ) )
		return;
	mprog = ch->in_room->mudprogs;
	set_char_color ( AT_GREEN, ch );
	if ( !str_cmp ( arg1, "list" ) )
	{
		cnt = 0;
		if ( !mprog )
		{
			send_to_char ( "This room has no room programs.\r\n", ch );
			return;
		}
		for ( mprg = mprog; mprg; mprg = mprg->next )
			ch_printf ( ch, "%d>%s %s\r\n%s\r\n", ++cnt, mprog_type_to_name ( mprg->type ), mprg->arglist, mprg->comlist );
		return;
	}
	if ( !str_cmp ( arg1, "edit" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "This room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg3 );
		if ( arg3[0] != STRING_NULL )
		{
			mptype = get_mpflag ( arg3 );
			if ( mptype == -1 )
			{
				send_to_char ( "Unknown program type.\r\n", ch );
				return;
			}
		}
		else
			mptype = -1;
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mpedit ( ch, mprg, mptype, argument );
				xCLEAR_BITS ( ch->in_room->progtypes );
				for ( mprg = mprog; mprg; mprg = mprg->next )
					xSET_BIT ( ch->in_room->progtypes, mprg->type );
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "delete" ) )
	{
		int num;
		bool found;
		if ( !mprog )
		{
			send_to_char ( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg3 );
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = 0;
		found = FALSE;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value )
			{
				mptype = mprg->type;
				found = TRUE;
				break;
			}
		}
		if ( !found )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		cnt = num = 0;
		for ( mprg = mprog; mprg; mprg = mprg->next )
			if ( mprg->type == mptype )
				num++;
		if ( value == 1 )
		{
			mprg_next = ch->in_room->mudprogs;
			ch->in_room->mudprogs = mprg_next->next;
		}
		else
			for ( mprg = mprog; mprg; mprg = mprg_next )
			{
				mprg_next = mprg->next;
				if ( ++cnt == ( value - 1 ) )
				{
					mprg->next = mprg_next->next;
					break;
				}
			}
		if ( mprg_next )
		{
			STRFREE ( mprg_next->arglist );
			STRFREE ( mprg_next->comlist );
			DISPOSE ( mprg_next );
			if ( num <= 1 )
				xREMOVE_BIT ( ch->in_room->progtypes, mptype );
			send_to_char ( "Program removed.\r\n", ch );
		}
		return;
	}
	if ( !str_cmp ( arg2, "insert" ) )
	{
		if ( !mprog )
		{
			send_to_char ( "That room has no room programs.\r\n", ch );
			return;
		}
		argument = one_argument ( argument, arg3 );
		mptype = get_mpflag ( arg2 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( value < 1 )
		{
			send_to_char ( "Program not found.\r\n", ch );
			return;
		}
		if ( value == 1 )
		{
			CREATE ( mprg, MPROG_DATA, 1 );
			xSET_BIT ( ch->in_room->progtypes, mptype );
			mpedit ( ch, mprg, mptype, argument );
			mprg->next = mprog;
			ch->in_room->mudprogs = mprg;
			return;
		}
		cnt = 1;
		for ( mprg = mprog; mprg; mprg = mprg->next )
		{
			if ( ++cnt == value && mprg->next )
			{
				CREATE ( mprg_next, MPROG_DATA, 1 );
				xSET_BIT ( ch->in_room->progtypes, mptype );
				mpedit ( ch, mprg_next, mptype, argument );
				mprg_next->next = mprg->next;
				mprg->next = mprg_next;
				return;
			}
		}
		send_to_char ( "Program not found.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "add" ) )
	{
		mptype = get_mpflag ( arg2 );
		if ( mptype == -1 )
		{
			send_to_char ( "Unknown program type.\r\n", ch );
			return;
		}
		if ( mprog )
			for ( ; mprog->next; mprog = mprog->next )
				;
		CREATE ( mprg, MPROG_DATA, 1 );
		if ( mprog )
			mprog->next = mprg;
		else
			ch->in_room->mudprogs = mprg;
		xSET_BIT ( ch->in_room->progtypes, mptype );
		mpedit ( ch, mprg, mptype, argument );
		mprg->next = NULL;
		return;
	}
	do_rpedit ( ch, "" );
}
void do_rdelete ( CHAR_DATA * ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	if ( ch->substate == SUB_RESTRICTED )
	{
		send_to_char ( "You can't do that while in a subprompt.\r\n", ch );
		return;
	}
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Delete which room?\r\n", ch );
		return;
	}
	/*
	 * Find the room.
	 */
	if ( ! ( location = find_location ( ch, argument ) ) )
	{
		send_to_char ( "No such location.\r\n", ch );
		return;
	}
	/*
	 * Does the player have the right to delete this room?
	 */
	if ( get_trust ( ch ) < sysdata.level_modify_proto && ( location->vnum < ch->pcdata->area->low_r_vnum || location->vnum > ch->pcdata->area->hi_r_vnum ) )
	{
		send_to_char ( "That room is not in your assigned range.\r\n", ch );
		return;
	}
	delete_room ( location );
	fix_exits( );  /* Need to call this to solve a crash */
	ch_printf ( ch, "Room %s has been deleted.\r\n", argument );
	return;
}
void do_odelete ( CHAR_DATA * ch, char *argument )
{
	OBJ_INDEX_DATA *obj;
	int vnum;
	if ( ch->substate == SUB_RESTRICTED )
	{
		send_to_char ( "You can't do that while in a subprompt.\r\n", ch );
		return;
	}
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Delete which object?\r\n", ch );
		return;
	}
	if ( !is_number ( argument ) )
	{
		send_to_char ( "You must specify the object's vnum to delete it.\r\n", ch );
		return;
	}
	vnum = atoi ( argument );
	/*
	 * Find the obj.
	 */
	if ( ! ( obj = get_obj_index ( vnum ) ) )
	{
		send_to_char ( "No such object.\r\n", ch );
		return;
	}
	/*
	 * Does the player have the right to delete this object?
	 */
	if ( get_trust ( ch ) < sysdata.level_modify_proto && ( obj->vnum < ch->pcdata->area->low_o_vnum || obj->vnum > ch->pcdata->area->hi_o_vnum ) )
	{
		send_to_char ( "That object is not in your assigned range.\r\n", ch );
		return;
	}
	delete_obj ( obj );
	ch_printf ( ch, "Object %d has been deleted.\r\n", vnum );
	return;
}
void do_mdelete ( CHAR_DATA * ch, char *argument )
{
	MOB_INDEX_DATA *mob;
	int vnum;
	if ( ch->substate == SUB_RESTRICTED )
	{
		send_to_char ( "You can't do that while in a subprompt.\r\n", ch );
		return;
	}
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Delete which mob?\r\n", ch );
		return;
	}
	if ( !is_number ( argument ) )
	{
		send_to_char ( "You must specify the mob's vnum to delete it.\r\n", ch );
		return;
	}
	vnum = atoi ( argument );
	/*
	 * Find the mob.
	 */
	if ( ! ( mob = get_mob_index ( vnum ) ) )
	{
		send_to_char ( "No such mob.\r\n", ch );
		return;
	}
	/*
	 * Does the player have the right to delete this mob?
	 */
	if ( get_trust ( ch ) < sysdata.level_modify_proto && ( mob->vnum < ch->pcdata->area->low_m_vnum || mob->vnum > ch->pcdata->area->hi_m_vnum ) )
	{
		send_to_char ( "That mob is not in your assigned range.\r\n", ch );
		return;
	}
	delete_mob ( mob );
	ch_printf ( ch, "Mob %d has been deleted.\r\n", vnum );
	return;
}

/*
 *  Mobile and Object Program Copying
 *  Last modified Feb. 24 1999
 *  Mystaric
 */
void mpcopy ( MPROG_DATA * source, MPROG_DATA * destination )
{
	destination->type = source->type;
	destination->triggered = source->triggered;
	destination->resetdelay = source->resetdelay;
	destination->arglist = STRALLOC ( source->arglist );
	destination->comlist = STRALLOC ( source->comlist );
	destination->next = NULL;
}
void do_opcopy ( CHAR_DATA * ch, char *argument )
{
	char sobj[MAX_INPUT_LENGTH];
	char prog[MAX_INPUT_LENGTH];
	char num[MAX_INPUT_LENGTH];
	char dobj[MAX_INPUT_LENGTH];
	OBJ_DATA *source = NULL, *destination = NULL;
	MPROG_DATA *source_oprog = NULL, *dest_oprog = NULL, *source_oprg = NULL, *dest_oprg = NULL;
	int value = -1, optype = -1, cnt = 0;
	bool COPY = FALSE;
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't opcopy\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	smash_tilde ( argument );
	argument = one_argument ( argument, sobj );
	argument = one_argument ( argument, prog );
	if ( sobj[0] == STRING_NULL || prog[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: opcopy <source object> <program> [number] <destination object>\r\n", ch );
		send_to_char ( "        opcopy <source object> all <destination object>\r\n", ch );
		send_to_char ( "        opcopy <source object> all <destination object> <program>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Program being one of:\r\n", ch );
		send_to_char ( "  act speech rand wear remove sac zap get\r\n", ch );
		send_to_char ( "  drop damage repair greet exa use\r\n", ch );
		send_to_char ( "  pull push (for levers,pullchains,buttons)\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Object should be in your inventory to edit.\r\n", ch );
		return;
	}
	if ( !strcmp ( prog, "all" ) )
	{
		argument = one_argument ( argument, dobj );
		argument = one_argument ( argument, prog );
		optype = get_mpflag ( prog );
		COPY = TRUE;
	}
	else
	{
		argument = one_argument ( argument, num );
		argument = one_argument ( argument, dobj );
		value = atoi ( num );
	}
	if ( get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( source = get_obj_carry ( ch, sobj ) ) == NULL )
		{
			send_to_char ( "You aren't carrying source object.\r\n", ch );
			return;
		}
		if ( ( destination = get_obj_carry ( ch, dobj ) ) == NULL )
		{
			send_to_char ( "You aren't carrying destination object.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( source = get_obj_world ( ch, sobj ) ) == NULL )
		{
			send_to_char ( "Can't find source object in all the realms.\r\n", ch );
			return;
		}
		if ( ( destination = get_obj_world ( ch, dobj ) ) == NULL )
		{
			send_to_char ( "Can't find destination object in all the realms.\r\n", ch );
			return;
		}
	}
	if ( source == destination )
	{
		send_to_char ( "Source and destination objects cannot be the same\r\n", ch );
		return;
	}
	if ( !can_omodify ( ch, destination ) )
	{
		send_to_char ( "You cannot modify destination object.\r\n", ch );
		return;
	}
	if ( !IS_OBJ_STAT ( destination, ITEM_PROTOTYPE ) )
	{
		send_to_char ( "Destination object must have prototype flag.\r\n", ch );
		return;
	}
	set_char_color ( AT_PLAIN, ch );
	source_oprog = source->pIndexData->mudprogs;
	dest_oprog = destination->pIndexData->mudprogs;
	set_char_color ( AT_GREEN, ch );
	if ( !source_oprog )
	{
		send_to_char ( "Source object has no mob programs.\r\n", ch );
		return;
	}
	if ( COPY )
	{
		for ( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
		{
			if ( optype == source_oprg->type || optype == -1 )
			{
				if ( dest_oprog != NULL )
					for ( ; dest_oprog->next; dest_oprog = dest_oprog->next )
						;
				CREATE ( dest_oprg, MPROG_DATA, 1 );
				if ( dest_oprog )
					dest_oprog->next = dest_oprg;
				else
				{
					destination->pIndexData->mudprogs = dest_oprg;
					dest_oprog = dest_oprg;
				}
				mpcopy ( source_oprg, dest_oprg );
				xSET_BIT ( destination->pIndexData->progtypes, dest_oprg->type );
				cnt++;
			}
		}
		if ( cnt == 0 )
		{
			ch_printf ( ch, "No such program in source object\r\n" );
			return;
		}
		ch_printf ( ch, "%d programs successfully copied from %s to %s.\r\n", cnt, sobj, dobj );
		return;
	}
	if ( value < 1 )
	{
		send_to_char ( "No such program in source object.\r\n", ch );
		return;
	}
	optype = get_mpflag ( prog );
	for ( source_oprg = source_oprog; source_oprg; source_oprg = source_oprg->next )
	{
		if ( ++cnt == value && source_oprg->type == optype )
		{
			if ( dest_oprog != NULL )
				for ( ; dest_oprog->next; dest_oprog = dest_oprog->next )
					;
			CREATE ( dest_oprg, MPROG_DATA, 1 );
			if ( dest_oprog )
				dest_oprog->next = dest_oprg;
			else
				destination->pIndexData->mudprogs = dest_oprg;
			mpcopy ( source_oprg, dest_oprg );
			xSET_BIT ( destination->pIndexData->progtypes, dest_oprg->type );
			ch_printf ( ch, "%s program %d from %s successfully copied to %s.\r\n", prog, value, sobj, dobj );
			return;
		}
	}
	if ( !source_oprg )
	{
		send_to_char ( "No such program in source object.\r\n", ch );
		return;
	}
	do_opcopy ( ch, "" );
}
void do_mpcopy ( CHAR_DATA * ch, char *argument )
{
	char smob[MAX_INPUT_LENGTH];
	char prog[MAX_INPUT_LENGTH];
	char num[MAX_INPUT_LENGTH];
	char dmob[MAX_INPUT_LENGTH];
	CHAR_DATA *source = NULL, *destination = NULL;
	MPROG_DATA *source_mprog = NULL, *dest_mprog = NULL, *source_mprg = NULL, *dest_mprg = NULL;
	int value = -1, mptype = -1, cnt = 0;
	bool COPY = FALSE;
	set_char_color ( AT_PLAIN, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mob's can't opcop\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor\r\n", ch );
		return;
	}
	smash_tilde ( argument );
	argument = one_argument ( argument, smob );
	argument = one_argument ( argument, prog );
	if ( smob[0] == STRING_NULL || prog[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: mpcopy <source mobile> <program> [number] <destination mobile>\r\n", ch );
		send_to_char ( "        mpcopy <source mobile> all <destination mobile>\r\n", ch );
		send_to_char ( "        mpcopy <source mobile> all <destination mobile> <program>\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Program being one of:\r\n", ch );
		send_to_char ( "  act speech rand fight hitprcnt greet allgreet\r\n", ch );
		send_to_char ( "  entry give bribe death time hour script\r\n", ch );
		return;
	}
	if ( !strcmp ( prog, "all" ) )
	{
		argument = one_argument ( argument, dmob );
		argument = one_argument ( argument, prog );
		mptype = get_mpflag ( prog );
		COPY = TRUE;
	}
	else
	{
		argument = one_argument ( argument, num );
		argument = one_argument ( argument, dmob );
		value = atoi ( num );
	}
	if ( get_trust ( ch ) < LEVEL_GOD )
	{
		if ( ( source = get_char_room ( ch, smob ) ) == NULL )
		{
			send_to_char ( "Source mobile is not present.\r\n", ch );
			return;
		}
		if ( ( destination = get_char_room ( ch, dmob ) ) == NULL )
		{
			send_to_char ( "Destination mobile is not present.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( source = get_char_world ( ch, smob ) ) == NULL )
		{
			send_to_char ( "Can't find source mobile\r\n", ch );
			return;
		}
		if ( ( destination = get_char_world ( ch, dmob ) ) == NULL )
		{
			send_to_char ( "Can't find destination mobile\r\n", ch );
			return;
		}
	}
	if ( source == destination )
	{
		send_to_char ( "Source and destination mobiles cannot be the same\r\n", ch );
		return;
	}
	if ( get_trust ( ch ) < source->level || !IS_NPC ( source ) || get_trust ( ch ) < destination->level || !IS_NPC ( destination ) )
	{
		send_to_char ( "You can't do that!\r\n", ch );
		return;
	}
	if ( !can_mmodify ( ch, destination ) )
	{
		send_to_char ( "You cannot modify destination mobile.\r\n", ch );
		return;
	}
	if ( !xIS_SET ( destination->act, ACT_PROTOTYPE ) )
	{
		send_to_char ( "Destination mobile must have a prototype flag to mpcopy.\r\n", ch );
		return;
	}
	source_mprog = source->pIndexData->mudprogs;
	dest_mprog = destination->pIndexData->mudprogs;
	set_char_color ( AT_GREEN, ch );
	if ( !source_mprog )
	{
		send_to_char ( "Source mobile has no mob programs.\r\n", ch );
		return;
	}
	if ( COPY )
	{
		for ( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
		{
			if ( mptype == source_mprg->type || mptype == -1 )
			{
				if ( dest_mprog != NULL )
					for ( ; dest_mprog->next; dest_mprog = dest_mprog->next )
						;
				CREATE ( dest_mprg, MPROG_DATA, 1 );
				if ( dest_mprog )
					dest_mprog->next = dest_mprg;
				else
				{
					destination->pIndexData->mudprogs = dest_mprg;
					dest_mprog = dest_mprg;
				}
				mpcopy ( source_mprg, dest_mprg );
				xSET_BIT ( destination->pIndexData->progtypes, dest_mprg->type );
				cnt++;
			}
		}
		if ( cnt == 0 )
		{
			ch_printf ( ch, "No such program in source mobile\r\n" );
			return;
		}
		ch_printf ( ch, "%d programs successfully copied from %s to %s.\r\n", cnt, smob, dmob );
		return;
	}
	if ( value < 1 )
	{
		send_to_char ( "No such program in source mobile.\r\n", ch );
		return;
	}
	mptype = get_mpflag ( prog );
	for ( source_mprg = source_mprog; source_mprg; source_mprg = source_mprg->next )
	{
		if ( ++cnt == value && source_mprg->type == mptype )
		{
			if ( dest_mprog != NULL )
				for ( ; dest_mprog->next; dest_mprog = dest_mprog->next )
					;
			CREATE ( dest_mprg, MPROG_DATA, 1 );
			if ( dest_mprog )
				dest_mprog->next = dest_mprg;
			else
				destination->pIndexData->mudprogs = dest_mprg;
			mpcopy ( source_mprg, dest_mprg );
			xSET_BIT ( destination->pIndexData->progtypes, dest_mprg->type );
			ch_printf ( ch, "%s program %d from %s successfully copied to %s.\r\n", prog, value, smob, dmob );
			return;
		}
	}
	if ( !source_mprg )
	{
		send_to_char ( "No such program in source mobile.\r\n", ch );
		return;
	}
	do_mpcopy ( ch, "" );
}

/*
 * function to allow modification of an area's climate
 * Last modified: July 15, 1997
 * Fireblade
 */
void do_climate ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	AREA_DATA *area;
	/*
	 * Little error checking
	 */
	if ( !ch )
	{
		bug ( "do_climate: NULL character." );
		return;
	}
	else if ( !ch->in_room )
	{
		bug ( "do_climate: character not in a room." );
		return;
	}
	else if ( !ch->in_room->area )
	{
		bug ( "do_climate: character not in an area." );
		return;
	}
	else if ( !ch->in_room->area->weather )
	{
		bug ( "do_climate: area with NULL weather data." );
		return;
	}
	set_char_color ( AT_BLUE, ch );
	area = ch->in_room->area;
	argument = strlower ( argument );
	argument = one_argument ( argument, arg );
	/*
	 * Display current climate settings
	 */
	if ( arg[0] == STRING_NULL )
	{
		NEIGHBOR_DATA *neigh;
		ch_printf ( ch, "%s:\r\n", area->name );
		ch_printf ( ch, "\tTemperature:\t%s\r\n", temp_settings[area->weather->climate_temp] );
		ch_printf ( ch, "\tPrecipitation:\t%s\r\n", precip_settings[area->weather->climate_precip] );
		ch_printf ( ch, "\tWind:\t\t%s\r\n", wind_settings[area->weather->climate_wind] );
		if ( area->weather->first_neighbor )
			ch_printf ( ch, "\r\nNeighboring weather systems:\r\n" );
		for ( neigh = area->weather->first_neighbor; neigh; neigh = neigh->next )
		{
			ch_printf ( ch, "\t%s\r\n", neigh->name );
		}
		return;
	}
	/*
	 * set climate temperature
	 */
	else if ( !str_cmp ( arg, "temp" ) )
	{
		int i;
		argument = one_argument ( argument, arg );
		for ( i = 0; i < MAX_CLIMATE; i++ )
		{
			if ( str_cmp ( arg, temp_settings[i] ) )
				continue;
			area->weather->climate_temp = i;
			ch_printf ( ch, "The climate temperature " "for %s is now %s.\r\n", area->name, temp_settings[i] );
			break;
		}
		if ( i == MAX_CLIMATE )
		{
			ch_printf ( ch, "Possible temperature " "settings:\r\n" );
			for ( i = 0; i < MAX_CLIMATE; i++ )
			{
				ch_printf ( ch, "\t%s\r\n", temp_settings[i] );
			}
		}
		return;
	}
	/*
	 * set climate precipitation
	 */
	else if ( !str_cmp ( arg, "precip" ) )
	{
		int i;
		argument = one_argument ( argument, arg );
		for ( i = 0; i < MAX_CLIMATE; i++ )
		{
			if ( str_cmp ( arg, precip_settings[i] ) )
				continue;
			area->weather->climate_precip = i;
			ch_printf ( ch, "The climate precipitation " "for %s is now %s.\r\n", area->name, precip_settings[i] );
			break;
		}
		if ( i == MAX_CLIMATE )
		{
			ch_printf ( ch, "Possible precipitation " "settings:\r\n" );
			for ( i = 0; i < MAX_CLIMATE; i++ )
			{
				ch_printf ( ch, "\t%s\r\n", precip_settings[i] );
			}
		}
		return;
	}
	/*
	 * set climate wind
	 */
	else if ( !str_cmp ( arg, "wind" ) )
	{
		int i;
		argument = one_argument ( argument, arg );
		for ( i = 0; i < MAX_CLIMATE; i++ )
		{
			if ( str_cmp ( arg, wind_settings[i] ) )
				continue;
			area->weather->climate_wind = i;
			ch_printf ( ch, "The climate wind for %s " "is now %s.\r\n", area->name, wind_settings[i] );
			break;
		}
		if ( i == MAX_CLIMATE )
		{
			ch_printf ( ch, "Possible wind settings:\r\n" );
			for ( i = 0; i < MAX_CLIMATE; i++ )
			{
				ch_printf ( ch, "\t%s\r\n", wind_settings[i] );
			}
		}
		return;
	}
	/*
	 * add or remove neighboring weather systems
	 */
	else if ( !str_cmp ( arg, "neighbor" ) )
	{
		NEIGHBOR_DATA *neigh;
		AREA_DATA *tarea;
		if ( argument[0] == STRING_NULL )
		{
			ch_printf ( ch, "Add or remove which area?\r\n" );
			return;
		}
		/*
		 * look for a matching list item
		 */
		for ( neigh = area->weather->first_neighbor; neigh; neigh = neigh->next )
		{
			if ( nifty_is_name ( argument, neigh->name ) )
				break;
		}
		/*
		 * if the a matching list entry is found, remove it
		 */
		if ( neigh )
		{
			/*
			 * look for the neighbor area in question
			 */
			if ( ! ( tarea = neigh->address ) )
				tarea = get_area ( neigh->name );
			/*
			 * if there is an actual neighbor area
			 */
			/*
			 * remove its entry to this area
			 */
			if ( tarea )
			{
				NEIGHBOR_DATA *tneigh;
				tarea = neigh->address;
				for ( tneigh = tarea->weather->first_neighbor; tneigh; tneigh = tneigh->next )
				{
					if ( !strcmp ( area->name, tneigh->name ) )
						break;
				}
				UNLINK ( tneigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
				STRFREE ( tneigh->name );
				DISPOSE ( tneigh );
			}
			UNLINK ( neigh, area->weather->first_neighbor, area->weather->last_neighbor, next, prev );
			ch_printf ( ch, "The weather in %s and %s " "no longer affect each other.\r\n", neigh->name, area->name );
			STRFREE ( neigh->name );
			DISPOSE ( neigh );
		}
		/*
		 * otherwise add an entry
		 */
		else
		{
			tarea = get_area ( argument );
			if ( !tarea )
			{
				ch_printf ( ch, "No such area exists.\r\n" );
				return;
			}
			else if ( tarea == area )
			{
				ch_printf ( ch, "%s already affects its " "own weather.\r\n", area->name );
				return;
			}
			/*
			 * add the entry
			 */
			CREATE ( neigh, NEIGHBOR_DATA, 1 );
			neigh->name = STRALLOC ( tarea->name );
			neigh->address = tarea;
			LINK ( neigh, area->weather->first_neighbor, area->weather->last_neighbor, next, prev );
			/*
			 * add an entry to the neighbor's list
			 */
			CREATE ( neigh, NEIGHBOR_DATA, 1 );
			neigh->name = STRALLOC ( area->name );
			neigh->address = area;
			LINK ( neigh, tarea->weather->first_neighbor, tarea->weather->last_neighbor, next, prev );
			ch_printf ( ch, "The weather in %s and %s now " "affect one another.\r\n", tarea->name, area->name );
		}
		return;
	}
	else
	{
		ch_printf ( ch, "Climate may only be followed by one " "of the following fields:\r\n" );
		ch_printf ( ch, "\ttemp\r\n" );
		ch_printf ( ch, "\tprecip\r\n" );
		ch_printf ( ch, "\twind\r\n" );
		ch_printf ( ch, "\tneighbor\r\n" );
		return;
	}
}

/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelCreate ( relation_type tp, void *actor, void *subject )
{
	REL_DATA *tmp;
	if ( tp < relMSET_ON || tp > relOSET_ON )
	{
		bug ( "RelCreate: invalid type (%d)", tp );
		return;
	}
	if ( !actor )
	{
		bug ( "RelCreate: NULL actor" );
		return;
	}
	if ( !subject )
	{
		bug ( "RelCreate: NULL subject" );
		return;
	}
	for ( tmp = first_relation; tmp; tmp = tmp->next )
		if ( tmp->Type == tp && tmp->Actor == actor && tmp->Subject == subject )
		{
			bug ( "RelCreate: duplicated relation" );
			return;
		}
	CREATE ( tmp, REL_DATA, 1 );
	tmp->Type = tp;
	tmp->Actor = actor;
	tmp->Subject = subject;
	LINK ( tmp, first_relation, last_relation, next, prev );
}

/*
 * Relations created to fix a crash bug with oset on and rset on
 * code by: gfinello@mail.karmanet.it
 */
void RelDestroy ( relation_type tp, void *actor, void *subject )
{
	REL_DATA *rq;
	if ( tp < relMSET_ON || tp > relOSET_ON )
	{
		bug ( "RelDestroy: invalid type (%d)", tp );
		return;
	}
	if ( !actor )
	{
		bug ( "RelDestroy: NULL actor" );
		return;
	}
	if ( !subject )
	{
		bug ( "RelDestroy: NULL subject" );
		return;
	}
	for ( rq = first_relation; rq; rq = rq->next )
		if ( rq->Type == tp && rq->Actor == actor && rq->Subject == subject )
		{
			UNLINK ( rq, first_relation, last_relation, next, prev );
			/*
			 * Dispose will also set to NULL the passed parameter
			 */
			DISPOSE ( rq );
			break;
		}
}
void do_makerooms ( CHAR_DATA * ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	AREA_DATA *pArea;
	int vnum, x;
	int room_count;
	int room_hold[MAX_RGRID_ROOMS];
	pArea = ch->pcdata->area;
	if ( !pArea )
	{
		send_to_char ( "You must have an area assigned to do this.\r\n", ch );
		return;
	}
	if ( !argument || argument[0] == STRING_NULL )
	{
		send_to_char ( "Create a block of rooms.\r\n", ch );
		send_to_char ( "Usage: makerooms <# of rooms>\r\n", ch );
		return;
	}
	x = atoi ( argument );
	ch_printf ( ch, "Attempting to create a block of %d rooms.\r\n", x );
	if ( x > 300 )
	{
		ch_printf ( ch, "The maximum number of rooms this mud can create at once is 300.\r\n" );
		return;
	}
	room_count = 0;
	if ( pArea->low_r_vnum + x > pArea->hi_r_vnum )
	{
		send_to_char ( "You don't even have that many rooms assigned to you.\r\n", ch );
		return;
	}
	for ( vnum = pArea->low_r_vnum; vnum <= pArea->hi_r_vnum; vnum++ )
	{
		if ( get_room_index ( vnum ) == NULL )
			room_count++;
		if ( room_count >= x )
			break;
	}
	if ( room_count < x )
	{
		send_to_char ( "There aren't enough free rooms in your assigned range!\r\n", ch );
		return;
	}
	send_to_char ( "Creating the rooms...\r\n", ch );
	room_count = 0;
	vnum = pArea->low_r_vnum;
	while ( room_count < x )
	{
		if ( get_room_index ( vnum ) == NULL )
		{
			room_hold[room_count++] = vnum;
			location = make_room ( vnum, ch->pcdata->area );
			if ( !location )
			{
				bug ( "%s: make_room failed", __FUNCTION__ );
				return;
			}
			location->area = ch->pcdata->area;
		}
		vnum++;
	}
	ch_printf ( ch, "%d rooms created.\r\n", room_count );
	return;
}

/* Consolidated *assign function.
 * Assigns room/obj/mob ranges and initializes new zone - Samson 2-12-99
 */
/* Bugfix: Vnum range would not be saved properly without placeholders at both ends - Samson 1-6-00 */
void do_vassign ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
	int lo, hi;
	CHAR_DATA *victim, *mob;
	ROOM_INDEX_DATA *room;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	AREA_DATA *tarea;
	char filename[256];
	set_char_color ( AT_IMMORT, ch );
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	lo = atoi ( arg2 );
	hi = atoi ( arg3 );
	if ( arg1[0] == STRING_NULL || lo < 0 || hi < 0 )
	{
		send_to_char ( "Syntax: vassign <who> <low> <high>\r\n", ch );
		return;
	}
	if ( ! ( victim = get_char_world ( ch, arg1 ) ) )
	{
		send_to_char ( "They don't seem to be around.\r\n", ch );
		return;
	}
	if ( IS_NPC ( victim ) || get_trust ( victim ) < LEVEL_CREATOR )
	{
		send_to_char ( "They wouldn't know what to do with a vnum range.\r\n", ch );
		return;
	}
	if ( victim->pcdata->area && lo != 0 )
	{
		send_to_char ( "You cannot assign them a range, they already have one!\r\n", ch );
		return;
	}
	if ( lo > hi )
	{
		send_to_char ( "Unacceptable vnum range.\r\n", ch );
		return;
	}
	if ( lo == 0 )
		hi = 0;
	victim->pcdata->r_range_lo = lo;
	victim->pcdata->r_range_hi = hi;
	victim->pcdata->o_range_lo = lo;
	victim->pcdata->o_range_hi = hi;
	victim->pcdata->m_range_lo = lo;
	victim->pcdata->m_range_hi = hi;
	assign_area ( victim );
	send_to_char ( "Done.\r\n", ch );
	ch_printf ( victim, "%s has assigned you the vnum range %d - %d.\r\n", ch->name, lo, hi );
	assign_area ( victim ); /* Put back by Thoric on 02/07/96 */
	if ( !victim->pcdata->area )
	{
		bug ( "%s: assign_area failed", __FUNCTION__ );
		return;
	}
	tarea = victim->pcdata->area;
	if ( lo == 0 ) /* Scryn 8/12/95 */
	{
		REMOVE_BIT ( tarea->status, AREA_LOADED );
		SET_BIT ( tarea->status, AREA_DELETED );
	}
	else
	{
		SET_BIT ( tarea->status, AREA_LOADED );
		REMOVE_BIT ( tarea->status, AREA_DELETED );
	}
	/*
	 * Initialize first and last rooms in range
	 */
	room = make_room ( lo, ch->pcdata->area );
	if ( !room )
	{
		bug ( "%s: make_room failed to initialize first room.", __FUNCTION__ );
		return;
	}
	room->area = tarea;
	room = make_room ( hi, ch->pcdata->area );
	if ( !room )
	{
		bug ( "%s: make_room failed to initialize last room.", __FUNCTION__ );
		return;
	}
	room->area = tarea;
	/*
	 * Initialize first mob in range
	 */
	pMobIndex = make_mobile ( lo, 0, "first mob", 1 );
	if ( !pMobIndex )
	{
		log_string ( "do_vassign: make_mobile failed to initialize first mob." );
		return;
	}
	mob = create_mobile ( pMobIndex );
	char_to_room ( mob, room );
	/*
	 * Initialize last mob in range
	 */
	pMobIndex = make_mobile ( hi, 0, "last mob", 1 );
	if ( !pMobIndex )
	{
		log_string ( "do_vassign: make_mobile failed to initialize last mob." );
		return;
	}
	mob = create_mobile ( pMobIndex );
	char_to_room ( mob, room );
	/*
	 * Initialize first obj in range
	 */
	pObjIndex = make_object ( lo, 0, "first obj" );
	if ( !pObjIndex )
	{
		log_string ( "do_vassign: make_object failed to initialize first obj." );
		return;
	}
	obj = create_object ( pObjIndex, 0 );
	obj_to_room ( obj, room, NULL );
	/*
	 * Initialize last obj in range
	 */
	pObjIndex = make_object ( hi, 0, "last obj" );
	if ( !pObjIndex )
	{
		log_string ( "do_vassign: make_object failed to initialize last obj." );
		return;
	}
	obj = create_object ( pObjIndex, 0 );
	obj_to_room ( obj, room, NULL );
	/*
	 * Save character and newly created zone
	 */
	save_char_obj ( victim );
	if ( !IS_SET ( tarea->status, AREA_DELETED ) )
	{
		snprintf ( filename, 256, "%s%s", BUILD_DIR, tarea->filename );
		fold_area ( tarea, filename, FALSE );
	}
	set_char_color ( AT_IMMORT, ch );
	ch_printf ( ch, "Vnum range set for %s and initialized.\r\n", victim->name );
	return;
}

void do_rdig ( CHAR_DATA * ch, char *argument )
{
	char arg[MIL];
	char buf[MSL];
	ROOM_INDEX_DATA *newloc, *orig;
	AREA_DATA *pArea;
	int vnum, exdir;
	char tmpcmd[MAX_INPUT_LENGTH];
	EXIT_DATA *rExit;

	orig = ch->in_room;

	argument = one_argument ( argument, arg );

	if ( arg[0] == '\0' )
	{
		send_to_char ( "Dig out a new room or dig into an existing room.\r\n", ch );
		send_to_char ( "Usage: rdig <dir>\r\n", ch );
		return;
	}
	if ( !ch->pcdata->area )
	{
		send_to_char ( "you need to aassign the zone first", ch );
		return;
	}

	/*
	 * Check to see if builder can modify area
	 */
	if ( !can_rmodify ( ch, orig ) )
		return;

	exdir = get_dir ( arg );
	rExit = get_exit ( orig, exdir );

	if ( !rExit )
	{
		pArea = ch->pcdata->area;
		vnum = pArea->low_r_vnum;

		/*
		 * Check area for empty rooms
		 */
		while ( vnum <= pArea->hi_r_vnum && get_room_index ( vnum ) != NULL )
		{
			vnum++;
		}

		if ( vnum > pArea->hi_r_vnum )
		{
			send_to_char ( "No empty upper rooms could be found.\r\n", ch );
			return;
		}

		sprintf ( buf, "Digging out room %d, Direction: %s.\r\n", vnum, arg );
		send_to_char ( buf, ch );

		newloc = make_room ( vnum, ch->pcdata->area );
		if ( !newloc )
		{
			bug ( "%s", "rdig: make_room failed" );
			return;
		}
		newloc->area = ch->pcdata->area;

		sprintf ( tmpcmd, "bexit %s %d", arg, vnum );
		send_to_char ( tmpcmd, ch );
		do_rset ( ch, tmpcmd );

	}
	else
	{
		vnum = rExit->vnum;
		newloc = get_room_index ( vnum );
		sprintf ( buf, "Digging22 %s into room %d.\r\n", arg, vnum );
		send_to_char ( buf, ch );
	}

	/*
	 * Copy original room to new room to save time
	 */
	newloc->name = QUICKLINK ( orig->name );
	newloc->description = str_dup ( orig->description );
	newloc->sector_type = orig->sector_type;
	newloc->room_flags = orig->room_flags;

	/*
	 * Move to new Room
	 */
	sprintf ( buf, "%d", vnum );
	do_goto ( ch, buf );
	return;
}
