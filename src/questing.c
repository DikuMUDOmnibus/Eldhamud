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
 ****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
/* Local func tions */
void generate_quest args ( ( CHAR_DATA * ch, CHAR_DATA * questman ) );
void quest_update args ( ( void ) );
bool qchance args ( ( int num ) );
CHAR_DATA *find_quest_mob args ( ( CHAR_DATA * ch ) );
void quest_list args ( ( CHAR_DATA * ch ) );
void quest_buy args ( ( CHAR_DATA * questman, CHAR_DATA * ch, short number ) );

#define OBJ_VNUM_TOKEN         	19
#define OBJ_VNUM_GLOBAL_QUEST 	20
#define QUEST_OBJQUEST1 	21
#define QUEST_OBJQUEST2 	22
#define QUEST_OBJQUEST3 	23
#define QUEST_OBJQUEST4 	24
#define QUEST_OBJQUEST5 	25

bool qchance ( int num )
{
	if ( number_range ( 1, 99 ) <= num )
		return TRUE;
	else
		return FALSE;
}

/*
 * Quest point bonus costs -- Gareth
 */
#define HUM_COST        50
#define GLOW_COST       50
#define HP_COST         50
#define MANA_COST       50
#define MOVE_COST       50
#define STR_COST        50
#define INT_COST        50
#define WIS_COST        50
#define DEX_COST        50
#define CON_COST        50
#define CHA_COST        100
#define LCK_COST        100


void do_create_token ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	OBJ_DATA *token;
	char buf[MAX_INPUT_LENGTH];
	char desc[10];
	desc[0] = STRING_NULL;
	argument = one_argument ( argument, arg1 );
	if ( arg1[0] == STRING_NULL )
	{
		send_to_char ( "&g|----------------------------------------------------------| \r\n", ch );
		send_to_char ( "|  &WToken  &g| &bPractices &g| &GQuest Points &g| &PExperience&g&g | &Y Gold  &g| \r\n", ch );
		send_to_char ( "|----------------------------------------------------------| \r\n", ch );
		send_to_char ( "| &WRoleplay&g|    &b 1    &g |    &G  20     &g |    &P 200   &g | &Y 1000 &g | \r\n", ch );
		send_to_char ( "| &WGlobal  &g|    &b 2    &g |    &G  100    &g |   &P  1000  &g | &Y 4000  &g| \r\n", ch );
		send_to_char ( "| &WHelper  &g|    &b 5    &g |    &G  200    &g |    &P 2000  &g |  &Y8000  &g| \r\n", ch );
		send_to_char ( "| &WImmortal&g|    &b 10   &g |    &G  400    &g |   &P  4000   &g|  &Y16000 &g| \r\n", ch );
		send_to_char ( "| &WNewbie  &g|    &b 20   &g |    &G  600    &g |    &P 6000   &g| &Y 32000 &g| \r\n", ch );
		send_to_char ( "|----------------------------------------------------------|&D\r\n", ch );
		send_to_char ( "See help token for issueing information. \r\n", ch );
		send_to_char ( "Syntax: create_token <token type> .\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "global" ) )
	{
		token = create_object ( get_obj_index ( OBJ_VNUM_TOKEN ), 0 );
		token->value[0] = 2; /* value for practices gained */
		token->value[1] = 100;  /* value for quest points gained */
		token->value[2] = 1000; /* value for experience gained */
		token->value[3] = 4000; /* value for gold gained */
		sprintf ( buf, "quest token global" );
		STRFREE ( token->name );
		token->name = STRALLOC ( buf );
		sprintf ( buf, "a &YGlobal&D quest token of Eldha" );
		STRFREE ( token->short_descr );
		token->short_descr = STRALLOC ( buf );
		sprintf ( buf, "A &YGlobal&D Quest Token of Eldha" );
		STRFREE ( token->description );
		token->description = STRALLOC ( buf );
	}
	else if ( !str_cmp ( arg1, "helper" ) )
	{
		token = create_object ( get_obj_index ( OBJ_VNUM_TOKEN ), 0 );
		token->value[0] = 5; /* value for practices gained */
		token->value[1] = 200;  /* value for quest points gained */
		token->value[2] = 2000; /* value for experience gained */
		token->value[3] = 8000; /* value for gold gained */
		sprintf ( buf, "quest token helper" );
		STRFREE ( token->name );
		token->name = STRALLOC ( buf );
		sprintf ( buf, "a &YHelper&D quest token of Eldha" );
		STRFREE ( token->short_descr );
		token->short_descr = STRALLOC ( buf );
		sprintf ( buf, "A &YHelper&D Quest Token of Eldha" );
		STRFREE ( token->description );
		token->description = STRALLOC ( buf );
	}
	else if ( !str_cmp ( arg1, "immortal" ) )
	{
		token = create_object ( get_obj_index ( OBJ_VNUM_TOKEN ), 0 );
		token->value[0] = 10;   /* value for practices gained */
		token->value[1] = 400;  /* value for quest points gained */
		token->value[2] = 4000; /* value for experience gained */
		token->value[3] = 16000;   /* value for gold gained */
		sprintf ( buf, "quest token immortal" );
		STRFREE ( token->name );
		token->name = STRALLOC ( buf );
		sprintf ( buf, "a &YImmortal&D quest token of Eldha" );
		STRFREE ( token->short_descr );
		token->short_descr = STRALLOC ( buf );
		sprintf ( buf, "A &YImmortal&D Quest Token of Eldha" );
		STRFREE ( token->description );
		token->description = STRALLOC ( buf );
	}
	else if ( !str_cmp ( arg1, "newbie" ) )
	{
		token = create_object ( get_obj_index ( OBJ_VNUM_TOKEN ), 0 );
		token->value[0] = 20;   /* value for practices gained */
		token->value[1] = 600;  /* value for quest points gained */
		token->value[2] = 6000; /* value for experience gained */
		token->value[3] = 32000;   /* value for gold gained */
		sprintf ( buf, "quest token newbie" );
		STRFREE ( token->name );
		token->name = STRALLOC ( buf );
		sprintf ( buf, "a &YNewbie&D quest token of Eldha" );
		STRFREE ( token->short_descr );
		token->short_descr = STRALLOC ( buf );
		sprintf ( buf, "A &YNewbie&D Quest Token of Eldha" );
		STRFREE ( token->description );
		token->description = STRALLOC ( buf );
	}
	else if ( !str_cmp ( arg1, "roleplay" ) )
	{
		token = create_object ( get_obj_index ( OBJ_VNUM_TOKEN ), 0 );
		token->value[0] = 1; /* value for practices gained */
		token->value[1] = 20;   /* value for quest points gained */
		token->value[2] = 200;  /* value for experience gained */
		token->value[3] = 1000; /* value for gold gained */
		sprintf ( buf, "quest token roleplay" );
		STRFREE ( token->name );
		token->name = STRALLOC ( buf );
		sprintf ( buf, "a &YRoleplay&D quest token of Eldha" );
		STRFREE ( token->short_descr );
		token->short_descr = STRALLOC ( buf );
		sprintf ( buf, "A &YRoleplay&D Quest Token of Eldha" );
		STRFREE ( token->description );
		token->description = STRALLOC ( buf );
	}
	else
	{
		send_to_char ( "That is not a valid argument, try create_token to see valid arguments\r\n", ch );
		return;
	}
	token->level = 2;
	token->cost = 0;
	SET_BIT ( token->wear_flags, ITEM_TAKE );
	act ( AT_ACTION, "$n creates a $p.", ch, token, NULL, TO_NOTVICT );
	act ( AT_ACTION, "You create a $p.", ch, token, NULL, TO_CHAR );
	obj_to_char ( token, ch );
	return;
}

void do_token_redeem ( CHAR_DATA * ch, OBJ_DATA * obj )
{
	ch_printf ( ch, "You receive %d Practice : %d Questpoints : %d Experience : %d Gold\r\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3] );
	ch->practice += obj->value[0];
	ch->pcdata->quest_curr += obj->value[1];
	ch->gold += obj->value[3];
	if ( ch->level < 100 )
	{
		ch->exp += obj->value[2];
		return;
	}
	return;
}

void update_questobj ( CHAR_DATA * ch, OBJ_DATA * obj )
{
	if ( xIS_SET ( obj->extra_flags, ITEM_QUEST ) )
	{
		switch ( obj->item_type )
		{
			case ITEM_WEAPON:
				obj->level = ch->level;
				obj->value[0] = 30;
				obj->value[1] = 16 + ( ( ch->level / 8 ) * 5 );
				obj->value[2] = number_fuzzy ( 2 * ch->level + ( ch->level / 2.5 ) );
				break;
			case ITEM_ARMOR:
				obj->level = ch->level;
				obj->value[0] = number_fuzzy ( obj->level / 2 + 2 );
				obj->value[3] = 30;
				break;
			default:
				break;
		}
		return;
	}
}
void update_all_qobjs ( CHAR_DATA * ch )
{
	OBJ_DATA *obj;
	for ( obj = ch->first_carrying; obj != NULL; obj = obj->next_content )
	{
		if ( xIS_SET ( obj->extra_flags, ITEM_QUEST ) )
		{
			send_to_char ( "Quest object updated.\r\n", ch );
			update_questobj ( ch, obj );
			if ( obj->wear_loc != WEAR_NONE )
			{
				de_equip_char ( ch );
				re_equip_char ( ch );
			}
		}
	}
}
void do_quest ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *questman;
	OBJ_DATA *obj = NULL, *obj_next;
	OBJ_INDEX_DATA *questinfoobj;
	MOB_INDEX_DATA *questinfo;
	char buf[MAX_STRING_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	if ( IS_NPC ( ch ) )
		return;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );

	set_char_color ( AT_QUEST, ch );

	if ( !strcmp ( arg1, "info" ) )
	{
		if ( xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			if ( ch->pcdata->questmob == -1 && ch->pcdata->questgiver->short_descr != NULL )
			{
				ch_printf ( ch, "&DYour quest is ALMOST complete! Get back to %s before your time runs out!\r\n", ch->pcdata->questgiver->short_descr );
			}
			else if ( ch->pcdata->questobj > 0 )
			{
				questinfoobj = get_obj_index ( ch->pcdata->questobj );
				if ( questinfoobj != NULL )
				{
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					send_to_char ( "&c Quest Recover Item\r\n", ch );
					send_to_char ( "-&D-------------------------------------------------------------------------------\r\n", ch );
					ch_printf ( ch, "&D  Recover Item: &w%s     \r\n", questinfoobj->name );
					ch_printf ( ch, "&D  Last Scene  : &w%s     \r\n", ch->pcdata->questroom );
					ch_printf ( ch, "&D  In          : &w%s     \r\n", ch->pcdata->questarea );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
				}
				else
					send_to_char ( "&wYou aren't currently on a quest.\r\n", ch );
				return;
			}
			else if ( ch->pcdata->questmob > 0 )
			{
				questinfo = get_mob_index ( ch->pcdata->questmob );
				if ( questinfo != NULL )
				{
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					send_to_char ( "&c Quest Assassinate\r\n", ch );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					ch_printf ( ch, "&D Assassinate  : &w%s     \r\n", questinfo->short_descr );
					ch_printf ( ch, "&D Last Location: &w%s     \r\n", ch->pcdata->questroom );
					ch_printf ( ch, "&D In           : &w%s     \r\n", ch->pcdata->questarea );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );

				}
				else
					send_to_char ( "&wYou aren't currently on a quest.\r\n", ch );
				return;
			}
		}
		else
			send_to_char ( "&wYou aren't currently on a quest.\r\n", ch );
		return;
	}
	if ( !strcmp ( arg1, "points" ) )
	{
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&c Quest Points                                                                   \r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		ch_printf ( ch, "&DYou have&w %d &Dglory.\r\n", ch->pcdata->quest_curr );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		return;
	}
	else if ( !strcmp ( arg1, "time" ) )
	{
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&c Quest Time                                                                     \r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		if ( !xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			send_to_char ( "&wYou aren't currently on a quest.\r\n", ch );
			if ( ch->pcdata->nextquest > 1 )
			{
				ch_printf ( ch, "&wThere are %d minutes remaining until you can go on another quest.\r\n", ch->pcdata->nextquest );
			}
			else if ( ch->pcdata->nextquest == 1 )
			{
				ch_printf ( ch, "There is less than a minute remaining until you can go on another quest.\r\n" );
			}
		}
		else if ( ch->pcdata->countdown > 0 )
		{
			ch_printf ( ch, "&wTime left for current quest: %d\r\n", ch->pcdata->countdown );
		}
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		return;
	}
	/*
	 * Check for questmaster in room.
	 */
	for ( questman = ch->in_room->first_person; questman != NULL; questman = questman->next_in_room )
		if ( IS_NPC ( questman ) && xIS_SET ( questman->act, ACT_QUESTMASTER ) )
			break;
	if ( !questman )
	{
		send_to_char ( "&wYou can't do that here.\r\n", ch );
		return;
	}
	if ( questman->fighting )
	{
		send_to_char ( "&wWait until the fighting stops.\r\n", ch );
		return;
	}
	ch->pcdata->questgiver = questman;
	if ( !strcmp ( arg1, "list" ) )
	{
		act ( AT_PLAIN, "$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM );
		act ( AT_PLAIN, "You ask $N for a list of quest items.", ch, NULL, questman, TO_CHAR );
		quest_list ( ch );
		return;
	}
	if ( !strcmp ( arg1, "enhancement" ) )
	{
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
			send_to_char ( "&c Quest Enhancement                                                              \r\n", ch );
			send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
			send_to_char ( "&D Base Stat increase (+1 wis, con, int, dex, str):                     &c  50 qps  \r\n", ch );
			send_to_char ( "&D Base Stat increase (+1 lck, cha):                                    &c 100 qps  \r\n", ch );
			send_to_char ( "&D 10 Hp, mana or moves                                                 &c  50 qps  \r\n", ch );
			send_to_char ( "&D Make an item glow or hum                                             &c  50 qps  \r\n", ch );
			send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "hum" ) )
		{
			if ( ch->pcdata->quest_curr < HUM_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( arg3[0] == STRING_NULL )
			{
				send_to_char ( "&wMake what item hum?\r\n", ch );
				return;
			}
			if ( ! ( obj = get_obj_carry ( ch, arg3 ) ) )
			{
				send_to_char ( "&&That item is not in your inventory.\r\n", ch );
				return;
			}
			if ( IS_OBJ_STAT ( obj, ITEM_HUM ) )
			{
				send_to_char ( "&wThat item is already humming.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= HUM_COST;
			separate_obj ( obj );
			xSET_BIT ( obj->extra_flags, ITEM_HUM );
			send_to_char ( "&wYour item begins to hum softly.\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "glow" ) )
		{
			if ( ch->pcdata->quest_curr < GLOW_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			/*
			 * arg1ument = one_arg1ument( arg1ument, arg2 );
			 */
			if ( arg3[0] == STRING_NULL )
			{
				send_to_char ( "&wMake what item glow?\r\n", ch );
				return;
			}
			if ( ! ( obj = get_obj_carry ( ch, arg3 ) ) )
			{
				send_to_char ( "&wThat item is not in your inventory.\r\n", ch );
				return;
			}
			if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) )
			{
				send_to_char ( "&wThat item is already glowing.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= GLOW_COST;
			separate_obj ( obj );
			xSET_BIT ( obj->extra_flags, ITEM_GLOW );
			send_to_char ( "&wYour item begins to glow.\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "hp" ) )
		{
			if ( ch->pcdata->quest_curr < HP_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= HP_COST;
			ch->max_hit += 10;
			ch->hit += 10;
			send_to_char ( "&wYou gain a 10 hitpoints!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "mana" ) )
		{
			if ( ch->pcdata->quest_curr < MANA_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= MANA_COST;
			ch->max_mana += 10;
			ch->mana += 10;
			send_to_char ( "&wYou gain a 10 points of mana!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "movement" ) )
		{
			if ( ch->pcdata->quest_curr < MOVE_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= MOVE_COST;
			ch->max_move += 10;
			ch->move += 10;
			send_to_char ( "&wYou gain 10 movement points!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "str" ) || !str_cmp ( arg2, "strength" ) )
		{
			if ( ch->pcdata->quest_curr < STR_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_str == 25 )
			{
				send_to_char ( "&wYou are already as strong as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= STR_COST;
			ch->perm_str++;
			send_to_char ( "&wYou grow stronger!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "int" ) || !str_cmp ( arg2, "intelligence" ) )
		{
			if ( ch->pcdata->quest_curr < INT_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_int == 25 )
			{
				send_to_char ( "&wYou are already as intelligent as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= INT_COST;
			ch->perm_int++;
			send_to_char ( "&wYou grow more intelligent!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "wis" ) || !str_cmp ( arg2, "wisdom" ) )
		{
			if ( ch->pcdata->quest_curr < WIS_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_int == 25 )
			{
				send_to_char ( "&wYou are already as wise as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= WIS_COST;
			ch->perm_wis++;
			send_to_char ( "&wYou grow wiser!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "dex" ) || !str_cmp ( arg2, "dexterity" ) )
		{
			if ( ch->pcdata->quest_curr < DEX_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_dex == 25 )
			{
				send_to_char ( "&wYou are already as dextrous as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= DEX_COST;
			ch->perm_dex++;
			send_to_char ( "&wYour dexterity increases!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "con" ) || !str_cmp ( arg2, "constitution" ) )
		{
			if ( ch->pcdata->quest_curr < CON_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_con == 25 )
			{
				send_to_char ( "&wYour constitution is as high as it can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= CON_COST;
			ch->perm_con++;
			send_to_char ( "&wYour constitution increases!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "cha" ) || !str_cmp ( arg2, "charisma" ) )
		{
			if ( ch->pcdata->quest_curr < CHA_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_cha == 25 )
			{
				send_to_char ( "&wYou are already as charismatic as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= CHA_COST;
			ch->perm_cha++;
			send_to_char ( "&wYou grow more charismatic!\r\n", ch );
			return;
		}
		if ( !str_cmp ( arg2, "lck" ) || !str_cmp ( arg2, "luck" ) )
		{
			if ( ch->pcdata->quest_curr < LCK_COST )
			{
				send_to_char ( "&wYou don't have the questpoints.\r\n", ch );
				return;
			}
			if ( ch->perm_lck == 25 )
			{
				send_to_char ( "&wYou are already as lucky as you can possibly get.\r\n", ch );
				return;
			}
			ch->pcdata->quest_curr -= LCK_COST;
			ch->perm_lck++;
			send_to_char ( "&wYou feel luckier!\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "&wThat cannot be enhanced. Read HELP Questfor details.\r\n", ch );
			return;
		}
	}
	else if ( !strcmp ( arg1, "buy" ) )
	{
		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "&wTo buy an item, type 'QUEST BUY ITEM <#>'.\r\n", ch );
			return;
		}
		quest_buy ( questman, ch, atoi ( arg2 ) );
		return;
	}
	else if ( !strcmp ( arg1, "request" ) )
	{
		act ( AT_PLAIN, "$n asks $N for a quest.", ch, NULL, questman, TO_ROOM );
		act ( AT_PLAIN, "You ask $N for a quest.", ch, NULL, questman, TO_CHAR );
		if ( sysdata.DENY_NEW_PLAYERS == TRUE )
		{
			sprintf ( buf, "&wI'm sorry. Please come back after the reboot. I cant give you a quest until then." );
			do_say ( questman, buf );
			return;
		}
		if ( xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			sprintf ( buf, "&wBut you're already on a quest!\r\nBetter hurry up and finish it!" );
			do_say ( questman, buf );
			return;
		}
		if ( ch->pcdata->nextquest > 0 )
		{
			sprintf ( buf, "&wYou're very brave, %s, but let someone else have a chance, Please come back in about %d minutes.", ch->name, ch->pcdata->nextquest );
			do_say ( questman, buf );
			return;
		}
		sprintf ( buf, "&wWell, well well then, %s...a brave one here we have.", ch->name );
		do_say ( questman, buf );
		generate_quest ( ch, questman );
		if ( ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0 )
		{
			ch->pcdata->countdown = number_range ( 5, 10 );
			xSET_BIT ( ch->act, PLR_QUESTOR );
			sprintf ( buf, "&wYou have been alloocated %d minutes to complete this task.", ch->pcdata->countdown );
			do_say ( questman, buf );
		}
		return;
	}
	else if ( !strcmp ( arg1, "refuse" ) )
	{
		act ( AT_PLAIN, "$n informs $N $e has refused $s quest.", ch, NULL, questman, TO_ROOM );
		act ( AT_PLAIN, "You inform $N you have refused the quest.", ch, NULL, questman, TO_CHAR );
		if ( ch->pcdata->questgiver != questman )
		{
			sprintf ( buf, "&wI never sent you on a quest! Perhaps you're thinking of someone else." );
			do_say ( questman, buf );
			return;
		}
		if ( !xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			ch->pcdata->questgiver = NULL;
			ch->pcdata->countdown = 0;
			ch->pcdata->questmob = 0;
			ch->pcdata->questobj = 0;
			sprintf ( buf, "&wYou should at least REQUEST a quest before you refuse it!" );
			do_say ( questman, buf );
			return;
		}
		if ( xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			xREMOVE_BIT ( ch->act, PLR_QUESTOR );
			ch->pcdata->questgiver = NULL;
			ch->pcdata->countdown = 0;
			ch->pcdata->questmob = 0;
			ch->pcdata->questobj = 0;
			ch->pcdata->nextquest = 5;
			sprintf ( buf, "&wAlright fine, i'll put it back in the heap for someone else, You can come back in about %d minutes.", ch->pcdata->nextquest );
			do_say ( questman, buf );
			return;
		}
	}
	else if ( !strcmp ( arg1, "complete" ) )
	{
		act ( AT_PLAIN, "$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM );
		act ( AT_PLAIN, "You inform $N you have completed $s quest.", ch, NULL, questman, TO_CHAR );
		if ( ch->pcdata->questgiver != questman )
		{
			sprintf ( buf, "&wI never sent you on a quest! Perhaps you're thinking of someone else." );
			do_say ( questman, buf );
			return;
		}
		if ( xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			if ( ch->pcdata->questmob == -1 && ch->pcdata->countdown > 0 )
			{
				int reward, pointreward, pracreward;
				reward = number_range ( 500, 2500 );
				pointreward = number_range ( 5, 15 );
				if ( double_qp == TRUE )
				{
					pointreward *= 2;
				}
				send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
				send_to_char ( "&c Quest Reward                                                                   \r\n", ch );
				send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
				ch_printf ( ch,"&DQuest Points: &w%d\r\n",pointreward );
				ch_printf ( ch,"&DGold        : &w%d\r\n", reward );
				if ( qchance ( 70 ) )
				{
					pracreward = number_range ( 1, 3 );
					ch_printf ( ch,"Practices   : %d\r\n", pracreward );
					ch->practice += pracreward;
				}
				send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
				xREMOVE_BIT ( ch->act, PLR_QUESTOR );
				ch->pcdata->questgiver = NULL;
				ch->pcdata->countdown = 0;
				ch->pcdata->questmob = 0;
				ch->pcdata->questobj = 0;
				ch->pcdata->nextquest = 5; /* yzek... was 30 */
				ch->gold += reward;
				ch->pcdata->quest_curr += pointreward;
				return;
			}
			else if ( ch->pcdata->questobj > 0 && ch->pcdata->countdown > 0 )
			{
				bool obj_found = FALSE;
				for ( obj = ch->first_carrying; obj != NULL; obj = obj_next )
				{
					obj_next = obj->next_content;
					if ( obj != NULL && obj->pIndexData->vnum == ch->pcdata->questobj )
					{
						obj_found = TRUE;
						break;
					}
				}
				if ( obj_found == TRUE )
				{
					int reward, pointreward, pracreward;
					reward = number_range ( 500, 2500 );
					pointreward = number_range ( 5, 15 );
					if ( double_qp == TRUE )
					{
						pointreward *= 2;
					}
					act ( AT_PLAIN, "You hand $p to $N.", ch, obj, questman, TO_CHAR );
					act ( AT_PLAIN, "$n hands $p to $N.", ch, obj, questman, TO_ROOM );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					send_to_char ( "&c Quest Reward                                                                   \r\n", ch );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					ch_printf ( ch,"&DQuest Points: &w%d\r\n",pointreward );
					ch_printf ( ch,"&DGold        : &w%d\r\n", reward );
					if ( qchance ( 70 ) )
					{
						pracreward = number_range ( 1, 3 );
						ch_printf ( ch,"&DPractices   : &w%d\r\n", pracreward );
						ch->practice += pracreward;
					}
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					xREMOVE_BIT ( ch->act, PLR_QUESTOR );
					ch->pcdata->questgiver = NULL;
					ch->pcdata->countdown = 0;
					ch->pcdata->questmob = 0;
					ch->pcdata->questobj = 0;
					ch->pcdata->nextquest = 5; /*Yzek ... was 30 */
					ch->gold += reward;
					ch->pcdata->quest_curr += pointreward;
					extract_obj ( obj );
					return;
				}
				else
				{
					sprintf ( buf, "&wYou haven't completed the quest yet, but there is still time!" );
					do_say ( questman, buf );
					return;
				}
				return;
			}
			else if ( ( ch->pcdata->questmob > 0 || ch->pcdata->questobj > 0 ) && ch->pcdata->countdown > 0 )
			{
				sprintf ( buf, "&wYou haven't completed the quest yet, but there is still time!" );
				do_say ( questman, buf );
				return;
			}
		}
		if ( ch->pcdata->nextquest > 0 )
			sprintf ( buf, "&wBut you didn't complete your quest in time!" );
		else
			sprintf ( buf, "&wYou have to REQUEST a quest first, %s.", ch->name );
		do_say ( questman, buf );
		return;
	}
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "&c Quest Commands:                                                                \r\n", ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "&D QUEST POINTS: &wThe number of Qp you have to spend.			       \r\n", ch );
	send_to_char ( "&D QUEST INFO: &wAn outline of your quest tasks.				       \r\n", ch );
	send_to_char ( "&D QUEST TIME: &wThe Ammount of time remaining on your quest.                       \r\n", ch );
	send_to_char ( "&D QUEST REQUEST: &wThe command to receive a new quest from the questmastor.        \r\n", ch );
	send_to_char ( "&D QUEST REFUSE: &wUsed to decline the quest on offer.                              \r\n", ch );
	send_to_char ( "&D QUEST COMPLETE: &wUse at the questmaster to complete the quest and gain rewards. \r\n", ch );
	send_to_char ( "&D QUEST LIST: &wShows the quest items for sale.                                    \r\n", ch );
	send_to_char ( "&D QUEST BUY: &w# will purchase an item using quest points.                         \r\n", ch );
	send_to_char ( "&D QUEST ENHANCEMENT: &wUse to purchase enhancements for your thingss.              \r\n", ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "&DFor more information, type 'HELP QUEST'.\r\n", ch );
	return;
}

void generate_quest ( CHAR_DATA * ch, CHAR_DATA * questman )
{
	CHAR_DATA *victim = NULL;
	ROOM_INDEX_DATA *room;
	OBJ_DATA *questitem;
	int objvnum = 0, random_number;
	char buf[MAX_STRING_LENGTH];
	char owner_name[30];

	set_char_color ( AT_QUEST, ch );
	random_number = number_range ( 1, 200000 );
	
	if ( random_number < 100000 )
	{
		victim = find_quest_mob ( ch );
		if ( ( room = find_location ( ch, victim->name ) ) == NULL )
		{
			sprintf ( buf, "&wI'm sorry, but I don't have any quests for you at this time Try again later." );
			do_say ( questman, buf );
			ch->pcdata->nextquest = 1;
			return;
		}
		switch ( number_range ( 0, 4 ) )
		{
			case 0:
				objvnum = QUEST_OBJQUEST1;
				break;
			case 1:
				objvnum = QUEST_OBJQUEST2;
				break;
			case 2:
				objvnum = QUEST_OBJQUEST3;
				break;
			case 3:
				objvnum = QUEST_OBJQUEST4;
				break;
			case 4:
				objvnum = QUEST_OBJQUEST5;
				break;
		}

		switch ( number_range ( 0, 4 ) )
		{
			case 0:
				sprintf ( owner_name, "The Emperor" );
				break;
			case 1:
				sprintf ( owner_name,"Princess Lu Wan" );
				break;
			case 2:
				sprintf ( owner_name, "General Zhao" );
				break;
			case 3:
				sprintf ( owner_name, "Master Long Ho" );
				break;
			case 4:
				sprintf ( owner_name, "The First Advisor" );
				break;
		}

		questitem = create_object ( get_obj_index ( objvnum ), victim->level );
		obj_to_room ( questitem, victim->in_room, victim );
		ch->pcdata->questobj = questitem->pIndexData->vnum;
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		ch_printf ( ch, "&c Quest Stolen Item     			        Quest Number %-6d of 200,000.\r\n", random_number );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&DBandits, theives and demons run rife all throughout Engen Wu. Not even the      \r\n",ch );
		send_to_char ( "&DEmporer himself has been spared from these nasty and foul beings. Being a strong\r\n",ch );
		send_to_char ( "&Dand noble student of the Engen Wu .PvP. Academy, you have been charged with the \r\n",ch );
		send_to_char ( "&Dlocation and recovery an object that have been reported stolen this very day.   \r\n",ch );
		send_to_char ( "&D By the Order of the Emporer.                                                   \r\n",ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		ch_printf ( ch, "&D  Recover Item: &w%s     \r\n", questitem->short_descr );
		ch_printf ( ch, "&D  Owned By    : &w%s     \r\n", owner_name );
		ch_printf ( ch, "&D  Last Scene  : &w%s at %dX-%dY\r\n", room->name,victim->x, victim->y );
		ch_printf ( ch, "&D  In          : &w%s     \r\n", room->area->name );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		ch->pcdata->questroom = room->area->name;
		ch->pcdata->questarea = room->name;
		return;
	}
	/*
	 * Quest to kill a mob
	 */
	else if ( random_number > 100000 )
	{
		victim = find_quest_mob ( ch );
		if ( !victim )
		{
			sprintf ( buf, "&wI'm sorry, but I don't have any quests for you at this time.Try again later" );
			do_say ( questman, buf );
			ch->pcdata->questmob = 0;
			ch->pcdata->questobj = 0;
			ch->pcdata->nextquest = 1;
			return;
		}
		room = victim->in_room;
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		ch_printf ( ch, "&c Quest Assassination     			 Quest Number %-6d of 200,000.\r\n", random_number );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&DBandits, theives and demons run rife all throughout Engen Wu. The Emporer       \r\n",ch );
		send_to_char ( "&Dhimself has decreed that all noble and civilized members of society should      \r\n",ch );
		send_to_char ( "&Dhunt kill and destroy these foul beings who have been declared enemies of the   \r\n",ch );
		send_to_char ( "&Dnation. As a fair and noble student of the Engen Wu .PvP. Academy, you have     \r\n",ch );
		send_to_char ( "&Dbeen charged to carry out the following, By Order of the Emporer.               \r\n",ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		switch ( number_range ( 0, 2 ) )
		{
			case 0:
				send_to_char ( "&D Charged With : &RFailing to honour a contract with the Emperor.\r\n",ch );
				break;
			case 1:
				send_to_char ( "&D Charged With : &RFailing to repay creditors.\r\n",ch );
				break;
			case 2:
				send_to_char ( "&D Charged With : &RTheft and Deception.\r\n",ch );
				break;
		}
		if ( room->name != NULL )
		{
			ch_printf ( ch, "&D Assassinate  : &w%s     \r\n", victim->short_descr );
			ch_printf ( ch, "&D Last Location: &w%s     \r\n", room->name );
			ch_printf ( ch, "&D In           : &w%s     \r\n", room->area->name );
			send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
			ch->pcdata->questroom = room->name;
			ch->pcdata->questarea = room->area->name;
		}
		ch->pcdata->questmob = victim->pIndexData->vnum;
		return;
	}
}

/* Called from update_handler() by pulse_area */
void quest_update ( void )
{
	CHAR_DATA *ch, *ch_next;
	for ( ch = first_char; ch != NULL; ch = ch_next )
	{
		ch_next = ch->next;
		if ( IS_NPC ( ch ) )
			continue;
		if ( ch->pcdata->nextquest > 0 )
		{
			ch->pcdata->nextquest--;
			if ( ch->pcdata->nextquest == 0 )
			{
				send_to_char ( "&BYou may now quest again.\r\n", ch );
				return;
			}
		}
		else if ( xIS_SET ( ch->act, PLR_QUESTOR ) )
		{
			if ( --ch->pcdata->countdown <= 0 )
			{
				ch->pcdata->nextquest = 5;
				ch_printf ( ch, "&RYou have run out of time for your quest! You may quest again in &W%d &Rminutes.\r\n", ch->pcdata->nextquest );
				xREMOVE_BIT ( ch->act, PLR_QUESTOR );
				if ( ch->pcdata->questroom )
					STRFREE ( ch->pcdata->questroom );
				if ( ch->pcdata->questarea )
					STRFREE ( ch->pcdata->questarea );
				ch->pcdata->questgiver = NULL;
				ch->pcdata->countdown = 0;
				ch->pcdata->questmob = 0;
			}
			if ( ch->pcdata->countdown > 0 && ch->pcdata->countdown < 2 )
			{
				send_to_char ( "&wBetter hurry, you're almost out of time for your quest!\r\n", ch );
				return;
			}
		}
	}
	return;
}


CHAR_DATA *find_quest_mob ( CHAR_DATA * ch )
{
	CHAR_DATA *victim = NULL;
	int counter, mob_vnum;
	for ( counter = 1; counter < 1000; counter++ )
	{
		mob_vnum = number_range ( 499, 550 );  /* Raise 32000 to your highest mobile vnum */
		if ( ( victim = get_mob ( mob_vnum ) ) != NULL )
		{
			//level_diff = victim->level - ch->level;
			       if(( ( ch->level > 1 && victim->level < 5 )
			            || ( ch->level > 5 && victim->level < 10 )
				    || ( ch->level > 10 && victim->level < 20 )
				    || ( ch->level > 15 && victim->level < 25 )
				    || ( ch->level > 20 && victim->level < 30 )
			            || ( ch->level > 25 && victim->level > 35 ) )
			          &&  victim->pIndexData->pShop == NULL
			          && victim->pIndexData->rShop == NULL
			          && !xIS_SET( victim->act, ACT_PRACTICE )
			          && !xIS_SET( victim->in_room->room_flags, ROOM_SAFE )
			          && !xIS_SET( victim->act, ACT_NOQUEST )
			          && !xIS_SET( victim->act, ACT_PET )
			          && !xIS_SET( victim->act, ACT_IS_TATTOOARTIST )
			          && !xIS_SET( victim->act, ACT_TRAINER )
			          && !xIS_SET( victim->act, ACT_MOUNTED )
			          && !xIS_SET( victim->act, ACT_MOUNTABLE )
			          && !xIS_SET( victim->act, ACT_PACIFIST )
			          && !xIS_SET( victim->act, ACT_NOATTACK )
			          && !xIS_SET( victim->act, ACT_PROTOTYPE )
			          && !xIS_SET( victim->act, ACT_BANKER )
			          && !xIS_SET( victim->act, ACT_PKTOGGLE )
			          && !xIS_SET( victim->act, ACT_QUESTMASTER )
			          && !xIS_SET( victim->act, ACT_NOQUEST )
			          && !xIS_SET( victim->act, ACT_TRAVELMAGE )
			          && !xIS_SET( victim->act, ACT_HEALER )
			          && !xIS_SET( victim->act, ACT_FORGE )
			          && !xIS_SET( victim->act, ACT_IMMORTAL ) ) 
			return victim;
//         else
//            continue;
		}
	}
	return NULL;
}

void quest_list ( CHAR_DATA * ch )
{
	set_char_color ( AT_QUEST, ch );
	short i, count = 0;
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "&cItem #           Item 						   Price  \r\n", ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	for ( i = 0; i <= MAX_QDATA; i++ )
	{
		if ( questmaster->award_vnum[i] && questmaster->award_value[i] )
		{
			++count;
			ch_printf ( ch, "&c[%2d]          &w %-35s                                             &c  %-6s \r\n",
			            count, get_obj_index ( questmaster->award_vnum[i] )->short_descr, num_punct ( questmaster->award_value[i] ) );
		}
	}
	ch_printf ( ch, "&c[%2d]          &w %s Gold                                       &c  120\r\n", count + 1, num_punct ( questmaster->gold_amt ) );
	ch_printf ( ch, "&c[%2d]          &w %d Practices                                  &c      %-6s\r\n", count + 2, questmaster->practice_amt, num_punct ( questmaster->practice_cost ) );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "&D Quest Buy # \r\n", ch );
	return;
}
void quest_buy ( CHAR_DATA * questman, CHAR_DATA * ch, short number )
{
	short i, count = 0;
	OBJ_DATA *obj;
	for ( i = 0; i <= MAX_QDATA; i++ )
	{
		if ( questmaster->award_vnum[i] > 0 && questmaster->award_value[i] > 0 )
		{
			++count;
			if ( count == number )
				break;
		}
	}
	if ( number > count + 2 || number < 1 )
	{
		do_say ( questman, "I don't know what you're trying to buy. Check the list again." );
		return;
	}
	if ( number == ( count + 1 ) ) /* Gold */
	{
		if ( ch->pcdata->quest_curr >= questmaster->gold_cost )
		{
			ch->pcdata->quest_curr -= questmaster->gold_cost;
			ch->gold += questmaster->gold_amt;
			act ( AT_MAGIC, "$N gives a sack of gold pieces to $n.", ch, NULL, questman, TO_ROOM );
			act ( AT_MAGIC, "$N hands you your gold.", ch, NULL, questman, TO_CHAR );
			return;
		}
		else
		{
			do_say ( questman, "You're not glorious enough to purchase that." );
			return;
		}
	}
	else if ( number == ( count + 2 ) ) /* Practices */
	{
		if ( ch->pcdata->quest_curr >= questmaster->practice_cost )
		{
			ch->pcdata->quest_curr -= questmaster->practice_cost;
			ch->practice += questmaster->practice_amt;
			act ( AT_MAGIC, "$N expands $n's mind to recieve greater knowledge.", ch, NULL, questman, TO_ROOM );
			act ( AT_MAGIC, "$N opens your mind to greater possibilities of learning.", ch, NULL, questman, TO_CHAR );
			return;
		}
		else
		{
			do_say ( questman, "&wYou're not glorious enough to purchase that." );
			return;
		}
	}
	else if ( ch->pcdata->quest_curr >= questmaster->award_value[i] && ( questmaster->award_value[i] != 0 ) )
	{
		ch->pcdata->quest_curr -= questmaster->award_value[i];
		obj = create_object ( get_obj_index ( questmaster->award_vnum[i] ), ch->level );
		act ( AT_PLAIN, "$N gives $p to $n.", ch, obj, questman, TO_ROOM );
		act ( AT_PLAIN, "$N gives you $p.", ch, obj, questman, TO_CHAR );
		obj_to_char ( obj, ch );
		update_all_qobjs ( ch );
	}
	else
	{
		do_say ( questman, "You're not glorious enough to purchase that." );
		return;
	}
	return;
}

void fwrite_questmaster( )
{
	FILE *fp;
	char filename[256];
	int counter;
	sprintf ( filename, "%s", QUEST_FILE );
	fclose ( fpReserve );
	if ( ( fp = fopen ( filename, "w" ) ) == NULL )
	{
		bug ( "fwrite_questmaster: fopen", 0 );
		perror ( filename );
	}
	fprintf ( fp, "Lrange %d\n", questmaster->level_range );
	fprintf ( fp, "Pamt %d\n", questmaster->practice_amt );
	fprintf ( fp, "Pcost %d\n", questmaster->practice_cost );
	fprintf ( fp, "Gamt %d\n", questmaster->gold_amt );
	fprintf ( fp, "Gcost %d\n", questmaster->gold_cost );
	for ( counter = 0; counter <= MAX_QDATA; counter++ )
		if ( questmaster->award_value[counter] && questmaster->award_vnum[counter] )
			fprintf ( fp, "Object %d %d\n", questmaster->award_vnum[counter], questmaster->award_value[counter] );
		else
			continue;
	fprintf ( fp, "End\n" );
	fclose ( fp );
	fpReserve = fopen ( NULL_FILE, "r" );
	return;
}

#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				      field = value;			\
				      fMatch = TRUE;			\
				      break;				\
				}
void fread_questmaster( )
{
	char filename[256];
	FILE *fp;
	char buf[MAX_STRING_LENGTH];
	char *word;
	bool fMatch;
	int vnum = -1, value = -1, counter;
	sprintf ( filename, "%s", QUEST_FILE );
	if ( ( fp = fopen ( filename, "r" ) ) == NULL )
	{
		bug ( "fread_questmaster: fopen", 0 );
		perror ( filename );
	}
	for ( ;; )
	{
		word = feof ( fp ) ? "End" : fread_word ( fp );
		fMatch = FALSE;
		switch ( UPPER ( word[0] ) )
		{
			case '*':
				fMatch = TRUE;
				fread_to_eol ( fp );
				break;
			case 'E':
				if ( !str_cmp ( word, "End" ) )
				{
					fclose ( fp );
					for ( counter = vnum; counter != MAX_QDATA; counter++ )
					{
						questmaster->award_vnum[++vnum] = 0;
						questmaster->award_value[++value] = 0;
					}
					return;
				}
				break;
			case 'G':
				KEY ( "Gamt", questmaster->gold_amt, fread_number ( fp ) );
				KEY ( "Gcost", questmaster->gold_cost, fread_number ( fp ) );
				break;
			case 'L':
				KEY ( "Lrange", questmaster->level_range, fread_number ( fp ) );
				break;
			case 'P':
				KEY ( "Pamt", questmaster->practice_amt, fread_number ( fp ) );
				KEY ( "Pcost", questmaster->practice_cost, fread_number ( fp ) );
				break;
			case 'O':
				if ( !str_cmp ( word, "Object" ) )
				{
					if ( vnum >= MAX_QDATA )
						break;
					questmaster->award_vnum[++vnum] = fread_number ( fp );
					questmaster->award_value[++value] = fread_number ( fp );
					fMatch = TRUE;
					break;
				}
				break;
		}
		if ( !fMatch )
		{
			sprintf ( buf, "fread_questmaster: no match: %s", word );
			bug ( buf, 0 );
		}
	}
}
void do_queststat ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	char arg4[MAX_INPUT_LENGTH];
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	argument = one_argument ( argument, arg4 );
	if ( arg1[0] == STRING_NULL )
	{
		short i, count = 0;
		for ( i = 0; i != MAX_QDATA; i++ )
		{
			ch_printf ( ch, "&p[&w%2d&p] [&g%-35s&p] [&w%6s&p] [&w%6d&p] \r\n", count,
			            questmaster->award_vnum[i] ? get_obj_index ( questmaster->award_vnum[i] )->short_descr : "Empty",
			            num_punct ( questmaster->award_value[i] ), questmaster->award_vnum[i] );
			++count;
		}
		ch_printf ( ch, "&p[&w**&p] [&Y%s Gold%-23s&p] [&w%6s&p] \r\n", num_punct ( questmaster->gold_amt ), "", num_punct ( questmaster->gold_cost ) );
		ch_printf ( ch, "&p[&w**&p] [&Y%d Practices%-23s&p] [&w%6s&p] \r\n", questmaster->practice_amt, "", num_punct ( questmaster->practice_cost ) );
		ch_printf ( ch, "&p[&w**&p] [%-37s&p] [&w%6d&p]\r\n", "&YLevel Range", questmaster->level_range );
		send_to_char ( "\r\n", ch );
		send_to_char ( "&wSyntax: queststat object # vnum value\r\n", ch );
		send_to_char ( "Syntax: queststat gold amount value\r\n", ch );
		send_to_char ( "Syntax: queststat practice amount value\r\n", ch );
		send_to_char ( "Syntax: queststat range value\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "Object" ) )
	{
		int seek = atoi ( arg2 ), vnum = atoi ( arg3 ), value = atoi ( arg4 );
		short i, count = 0;
		if ( vnum < 0 || seek < 0 || value < 0 )
		{
			send_to_char ( "Syntax: queststat object # vnum value\r\n", ch );
			return;
		}
		for ( i = 0; i != MAX_QDATA; i++ )
		{
			if ( count == seek )
				break;
			++count;
		}
		if ( seek != count )
		{
			send_to_char ( "# Not found.\r\n", ch );
			send_to_char ( "Syntax: queststat object # vnum value\r\n", ch );
			return;
		}
		else
		{
			questmaster->award_vnum[i] = vnum;
			questmaster->award_value[i] = value;
		}
		fwrite_questmaster( );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "Gold" ) )
	{
		int amount = atoi ( arg2 ), value = atoi ( arg3 );
		if ( !amount || !value )
		{
			send_to_char ( "Syntax: queststat gold amount value\r\n", ch );
			return;
		}
		questmaster->gold_amt = amount;
		questmaster->gold_cost = value;
		fwrite_questmaster( );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "Practices" ) )
	{
		int amount = atoi ( arg2 ), value = atoi ( arg3 );
		if ( !amount || !value )
		{
			send_to_char ( "Syntax: queststat practices amount value\r\n", ch );
			return;
		}
		questmaster->practice_amt = amount;
		questmaster->practice_cost = value;
		fwrite_questmaster( );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	if ( !str_cmp ( arg1, "Range" ) )
	{
		int value = atoi ( arg2 );
		if ( !value )
		{
			send_to_char ( "Syntax: queststat range value\r\n", ch );
			return;
		}
		questmaster->level_range = value;
		fwrite_questmaster( );
		send_to_char ( "Done.\r\n", ch );
		return;
	}
	do_queststat ( ch, "" );
	return;
}
void do_quest_invade ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	AREA_DATA *tarea;
	int count, created;
	bool found = FALSE;
	MOB_INDEX_DATA *pMobIndex;
	ROOM_INDEX_DATA *location;
	OBJ_DATA *gq_obj;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	count = atoi ( arg2 );
	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Invade <area> <# of invaders> <mob vnum>\r\n", ch );
		send_to_char ( "----------------------------------------\r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 5  Vnum: 20     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 10  Vnum: 21     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 20  Vnum: 22     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 30  Vnum: 23     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 40  Vnum: 24     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 50  Vnum: 25     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 60  Vnum: 26     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 70  Vnum: 27     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 80  Vnum: 28     \r\n", ch );
		send_to_char ( "GLobal Quest Mobile Level: 90  Vnum: 29     \r\n", ch );
		send_to_char ( "----------------------------------------\r\n", ch );
		send_to_char ( "These Mob Vnums are the ONLY ones to be used\r\n", ch );
		send_to_char ( "for global quests, NO QUESTIONS ASKED.\r\n", ch );
		return;
	}
	for ( tarea = first_area; tarea; tarea = tarea->next )
		if ( !str_cmp ( tarea->filename, arg1 ) )
		{
			found = TRUE;
			break;
		}
	if ( !found )
	{
		send_to_char ( "Area not found.\r\n", ch );
		return;
	}
	if ( count > 300 )
	{
		send_to_char ( "Whoa...Less than 300 please.\r\n", ch );
		return;
	}
	if ( ( pMobIndex = get_mob_index ( atoi ( arg3 ) ) ) == NULL )
	{
		send_to_char ( "No mobile has that vnum.\r\n", ch );
		return;
	}
	for ( created = 0; created < count; created++ )
	{
		if ( ( location = get_room_index ( number_range ( tarea->low_r_vnum, tarea->hi_r_vnum ) ) ) == NULL )
		{
			--created;
			continue;
		}
		if ( xIS_SET ( location->room_flags, ROOM_SAFE ) )
		{
			--created;
			continue;
		}
		victim = create_mobile ( pMobIndex );
		char_to_room ( victim, location );
		gq_obj = create_object ( get_obj_index ( OBJ_VNUM_GLOBAL_QUEST ), 0 );
		obj_to_char ( gq_obj, victim );
		act ( AT_IMMORT, "$N appears as part of an invasion force!", ch, NULL, victim, TO_ROOM );
	}
	send_to_char ( "The invasion was successful!\r\n", ch );
	return;
}
