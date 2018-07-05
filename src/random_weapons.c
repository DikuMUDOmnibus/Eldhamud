/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                  Random Weapons Drop Module                              *
 ****************************************************************************/
#include "./Headers/mud.h"

#define OBJ_VNUM_RND_WEAPON  15
#define PRE_LAST_ROW 4
#define SUF_LAST_ROW 39
#define LAST_ROW 41

struct random_weapon_prefix
{
	short row_number; 	/* row number in the table         */
	char *prefix_up;     /* upper case suffix for long desc */
	char *prefix_lo;     /* lower case suffix for short desc*/
	char *prefix_name; 	/* prefix, keywords                */
};
const struct random_weapon_prefix random_weapon_prefix_table[] =
{
        //Row Up  Lo   Name
	{0,"An", "an", "iron"},
	{1, "A",   "a","steel"},
	{2, "A",   "a","bronze"},
	{3, "A",   "a","gold"},
	{4, "A",   "a","bamboo"}
};

struct random_weapon_suffix
{
	short row_number; 	/* row number in the table */
	char *suffix_name; 		/* prefix, keywords          */
};
const struct random_weapon_suffix random_weapon_suffix_table[] =
{
	//Row  Name
	{0, "of the mantis"},
	{1, "of the ox"},
	{2, "of the tiger"},
	{3, "of the monkey"},
	{4, "of the tiger"},
	{5, "of the snake"},
	{6, "of the crane"},
	{7, "of the bear"},
	{8, "of the dragon"},
	{9, "of the crab"},
	{10, "of the horse"},
	{11, "of the eagle"},
	{12, "of thunder"},
	{13, "of the sun"},
	{14, "of wind"},
	{15, "of rain"},
	{16, "of stars"},
	{17, "of death"},
	{18, "of life"},
	{19, "of earthquakes"},
	{20, "of ghouls"},
	{21, "of fire"},
	{22, "of nature"},
	{23, "of water"},
	{24, "of air"},
	{25, "of night"},
	{26, "of day"},
	{27, "of war"},
	{28, "of truth"},
	{29, "of glory"},
	{30, "of the panda"},
	{31, "of fate"},
	{32, "of ascendance"},
	{33, "of the fist"},
	{34, "of the foot"},
	{35, "of heart"},
	{36, "of battle"},
	{37, "of the gods"},
	{38, "of the lion"},
	{39, "of the scorpian"},
};

struct random_weapons
{
	short row_number; /* row number in the table */
	char *name;       /* Name, keywords       */
	short itemvalue4; /* Weapon Type          */
	short itemvalue3; /* Damge Type           */
	short weight;     /* Base weight of item  */
};
const struct random_weapons random_weapons_table[] =
{
//      row  name                V4 V3  Weight
	{0, "dagger",  	 	  2, 6, 5},
	{1, "sai",  	 	  2, 6, 5},
	{2, "tanto",  	 	  2, 6, 5},
	{3, "emi peircers",  	  2, 6, 5},
	{4, "tamo daggers",  	  2, 6, 5},
	{5, "butterfly knives",   2, 6, 5},
	{6, "sacrificial dagger", 2, 6, 5},
	{7, "knife",  	 	  2, 6, 5},
	{8, "blade",  	 	  2, 6, 5},
	{9, "dagger",  	 	  2, 6, 10},
	{10, "sword",   	  1, 1, 10},
	{11, "katana",   	  1, 1, 10},
	{12, "twin hook sword",   1, 1, 10},
	{13, "archaic sword",     1, 1, 10},
	{14, "kang si sword",     1, 1, 10},
	{15, "dadao",   	  1, 1, 10},
	{16, "odachi",    	  1, 1, 10},
	{17, "9 ring sword", 	  1, 1, 10},
	{18, "wakizashi",    	  1, 1, 10},
	{19, "jintachi",    	  1, 1, 10},
	{20, "hammer",   	  5, 4, 15},
	{21, "mace",   		  5, 4, 15},
	{22, "war hammer",   	  5, 4, 15},
	{23, "jitte",   	  5, 4, 15},
	{24, "masakari",   	  5, 4, 15},
	{25, "war scythe",   	  5, 4, 15},
	{26, "muck rake",   	  5, 4, 15},
	{27, "monk cudgel",   	  5, 4, 15},
	{28, "hammer",   	  5, 4, 15},
	{29, "hammer",   	  5, 4, 15},
	{30, "polearm", 	 12, 0, 15},
	{31, "bo Stick", 	 12, 0, 15},
	{32, "jo Stick", 	 12, 0, 15},
	{33, "escrima stick",    12, 0, 15},
	{34, "naginata", 	 12, 0, 15},
	{35, "sodegarami", 	 12, 0, 15},
	{36, "long yari", 	 12, 0, 15},
	{37, "wushu kwadao", 	 12, 0, 15},
	{38, "water staff", 	 12, 0, 15},
	{39, "short yari", 	 12, 0, 15},
	{40, "tiger talon", 	 4, 8, 5},
	{41, "whip", 		 3, 5, 10},
};


void do_random_weapon ( CHAR_DATA * ch )
{
	OBJ_DATA *created_weapon;
	char buf[MAX_INPUT_LENGTH];
	short rownumber, pre_rownumber, suf_rownumber, random_number;

	pre_rownumber = number_range ( 0, PRE_LAST_ROW );
	suf_rownumber = number_range ( 0, SUF_LAST_ROW );
	rownumber     = number_range ( 0, LAST_ROW );


	created_weapon = create_object ( get_obj_index ( OBJ_VNUM_RND_WEAPON ), ch->level );

	sprintf ( buf, "%s %s %s",
	          random_weapon_prefix_table[pre_rownumber].prefix_name,
	          random_weapons_table[rownumber].name,
	          random_weapon_suffix_table[suf_rownumber].suffix_name
	        );
	STRFREE ( created_weapon->name );
	created_weapon->name = STRALLOC ( buf );

	sprintf ( buf, "%s %s %s %s",
	          random_weapon_prefix_table[pre_rownumber].prefix_lo,
	          random_weapon_prefix_table[pre_rownumber].prefix_name,
	          random_weapons_table[rownumber].name,
	          random_weapon_suffix_table[suf_rownumber].suffix_name
	        );
	STRFREE ( created_weapon->short_descr );
	created_weapon->short_descr = STRALLOC ( buf );

	sprintf ( buf, "%s %s %s %s",
	          random_weapon_prefix_table[pre_rownumber].prefix_up,
	          random_weapon_prefix_table[pre_rownumber].prefix_name,
	          random_weapons_table[rownumber].name,
	          random_weapon_suffix_table[suf_rownumber].suffix_name
	        );
	STRFREE ( created_weapon->description );
	created_weapon->description = STRALLOC ( buf );

	created_weapon->level = ch->level;
	created_weapon->value[5] = ch->level;
	created_weapon->value[4] = random_weapons_table[rownumber].itemvalue4;
	created_weapon->value[3] = random_weapons_table[rownumber].itemvalue3;
	created_weapon->value[2] = number_fuzzy ( 8 + created_weapon->level * 3 );
	created_weapon->value[1] = number_fuzzy ( 3 + created_weapon->level * 1.5 );
	created_weapon->value[0] = 13;
	created_weapon->timer = 5;

	created_weapon->cost = number_fuzzy ( ch->level * 100 );
	created_weapon->weight = random_weapons_table[rownumber].weight;

	random_number = number_range ( 0, 100 );
	if ( random_number > 65 && random_number < 85 )
	{
		xSET_BIT ( created_weapon->extra_flags, ITEM_REFINED );
		created_weapon->value[1] += 1;
		created_weapon->value[2] += 2;
	}
	else if ( random_number > 84 && random_number < 95 )
	{
		xSET_BIT ( created_weapon->extra_flags, ITEM_UNIQUE );
		created_weapon->value[1] += 2;
		created_weapon->value[2] += 4;
	}
	else if ( random_number > 94 && random_number < 101 )
	{
		xSET_BIT ( created_weapon->extra_flags, ITEM_ELITE );
		created_weapon->value[1] += 4;
		created_weapon->value[2] += 8;
	}
	obj_to_room ( created_weapon, ch->in_room, ch );
	return;
}
