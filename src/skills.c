/****************************************************************************
 *   Eldhamud Codebase V2.2        *
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
 *        Player skills module       *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
extern int get_secflag ( char *flag );
ch_ret spell_attack ( int, int, CHAR_DATA *, void * );

char *const spell_flag[] = { "water", "earth", "air", "astral", "area", "distant", "reverse",
                             "noself", "_unused2_", "accumulative", "recastable", "noscribe",
                             "nobrew", "group", "object", "character", "secretskill", "pksensitive",
                             "stoponfail", "nofight", "nodispel", "randomtarget", "r2", "r3", "r4",
                             "r5", "r6", "r7", "r8", "r9", "r10", "r11"
                           };
char *const spell_saves[] = { "none", "poison_death", "wands", "para_petri", "breath", "spell_staff" };

char *const spell_save_effect[] = { "none", "negate", "eightdam", "quarterdam", "halfdam", "3qtrdam", "reflect", "absorb" };

char *const spell_damage[] = { "none", "fire", "cold", "electricity", "energy", "acid", "poison", "drain" };

char *const spell_action[] = { "none", "create", "destroy", "resist", "suscept", "divinate", "obscure", "change" };

char *const spell_power[] = { "none", "minor", "greater", "major" };

char *const spell_Class[] = { "none", "lunar", "solar", "travel", "summon", "life", "death", "illusion" };

char *const target_type[] = { "ignore", "offensive", "defensive", "self", "objinv" };


void show_char_to_char ( CHAR_DATA * list, CHAR_DATA * ch );
bool check_illegal_psteal ( CHAR_DATA * ch, CHAR_DATA * victim );
/*
 * Dummy function
 */
void skill_notfound ( CHAR_DATA * ch, char *argument )
{
	send_to_char ( "Huh?\r\n", ch );
	return;
}

int get_ssave ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_saves ) / sizeof ( spell_saves[0] ); x++ )
		if ( !str_cmp ( name, spell_saves[x] ) )
			return x;

	return -1;
}

int get_starget ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( target_type ) / sizeof ( target_type[0] ); x++ )
		if ( !str_cmp ( name, target_type[x] ) )
			return x;

	return -1;
}

int get_sflag ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_flag ) / sizeof ( spell_flag[0] ); x++ )
		if ( !str_cmp ( name, spell_flag[x] ) )
			return x;

	return -1;
}

int get_sdamage ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_damage ) / sizeof ( spell_damage[0] ); x++ )
		if ( !str_cmp ( name, spell_damage[x] ) )
			return x;

	return -1;
}

int get_saction ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_action ) / sizeof ( spell_action[0] ); x++ )
		if ( !str_cmp ( name, spell_action[x] ) )
			return x;

	return -1;
}

int get_ssave_effect ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_save_effect ) / sizeof ( spell_save_effect[0] ); x++ )
		if ( !str_cmp ( name, spell_save_effect[x] ) )
			return x;

	return -1;
}

int get_spower ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_power ) / sizeof ( spell_power[0] ); x++ )
		if ( !str_cmp ( name, spell_power[x] ) )
			return x;

	return -1;
}

int get_sClass ( char *name )
{
	int x;

	for ( x = 0; x < sizeof ( spell_Class ) / sizeof ( spell_Class[0] ); x++ )
		if ( !str_cmp ( name, spell_Class[x] ) )
			return x;

	return -1;
}

bool is_legal_kill ( CHAR_DATA * ch, CHAR_DATA * vch )
{
	if ( IS_NPC ( ch ) || IS_NPC ( vch ) )
		return TRUE;

	if ( !IS_PKILL ( ch ) || !IS_PKILL ( vch ) )
		return FALSE;

	if ( ch->pcdata->clan && ch->pcdata->clan == vch->pcdata->clan )
		return FALSE;

	return TRUE;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows    -Thoric
 */
bool check_skill ( CHAR_DATA * ch, char *command, char *argument )
{
	int sn;
	int first = gsn_first_skill;
	int top = gsn_first_weapon - 1;
	int mana, blood;

	struct timeval time_used;
	/*
	 * bsearch for the skill
	 */

	for ( ;; )
	{
		sn = ( first + top ) >> 1;

		if ( LOWER ( command[0] ) == LOWER ( skill_table[sn]->name[0] )
		        && !str_prefix ( command, skill_table[sn]->name ) && ( skill_table[sn]->skill_fun || skill_table[sn]->spell_fun != spell_null ) && ( can_use_skill ( ch, 0, sn ) ) )
			/*
			 * &&  (IS_NPC(ch)
			 * ||  (ch->pcdata->learned[sn] > 0
			 * &&   ch->level >= skill_table[sn]->skill_level[ch->Class])) )
			 */
			break;

		if ( first >= top )
			return FALSE;

		if ( strcmp ( command, skill_table[sn]->name ) < 1 )
			top = sn - 1;
		else
			first = sn + 1;
	}

	if ( !check_pos ( ch, skill_table[sn]->minimum_position ) )
		return TRUE;

	if ( IS_NPC ( ch ) && ( IS_AFFECTED ( ch, AFF_CHARM ) || IS_AFFECTED ( ch, AFF_POSSESS ) ) )
	{
		send_to_char ( "For some reason, you seem unable to perform that...\r\n", ch );
		act ( AT_GREY, "$n wanders around aimlessly.", ch, NULL, NULL, TO_ROOM );
		return TRUE;
	}

	/*
	 * check if mana is required
	 */
	if ( skill_table[sn]->min_mana )
	{
		mana = IS_NPC ( ch ) ? 0 : UMAX ( skill_table[sn]->min_mana, 100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );
		blood = UMAX ( 1, ( mana + 4 ) / 8 );  /* NPCs don't have PCDatas. -- Altrag */

		if ( !IS_NPC ( ch ) && ch->mana < mana )
		{
			send_to_char ( "You don't have enough mana.\r\n", ch );
			return TRUE;
		}
	}
	else
	{
		mana = 0;
		blood = 0;
	}

	/*
	 * Is this a real do-fun, or a really a spell?
	 */
	if ( !skill_table[sn]->skill_fun )
	{
		ch_ret retcode = rNONE;
		void *vo = NULL;
		CHAR_DATA *victim = NULL;
		OBJ_DATA *obj = NULL;
		target_name = "";

		switch ( skill_table[sn]->target )
		{

			default:
				bug ( "Check_skill: bad target for sn %d.", sn );
				send_to_char ( "Something went wrong...\r\n", ch );
				return TRUE;

			case TAR_IGNORE:
				vo = NULL;

				if ( argument[0] == STRING_NULL )
				{
					if ( ( victim = who_fighting ( ch ) ) != NULL )
						target_name = victim->name;
				}
				else
					target_name = argument;

				break;

			case TAR_CHAR_OFFENSIVE:
			{
				if ( argument[0] == STRING_NULL && ( victim = who_fighting ( ch ) ) == NULL )
				{
					ch_printf ( ch, "Confusion overcomes you as your '%s' has no target.\r\n", skill_table[sn]->name );
					return TRUE;
				}
				else
					if ( argument[0] != STRING_NULL && ( victim = get_char_room ( ch, argument ) ) == NULL )
					{
						send_to_char ( "They aren't here.\r\n", ch );
						return TRUE;
					}
			}

			if ( is_safe ( ch, victim, TRUE ) )
				return TRUE;

			if ( ch == victim && SPELL_FLAG ( skill_table[sn], SF_NOSELF ) )
			{
				send_to_char ( "You can't target yourself!\r\n", ch );
				return TRUE;
			}

			if ( !IS_NPC ( ch ) )
			{
				if ( !IS_NPC ( victim ) )
				{
					/*
					 * Sheesh! can't do anything
					 * send_to_char( "You can't do that on a player.\r\n", ch );
					 * return TRUE;
					 */
					/*
					 * if ( xIS_SET(victim->act, PLR_PK))
					 */
					if ( get_timer ( ch, TIMER_PKILLED ) > 0 )
					{
						send_to_char ( "You have been killed in the last 5 minutes.\r\n", ch );
						return TRUE;
					}

					if ( get_timer ( victim, TIMER_PKILLED ) > 0 )
					{
						send_to_char ( "This player has been killed in the last 5 minutes.\r\n", ch );
						return TRUE;
					}

					if ( victim != ch )
						send_to_char ( "You really shouldn't do this to another player...\r\n", ch );
				}

				if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master == victim )
				{
					send_to_char ( "You can't do that on your own follower.\r\n", ch );
					return TRUE;
				}
			}

			check_illegal_pk ( ch, victim );

			vo = ( void * ) victim;
			break;

			case TAR_CHAR_DEFENSIVE:
			{
				if ( argument[0] != STRING_NULL && ( victim = get_char_room ( ch, argument ) ) == NULL )
				{
					send_to_char ( "They aren't here.\r\n", ch );
					return TRUE;
				}

				if ( !victim )
					victim = ch;
			}

			if ( ch == victim && SPELL_FLAG ( skill_table[sn], SF_NOSELF ) )
			{
				send_to_char ( "You can't target yourself!\r\n", ch );
				return TRUE;
			}

			vo = ( void * ) victim;

			break;

			case TAR_CHAR_SELF:
				victim = ch;
				break;

			case TAR_OBJ_INV:
			{
				if ( ( obj = get_obj_carry ( ch, argument ) ) == NULL )
				{
					send_to_char ( "You can't find that.\r\n", ch );
					return TRUE;
				}
			}

			vo = ( void * ) obj;
			break;
		}

		/*
		 * waitstate
		 */
		WAIT_STATE ( ch, skill_table[sn]->beats );

		/*
		 * check for failure
		 */
		if ( ( number_percent( ) + skill_table[sn]->difficulty * 5 ) > ( IS_NPC ( ch ) ? 75 : LEARNED ( ch, sn ) ) )
		{
			failed_casting ( skill_table[sn], ch, victim, obj );
			learn_from_failure ( ch, sn );

			if ( mana )
			{
				ch->mana -= mana / 2;
			}

			return TRUE;
		}

		if ( mana )
		{
			ch->mana -= mana;
		}

		start_timer ( &time_used );

		retcode = ( *skill_table[sn]->spell_fun ) ( sn, ch->level, ch, vo );
		end_timer ( &time_used );
		update_userec ( &time_used, &skill_table[sn]->userec );

		if ( retcode == rCHAR_DIED || retcode == rERROR )
			return TRUE;

		if ( char_died ( ch ) )
			return TRUE;

		if ( retcode == rSPELL_FAILED )
		{
			learn_from_failure ( ch, sn );
			retcode = rNONE;
		}
		else
			learn_from_success ( ch, sn );

		if ( skill_table[sn]->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died ( victim ) )
		{
			CHAR_DATA *vch;
			CHAR_DATA *vch_next;

			for ( vch = ch->in_room->first_person; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;

				if ( victim == vch && !victim->fighting && victim->master != ch )
				{
					retcode = multi_hit ( victim, ch, TYPE_UNDEFINED );
					break;
				}
			}
		}

		return TRUE;
	}

	if ( mana )
	{
		ch->mana -= mana;
	}

	ch->prev_cmd = ch->last_cmd;  /* haus, for automapping */

	ch->last_cmd = skill_table[sn]->skill_fun;
	start_timer ( &time_used );
	( *skill_table[sn]->skill_fun ) ( ch, argument );
	end_timer ( &time_used );
	update_userec ( &time_used, &skill_table[sn]->userec );
	tail_chain( );
	return TRUE;
}

/*
 * Lookup a skills information
 * High god command
 */
void do_slookup ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int sn;
	SKILLTYPE *skill = NULL;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Slookup what?\r\n", ch );
		return;
	}

				
	if ( !str_cmp ( arg, "all" ) )
	{
		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		send_to_char ( "SKILL ##  SKILL NAME         MANA BEATS RANGE  POSITION\r\n", ch );
		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		for ( sn = 0; sn < top_sn && skill_table[sn] && skill_table[sn]->name; sn++ )
		{
			switch ( skill_table[sn]->minimum_position )
				{

					default:
						sprintf ( buf, "Invalid" );
						bug ( "do_slist: skill with invalid minpos, skill=%s", skill_table[sn]->name );
						break;

					case POS_DEAD:
						sprintf ( buf, "any" );
						break;

					case POS_MORTAL:
						sprintf ( buf, "mortally wounded" );
						break;

					case POS_INCAP:
						sprintf ( buf, "incapacitated" );
						break;

					case POS_STUNNED:
						sprintf ( buf, "stunned" );
						break;

					case POS_SLEEPING:
						sprintf ( buf, "sleeping" );
						break;

					case POS_RESTING:
						sprintf ( buf, "resting" );
						break;

					case POS_STANDING:
						sprintf ( buf, "standing" );
						break;

					case POS_FIGHTING:
						sprintf ( buf, "fighting" );
						break;

					case POS_EVASIVE:
						sprintf ( buf, "evasive" ); /* Fighting style support -haus */
						break;

					case POS_DEFENSIVE:
						sprintf ( buf, "defensive" );
						break;

					case POS_AGGRESSIVE:
						sprintf ( buf, "aggressive" );
						break;

					case POS_BERSERK:
						sprintf ( buf, "berserk" );
						break;

					case POS_MOUNTED:
						sprintf ( buf, "mounted" );
						break;

					case POS_SITTING:
						sprintf ( buf, "sitting" );
						break;
				}

			pager_printf ( ch, "Sn: %3d : %-20s %-3d  %-3d  %-3d  %s\r\n", 
					sn, skill_table[sn]->name, skill_table[sn]->min_mana, skill_table[sn]->beats, skill_table[sn]->range, buf );

		}
	}
	else
		if ( !str_cmp ( arg, "herbs" ) )
		{
			for ( sn = 0; sn < top_herb && herb_table[sn] && herb_table[sn]->name; sn++ )
				pager_printf ( ch, "%d) %s\r\n", sn, herb_table[sn]->name );
		}
		else
		{
			SMAUG_AFF *aff;
			int cnt = 0;

			if ( arg[0] == 'h' && is_number ( arg + 1 ) )
			{
				sn = atoi ( arg + 1 );

				if ( !IS_VALID_HERB ( sn ) )
				{
					send_to_char ( "Invalid herb.\r\n", ch );
					return;
				}

				skill = herb_table[sn];
			}
			else
				if ( is_number ( arg ) )
				{
					sn = atoi ( arg );

					if ( ( skill = get_skilltype ( sn ) ) == NULL )
					{
						send_to_char ( "Invalid sn.\r\n", ch );
						return;
					}

					sn %= 1000;
				}
				else
					if ( ( sn = skill_lookup ( arg ) ) >= 0 )
						skill = skill_table[sn];
					else
						if ( ( sn = herb_lookup ( arg ) ) >= 0 )
							skill = herb_table[sn];
						else
						{
							send_to_char ( "No such skill, spell, proficiency or tongue.\r\n", ch );
							return;
						}

			if ( !skill )
			{
				send_to_char ( "Not created yet.\r\n", ch );
				return;
			}

			ch_printf ( ch, "Sn: %4d %s: '%-20s'\r\n", sn, skill_tname[skill->type], skill->name );

			if ( skill->info )
				ch_printf ( ch, "DamType: %s  ActType: %s   ClassType: %s   PowerType: %s\r\n",
				            spell_damage[SPELL_DAMAGE ( skill ) ], spell_action[SPELL_ACTION ( skill ) ], spell_Class[SPELL_CLASS ( skill ) ], spell_power[SPELL_POWER ( skill ) ] );

			if ( skill->flags )
			{
				int x;
				strcpy ( buf, "Flags:" );

				for ( x = 0; x < 32; x++ )
					if ( SPELL_FLAG ( skill, 1 << x ) )
					{
						strcat ( buf, " " );
						strcat ( buf, spell_flag[x] );
					}

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );
			}

			ch_printf ( ch, "Saves: %s  SaveEffect: %s\r\n", spell_saves[ ( int ) skill->saves], spell_save_effect[SPELL_SAVE ( skill ) ] );

			if ( skill->difficulty != STRING_NULL )
				ch_printf ( ch, "Difficulty: %d\r\n", ( int ) skill->difficulty );

			ch_printf ( ch, "Type: %s  Target: %s  Minpos: %d  Mana: %d  Beats: %d  Range: %d\r\n",
			            skill_tname[skill->type],
			            target_type[URANGE ( TAR_IGNORE, skill->target, TAR_OBJ_INV ) ], skill->minimum_position, skill->min_mana, skill->beats, skill->range );

			ch_printf ( ch, "Flags: %d  Guild: %d  Value: %d  Info: %d  Code: %s\r\n",
			            skill->flags, skill->guild, skill->value, skill->info, skill->skill_fun ? skill->skill_fun_name : skill->spell_fun_name );

			ch_printf ( ch, "Difficulty: %d \r\n", skill->difficulty );

			ch_printf ( ch, "Sectors Allowed: %s\n", skill->spell_sector ? flag_string ( skill->spell_sector, sec_flags ) : "All" );

			ch_printf ( ch, "Dammsg: %s\r\nWearoff: %s\n", skill->noun_damage, skill->msg_off ? skill->msg_off : "(none set)" );

			if ( skill->dice && skill->dice[0] != STRING_NULL )
				ch_printf ( ch, "Dice: %s\r\n", skill->dice );

			if ( skill->teachers && skill->teachers[0] != STRING_NULL )
				ch_printf ( ch, "Teachers: %s\r\n", skill->teachers );

			if ( skill->components && skill->components[0] != STRING_NULL )
				ch_printf ( ch, "Components: %s\r\n", skill->components );

			if ( skill->participants )
				ch_printf ( ch, "Participants: %d\r\n", ( int ) skill->participants );

			if ( skill->userec.num_uses )
				send_timer ( &skill->userec, ch );

			for ( aff = skill->affects; aff; aff = aff->next )
			{
				if ( aff == skill->affects )
					send_to_char ( "\r\n", ch );

				sprintf ( buf, "Affect %d", ++cnt );

				if ( aff->location )
				{
					strcat ( buf, " modifies " );
					strcat ( buf, a_types[aff->location % REVERSE_APPLY] );
					strcat ( buf, " by '" );
					strcat ( buf, aff->modifier );

					if ( aff->bitvector != -1 )
						strcat ( buf, "' and" );
					else
						strcat ( buf, "'" );
				}

				if ( aff->bitvector != -1 )
				{
					strcat ( buf, " applies " );
					strcat ( buf, a_flags[aff->bitvector] );
				}

				if ( aff->duration[0] != STRING_NULL && aff->duration[0] != '0' )
				{
					strcat ( buf, " for '" );
					strcat ( buf, aff->duration );
					strcat ( buf, "' rounds" );
				}

				if ( aff->location >= REVERSE_APPLY )
					strcat ( buf, " (affects caster only)" );

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );

				if ( !aff->next )
					send_to_char ( "\r\n", ch );
			}

			if ( skill->hit_char && skill->hit_char[0] != STRING_NULL )
				ch_printf ( ch, "Hitchar   : %s\r\n", skill->hit_char );

			if ( skill->hit_vict && skill->hit_vict[0] != STRING_NULL )
				ch_printf ( ch, "Hitvict   : %s\r\n", skill->hit_vict );

			if ( skill->hit_room && skill->hit_room[0] != STRING_NULL )
				ch_printf ( ch, "Hitroom   : %s\r\n", skill->hit_room );

			if ( skill->hit_dest && skill->hit_dest[0] != STRING_NULL )
				ch_printf ( ch, "Hitdest   : %s\r\n", skill->hit_dest );

			if ( skill->miss_char && skill->miss_char[0] != STRING_NULL )
				ch_printf ( ch, "Misschar  : %s\r\n", skill->miss_char );

			if ( skill->miss_vict && skill->miss_vict[0] != STRING_NULL )
				ch_printf ( ch, "Missvict  : %s\r\n", skill->miss_vict );

			if ( skill->miss_room && skill->miss_room[0] != STRING_NULL )
				ch_printf ( ch, "Missroom  : %s\r\n", skill->miss_room );

			if ( skill->die_char && skill->die_char[0] != STRING_NULL )
				ch_printf ( ch, "Diechar   : %s\r\n", skill->die_char );

			if ( skill->die_vict && skill->die_vict[0] != STRING_NULL )
				ch_printf ( ch, "Dievict   : %s\r\n", skill->die_vict );

			if ( skill->die_room && skill->die_room[0] != STRING_NULL )
				ch_printf ( ch, "Dieroom   : %s\r\n", skill->die_room );

			if ( skill->imm_char && skill->imm_char[0] != STRING_NULL )
				ch_printf ( ch, "Immchar   : %s\r\n", skill->imm_char );

			if ( skill->imm_vict && skill->imm_vict[0] != STRING_NULL )
				ch_printf ( ch, "Immvict   : %s\r\n", skill->imm_vict );

			if ( skill->imm_room && skill->imm_room[0] != STRING_NULL )
				ch_printf ( ch, "Immroom   : %s\r\n", skill->imm_room );


			send_to_char ( "\r\n", ch );
		}

	return;
}


/*
 * Check for parry.
 */
bool check_parry ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	int chances;

	if ( victim->level < skill_table[gsn_parry]->skill_level[victim->Class] )
		return FALSE;

	if ( !IS_AWAKE ( victim ) )
		return FALSE;

	if ( IS_NPC ( victim ) && !xIS_SET ( victim->defenses, DFND_PARRY ) )
		return FALSE;

	if ( IS_NPC ( victim ) )
		chances = 10 + victim->perm_dex + ( victim->saving_weapons - ch->saving_weapons );
	else
	{
		if ( get_eq_char ( victim, WEAR_WIELD ) == NULL )
			return FALSE;
		chances = ( int ) ( ( LEARNED ( victim, gsn_dodge ) + ( victim->saving_weapons - ch->saving_weapons ) + ( victim->perm_dex - ch->perm_dex ) ) / 3 );
		if ( chances == 0 )
			return FALSE;
	}

	if ( !chance ( victim, chances + victim->level - ch->level ) )
	{
		learn_from_failure ( victim, gsn_parry );
		return FALSE;
	}

	if ( !IS_NPC ( victim ) && !IS_SET ( victim->pcdata->flags, PCFLAG_GAG ) )
		act ( AT_SKILL, "You parry $n's attack.", ch, NULL, victim, TO_VICT );
	if ( !IS_NPC ( ch ) && !IS_SET ( ch->pcdata->flags, PCFLAG_GAG ) )   /* SB */
		act ( AT_SKILL, "$N parries your attack.", ch, NULL, victim, TO_CHAR );

	learn_from_success ( victim, gsn_parry );
	return TRUE;
}

/*
 * Check for dodge.
 */
bool check_dodge ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	int chances;

	if ( victim->level < skill_table[gsn_dodge]->skill_level[victim->Class] )
		return FALSE;

	if ( !IS_AWAKE ( victim ) )
		return FALSE;

	if ( IS_NPC ( victim ) && !xIS_SET ( victim->defenses, DFND_DODGE ) )
		return FALSE;

	if ( IS_NPC ( victim ) )
		chances = 10 + victim->perm_dex + ( victim->saving_weapons - ch->saving_weapons );
	else
	{
		chances = ( int ) ( ( LEARNED ( victim, gsn_dodge ) + ( victim->saving_weapons - ch->saving_weapons ) + ( victim->perm_dex - ch->perm_dex ) ) / 3 );
		if ( chances == 0 )
			return FALSE;
	}
	/*
	 * Saving Roll For Weapons
	 */
	if ( !chance ( victim, chances + victim->level - ch->level ) )
	{
		learn_from_failure ( victim, gsn_dodge );
		return FALSE;
	}

	if ( !IS_NPC ( victim ) && !IS_SET ( victim->pcdata->flags, PCFLAG_GAG ) )
		act ( AT_SKILL, "You dodge $n's attack.", ch, NULL, victim, TO_VICT );

	if ( !IS_NPC ( ch ) && !IS_SET ( ch->pcdata->flags, PCFLAG_GAG ) )
		act ( AT_SKILL, "$N dodges your attack.", ch, NULL, victim, TO_CHAR );

	learn_from_success ( victim, gsn_dodge );
	return TRUE;
}

bool check_tumble ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	int chances;

	if ( victim->level < skill_table[gsn_tumble]->skill_level[victim->Class] )
		return FALSE;

	if ( !IS_AWAKE ( victim ) )
		return FALSE;

	if ( IS_NPC ( victim ) && !xIS_SET ( victim->defenses, DFND_TUMBLE ) )
		return FALSE;

	if ( IS_NPC ( victim ) )
		chances = 10 + victim->perm_dex + ( victim->saving_weapons - ch->saving_weapons );
	else
	{
		chances = ( int ) ( ( LEARNED ( victim, gsn_dodge ) + ( victim->saving_weapons - ch->saving_weapons ) + ( victim->perm_dex - ch->perm_dex ) ) / 3 );
		if ( chances == 0 )
			return FALSE;
	}
	if ( !chance ( victim, chances + victim->level - ch->level ) )
	{
		learn_from_failure ( victim, gsn_tumble );
		return FALSE;
	}

	if ( !IS_NPC ( victim ) && !IS_SET ( victim->pcdata->flags, PCFLAG_GAG ) )
		act ( AT_SKILL, "You tumble away from $n's attack.", ch, NULL, victim, TO_VICT );

	if ( !IS_NPC ( ch ) && !IS_SET ( ch->pcdata->flags, PCFLAG_GAG ) )
		act ( AT_SKILL, "$N tumbles away from your attack.", ch, NULL, victim, TO_CHAR );

	learn_from_success ( victim, gsn_tumble );
	return TRUE;
}

bool check_grip ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	int schance;

	if ( !IS_AWAKE ( victim ) )
		return FALSE;

	if ( IS_NPC ( victim ) && !xIS_SET ( victim->defenses, DFND_GRIP ) )
		return FALSE;

	if ( IS_NPC ( victim ) )
		schance = UMIN ( 20, victim->level / 1.5 );
	else
		schance = ( LEARNED ( victim, gsn_grip ) );

	/*
	 * Consider luck as a factor
	 */
	schance += ( 2 * ( get_curr_lck ( victim ) - 13 ) );

	if ( number_percent( ) >= schance + victim->level - ch->level )
	{
		learn_from_failure ( victim, gsn_grip );
		return FALSE;
	}
	else
	{
		act ( AT_SKILL, "You evade $n's attempt to disarm you.", ch, NULL, victim, TO_VICT );
		act ( AT_SKILL, "$N holds $S weapon strongly, and is not disarmed.", ch, NULL, victim, TO_CHAR );
		learn_from_success ( victim, gsn_grip );
		return TRUE;
	}
}

/*
 * Set a skill's attributes or what skills a player has.
 * High god command, with support for creating skills/spells/herbs/etc
 */
void do_sset ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value;
	int sn, i;
	bool fAll;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );

	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL || argument[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: sset <victim> <skill> <value>\r\n", ch );
		send_to_char ( "or:     sset <victim> all     <value>\r\n", ch );

		if ( get_trust ( ch ) > LEVEL_SUB_IMPLEM )
		{
			send_to_char ( "or:     sset save skill table\r\n", ch );
			send_to_char ( "or:     sset save herb table\r\n", ch );
			send_to_char ( "or:     sset create skill 'new skill'\r\n", ch );
			send_to_char ( "or:     sset create herb 'new herb'\r\n", ch );
			send_to_char ( "or:     sset create ability 'new ability'\r\n", ch );
		}

		if ( get_trust ( ch ) > LEVEL_GREATER )
		{
			send_to_char ( "or:     sset <sn>     <field> <value>\r\n", ch );
			send_to_char ( "\r\nField being one of:\r\n", ch );
			send_to_char ( "  name code target minpos slot mana beats dammsg wearoff guild minlevel\r\n", ch );
			send_to_char ( "  type damtype acttype Classtype powertype seffect flag dice value difficulty\r\n", ch );
			send_to_char ( "  affect rmaffect level adept hit miss die imm (char/vict/room)\r\n", ch );
			send_to_char ( "  components teachers racelevel raceadept\r\n", ch );
			send_to_char ( "  sector\r\n", ch );
			send_to_char ( "Affect having the fields: <location> <modfifier> [duration] [bitvector]\r\n", ch );
			send_to_char ( "(See AFFECTTYPES for location, and AFFECTED_BY for bitvector)\r\n", ch );
		}

		send_to_char ( "Skill being any skill or spell.\r\n", ch );

		return;
	}

	if ( get_trust ( ch ) > LEVEL_SUB_IMPLEM && !str_cmp ( arg1, "save" ) && !str_cmp ( argument, "table" ) )
	{
		if ( !str_cmp ( arg2, "skill" ) )
		{
			send_to_char ( "Saving skill table...\r\n", ch );
			save_skill_table( );
			save_Classes( );
			/*
			 * save_races();
			 */
			return;
		}
	}

	if ( get_trust ( ch ) > LEVEL_SUB_IMPLEM && !str_cmp ( arg1, "create" ) && ( !str_cmp ( arg2, "skill" ) ) )
	{

		struct skill_type *skill;
		short type = SKILL_UNKNOWN;

		if ( top_sn >= MAX_SKILL )
			{
				ch_printf ( ch, "The current top sn is %d, which is the maximum. To add more skills, MAX_SKILL will"
                                                "have to be " "raised in mud.h, and the mud recompiled.\r\n", top_sn );
				return;
			}

		CREATE ( skill, struct skill_type, 1 );

		skill_table[top_sn++] = skill;
		skill->min_mana = 0;
		skill->name = str_dup ( argument );
		skill->noun_damage = str_dup ( "" );
		skill->msg_off = str_dup ( "" );
		skill->spell_fun = spell_null;
		skill->type = type;
		skill->spell_sector = 0;
		skill->guild = -1;

		for ( i = 0; i < MAX_PC_CLASS; i++ )
		{
			skill->skill_level[i] = LEVEL_IMMORTAL;
			skill->skill_adept[i] = 95;
		}

		for ( i = 0; i < MAX_PC_RACE; i++ )
		{
			skill->race_level[i] = LEVEL_IMMORTAL;
			skill->race_adept[i] = 95;
		}

		send_to_char ( "Done.\r\n", ch );

		return;
	}

	if ( arg1[0] == 'h' )
		sn = atoi ( arg1 + 1 );
	else
		sn = atoi ( arg1 );

	if ( get_trust ( ch ) > LEVEL_GREATER
	        && ( ( arg1[0] == 'h' && is_number ( arg1 + 1 ) && ( sn = atoi ( arg1 + 1 ) ) >= 0 ) || ( is_number ( arg1 ) && ( sn = atoi ( arg1 ) ) >= 0 ) ) )
	{

		struct skill_type *skill;

		if ( arg1[0] == 'h' )
		{
			if ( sn >= top_herb )
			{
				send_to_char ( "Herb number out of range.\r\n", ch );
				return;
			}

			skill = herb_table[sn];
		}
		else
		{
			if ( ( skill = get_skilltype ( sn ) ) == NULL )
			{
				send_to_char ( "Skill number out of range.\r\n", ch );
				return;
			}

			sn %= 1000;
		}

		if ( !str_cmp ( arg2, "difficulty" ) )
		{
			skill->difficulty = atoi ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "participants" ) )
		{
			skill->participants = atoi ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "damtype" ) )
		{
			int x = get_sdamage ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell damage type.\r\n", ch );
			else
			{
				SET_SDAM ( skill, x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "acttype" ) )
		{
			int x = get_saction ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell action type.\r\n", ch );
			else
			{
				SET_SACT ( skill, x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "Classtype" ) )
		{
			int x = get_sClass ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell Class type.\r\n", ch );
			else
			{
				SET_SCLA ( skill, x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "powertype" ) )
		{
			int x = get_spower ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell power type.\r\n", ch );
			else
			{
				SET_SPOW ( skill, x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "seffect" ) )
		{
			int x = get_ssave_effect ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell save effect type.\r\n", ch );
			else
			{
				SET_SSAV ( skill, x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "flag" ) )
		{
			int x = get_sflag ( argument );

			if ( x == -1 )
				send_to_char ( "Not a spell flag.\r\n", ch );
			else
			{
				TOGGLE_BIT ( skill->flags, 1 << x );
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "saves" ) )
		{
			int x = get_ssave ( argument );

			if ( x == -1 )
				send_to_char ( "Not a saving type.\r\n", ch );
			else
			{
				skill->saves = x;
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "code" ) )
		{
			SPELL_FUN *spellfun;
			DO_FUN *dofun;

			if ( !str_prefix ( "do_", argument ) && ( dofun = skill_function ( argument ) ) != skill_notfound )
			{
				skill->skill_fun = dofun;
				skill->spell_fun = NULL;
				DISPOSE ( skill->skill_fun_name );
				skill->skill_fun_name = str_dup ( argument );
			}
			else
				if ( ( spellfun = spell_function ( argument ) ) != spell_notfound )
				{
					skill->spell_fun = spellfun;
					skill->skill_fun = NULL;
					DISPOSE ( skill->skill_fun_name );
					skill->spell_fun_name = str_dup ( argument );
				}
				else
				{
					send_to_char ( "Not a spell or skill.\r\n", ch );
					return;
				}

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "target" ) )
		{
			int x = get_starget ( argument );

			if ( x == -1 )
				send_to_char ( "Not a valid target type.\r\n", ch );
			else
			{
				skill->target = x;
				send_to_char ( "Ok.\r\n", ch );
			}

			return;
		}

		if ( !str_cmp ( arg2, "minpos" ) )
		{
			skill->minimum_position = URANGE ( POS_DEAD, atoi ( argument ), POS_DRAG );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "minlevel" ) )
		{
			skill->min_level = URANGE ( 1, atoi ( argument ), MAX_LEVEL );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "sector" ) )
		{
			char tmp_arg[MAX_STRING_LENGTH];

			while ( argument[0] != STRING_NULL )
			{
				argument = one_argument ( argument, tmp_arg );
				value = get_secflag ( tmp_arg );

				if ( value < 0 || value > SECT_MAX )
					ch_printf ( ch, "Unknown flag: %s\r\n", tmp_arg );
				else
					TOGGLE_BIT ( skill->spell_sector, ( 1 << value ) );
			}

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "mana" ) )
		{
			skill->min_mana = URANGE ( 0, atoi ( argument ), 2000 );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "beats" ) )
		{
			skill->beats = URANGE ( 0, atoi ( argument ), 120 );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "range" ) )
		{
			skill->range = URANGE ( 0, atoi ( argument ), 20 );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "guild" ) )
		{
			skill->guild = atoi ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "value" ) )
		{
			skill->value = atoi ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "type" ) )
		{
			skill->type = get_skill ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "rmaffect" ) )
		{
			SMAUG_AFF *aff = skill->affects;
			SMAUG_AFF *aff_next;
			int num = atoi ( argument );
			int cnt = 1;

			if ( !aff )
			{
				send_to_char ( "This spell has no special affects to remove.\r\n", ch );
				return;
			}

			if ( num == 1 )
			{
				skill->affects = aff->next;
				DISPOSE ( aff->duration );
				DISPOSE ( aff->modifier );
				DISPOSE ( aff );
				send_to_char ( "Removed.\r\n", ch );
				return;
			}

			for ( ; aff; aff = aff->next )
			{
				if ( ++cnt == num && ( aff_next = aff->next ) != NULL )
				{
					aff->next = aff_next->next;
					DISPOSE ( aff_next->duration );
					DISPOSE ( aff_next->modifier );
					DISPOSE ( aff_next );
					send_to_char ( "Removed.\r\n", ch );
					return;
				}
			}

			send_to_char ( "Not found.\r\n", ch );

			return;
		}

		/*
		 * affect <location> <modifier> <duration> <bitvector>
		 */
		if ( !str_cmp ( arg2, "affect" ) )
		{
			char location[MAX_INPUT_LENGTH];
			char modifier[MAX_INPUT_LENGTH];
			char duration[MAX_INPUT_LENGTH];
			/*
			 * char bitvector[MAX_INPUT_LENGTH];
			 */
			int loc, bit, tmpbit;
			SMAUG_AFF *aff;
			argument = one_argument ( argument, location );
			argument = one_argument ( argument, modifier );
			argument = one_argument ( argument, duration );

			if ( location[0] == '!' )
				loc = get_atype ( location + 1 ) + REVERSE_APPLY;
			else
				loc = get_atype ( location );

			if ( ( loc % REVERSE_APPLY ) < 0 || ( loc % REVERSE_APPLY ) >= MAX_APPLY_TYPE )
			{
				send_to_char ( "Unknown affect location.  See AFFECTTYPES.\r\n", ch );
				return;
			}

			bit = -1;

			if ( argument[0] != STRING_NULL )
			{
				if ( ( tmpbit = get_aflag ( argument ) ) == -1 )
					ch_printf ( ch, "Unknown bitvector: %s.  See AFFECTED_BY\r\n", argument );
				else
					bit = tmpbit;
			}

			CREATE ( aff, SMAUG_AFF, 1 );

			if ( !str_cmp ( duration, "0" ) )
				duration[0] = STRING_NULL;

			if ( !str_cmp ( modifier, "0" ) )
				modifier[0] = STRING_NULL;

			aff->duration = str_dup ( duration );

			aff->location = loc;

			if ( loc == APPLY_AFFECT || loc == APPLY_RESISTANT || loc == APPLY_IMMUNE || loc == APPLY_SUSCEPTIBLE )
			{
				int modval = get_aflag ( modifier );
				/*
				 * Sanitize the flag input for the modifier if needed -- Samson
				 */

				if ( modval < 0 )
					modval = 0;

				snprintf ( modifier, MAX_INPUT_LENGTH, "%d", modval );
			}

			aff->modifier = str_dup ( modifier );

			aff->bitvector = bit;
			aff->next = skill->affects;
			skill->affects = aff;
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "level" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int Class;
			argument = one_argument ( argument, arg3 );
			Class = atoi ( arg3 );

			if ( Class >= MAX_PC_CLASS || Class < 0 )
				send_to_char ( "Not a valid Class.\r\n", ch );
			else
				skill->skill_level[Class] = URANGE ( 0, atoi ( argument ), MAX_LEVEL );

			return;
		}

		if ( !str_cmp ( arg2, "racelevel" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int race;
			argument = one_argument ( argument, arg3 );
			race = atoi ( arg3 );

			if ( race >= MAX_PC_RACE || race < 0 )
				send_to_char ( "Not a valid race.\r\n", ch );
			else
				skill->race_level[race] = URANGE ( 0, atoi ( argument ), MAX_LEVEL );

			return;
		}

		if ( !str_cmp ( arg2, "adept" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int Class;
			argument = one_argument ( argument, arg3 );
			Class = atoi ( arg3 );

			if ( Class >= MAX_PC_CLASS || Class < 0 )
				send_to_char ( "Not a valid Class.\r\n", ch );
			else
				skill->skill_adept[Class] = URANGE ( 0, atoi ( argument ), 100 );

			return;
		}

		if ( !str_cmp ( arg2, "raceadept" ) )
		{
			char arg3[MAX_INPUT_LENGTH];
			int race;
			argument = one_argument ( argument, arg3 );
			race = atoi ( arg3 );

			if ( race >= MAX_PC_RACE || race < 0 )
				send_to_char ( "Not a valid race.\r\n", ch );
			else
				skill->race_adept[race] = URANGE ( 0, atoi ( argument ), 100 );

			return;
		}

		if ( !str_cmp ( arg2, "name" ) )
		{
			DISPOSE ( skill->name );
			skill->name = str_dup ( argument );
			send_to_char ( "Ok.\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg2, "dammsg" ) )
		{
			DISPOSE ( skill->noun_damage );

			if ( !str_cmp ( argument, "clear" ) )
				skill->noun_damage = str_dup ( "" );
			else
				skill->noun_damage = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "wearoff" ) )
		{
			DISPOSE ( skill->msg_off );

			if ( str_cmp ( argument, "clear" ) )
				skill->msg_off = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "hitchar" ) )
		{
			if ( skill->hit_char )
				DISPOSE ( skill->hit_char );

			if ( str_cmp ( argument, "clear" ) )
				skill->hit_char = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "hitvict" ) )
		{
			if ( skill->hit_vict )
				DISPOSE ( skill->hit_vict );

			if ( str_cmp ( argument, "clear" ) )
				skill->hit_vict = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "hitroom" ) )
		{
			if ( skill->hit_room )
				DISPOSE ( skill->hit_room );

			if ( str_cmp ( argument, "clear" ) )
				skill->hit_room = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "hitdest" ) )
		{
			if ( skill->hit_dest )
				DISPOSE ( skill->hit_dest );

			if ( str_cmp ( argument, "clear" ) )
				skill->hit_dest = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "misschar" ) )
		{
			if ( skill->miss_char )
				DISPOSE ( skill->miss_char );

			if ( str_cmp ( argument, "clear" ) )
				skill->miss_char = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "missvict" ) )
		{
			if ( skill->miss_vict )
				DISPOSE ( skill->miss_vict );

			if ( str_cmp ( argument, "clear" ) )
				skill->miss_vict = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "missroom" ) )
		{
			if ( skill->miss_room )
				DISPOSE ( skill->miss_room );

			if ( str_cmp ( argument, "clear" ) )
				skill->miss_room = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "diechar" ) )
		{
			if ( skill->die_char )
				DISPOSE ( skill->die_char );

			if ( str_cmp ( argument, "clear" ) )
				skill->die_char = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "dievict" ) )
		{
			if ( skill->die_vict )
				DISPOSE ( skill->die_vict );

			if ( str_cmp ( argument, "clear" ) )
				skill->die_vict = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "dieroom" ) )
		{
			if ( skill->die_room )
				DISPOSE ( skill->die_room );

			if ( str_cmp ( argument, "clear" ) )
				skill->die_room = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "immchar" ) )
		{
			if ( skill->imm_char )
				DISPOSE ( skill->imm_char );

			if ( str_cmp ( argument, "clear" ) )
				skill->imm_char = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "immvict" ) )
		{
			if ( skill->imm_vict )
				DISPOSE ( skill->imm_vict );

			if ( str_cmp ( argument, "clear" ) )
				skill->imm_vict = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "immroom" ) )
		{
			if ( skill->imm_room )
				DISPOSE ( skill->imm_room );

			if ( str_cmp ( argument, "clear" ) )
				skill->imm_room = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "dice" ) )
		{
			if ( skill->dice )
				DISPOSE ( skill->dice );

			if ( str_cmp ( argument, "clear" ) )
				skill->dice = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "components" ) )
		{
			if ( skill->components )
				DISPOSE ( skill->components );

			if ( str_cmp ( argument, "clear" ) )
				skill->components = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		if ( !str_cmp ( arg2, "teachers" ) )
		{
			if ( skill->teachers )
				DISPOSE ( skill->teachers );

			if ( str_cmp ( argument, "clear" ) )
				skill->teachers = str_dup ( argument );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}

		do_sset ( ch, "" );

		return;
	}

	if ( ( victim = get_char_world ( ch, arg1 ) ) == NULL )
	{
		if ( ( sn = skill_lookup ( arg1 ) ) >= 0 )
		{
			sprintf ( arg1, "%d %s %s", sn, arg2, argument );
			do_sset ( ch, arg1 );
		}
		else
			send_to_char ( "They aren't here.\r\n", ch );

		return;
	}

	if ( IS_NPC ( victim ) )
	{
		send_to_char ( "Not on NPC's.\r\n", ch );
		return;
	}

	fAll = !str_cmp ( arg2, "all" );

	sn = 0;

	if ( !fAll && ( sn = skill_lookup ( arg2 ) ) < 0 )
	{
		send_to_char ( "No such skill or spell.\r\n", ch );
		return;
	}

	/*
	 * Snarf the value.
	 */
	if ( !is_number ( argument ) )
	{
		send_to_char ( "Value must be numeric.\r\n", ch );
		return;
	}

	value = atoi ( argument );

	if ( value < 0 || value > 100 )
	{
		send_to_char ( "Value range is 0 to 100.\r\n", ch );
		return;
	}

	if ( fAll )
	{
		for ( sn = 0; sn < top_sn; sn++ )
		{
			/*
			 * Fix by Narn to prevent ssetting skills the player shouldn't have.
			 */
			if ( skill_table[sn]->name && ( victim->level >= skill_table[sn]->skill_level[victim->Class] || value == 0 ) )
			{
				if ( value == 100 && !IS_IMMORTAL ( victim ) )
					victim->pcdata->learned[sn] = GET_ADEPT ( victim, sn );
				else
					victim->pcdata->learned[sn] = value;
			}
		}
	}
	else
		victim->pcdata->learned[sn] = value;

	return;
}

void learn_from_success ( CHAR_DATA * ch, int sn )
{
	int adept, gain, sklvl, percent, schance, lchance, learn;
	/*
	 * the following added by tommi Jan 6006 inspired by Tarl of AFKmud
	 * the point of the following code is to make skills learn at a much
	 * slower rate than normal
	 */

	if ( skill_table[sn]->type == SKILL_WEAPON )
	{
		lchance = number_range ( 1, 10 );

		if ( lchance != 5 )
		{
			return;
		}
	}

	if ( skill_table[sn]->type == SKILL_SKILL )
	{
		lchance = number_range ( 1, 5 );

		if ( lchance != 3 )
		{
			return;
		}
	}

	if ( skill_table[sn]->type == SKILL_SPELL )
	{
		lchance = number_range ( 1, 3 );

		if ( lchance != 2 )
		{
			return;
		}
	}

	if ( IS_NPC ( ch ) || ch->pcdata->learned[sn] <= 0 )
		return;

	adept = GET_ADEPT ( ch, sn );

	sklvl = skill_table[sn]->skill_level[ch->Class];

	if ( sklvl == 0 )
		sklvl = ch->level;

	if ( ch->pcdata->learned[sn] < adept )
	{
		schance = ch->pcdata->learned[sn] + ( 5 * skill_table[sn]->difficulty );
		percent = number_percent( );

		if ( percent >= schance )
			learn = 2;
		else
			if ( schance - percent > 25 )
				return;
			else
				learn = 1;

		ch->pcdata->learned[sn] = UMIN ( adept, ch->pcdata->learned[sn] + learn );

		if ( ch->pcdata->learned[sn] == adept ) /* fully learned! */
		{
			gain = 1500 * sklvl;
			set_char_color ( AT_WHITE, ch );
			ch_printf ( ch, "You are now an adept of %s!  You gain %d bonus experience!\r\n", skill_table[sn]->name, gain );
		}
		else
		{
			gain = 50 * sklvl;

			if ( !ch->fighting )
			{
				set_char_color ( AT_WHITE, ch );
				ch_printf ( ch, "You gain %d experience points from your success!\r\n", gain );
			}
		}

		gain_exp ( ch, gain );
	}
}

void learn_from_failure ( CHAR_DATA * ch, int sn )
{
	int adept, schance, lchance;
	/*
	 * the following added by tommi Jan 6006 inspited by Tarl of AFKmud
	 * the point of the following code is to make skills learn at a much
	 * slower rate than normal
	 */

	if ( skill_table[sn]->type == SKILL_WEAPON )
	{
		lchance = number_range ( 1, 7 );

		if ( lchance != 4 )
		{
			return;
		}
	}

	if ( skill_table[sn]->type == SKILL_SKILL )
	{
		lchance = number_range ( 1, 3 );

		if ( lchance != 2 )
		{
			return;
		}
	}

	if ( skill_table[sn]->type == SKILL_SPELL )
	{
		lchance = number_range ( 1, 2 );

		if ( lchance != 2 )
		{
			return;
		}
	}

	if ( IS_NPC ( ch ) || ch->pcdata->learned[sn] <= 0 )
		return;

	schance = ch->pcdata->learned[sn] + ( 5 * skill_table[sn]->difficulty );

	if ( schance - number_percent( ) > 25 )
		return;

	adept = GET_ADEPT ( ch, sn );

	if ( ch->pcdata->learned[sn] < ( adept - 1 ) )
	{
		ch->pcdata->learned[sn] += 1;
		send_to_char ( "You learn from your mistakes\r\n", ch );
	}
}


/*
 * Disarm a creature.
 * Caller must check for successful attack.
 * Check for loyalty flag (weapon disarms to inventory) for pkillers -Blodkai
 */
void disarm ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	OBJ_DATA *obj, *tmpobj;

	if ( ( obj = get_eq_char ( victim, WEAR_WIELD ) ) == NULL )
		return;

	if ( ( tmpobj = get_eq_char ( victim, WEAR_DUAL_WIELD ) ) != NULL && number_bits ( 1 ) == 0 )
		obj = tmpobj;

	if ( get_eq_char ( ch, WEAR_WIELD ) == NULL && number_bits ( 1 ) == 0 )
	{
		learn_from_failure ( ch, gsn_disarm );
		return;
	}

	if ( IS_NPC ( ch ) && !can_see_obj ( ch, obj ) && number_bits ( 1 ) == 0 )
	{
		learn_from_failure ( ch, gsn_disarm );
		return;
	}

	if ( check_grip ( ch, victim ) )
	{
		learn_from_failure ( ch, gsn_disarm );
		return;
	}

	act ( AT_RED, "$n DISARMS you, sending your weapon flying accross the room!", ch, NULL, victim, TO_VICT );

	act ( AT_SKILL, "You disarm $N sending their weapon flying accross the room", ch, NULL, victim, TO_CHAR );
	act ( AT_SKILL, "$n disarms $N!", ch, NULL, victim, TO_NOTVICT );
	learn_from_success ( ch, gsn_disarm );

	if ( obj == get_eq_char ( victim, WEAR_WIELD ) && ( tmpobj = get_eq_char ( victim, WEAR_DUAL_WIELD ) ) != NULL )
		tmpobj->wear_loc = WEAR_WIELD;

	obj_from_char ( obj );

	obj_to_char ( obj, victim );

	return;
}

void do_disarm ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	int percent;

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You can't concentrate enough for that.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_disarm]->skill_level[ch->Class] )
	{
		send_to_char ( "You don't know how to disarm opponents.\r\n", ch );
		return;
	}

	if ( get_eq_char ( ch, WEAR_WIELD ) == NULL )
	{
		send_to_char ( "You must wield a weapon to disarm.\r\n", ch );
		return;
	}

	if ( ( victim = who_fighting ( ch ) ) == NULL )
	{
		send_to_char ( "You aren't fighting anyone.\r\n", ch );
		return;
	}

	if ( ( obj = get_eq_char ( victim, WEAR_WIELD ) ) == NULL )
	{
		send_to_char ( "Your opponent is not wielding a weapon.\r\n", ch );
		return;
	}

	WAIT_STATE ( ch, skill_table[gsn_disarm]->beats );

	percent = number_percent( ) + victim->level - ch->level - ( get_curr_lck ( ch ) - 15 ) + ( get_curr_lck ( victim ) - 15 );

	if ( !can_see_obj ( ch, obj ) )
		percent += 10;

	if ( can_use_skill ( ch, ( percent * 3 / 2 ), gsn_disarm ) )
		disarm ( ch, victim );
	else
	{
		send_to_char ( "You failed.\r\n", ch );
		learn_from_failure ( ch, gsn_disarm );
	}

	return;
}

/*
 * Trip a creature.
 * Caller must check for successful attack.
 */
void trip ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	if ( IS_AFFECTED ( victim, AFF_FLYING ) || IS_AFFECTED ( victim, AFF_FLOATING ) )
		return;

	if ( victim->mount )
	{
		if ( IS_AFFECTED ( victim->mount, AFF_FLYING ) || IS_AFFECTED ( victim->mount, AFF_FLOATING ) )
			return;
		act ( AT_SKILL, "$n trips your mount and you fall off!", ch, NULL, victim, TO_VICT );
		act ( AT_SKILL, "You trip $N's mount and $N falls off!", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n trips $N's mount and $N falls off!", ch, NULL, victim, TO_NOTVICT );
		xREMOVE_BIT ( victim->mount->act, ACT_MOUNTED );
		victim->mount = NULL;
		WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );
		WAIT_STATE ( victim, 2 * PULSE_VIOLENCE );
		victim->position = POS_RESTING;
		return;
	}

	if ( victim->wait == 0 )
	{
		act ( AT_SKILL, "$n trips you and you go down!", ch, NULL, victim, TO_VICT );
		act ( AT_SKILL, "You trip $N and $N goes down!", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n trips $N and $N goes down!", ch, NULL, victim, TO_NOTVICT );
		WAIT_STATE ( ch, 2 * PULSE_VIOLENCE );
		WAIT_STATE ( victim, 2 * PULSE_VIOLENCE );
		victim->position = POS_RESTING;
	}

	return;
}

void do_recall ( CHAR_DATA * ch, char *argument )
{
	ROOM_INDEX_DATA *location;
	location = NULL;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_NO_RECALL ) || IS_AFFECTED ( ch, AFF_CURSE ) || ch->position == POS_FIGHTING || ch->in_room == location )
	{
		send_to_char ( "For some strange reason... the God's forbid you from recalling.\r\n", ch );
		return;
	}

	if ( !str_cmp ( argument, "clanhall" ) )
	{
		if ( !IS_NPC ( ch ) && ch->pcdata->clan )
		{
			location = get_room_index ( ch->pcdata->clan->recall );

			if ( location == NULL )
				return;
			else
				char_from_room ( ch );

			send_mip_sound ( ch, "fury_action_recall.wav" );
			char_to_room ( ch, location );
			do_look ( ch, "auto" );
			return;
		}
		else
		{
			send_to_char ( "You do not belong to a clan.\r\n", ch );
			return;
		}
	}

	if ( !str_cmp ( ch->in_room->area->filename, "school.are" ) )
	{
		send_to_char ( "Recalling is FORBIDEN in this location.\r\n", ch );
		return;
	}
	else
	{
		location = get_room_index ( ROOM_VNUM_RECALL );

		if ( IS_PLR_FLAG ( ch, PLR_ONMAP ) || IS_ACT_FLAG ( ch, ACT_ONMAP ) )
		{
			leave_map ( ch, NULL, location );
			send_mip_sound ( ch, "fury_action_recall.wav" );
			return;
		}
	}

	act ( AT_ACTION, "$n disappears in a swirl of smoke.", ch, NULL, NULL, TO_ROOM );
	char_from_room ( ch );
	char_to_room ( ch, location );

	if ( ch->mount )
	{
		char_from_room ( ch->mount );
		char_to_room ( ch->mount, location );
	}
	send_mip_sound ( ch, "fury_action_recall.wav" );
	act ( AT_ACTION, "$n appears in the room.", ch, NULL, NULL, TO_ROOM );
	do_look ( ch, "auto" );

	if ( ch->on )
	{
		ch->on = NULL;
		ch->position = POS_STANDING;
	}

	if ( ch->position != POS_STANDING )
	{
		ch->position = POS_STANDING;
	}
	return;
}

void do_mount ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_mount]->skill_level[ch->Class] )
	{
		send_to_char ( "I don't think that would be a good idea...\r\n", ch );
		return;
	}

	if ( ch->mount )
	{
		send_to_char ( "You're already mounted!\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, argument ) ) == NULL )
	{
		send_to_char ( "You can't find that here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) || !xIS_SET ( victim->act, ACT_MOUNTABLE ) )
	{
		send_to_char ( "You can't mount that!\r\n", ch );
		return;
	}

	if ( xIS_SET ( victim->act, ACT_MOUNTED ) )
	{
		send_to_char ( "That mount already has a rider.\r\n", ch );
		return;
	}

	if ( victim->position < POS_STANDING )
	{
		send_to_char ( "Your mount must be standing.\r\n", ch );
		return;
	}

	if ( victim->position == POS_FIGHTING || victim->fighting )
	{
		send_to_char ( "Your mount is moving around too much.\r\n", ch );
		return;
	}

	WAIT_STATE ( ch, skill_table[gsn_mount]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_mount ) )
	{
		xSET_BIT ( victim->act, ACT_MOUNTED );
		ch->mount = victim;
		act ( AT_SKILL, "You mount $N.", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n skillfully mounts $N.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_SKILL, "$n mounts you.", ch, NULL, victim, TO_VICT );
		learn_from_success ( ch, gsn_mount );
		ch->position = POS_MOUNTED;
	}
	else
	{
		act ( AT_SKILL, "You unsuccessfully try to mount $N.", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n unsuccessfully attempts to mount $N.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_SKILL, "$n tries to mount you.", ch, NULL, victim, TO_VICT );
		learn_from_failure ( ch, gsn_mount );
	}

	return;
}

void do_dismount ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;

	if ( ( victim = ch->mount ) == NULL )
	{
		send_to_char ( "You're not mounted.\r\n", ch );
		return;
	}

	WAIT_STATE ( ch, skill_table[gsn_mount]->beats );

	if ( can_use_skill ( ch, number_percent( ), gsn_mount ) )
	{
		act ( AT_SKILL, "You dismount $N.", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n skillfully dismounts $N.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_SKILL, "$n dismounts you.  Whew!", ch, NULL, victim, TO_VICT );
		xREMOVE_BIT ( victim->act, ACT_MOUNTED );
		ch->mount = NULL;
		ch->position = POS_STANDING;
		learn_from_success ( ch, gsn_mount );
	}
	else
	{
		act ( AT_SKILL, "You fall off while dismounting $N.  Ouch!", ch, NULL, victim, TO_CHAR );
		act ( AT_SKILL, "$n falls off of $N while dismounting.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_SKILL, "$n falls off your back.", ch, NULL, victim, TO_VICT );
		learn_from_failure ( ch, gsn_mount );
		xREMOVE_BIT ( victim->act, ACT_MOUNTED );
		ch->mount = NULL;
		ch->position = POS_SITTING;
		global_retcode = damage ( ch, ch, 1, TYPE_UNDEFINED );
	}

	return;
}


/* Re-worked by Sadiq to allow PCs to scribe up to 3 spells on a single *
 * scroll. Second and third spells progressive increase the chance of   *
 * destroying the scroll.  --Sadiq                                      */
void do_scribe ( CHAR_DATA * ch, char *argument )
{
	OBJ_DATA *scroll;
	int sn;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	int mana;

	if ( IS_NPC ( ch ) )
		return;

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_scribe]->skill_level[ch->Class] )
	{
		send_to_char ( "A skill such as this requires more magical ability than that of your Class.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL || !str_cmp ( argument, "" ) )
	{
		send_to_char ( "Scribe what?\r\n", ch );
		return;
	}

	if ( ( sn = find_spell ( ch, argument, TRUE ) ) < 0 )
	{
		send_to_char ( "You have not learned that spell.\r\n", ch );
		return;
	}

	if ( skill_table[sn]->spell_fun == spell_null )
	{
		send_to_char ( "That's not a spell!\r\n", ch );
		return;
	}

	if ( SPELL_FLAG ( skill_table[sn], SF_NOSCRIBE ) )
	{
		send_to_char ( "You cannot scribe that spell.\r\n", ch );
		return;
	}

	mana = IS_NPC ( ch ) ? 0 : UMAX ( skill_table[sn]->min_mana, 100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );

	mana *= 5;

	if ( !IS_NPC ( ch ) && ch->mana < mana )
	{
		send_to_char ( "You don't have enough mana.\r\n", ch );
		return;
	}

	if ( ( scroll = get_eq_char ( ch, WEAR_HOLD ) ) == NULL )
	{
		send_to_char ( "You must be holding a blank scroll to scribe it.\r\n", ch );
		return;
	}

	if ( scroll->pIndexData->vnum != OBJ_VNUM_SCROLL_SCRIBING )
	{
		send_to_char ( "You must be holding a blank scroll to scribe it.\r\n", ch );
		return;
	}

	if ( ( scroll->value[1] != -1 ) && ( scroll->value[2] != -1 ) && ( scroll->value[3] != -1 ) && ( scroll->pIndexData->vnum == OBJ_VNUM_SCROLL_SCRIBING ) )
	{
		send_to_char ( "That scroll has already contains as much magic as it can hold.\r\n", ch );
		return;
	}

	if ( !process_spell_components ( ch, sn ) )
	{
		learn_from_failure ( ch, gsn_scribe );
		ch->mana -= ( mana / 2 );
		return;
	}

	if ( !IS_NPC ( ch ) && number_percent( ) > ch->pcdata->learned[gsn_scribe] )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char ( "The magic surges outof control and destroys the scroll!.\r\n", ch );
		learn_from_failure ( ch, gsn_scribe );
		ch->mana -= ( mana / 2 );
		extract_obj ( scroll );
		return;
	}

	if ( scroll->value[1] == -1 )
	{
		scroll->value[1] = sn;
		scroll->value[0] = ch->level;
		sprintf ( buf1, "magically scribed scroll" );
		STRFREE ( scroll->short_descr );
		scroll->short_descr = STRALLOC ( aoran ( buf1 ) );
		sprintf ( buf2, "A magically scribed scroll lies in the dust." );
		STRFREE ( scroll->description );
		scroll->description = STRALLOC ( buf2 );
		sprintf ( buf3, "scroll scribing %s", skill_table[sn]->name );
		STRFREE ( scroll->name );
		scroll->name = STRALLOC ( buf3 );
		act ( AT_MAGIC, "$n magiclly scribes a scroll.", ch, NULL, NULL, TO_ROOM );
		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the scroll with %s.\r\n", skill_table[sn]->name );
		/*
		 * act( AT_MAGIC, "$n magically scribes a scroll.",   ch, NULL, NULL, TO_ROOM );
		 */
		/*
		 * act( AT_MAGIC, "You magically scribe $p.",   ch, NULL, NULL, TO_CHAR );
		 */
		learn_from_success ( ch, gsn_scribe );
		ch->mana -= mana;
		return;
	}

	if ( scroll->value[2] == -1 )
	{
		if ( number_percent( ) > 80 )
		{
			set_char_color ( AT_MAGIC, ch );
			send_to_char ( "The magic surges out of control and destroys the scroll!.\r\n", ch );
			learn_from_failure ( ch, gsn_scribe );
			ch->mana -= ( mana / 2 );
			extract_obj ( scroll );
			return;
		}

		if ( scroll->value[0] > ch->level )
		{
			scroll->value[0] = ch->level;
		}

		scroll->value[2] = sn;

		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the scroll with %s.\r\n", skill_table[sn]->name );
		learn_from_success ( ch, gsn_scribe );
		ch->mana -= mana;
		return;
	}

	if ( scroll->value[3] == -1 )
	{
		if ( number_percent( ) > 60 )
		{
			set_char_color ( AT_MAGIC, ch );
			send_to_char ( "The magic surges outof control and destroys the scroll!.\r\n", ch );
			learn_from_failure ( ch, gsn_scribe );
			ch->mana -= ( mana / 2 );
			extract_obj ( scroll );
			return;
		}

		if ( scroll->value[0] > ch->level )
		{
			scroll->value[0] = ch->level;
		}

		scroll->value[3] = sn;

		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the scroll with %s.\r\n", skill_table[sn]->name );
		learn_from_success ( ch, gsn_scribe );
		ch->mana -= mana;
		return;
	}
}

/*
brewing 3 spells to one vial, based on do_scribe 3 spells by sadiq
*/
void do_brew ( CHAR_DATA * ch, char *argument )
{
	OBJ_DATA *potion;
	int sn;
	char buf1[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	char buf3[MAX_STRING_LENGTH];
	int mana;

	if ( IS_NPC ( ch ) )
		return;

	if ( !IS_NPC ( ch ) && ch->level < skill_table[gsn_brew]->skill_level[ch->Class] )
	{
		send_to_char ( "A skill such as this requires more magical ability than that of your Class.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL || !str_cmp ( argument, "" ) )
	{
		send_to_char ( "brew what?\r\n", ch );
		return;
	}

	if ( ( sn = find_spell ( ch, argument, TRUE ) ) < 0 )
	{
		send_to_char ( "You have not learned that spell.\r\n", ch );
		return;
	}

	if ( skill_table[sn]->spell_fun == spell_null )
	{
		send_to_char ( "That's not a spell!\r\n", ch );
		return;
	}

	if ( SPELL_FLAG ( skill_table[sn], SF_NOBREW ) )
	{
		send_to_char ( "You cannot brew that spell.\r\n", ch );
		return;
	}

	mana = IS_NPC ( ch ) ? 0 : UMAX ( skill_table[sn]->min_mana, 100 / ( 2 + ch->level - skill_table[sn]->skill_level[ch->Class] ) );

	mana *= 5;

	if ( !IS_NPC ( ch ) && ch->mana < mana )
	{
		send_to_char ( "You don't have enough mana.\r\n", ch );
		return;
	}

	if ( ( potion = get_eq_char ( ch, WEAR_HOLD ) ) == NULL )
	{
		send_to_char ( "You must be holding a blank potion to brew it.\r\n", ch );
		return;
	}

	if ( potion->pIndexData->vnum != OBJ_VNUM_FLASK_BREWING )
	{
		send_to_char ( "You must be holding a blank potion to brew it.\r\n", ch );
		return;
	}

	if ( ( potion->value[1] != -1 ) && ( potion->value[2] != -1 ) && ( potion->value[3] != -1 ) && ( potion->pIndexData->vnum == OBJ_VNUM_FLASK_BREWING ) )
	{
		send_to_char ( "That potion has already contains as much magic as it can hold.\r\n", ch );
		return;
	}

	if ( !process_spell_components ( ch, sn ) )
	{
		learn_from_failure ( ch, gsn_brew );
		ch->mana -= ( mana / 2 );
		return;
	}

	if ( !IS_NPC ( ch ) && number_percent( ) > ch->pcdata->learned[gsn_brew] )
	{
		set_char_color ( AT_MAGIC, ch );
		send_to_char ( "The magic surges outof control and destroys the potion!.\r\n", ch );
		learn_from_failure ( ch, gsn_brew );
		ch->mana -= ( mana / 2 );
		extract_obj ( potion );
		return;
	}

	if ( potion->value[1] == -1 )
	{
		potion->value[1] = sn;
		potion->value[0] = ch->level;
		sprintf ( buf1, "A magically brewed potion %s", skill_table[sn]->name );
		STRFREE ( potion->short_descr );
		potion->short_descr = STRALLOC ( aoran ( buf1 ) );
		sprintf ( buf2, "A magically brewed potion lies in the dust." );
		STRFREE ( potion->description );
		potion->description = STRALLOC ( buf2 );
		sprintf ( buf3, "potion scribing %s", skill_table[sn]->name );
		STRFREE ( potion->name );
		potion->name = STRALLOC ( buf3 );
		act ( AT_MAGIC, "$n magiclly brews a potion.", ch, NULL, NULL, TO_ROOM );
		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the potion with %s.\r\n", skill_table[sn]->name );
		learn_from_success ( ch, gsn_brew );
		ch->mana -= mana;
		return;
	}

	if ( potion->value[2] == -1 )
	{
		if ( number_percent( ) > 80 )
		{
			set_char_color ( AT_MAGIC, ch );
			send_to_char ( "The magic surges out of control and destroys the potion!.\r\n", ch );
			learn_from_failure ( ch, gsn_brew );
			ch->mana -= ( mana / 2 );
			extract_obj ( potion );
			return;
		}

		if ( potion->value[0] > ch->level )
		{
			potion->value[0] = ch->level;
		}

		potion->value[2] = sn;

		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the potion with %s.\r\n", skill_table[sn]->name );
		learn_from_success ( ch, gsn_brew );
		ch->mana -= mana;
		return;
	}

	if ( potion->value[3] == -1 )
	{
		if ( number_percent( ) < 20 )
		{
			set_char_color ( AT_MAGIC, ch );
			send_to_char ( "The magic surges outof control and destroys the potion!.\r\n", ch );
			learn_from_failure ( ch, gsn_brew );
			ch->mana -= ( mana / 2 );
			extract_obj ( potion );
			return;
		}

		if ( potion->value[0] > ch->level )
		{
			potion->value[0] = ch->level;
		}

		potion->value[3] = sn;

		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "You imbue the potion with %s.\r\n", skill_table[sn]->name );
		learn_from_success ( ch, gsn_brew );
		ch->mana -= mana;
		return;
	}
}



bool check_illegal_psteal ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	if ( !IS_NPC ( victim ) && !IS_NPC ( ch ) )
	{
		if ( ( !IS_SET ( victim->pcdata->flags, PCFLAG_DEADLY )
		        || ch->level - victim->level > 10
		        || !IS_SET ( ch->pcdata->flags, PCFLAG_DEADLY ) ) && ( ch->in_room->vnum < 29
		                || ch->in_room->vnum > 43 ) && ch != victim )
		{
			return TRUE;
		}
	}

	return FALSE;
}

static char *dir_desc[] =
{
	"to the north",
	"to the east",
	"to the south",
	"to the west",
	"upwards",
	"downwards",
	"to the northeast",
	"to the northwest",
	"to the southeast",
	"to the southwest",
	"through the portal"
};
static char *rng_desc[] =
{
	"right here",
	"immediately",
	"nearby",
	"a ways",
	"a good ways",
	"far",
	"far off",
	"very far",
	"very far off",
	"in the distance"
};
static void scanroom ( CHAR_DATA * ch, ROOM_INDEX_DATA * room, int dir, int maxdist, int dist )
{
	CHAR_DATA *tch;
	EXIT_DATA *ex;

	for ( tch = room->first_person; tch; tch = tch->next_in_room )
	{
		if ( can_see ( ch, tch, FALSE ) && !is_ignoring ( tch, ch ) )
			ch_printf ( ch, "%-30s : %s %s\r\n", IS_NPC ( tch ) ? tch->short_descr : tch->name, rng_desc[dist], dist == 0 ? "" : dir_desc[dir] );
	}

	for ( ex = room->first_exit; ex; ex = ex->next )
		if ( ex->vdir == dir )
			break;

	if ( !ex || ex->vdir != dir || ex->vdir == DIR_SOMEWHERE || maxdist - 1 == 0
	        || IS_EXIT_FLAG ( ex, EX_CLOSED ) || IS_EXIT_FLAG ( ex, EX_DIG ) || IS_EXIT_FLAG ( ex, EX_OVERLAND ) )
		return;

	scanroom ( ch, ex->to_room, dir, maxdist - 1, dist + 1 );
}

void map_scan ( CHAR_DATA * ch );
/* Scan no longer accepts a direction argument */
void do_scan ( CHAR_DATA * ch, char *argument )
{
	int maxdist = 1;
	EXIT_DATA *ex;
	maxdist = ch->level / 10;
	maxdist = URANGE ( 1, maxdist, 9 );

	if ( IS_PLR_FLAG ( ch, PLR_ONMAP ) || IS_ACT_FLAG ( ch, ACT_ONMAP ) )
	{
		map_scan ( ch );
		return;
	}

	scanroom ( ch, ch->in_room, -1, 1, 0 );

	for ( ex = ch->in_room->first_exit; ex; ex = ex->next )
	{
		if ( IS_EXIT_FLAG ( ex, EX_DIG ) || IS_EXIT_FLAG ( ex, EX_CLOSED ) || IS_EXIT_FLAG ( ex, EX_OVERLAND ) )
			continue;

		if ( ex->vdir == DIR_SOMEWHERE && !IS_IMMORTAL ( ch ) )
			continue;

		scanroom ( ch, ex->to_room, ex->vdir, maxdist, 1 );

		learn_from_success ( ch, gsn_scan );
	}
}

/*------------------------------------------------------------
 *  Fighting Styles - haus
 */
void do_style ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	/*
	 * char buf[MAX_INPUT_LENGTH];
	 * int percent;
	 */

	if ( IS_NPC ( ch ) )
		return;

	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		ch_printf_color ( ch, "&wAdopt which fighting style?  (current:  %s&w)\r\n",
		                  ch->style == STYLE_DRAGON ? "dragon" :
		                  ch->style == STYLE_NAGA ? "naga" :
		                  ch->style == STYLE_SCORPION ? "scorpion" :
		                  ch->style == STYLE_TIGER ? "tiger" :
		                  ch->style == STYLE_UNICORN ? "unicorn" :
		                  ch->style == STYLE_PHOENIX ? "phoenix" :
		                  ch->style == STYLE_CRANE ? "crane" :
		                  ch->style == STYLE_BADGER ? "badger" : "standard" );
		return;
	}

	if ( !str_prefix ( arg, "dragon" ) )
	{
		if ( ch->level < skill_table[gsn_style_dragon]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with dragon style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_dragon]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_dragon ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_BERSERK;
				learn_from_success ( ch, gsn_style_dragon );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into a beserk stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_DRAGON;

			send_to_char ( "You adopt an beserk fighting style, dragon style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt a berserk fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_dragon );
			return;
		}
	}
	
	else if ( !str_prefix ( arg, "naga" ) )
	{
		if ( ch->level < skill_table[gsn_style_naga]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_naga]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_naga ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_EVASIVE;
				learn_from_success ( ch, gsn_style_naga );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into a naga stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_NAGA;

			send_to_char ( "You adopt a berserk fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt a beserk fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_naga );
			return;
		}
	}

	else if ( !str_prefix ( arg, "scorpion" ) )
	{
		if ( ch->level < skill_table[gsn_style_scorpion]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with scorpion style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_scorpion]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_scorpion ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_AGGRESSIVE;
				learn_from_success ( ch, gsn_style_scorpion );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into an aggressive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_SCORPION;

			send_to_char ( "You adopt an aggressive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt an aggressive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_scorpion );
			return;
		}
	}

	else if ( !str_prefix ( arg, "tiger" ) )
	{
		if ( ch->level < skill_table[gsn_style_tiger]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with tiger style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_tiger]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_tiger ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_EVASIVE;
				learn_from_success ( ch, gsn_style_tiger );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into an aggressive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_TIGER;

			send_to_char ( "You adopt an aggressive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt an aggressive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_tiger );
			return;
		}
	}

	else if ( !str_prefix ( arg, "unicorn" ) )
	{
		if ( ch->level < skill_table[gsn_style_unicorn]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with unicorn style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_unicorn]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_unicorn ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_DEFENSIVE;
				learn_from_success ( ch, gsn_style_unicorn );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into a defensive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_UNICORN;

			send_to_char ( "You adopt an defensive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt a defensive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_unicorn );
			return;
		}
	}

	else if ( !str_prefix ( arg, "phoenix" ) )
	{
		if ( ch->level < skill_table[gsn_style_phoenix]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with phoenix style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_phoenix]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_phoenix ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_DEFENSIVE;
				learn_from_success ( ch, gsn_style_phoenix );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into a defensive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_PHOENIX;

			send_to_char ( "You adopt a defensive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt a defensive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_phoenix );
			return;
		}
	}

	else if ( !str_prefix ( arg, "crane" ) )
	{
		if ( ch->level < skill_table[gsn_style_crane]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with crane style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_crane]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_crane ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_EVASIVE;
				learn_from_success ( ch, gsn_style_crane );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into an evasive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_CRANE;

			send_to_char ( "You adopt an evasive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt an evasive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_crane );
			return;
		}
	}

	else if ( !str_prefix ( arg, "badger" ) )
	{
		if ( ch->level < skill_table[gsn_style_badger]->skill_level[ch->Class] )
		{
			send_to_char ( "You have not yet learned enough to fight with badger style.\r\n", ch );
			return;
		}

		WAIT_STATE ( ch, skill_table[gsn_style_badger]->beats );

		if ( number_percent( ) < LEARNED ( ch, gsn_style_badger ) )
		{
			/*
			 * success
			 */
			if ( ch->fighting )
			{
				ch->position = POS_EVASIVE;
				learn_from_success ( ch, gsn_style_badger );

				if ( IS_PKILL ( ch ) )
					act ( AT_ACTION, "$n falls back into an evasive stance.", ch, NULL, NULL, TO_ROOM );
			}

			ch->style = STYLE_BADGER;

			send_to_char ( "You adopt an evasive fighting style.\r\n", ch );
			return;
		}
		else
		{
			send_to_char ( "You nearly trip in a lame attempt to adopt an evasive fighting style.\r\n", ch );
			learn_from_failure ( ch, gsn_style_badger );
			return;
		}
	}

	send_to_char ( "Adopt which fighting style?\r\n", ch );

	return;
}

bool can_use_skill ( CHAR_DATA * ch, int percent, int gsn )
{
	bool check = FALSE;

	if ( IS_NPC ( ch ) && percent < 85 )
		check = TRUE;
	else
		if ( !IS_NPC ( ch ) && percent < LEARNED ( ch, gsn ) )
			check = TRUE;

	return check;
}


