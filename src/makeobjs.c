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
 *			Specific object creation module			    *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"


void do_random_armor ( CHAR_DATA * ch );
void do_random_weapon ( CHAR_DATA * ch );
void do_random_treasure ( CHAR_DATA * ch );

/*
 * Make a fire.
 */
void make_fire ( ROOM_INDEX_DATA * in_room, short timer )
{
	OBJ_DATA *fire;
	fire = create_object ( get_obj_index ( OBJ_VNUM_FIRE ), 0 );
	fire->timer = number_fuzzy ( timer );
	obj_to_room ( fire, in_room, NULL );
	return;
}

/*
 * Turn an object into scraps.		-Thoric
 */
void make_scraps ( OBJ_DATA * obj )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *scraps, *tmpobj;
	CHAR_DATA *ch = NULL;
	separate_obj ( obj );
	scraps = create_object ( get_obj_index ( OBJ_VNUM_SCRAPS ), 0 );
	scraps->timer = number_range ( 5, 15 );
	if ( IS_OBJ_STAT ( obj, ITEM_ONMAP ) )
	{
		SET_OBJ_STAT ( scraps, ITEM_ONMAP );
		scraps->map = obj->map;
		scraps->x = obj->x;
		scraps->y = obj->y;
	}
	/*
	 * don't make scraps of scraps of scraps of ...
	 */
	if ( obj->pIndexData->vnum == OBJ_VNUM_SCRAPS )
	{
		STRFREE ( scraps->short_descr );
		scraps->short_descr = STRALLOC ( "some debris" );
		STRFREE ( scraps->description );
		scraps->description = STRALLOC ( "Bits of debris lie on the ground here." );
	}
	else
	{
		sprintf ( buf, scraps->short_descr, obj->short_descr );
		STRFREE ( scraps->short_descr );
		scraps->short_descr = STRALLOC ( buf );
		sprintf ( buf, scraps->description, obj->short_descr );
		STRFREE ( scraps->description );
		scraps->description = STRALLOC ( buf );
	}
	if ( obj->carried_by )
	{
		act ( AT_OBJECT, "$p falls to the ground in scraps!", obj->carried_by, obj, NULL, TO_CHAR );
		if ( obj == get_eq_char ( obj->carried_by, WEAR_WIELD ) && ( tmpobj = get_eq_char ( obj->carried_by, WEAR_DUAL_WIELD ) ) != NULL )
			tmpobj->wear_loc = WEAR_WIELD;
		obj_to_room ( scraps, obj->carried_by->in_room, ch );
	}
	else if ( obj->in_room )
	{
		if ( ( ch = obj->in_room->first_person ) != NULL )
		{
			act ( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_ROOM );
			act ( AT_OBJECT, "$p is reduced to little more than scraps.", ch, obj, NULL, TO_CHAR );
		}
		obj_to_room ( scraps, obj->in_room, ch );
	}
	if ( ( obj->item_type == ITEM_CONTAINER || obj->item_type == ITEM_KEYRING || obj->item_type == ITEM_QUIVER || obj->item_type == ITEM_CORPSE_PC ) && obj->first_content )
	{
		if ( ch && ch->in_room )
		{
			act ( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_ROOM );
			act ( AT_OBJECT, "The contents of $p fall to the ground.", ch, obj, NULL, TO_CHAR );
		}
		if ( obj->carried_by )
			empty_obj ( obj, NULL, obj->carried_by->in_room );
		else if ( obj->in_room )
			empty_obj ( obj, NULL, obj->in_room );
		else if ( obj->in_obj )
			empty_obj ( obj, obj->in_obj, NULL );
	}
	extract_obj ( obj );
}

/*
 * Make a corpse out of a character.
 */
void make_corpse ( CHAR_DATA * ch, CHAR_DATA * killer )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	char *name;
	ROOM_INDEX_DATA *location;
	location = get_room_index ( ROOM_VNUM_MORGUE );
	if ( IS_NPC ( ch ) )
	{
		name = ch->short_descr;
		corpse = create_object ( get_obj_index ( OBJ_VNUM_CORPSE_NPC ), 0 );
		corpse->timer = 6;
		if ( ch->gold > 0 )
		{
			if ( ch->in_room )
			{
				ch->in_room->area->gold_looted += ch->gold;
				sysdata.global_looted += ch->gold / 100;
			}
			obj_to_obj ( create_money ( ch->gold ), corpse );
			ch->gold = 0;
		}
		/*
		 * Using corpse cost to cheat, since corpses not sellable
		 */
		corpse->cost = ( - ( int ) ch->pIndexData->vnum );
		corpse->value[2] = corpse->timer;
	}
	else
	{
		name = ch->name;
		corpse = create_object ( get_obj_index ( OBJ_VNUM_CORPSE_PC ), 0 );
		corpse->timer = 40;
		corpse->value[2] = ( int ) ( corpse->timer / 8 );
		corpse->value[4] = ch->level;
		if ( CAN_PKILL ( ch ) && sysdata.pk_loot )
			xSET_BIT ( corpse->extra_flags, ITEM_CLANCORPSE );
		/*
		 * Pkill corpses get save timers, in ticks (approx 70 seconds)
		 * This should be anough for the killer to type 'get all corpse'.
		 */
		if ( !IS_NPC ( ch ) && !IS_NPC ( killer ) )
			corpse->value[3] = 1;
		else
			corpse->value[3] = 0;
	}
	if ( CAN_PKILL ( ch ) && CAN_PKILL ( killer ) && ch != killer )
	{
		sprintf ( buf, "%s", killer->name );
		STRFREE ( corpse->action_desc );
		corpse->action_desc = STRALLOC ( buf );
	}
	/*
	 * Added corpse name - make locate easier , other skills
	 */
	sprintf ( buf, "corpse %s", name );
	STRFREE ( corpse->name );
	corpse->name = STRALLOC ( buf );
	sprintf ( buf, corpse->short_descr, name );
	STRFREE ( corpse->short_descr );
	corpse->short_descr = STRALLOC ( buf );
	sprintf ( buf, corpse->description, name );
	STRFREE ( corpse->description );
	corpse->description = STRALLOC ( buf );
	if ( IS_NPC ( ch ) )
	{
		for ( obj = ch->first_carrying; obj; obj = obj_next )
		{
			obj_next = obj->next_content;
			obj_from_char ( obj );
			if ( IS_OBJ_STAT ( obj, ITEM_INVENTORY ) || IS_OBJ_STAT ( obj, ITEM_DEATHROT ) )
				extract_obj ( obj );
			else
				obj_to_obj ( obj, corpse );
		}
		obj_to_room ( corpse, ch->in_room, ch );
		return;
	}
	else if ( IS_NPC ( ch ) )
	{
		obj_to_room ( corpse, ch->in_room, ch );
		return;
	}
}


void make_drops ( CHAR_DATA * ch )
{
	short random_number;

	if ( !str_cmp ( ch->in_room->area->filename, "map.are" ) )
	{
		random_number = number_range ( 0, 4 );
		if ( random_number == 1 )
			do_random_treasure ( ch );
		else if ( random_number == 2 )
		{
			do_random_treasure ( ch );
			do_random_weapon ( ch );
		}
		else if ( random_number == 3 )
		{
			do_random_treasure ( ch );
			do_random_armor ( ch );
		}
	}
}

void make_bloodstain ( CHAR_DATA * ch )
{
	OBJ_DATA *obj;
	obj = create_object ( get_obj_index ( OBJ_VNUM_BLOODSTAIN ), 0 );
	obj->timer = number_range ( 1, 2 );
	obj_to_room ( obj, ch->in_room, ch );
}

/*
 * make some coinage
 */
OBJ_DATA *create_money ( int amount )
{
	char buf[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	if ( amount <= 0 )
	{
		bug ( "Create_money: zero or negative money %d.", amount );
		amount = 1;
	}
	if ( amount == 1 )
	{
		obj = create_object ( get_obj_index ( OBJ_VNUM_MONEY_ONE ), 0 );
	}
	else
	{
		obj = create_object ( get_obj_index ( OBJ_VNUM_MONEY_SOME ), 0 );
		sprintf ( buf, obj->short_descr, amount );
		STRFREE ( obj->short_descr );
		obj->short_descr = STRALLOC ( buf );
		obj->value[0] = amount;
	}
	return obj;
}
