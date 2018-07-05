/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                  Random Treasure Drop Module                             *
 ****************************************************************************/
#include "./Headers/mud.h"
#define OBJ_VNUM_RND_TREASURE  16
#define PRE_ROW 14
#define MID_ROW 15
#define LAST_ROW 17
struct random_treasures
{
	short row_number; /* row number in the table */
	char *name; /* Name, keywords       */
	char *short_descr;   /* Short Descr          */
	char *long_descr; /* Long descr           */
	short cost; /* Base cost of item    */
	short weight;  /* Base weight of item  */
};

struct prefix_treasures
{
	short row_number; /* row number in the table */
	char *name; /* Name, keywords       */
};

const struct prefix_treasures prefix_treasures_table[] =
{
// row name
	{0, "A finely crafted"},
	{1, "A well made"},
	{2, "An exquisite"},
	{3, "A masterly created"},
	{4, "A poorly crafted"},
	{5, "A crafted"},
	{6, "An attractive"},
	{7, "An elegant"},
	{8, "A rare"},
	{9, "A flawless"},
	{10, "A superiorly crafted"},
	{11, "A shabby old"},
	{12, "A broken down"},
	{13, "A tatered old"},
	{14, "A ragged old"},
};

struct middle_treasures
{
	short row_number; /* row number in the table */
	char *name; /* Name, keywords       */
};

const struct middle_treasures middle_treasures_table[] =
{
// row name
	{0, "golden"},
	{1, "silvered"},
	{2, "bronze"},
	{3, "copper"},
	{4, "emerald"},
	{5, "topaz"},
	{6, "obsidian"},
	{7, "ruby"},
	{8, "gem encrusted"},
	{9, "wooden"},
	{10, "aquamarine"},
	{11, "crystal"},
	{12, "metal"},
	{13, "ruby"},
	{14, "amber"},
	{15, "jade"},
};


const struct random_treasures random_treasures_table[] =
{
// row name
	{0, "ewer"},
	{1, "bone"},
	{2, "harp"},
	{3, "crown"},
	{4, "ring"},
	{5, "statue"},
	{6, "statuette"},
	{7, "bottle"},
	{8, "figurine"},
	{9, "mirror"},
	{10, "cup"},
	{11, "mug"},
	{12, "scepter"},
	{13, "combe"},
	{14, "brush"},
	{15, "circlet"},
	{16, "eyepatch"},
	{17, "tea pot"},

};
void do_random_treasure ( CHAR_DATA * ch )
{
	OBJ_DATA *created_treasure;
	char buf[MAX_INPUT_LENGTH];
	short rownumber, midnumber, prenumber, random_number;

	rownumber = number_range ( 0, LAST_ROW );
	midnumber = number_range ( 0, MID_ROW );
	prenumber = number_range ( 0, PRE_ROW );
	created_treasure = create_object ( get_obj_index ( OBJ_VNUM_RND_TREASURE ), 1 );

	sprintf ( buf, "%s %s %s",
	          prefix_treasures_table[prenumber].name,
	          middle_treasures_table[midnumber].name,
	          random_treasures_table[rownumber].name
	        );
	STRFREE ( created_treasure->name );
	created_treasure->name = STRALLOC ( buf );

	sprintf ( buf, "%s %s %s",
	          prefix_treasures_table[prenumber].name,
	          middle_treasures_table[midnumber].name,
	          random_treasures_table[rownumber].name
	        );
	STRFREE ( created_treasure->short_descr );
	created_treasure->short_descr = STRALLOC ( buf );

	sprintf ( buf, "%s %s %s.",
	          prefix_treasures_table[prenumber].name,
	          middle_treasures_table[midnumber].name,
	          random_treasures_table[rownumber].name
	        );
	STRFREE ( created_treasure->description );
	created_treasure->description = STRALLOC ( buf );


	created_treasure->cost = ch->level * 10;
	created_treasure->weight = 10;
	created_treasure->timer = 5;
	random_number = number_range ( 0, 100 );
	if ( random_number > 65 && random_number < 85 )
	{
		xSET_BIT ( created_treasure->extra_flags, ITEM_REFINED );
		created_treasure->cost += 100;
	}
	else if ( random_number > 84 && random_number < 95 )
	{
		xSET_BIT ( created_treasure->extra_flags, ITEM_UNIQUE );
		created_treasure->cost += 200;
	}
	else if ( random_number > 94 && random_number < 101 )
	{
		xSET_BIT ( created_treasure->extra_flags, ITEM_ELITE );
		created_treasure->cost += 400;
	}
	obj_to_room ( created_treasure, ch->in_room, ch );
	return;
}
