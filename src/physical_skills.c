/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                         mental skills file                               *
 ****************************************************************************/

#include "./Headers/mud.h"


void do_gouge ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	short dam;
	int schance;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !can_use_skill ( ch, 0, gsn_gouge ) )
	{
		send_to_char ( "You do not yet know of this skill.\r\n", ch );
		return;
	}

	if ( ch->mount )
	{
		send_to_char ( "You can't get close enough while mounted.\r\n", ch );
		return;
	}

	if ( ( victim = who_fighting ( ch ) ) == NULL )
	{
		send_to_char ( "You aren't fighting anyone.\r\n", ch );
		return;
	}

	schance = ( ( get_curr_dex ( victim ) - get_curr_dex ( ch ) ) * 10 ) + 10;

	if ( !IS_NPC ( ch ) && !IS_NPC ( victim ) )
		schance += sysdata.gouge_plr_vs_plr;

	if ( victim->fighting && victim->fighting->who != ch )
		schance += sysdata.gouge_nontank;

	if ( can_use_skill ( ch, ( number_percent( ) + schance ), gsn_gouge ) )
	{
		dam = number_range ( 5, ch->level );
		global_retcode = damage ( ch, victim, dam, gsn_gouge );

		if ( global_retcode == rNONE )
		{
			if ( !IS_AFFECTED ( victim, AFF_BLIND ) )
			{
				af.type = gsn_blindness;
				af.location = APPLY_HITROLL;
				af.modifier = -6;

				if ( !IS_NPC ( victim ) && !IS_NPC ( ch ) )
					af.duration = ( ch->level + 10 ) / get_curr_con ( victim );
				else
					af.duration = 3 + ( ch->level / 15 );

				af.bitvector = meb ( AFF_BLIND );

				affect_to_char ( victim, &af );

				act ( AT_SKILL, "You can't see a thing!", victim, NULL, NULL, TO_CHAR );
			}

			WAIT_STATE ( ch, PULSE_VIOLENCE );

			if ( !IS_NPC ( ch ) && !IS_NPC ( victim ) )
			{
				if ( number_bits ( 1 ) == 0 )
				{
					ch_printf ( ch, "%s looks momentarily dazed.\r\n", victim->name );
					send_to_char ( "You are momentarily dazed ...\r\n", victim );
					WAIT_STATE ( victim, PULSE_VIOLENCE );
				}
			}
			else
				WAIT_STATE ( victim, PULSE_VIOLENCE );

		}
		else
			if ( global_retcode == rVICT_DIED )
			{
				act ( AT_BLOOD, "Your fingers plunge into your victim's brain, causing immediate death!", ch, NULL, NULL, TO_CHAR );
			}

		if ( global_retcode != rCHAR_DIED && global_retcode != rBOTH_DIED )
			learn_from_success ( ch, gsn_gouge );
	}
	else
	{
		WAIT_STATE ( ch, skill_table[gsn_gouge]->beats );
		global_retcode = damage ( ch, victim, 0, gsn_gouge );
		learn_from_failure ( ch, gsn_gouge );
	}

	return;
}

void do_front_kick ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_front_kick]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_front_kick]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_front_kick ) )
	{
		learn_from_success ( ch, gsn_front_kick );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N deflects a kick by $n", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N gets kicked by $n", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_front_kick );
		act ( AT_RED, "Your kick misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_front_kick );
	return;
}

void do_side_kick ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_side_kick]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_side_kick]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_side_kick ) )
	{
		learn_from_success ( ch, gsn_side_kick );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You side kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N deflects a side kick by $n", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You side kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N gets side kicked by $n", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_side_kick );
		act ( AT_RED, "Your side kick misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_side_kick );
	return;
}

void do_roundhouse ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_roundhouse]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_roundhouse]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_roundhouse ) )
	{
		learn_from_success ( ch, gsn_roundhouse );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You roundhouse kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N deflects a roundhouse kick by $n", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You roundhouse kick $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$n's foot curves around in an arc striking $N in the head", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_roundhouse );
		act ( AT_RED, "Your roundhouse misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_roundhouse );
	return;
}

void do_jab ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_jab]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_jab]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_jab ) )
	{
		learn_from_success ( ch, gsn_jab );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You jab $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N deflects a jab by $n", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You jab $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N gets headbutted by $n", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_jab );
		act ( AT_RED, "Your jab misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_jab );
	return;
}

void do_punch ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_punch]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_punch]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_punch ) )
	{
		learn_from_success ( ch, gsn_punch );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You punch $N in the face.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N dodges $n punch", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You punch $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N gets punched by $n", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_punch );
		act ( AT_RED, "Your punch misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_punch );
	return;
}

void do_hook ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	int dam;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_hook]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		if ( ( victim = who_fighting ( ch ) ) == NULL )
		{
			send_to_char ( "You aren't fighting anyone.\r\n", ch );
			return;
		}
	}
	else
		if ( argument[0] != STRING_NULL )
		{
			if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
			{
				send_to_char ( "They aren't here.\r\n", ch );
				return;
			}
		}

	WAIT_STATE ( ch, skill_table[gsn_hook]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_hook ) )
	{
		learn_from_success ( ch, gsn_hook );
		dam = dice ( ch->level / 1.5, 6 );

		if ( saves_physical ( victim ) )
		{
			dam /= 2;
			act ( AT_RED, "You hook $N in the face.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N dodges $n punch", ch, NULL, victim, TO_ROOM );
		}
		else
		{
			act ( AT_RED, "You hook $N.", ch, NULL, victim, TO_CHAR );
			act ( AT_RED, "$N gets punched by $n", ch, NULL, victim, TO_ROOM );
		}
	}
	else
	{
		learn_from_failure ( ch, gsn_hook );
		act ( AT_RED, "Your hook misses $N.", ch, NULL, victim, TO_CHAR );
		dam = 0;
	}
	global_retcode = damage ( ch, victim, dam, gsn_hook );
	return;
}

void do_stun ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	AFFECT_DATA af;
	int schance;
	bool fail;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_stun]->skill_level[ch->Class] )
	{
		send_to_char ( "You better leave the martial arts to fighters.\r\n", ch );
		return;
	}

	if ( ( victim = who_fighting ( ch ) ) == NULL )
	{
		send_to_char ( "You aren't fighting anyone.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->move < ch->max_move / 10 )
	{
		set_char_color ( AT_SKILL, ch );
		send_to_char ( "You are far too tired to do that.\r\n", ch );
		return;  /* missing return fixed March 11/96 */
	}

	WAIT_STATE ( ch, skill_table[gsn_stun]->beats );

	fail = FALSE;
	fail = saves_physical ( victim );

	schance = ( ( ( get_curr_dex ( victim ) + get_curr_str ( victim ) ) - ( get_curr_dex ( ch ) + get_curr_str ( ch ) ) ) * 10 ) + 10;

	/*
	 * harder for player to stun another player
	 */
	if ( !IS_NPC ( ch ) && !IS_NPC ( victim ) )
		schance += sysdata.stun_plr_vs_plr;
	else
		schance += sysdata.stun_regular;

	if ( !fail && can_use_skill ( ch, ( number_percent( ) + schance ), gsn_stun ) )
	{
		learn_from_success ( ch, gsn_stun );
		/*
		 * DO *NOT* CHANGE!    -Thoric
		 */

		if ( !IS_NPC ( ch ) )
			ch->move -= ch->max_move / 10;

		WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );
		WAIT_STATE ( victim, PULSE_VIOLENCE );
		act ( AT_SKILL, "$N smashes into you, leaving you stunned!", victim, NULL, ch, TO_CHAR );
		act ( AT_SKILL, "You smash into $N, leaving $M stunned!", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n smashes into $N, leaving $M stunned!", ch, NULL, victim, TO_NOTVICT );
		if ( !IS_AFFECTED ( victim, AFF_PARALYSIS ) )
		{
			af.type = gsn_stun;
			af.location = APPLY_AC;
			af.modifier = 20;
			af.duration = 3;
			af.bitvector = meb ( AFF_PARALYSIS );
			affect_to_char ( victim, &af );
			update_pos ( victim );
		}
	}
	else
	{
		WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );
		if ( !IS_NPC ( ch ) )
			ch->move -= ch->max_move / 15;
		learn_from_failure ( ch, gsn_stun );
		act ( AT_SKILL, "$n charges at you screaming, but you dodge out of the way.", ch, NULL, victim, TO_VICT );
		act ( AT_SKILL, "You try to stun $N, but $E dodges out of the way.", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n charges screaming at $N, but keeps going right on past.", ch, NULL, victim, TO_NOTVICT );
	}
	return;
}

void do_meditate ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;

	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mobs cannot use this skill.\r\n", ch );
		return;
	}

	if ( IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( ch->mount )
	{
		send_to_char ( "You can't do that while mounted.\r\n", ch );
		return;
	}

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char ( "You failed.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && number_percent( ) > ch->pcdata->learned[gsn_meditate] )
	{
		act ( AT_MAGIC, "You get distracted and stop meditating.", ch, NULL, victim, TO_CHAR );
		act ( AT_MAGIC, "$n is distracted by a vuage memory and stops meditating.", ch, NULL, victim, TO_ROOM );
		learn_from_failure ( ch, gsn_meditate );
		return;
	}
	else
	{
		act ( AT_MAGIC, "You meditate peacefully, collecting mana from the cosmos.", ch, NULL, victim, TO_CHAR );
		act ( AT_MAGIC, "$n meditates peacefully, collecting mana from the cosmos.", ch, NULL, victim, TO_ROOM );
		ch->mana += ( ch->level / 2 + 20 );
		learn_from_success ( ch, gsn_meditate );

		if ( ch->mana > ch->max_mana )
			ch->mana = ch->max_mana;

		return;
	}
}

/* Allows PCs to learn spells embedded in object. Should prove interesting. - Samson 8-9-98 */
void do_study ( CHAR_DATA * ch, char *argument ) /* study by Absalom */
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	int sn = 0;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Study what?\r\n", ch );
		return;
	}

	if ( ( obj = get_obj_carry ( ch, arg ) ) == NULL )
	{
		send_to_char ( "You do not have that item.\r\n", ch );
		return;
	}

	if ( obj->pIndexData->vnum == 34 )
	{
		send_to_char ( "You cannot study a scribed scroll.\r\n", ch );
		return;
	}

	if ( obj->item_type != ITEM_SCROLL )
	{
		send_to_char ( "You can only study scrolls.\r\n", ch );
		return;
	}

	act ( AT_MAGIC, "$n studies $p.", ch, obj, NULL, TO_ROOM );
	act ( AT_MAGIC, "You study $p.", ch, obj, NULL, TO_CHAR );
	sn = obj->value[1];

	if ( sn < 0 || sn >= MAX_SKILL || skill_table[sn]->spell_fun == spell_null )
	{
		bug ( "Do_study: bad sn %d.", sn );
		return;
	}

	if ( number_percent( ) >= 15 + ch->pcdata->learned[gsn_study] * 4 / 5 )
	{
		send_to_char ( "You cannot glean any knowledge from it.\r\n", ch );
		learn_from_failure ( ch, gsn_study );
		act ( AT_FIRE, "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
		separate_obj ( obj );
		extract_obj ( obj );
		return;
	}

	if ( ch->pcdata->learned[sn] )
	{
		send_to_char ( "You already know that spell!\r\n", ch );
		return;
	}

	ch->pcdata->learned[sn] = 1;

	act ( AT_MAGIC, "You have learned the ancient art of $t!", ch, skill_table[sn]->name, NULL, TO_CHAR );
	learn_from_success ( ch, gsn_study );
	act ( AT_FIRE, "$p burns brightly and is gone.", ch, obj, NULL, TO_CHAR );
	separate_obj ( obj );
	extract_obj ( obj );
	return;

}
