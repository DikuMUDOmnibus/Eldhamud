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
 ***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "./Headers/mud.h"
#include "./Headers/commands.h"

void do_travel ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *travel_mage;
	CHAR_DATA *mount;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_STRING_LENGTH];
	char arg2[MAX_STRING_LENGTH];
	int j;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	
	for ( travel_mage = ch->in_room->first_person; travel_mage; travel_mage = travel_mage->next_in_room )
	{
		if ( IS_NPC ( travel_mage ) && xIS_SET ( travel_mage->act, ACT_TRAVELMAGE ) )
			break;
	}
	if ( !travel_mage )
	{
		ch_printf ( ch, "There is no travel mage here.\r\n" );
		return;
	}
	if ( arg1[0] == STRING_NULL || !strcmp ( arg1, "list" ) )
	{
		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( " Destination                                                          Price     \r\n", ch );
		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( " Yagonza Provence                                                     100       \r\n", ch );
		send_to_char ( " Bangoin Provence                                                     100       \r\n", ch );
		send_to_char ( " Nyemo Provence                                                       100       \r\n", ch );
		send_to_char ( " Zogang Provence                                                      100       \r\n", ch );
		send_to_char ( " Sangang Provence                                                     100       \r\n", ch );
		send_to_char ( " Jaili Provence                                                       100       \r\n", ch );
		send_to_char ( " Konjo Provence                                                       100       \r\n", ch );
		send_to_char ( " Tangmai Provence                                                     100       \r\n", ch );
		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		ch_printf ( ch, "Please type 'travel buy <location>' and have your gold ready!\r\n" );
		ch_printf ( ch, "For more information type 'help travelmage' and read!\r\n" );
		return;
	}
	if ( !strcmp ( arg1, "buy" ) )
	{
		if ( arg2[0] == STRING_NULL )
		{
			sprintf ( buf, "%s You must tell me where you wish to travel to.\r\n", ch->name );
			send_to_char ( buf, travel_mage );
			return;
		}
		for ( j = 0; j < MAX_TRAVEL_LOCATION; j++ )
		{
			if ( !str_cmp ( arg2, travel_table[j].keyword ) )
			{
				if ( ch->gold < travel_table[j].cost )
				{
					sprintf ( buf, "%s Sorry you dont have enough gold to travel to %s\r\n", ch->name, travel_table[j].keyword );
					send_to_char ( buf, travel_mage );
					return;
				}
				ch->gold -= travel_table[j].cost;
				act ( AT_PLAIN, "$n disappears in a violet fog.\r\n", ch, NULL, NULL, TO_ROOM );
				sprintf ( buf, "You are surrounded by a violet fog.{x\r\n" );
				ch_printf ( ch, "%s", buf );
				//char_from_room ( ch );
				enter_map ( ch, travel_table[j].x, travel_table[j].y, travel_table[j].map );
				mount = ch->mount;
				if ( ch->mount != NULL && ch->mount->in_room == ch->in_room )
				{
					char_from_room ( mount );
					enter_map ( mount, travel_table[j].x, travel_table[j].y, travel_table[j].map );
					act ( AT_PLAIN, "&M$n disappears in a violet fog.&x\r\n", ch, NULL, NULL, TO_ROOM );
				}
				do_look ( ch, "" );
				return;
			}
		}
		send_to_char ( "Sorry, that location is not on the list.\r\n", ch );
		return;
	}
}
void do_heal ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	int cost = 100;
	/*
	 * Search for an act_healer
	 */
	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC ( mob ) && xIS_SET ( mob->act, ACT_HEALER ) )
			break;
	}
	if ( mob == NULL )
	{
		send_to_char ( "&RYou can't do that here.\r\n", ch );
		return;
	}
	if ( cost > ch->gold )
	{
		act ( AT_PLAIN, "$N says 'You do not have enough gold for my services.'", ch, NULL, mob, TO_CHAR );
		return;
	}
	one_argument ( argument, arg );
	if ( arg[0] == STRING_NULL )
	{
		act ( AT_PLAIN, "$N says 'I offer the following spells:'", ch, NULL, mob, TO_CHAR );
		set_char_color ( AT_HEALER, ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&c  Keyword               Heals                                          Cost     \r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&w  heal:                 Hit Points                                     100 gold \r\n", ch );
		send_to_char ( "&w  refresh:              Move Points                                    100 gold \r\n", ch );
		send_to_char ( "&w  mana:                 Mana Points                                    100 gold \r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&w  Type heal <keyword> to be healed.\r\n", ch );
		return;
	}
	else if ( !str_cmp ( arg, "heal" ) )
	{
		ch->gold -= cost;
		act ( AT_PLAIN, "$n utters the words pzar.", mob, NULL, NULL, TO_ROOM );
		ch->hit = ch->max_hit;
		send_to_char ( "&wA warm glow passes through you, as your hit points are restored.\r\n", ch );
	}
	else if ( !str_cmp ( arg, "mana" ) )
	{
		ch->gold -= cost;
		act ( AT_PLAIN, "$n utters the words pzar.", mob, NULL, NULL, TO_ROOM );
		ch->mana = ch->max_mana;
		send_to_char ( "&wA warm glow passes through you, as your mana points are restored.\r\n", ch );
		return;
	}
	else if ( !str_cmp ( arg, "refresh" ) )
	{
		ch->gold -= cost;
		act ( AT_PLAIN, "$n utters the words candusima.", mob, NULL, NULL, TO_ROOM );
		ch->move = ch->max_mana;
		send_to_char ( "&wA warm glow passes through you, as your move points are restored.\r\n", ch );
	}
	else
	{
		act ( AT_PLAIN, "$N says ' Type 'heal' for a list of spells.'", ch, NULL, mob, TO_CHAR );
		return;
	}
}
void do_restring ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *mob = NULL;
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj = NULL;
	int value;
	smash_tilde ( argument );
	argument = one_argument ( argument, arg );
	argument = one_argument ( argument, arg1 );
	strcpy ( arg2, argument );
	if ( arg[0] == STRING_NULL || arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Syntax:\r\n", ch );
		send_to_char ( "  restring <Obj-Name> <Field> <String>\r\n", ch );
		send_to_char ( "    fields: name short long\r\n", ch );
		return;
	}
	if ( ( obj = get_obj_world ( ch, arg ) ) == NULL )
	{
		send_to_char ( "There is nothing like that in all the realms.\r\n", ch );
		return;
	}
	for ( mob = ch->in_room->first_person; mob; mob = ch->next_in_room )
	{
		if ( IS_NPC ( mob ) && xIS_SET ( mob->act, ACT_RESTRING ) )
			break;
	}
	if ( !mob )
	{
		send_to_char ( "You need to be at a restringer to do that.\r\n", ch );
		return;
	}
	if ( ch->gold < 2000 )
	{
		send_to_char ( "&rYou do not have enough money to restring anything!\r\n", ch );
		return;
	}
	strcpy ( arg, obj->name );
	separate_obj ( obj );
	value = atoi ( arg2 );
	if ( !str_cmp ( arg1, "name" ) )
	{
		STRFREE ( obj->name );
		obj->name = STRALLOC ( arg2 );
		send_to_char ( "Ok.\r\n", ch );
		ch->gold -= 2000;
		return;
	}
	if ( !str_cmp ( arg1, "short" ) )
	{
		STRFREE ( obj->short_descr );
		obj->short_descr = STRALLOC ( arg2 );
		send_to_char ( "Ok.\r\n", ch );
		ch->gold -= 2000;
		return;
	}
	if ( !str_cmp ( arg1, "long" ) )
	{
		STRFREE ( obj->description );
		obj->description = STRALLOC ( arg2 );
		send_to_char ( "Ok.\r\n", ch );
		ch->gold -= 2000;
		return;
	}
}
