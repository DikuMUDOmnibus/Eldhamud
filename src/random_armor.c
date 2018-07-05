/****************************************************************************
 *			Eldhamud Codebase V2.0				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                       Random Armor Drop Module                           *
 ****************************************************************************/
#include "./Headers/mud.h"

#define OBJ_VNUM_RND_ARMOR  14
#define PRE_LAST_ROW 3
#define ARM_LAST_ROW 6
#define SUF_LAST_ROW 39

struct random_armor_prefix
{
   short row_number; 	/* row number in the table */
   char *name; 		/* Name, keywords          */
};
const struct random_armor_prefix random_armor_prefix_table[] = {
   //Row  Name
   {0, "brigandine"},
   {1, "lamellar"},
   {2, "dhenuka"},
   {3, "ashigaru"}
};

struct random_armor
{
   short row_number; 	/* row number in the table */
   char *prefix; 	/* Prefix, keywords        */
   char *prefix2; 	/* Prefix, keywords        */
   char *name; 		/* Name, keywords          */
   int wear_loc;  	/* Wear Location           */
   short cost;		/* Base cost of item       */
   short weight;  	/* Base weight of item     */
};
const struct random_armor random_armor_table[] = {
//Row         Prefixes              Name        Wear              Cost Weight
   {0, "A",	     "a", 	 "helmet",     ITEM_WEAR_HEAD,    100, 10},
   {1, "A",	    "a", 	 "chestplate", ITEM_WEAR_BODY,    100, 10},
   {2, "A pair of", "a pair of", "sleeves",    ITEM_WEAR_ARMS,    100, 10},
   {3, "A pair of", "a pair of", "gloves",     ITEM_WEAR_HANDS,   100, 10},
   {4, "A",	    "a",	 "shield",     ITEM_WEAR_SHIELD,  100, 10},
   {5, "A pair of", "a pair of", "leggings",   ITEM_WEAR_LEGS,    100, 10},
   {6, "A pair of", "a pair of", "boots",      ITEM_WEAR_FEET,    100, 10},
};

struct random_armor_suffix
{
   short row_number; /* row number in the table */
   char *suffix;     /* Suffix, keywords        */
};
const struct random_armor_suffix random_armor_suffix_table[] = {
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

void do_random_armor( CHAR_DATA * ch )
{
   OBJ_DATA *created_armor;
   char buf[MAX_INPUT_LENGTH];
   short prefix_rownumber, armor_rownumber, suffix_rownumber, random_number;

   prefix_rownumber = number_range( 0, PRE_LAST_ROW );
   armor_rownumber  = number_range( 0, ARM_LAST_ROW );
   suffix_rownumber = number_range( 0, SUF_LAST_ROW );

   created_armor = create_object( get_obj_index( OBJ_VNUM_RND_ARMOR ), ch->level);//random_armor_table[armor_rownumber].level );

   sprintf( buf, "%s %s %s",  random_armor_prefix_table[prefix_rownumber].name,
                              random_armor_table[armor_rownumber].name,
                              random_armor_suffix_table[suffix_rownumber].suffix );
   STRFREE( created_armor->name );
   created_armor->name = STRALLOC( buf );
 
   sprintf( buf, "%s %s %s %s", random_armor_table[armor_rownumber].prefix2,
	                               random_armor_prefix_table[prefix_rownumber].name,
                                       random_armor_table[armor_rownumber].name,
                                       random_armor_suffix_table[suffix_rownumber].suffix );
   STRFREE( created_armor->short_descr );
   created_armor->short_descr = STRALLOC( buf );
 
   sprintf( buf, "%s %s %s %s.", random_armor_table[armor_rownumber].prefix,
	                                random_armor_prefix_table[prefix_rownumber].name,
                                        random_armor_table[armor_rownumber].name,
                                        random_armor_suffix_table[suffix_rownumber].suffix );
   STRFREE( created_armor->description );
   created_armor->description = STRALLOC( buf );
 
   created_armor->level = ch->level;
   created_armor->value[5] = ch->level;
   created_armor->value[3] = 13;
   created_armor->value[1] = ch->level;
   created_armor->value[0] = ch->level;
   created_armor->cost = ch->level * 100;
   created_armor->timer = 5;
   created_armor->weight = random_armor_table[armor_rownumber].weight;
   SET_BIT( created_armor->wear_flags, random_armor_table[armor_rownumber].wear_loc );


   random_number = number_range( 0, 100 );
   if( random_number > 65 && random_number < 85 )
   {
      xSET_BIT( created_armor->extra_flags, ITEM_REFINED );
      created_armor->value[0] += 2;
      created_armor->value[1] += 2;
   }
   else if( random_number > 84 && random_number < 95 )
   {
      xSET_BIT( created_armor->extra_flags, ITEM_UNIQUE );
      created_armor->value[0] += 4;
      created_armor->value[1] += 4;
   }
   else if( random_number > 94 && random_number < 101 )
   {
      xSET_BIT( created_armor->extra_flags, ITEM_ELITE );
      created_armor->value[0] += 8;
      created_armor->value[1] += 8;
   }

   obj_to_room ( created_armor, ch->in_room, ch );
   return;
}
