/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ */


#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
char *tattoo_bit_name ( int tattoo_flags )
{
	static char buf[512];
	buf[0] = STRING_NULL;
	if ( ( tattoo_flags & TATTOO_BULL ) == TATTOO_BULL )
		strcat ( buf, " A raging bull on your right arm.  \r\n" );
	if ( ( tattoo_flags & TATTOO_PANTHER ) == TATTOO_PANTHER )
		strcat ( buf, " A sleek black panther on your left arm. \r\n" );
	if ( ( tattoo_flags & TATTOO_WOLF ) == TATTOO_WOLF )
		strcat ( buf, " A wolf stalking it's prey on your right leg. \r\n" );
	if ( ( tattoo_flags & TATTOO_BEAR ) == TATTOO_BEAR )
		strcat ( buf, " A giant sleeping bear on your left leg. \r\n" );
	if ( ( tattoo_flags & TATTOO_RABBIT ) == TATTOO_RABBIT )
		strcat ( buf, " A little white rabbit on your ankle. \r\n" );
	if ( ( tattoo_flags & TATTOO_DRAGON ) == TATTOO_DRAGON )
		strcat ( buf, " A great flying flaming dragon on your chest.\r\n" );
	if ( ( tattoo_flags & TATTOO_MOON ) == TATTOO_MOON )
		strcat ( buf, " A crescent moon across your back. \r\n" );
	return ( buf[0] != STRING_NULL ) ? buf + 1 : "none";
}

/*---------------------------------------------------------------------------------------------*/
void do_tattoo ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *mob;
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	int cost;
	/*
	 * check for artist
	 */
	for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
	{
		if ( IS_NPC ( mob ) && xIS_SET ( mob->act, ACT_IS_TATTOOARTIST ) )
			break;
	}
	/*
	 * if there are none , display the characters tattoos
	 */
	if ( mob == NULL )
	{
		sprintf ( buf, "&C+----------------- Magical Tattoos -----------------------+&W\r\n %s\r\n", tattoo_bit_name ( ch->tattoo ) );
		send_to_char ( buf, ch );
		return;
	}
	one_argument ( argument, arg );
	/*
	 * if there are a surgeon, give a list
	 */
	if ( arg[0] == STRING_NULL )
	{
		/*
		 * display price list
		 */
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "&D   Tattoo                                                          Price        \r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "  &CRaging_Bull                                                     &Y5,000 &CGold\r\n", ch );
		send_to_char ( "  &CSleek_Panther                                                  &Y10,000 &CGold\r\n", ch );
		send_to_char ( "  &CStalking_Wolf                                                  &Y25,000 &CGold\r\n", ch );
		send_to_char ( "  &CSleeping_Bear                                                  &Y50,000 &CGold\r\n", ch );
		send_to_char ( "  &CRabbit                                                        &Y100,000 &CGold\r\n", ch );
		send_to_char ( "  &CFlying_Dragon                                                 &Y200,000 &CGold\r\n", ch );
		send_to_char ( "  &CRising_Moon                                                   &Y500,000 &CGold\r\n", ch );
		send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( " &CType &Wtattoo &B<&Wtype&B>&C to buy one, or help tattoo to get more info.\r\n", ch );
		return;
	}
	/*
	 * Lets see what the character wants to have
	 */
	/* Raging Bull*/
	if ( !str_prefix ( arg, "raging_bull" ) )
	{
		cost = 5000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_BULL ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_BULL );
		ch->max_hit += 10;
		ch->max_mana += 10;
		ch->max_move += 10;
	}
	/* Sleek Panther*/
	else if ( !str_prefix ( arg, "sleek_panther" ) )
	{
		cost = 10000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_PANTHER ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_PANTHER );
		ch->max_hit += 20;
		ch->max_mana += 20;
		ch->max_move += 20;
	}
	/*  Stalking Wolf*/
	else if ( !str_prefix ( arg, "stalking_wolf" ) )
	{
		cost = 25000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_WOLF ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_WOLF );
		ch->max_hit += 30;
		ch->max_mana += 30;
		ch->max_move += 30;
	}
	/* Sleeping Bear*/
	else if ( !str_prefix ( arg, "sleeping_bear" ) )
	{
		cost = 50000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_BEAR ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		ch->max_hit += 50;
		ch->max_mana += 50;
		ch->max_move += 50;
	}
	/* Rabbit*/
	else if ( !str_prefix ( arg, "Rabbit" ) )
	{
		cost = 100000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_RABBIT ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_RABBIT );
		ch->max_hit += 60;
		ch->max_mana += 60;
		ch->max_move += 60;
	}
	/* Flying Dragon*/
	else if ( !str_prefix ( arg, "Flying_Dragon" ) )
	{
		cost = 200000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_DRAGON ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_DRAGON );
		ch->max_hit += 80;
		ch->max_mana += 80;
		ch->max_move += 80;


	}
	/* Rising Moon */
	else if ( !str_prefix ( arg, "Rising_Moon" ) )
	{
		cost = 500000;
		if ( cost > ( ch->gold ) )
		{
			act ( AT_CYAN, "$N says 'I'm sorry but please return to me when you have more gold.'", ch, NULL, mob, TO_CHAR );
			return;
		}
		if ( IS_SET ( ch->tattoo, TATTOO_MOON ) )
		{
			send_to_char ( "You already have that tattoo.\r\n", ch );
			return;
		}
		SET_BIT ( ch->tattoo, TATTOO_MOON );
		xSET_BIT ( ch->affected_by, AFF_TRUESIGHT );
		SET_BIT ( ch->resistant, RIS_DRAIN );
		ch->max_hit += 100;
		ch->max_mana += 100;
		ch->max_move += 100;
	}
	else
	{
		act ( AT_CYAN, "$N says 'Type 'tattoo' for a list of tattoos.'", ch, NULL, mob, TO_CHAR );
		return;
	}
	WAIT_STATE ( ch, PULSE_VIOLENCE );
	ch->gold -= cost;
	act ( AT_CYAN, "$n says 'There we go, all finished'.", mob, NULL, NULL, TO_ROOM );
}
