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
 * 		Commands for personal player settings/statictics	    *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
/*
 *  Locals
 */
char *tiny_affect_loc_name ( int location );
int strlen_color ( char *argument );
void do_gold ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_GOLD, ch );
	ch_printf ( ch, "You have %s gold pieces.\r\n", num_punct ( ch->gold ) );
	return;
}

void do_score ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	AFFECT_DATA *paf;
	if ( IS_NPC ( ch ) )
	{
		return;
	}
	sprintf ( buf, "%s", num_punct ( exp_level ( ch, ch->level + 1 ) - ch->exp ) );
	set_pager_color ( AT_SCORE, ch );
	if ( get_trust ( ch ) != ch->level )
		pager_printf ( ch, "You are trusted at level %d.&D\r\n", get_trust ( ch ) );
	pager_printf_color ( ch, "&D--[&c%-10s %-25s&D]--------------------------------------&D\r\n", ch->name, ch->pcdata->title );
	pager_printf_color ( ch, "&D Level: &W%-4d        &DClan : &w%-10.10s   &DItems:  &w%d &w(max %d)     &D\r\n", ch->level, capitalize ( get_race ( ch ) ), ch->carry_number,
	                     can_carry_n ( ch ) );
	pager_printf_color ( ch, "&D Years: &W%-4d        &DClass: &w%-10.10s   &DWeight: &w%d &w(max %d)     &D\r\n", get_age ( ch ), capitalize ( get_Class ( ch ) ),
	                     ch->carry_weight, can_carry_w ( ch ) );
	pager_printf_color ( ch, "&D STR  : &W%2.2d&D(&w%2.2d&D)      HitRoll: &c%-4d &D      Armor: &w%-d    &D\r\n", get_curr_str ( ch ), ch->perm_str, GET_HITROLL ( ch ),
	                     GET_AC ( ch ) );
	pager_printf_color ( ch, "&D INT  : &W%2.2d&D(&w%2.2d&D)      DamRoll: &c%-4d &D      Align: &w%-4d   &D\r\n", get_curr_int ( ch ), ch->perm_int, GET_DAMROLL ( ch ),
	                     ch->perm_dex, ch->alignment );
	pager_printf_color ( ch, "&D WIS  : &W%2.2d&D(&w%2.2d&D)                                              &D\r\n", get_curr_wis ( ch ), ch->perm_wis );
	pager_printf_color ( ch, "&D DEX  : &W%2.2d&D(&w%2.2d&D)      Quest: &W%d points.&D                   &D\r\n", get_curr_dex ( ch ), ch->perm_dex, ch->pcdata->quest_curr );
	pager_printf_color ( ch, "&D CON  : &W%2.2d&D(&w%2.2d&D)      PRACT: &W%3d                            &D\r\n", get_curr_con ( ch ), ch->perm_con, ch->practice );
	pager_printf_color ( ch, "&D LCK  : &W%2.2d&D(&w%2.2d&D)      EXP  : &W%-12s&D TNL : &W%-12s&D        &D\r\n", get_curr_lck ( ch ), ch->perm_lck, num_punct ( ch->exp ), buf );
	pager_printf_color ( ch, "&D CHA  : &W%2.2d&D(&w%2.2d&D)      GOLD : &Y%-11s                          &D\r\n", get_curr_cha ( ch ), ch->perm_cha, num_punct ( ch->gold ) );


	send_to_pager ( "&D-[&cHealth Status&D]----------------------------------------------------------------&D\r\n", ch );
	pager_printf_color ( ch, "&D HP  : &G%-5d &Dof &G%5d      &D\r\n", ch->hit, ch->max_hit );
	pager_printf_color ( ch, "&D Mana: &G%-5d &Dof &G%5d      &D\r\n", ch->mana, ch->max_mana );
	pager_printf_color ( ch, "&D Move: &G%-5d &Dof &G%5d      &D\r\n", ch->move, ch->max_move );

	send_to_pager ( "&D-[&cPlay Data&D]--------------------------------------------------------------------&D\r\n", ch );
	pager_printf_color ( ch, "&D Played: &W%d &Dhours&D\r\n", ( get_age ( ch ) - 17 ) * 2 );
	pager_printf_color ( ch, "&D Log In: &w%s&D\r", ctime ( & ( ch->logon ) ) );
	pager_printf_color ( ch, "&D Saved : &w%s&D\r", ch->save_time ? ctime ( & ( ch->save_time ) ) : "no save this session\n" );
	pager_printf_color ( ch, "&D Time  : &w%s&D\r", ctime ( &current_time ) );


	send_to_pager ( "&D-[&cFight Data&D]-------------------------------------------------------------------&D\r\n", ch );
	if ( ch->style == STYLE_DRAGON )
		sprintf ( buf, "Dragon" );
	else if ( ch->style == STYLE_NAGA )
		sprintf ( buf, "Naga" );
	else if ( ch->style == STYLE_SCORPION )
		sprintf ( buf, "Scorpion" );
	else if ( ch->style == STYLE_TIGER )
		sprintf ( buf, "Tiger" );
	else if ( ch->style == STYLE_UNICORN )
		sprintf ( buf, "Unicorn" );
	else if ( ch->style == STYLE_PHOENIX )
		sprintf ( buf, "Phoenix" );
	else if ( ch->style == STYLE_CRANE )
		sprintf ( buf, "Crane" );
	else if ( ch->style == STYLE_BADGER )
		sprintf ( buf, "Badger" );
	else
		sprintf ( buf, "Standard" );

	pager_printf ( ch, " Pkills: &w%-3d&D       Pdeaths: &w%-3d&D       Illegal Pkills: &w%-3d&D\r\n", ch->pcdata->pkills, ch->pcdata->pdeaths, ch->pcdata->illegal_pk );
	pager_printf ( ch, " MKills: &w%-5d &D    Mdeaths: &w%-5d             &D\r\n", ch->pcdata->mkills, ch->pcdata->mdeaths );
	pager_printf ( ch, " Wimpy : &Y%-5d &D    Style  : &w%-10.10s         &D\r\n", ch->wimpy, buf );


	if ( ch->pcdata->clan )
	{
		send_to_pager ( "&D-[&cClan Stats&D]-------------------------------------------------------------------&D\r\n", ch );
		pager_printf ( ch, "%-14.14s  Clan AvPkills : %-5d  Clan NonAvpkills : %-5d&D\r\n",
		               ch->pcdata->clan->name, ch->pcdata->clan->pkills[6],
		               ( ch->pcdata->clan->pkills[1] + ch->pcdata->clan->pkills[2] + ch->pcdata->clan->pkills[3] + ch->pcdata->clan->pkills[4] + ch->pcdata->clan->pkills[5] ) );
		pager_printf ( ch, "                             Clan AvPdeaths: %-5d  Clan NonAvpdeaths: %-5d&D\r\n",
		               ch->pcdata->clan->pdeaths[6],
		               ( ch->pcdata->clan->pdeaths[1] + ch->pcdata->clan->pdeaths[2] +
		                 ch->pcdata->clan->pdeaths[3] + ch->pcdata->clan->pdeaths[4] + ch->pcdata->clan->pdeaths[5] ) );
	}


	if ( ch->pcdata->deity )
	{
		send_to_pager ( "&D-[&cDeity Data&D]-------------------------------------------------------------------&D\r\n", ch );
		if ( ch->pcdata->favor > 2250 )
			sprintf ( buf, "loved" );
		else if ( ch->pcdata->favor > 2000 )
			sprintf ( buf, "cherished" );
		else if ( ch->pcdata->favor > 1750 )
			sprintf ( buf, "honored" );
		else if ( ch->pcdata->favor > 1500 )
			sprintf ( buf, "praised" );
		else if ( ch->pcdata->favor > 1250 )
			sprintf ( buf, "favored" );
		else if ( ch->pcdata->favor > 1000 )
			sprintf ( buf, "respected" );
		else if ( ch->pcdata->favor > 750 )
			sprintf ( buf, "liked" );
		else if ( ch->pcdata->favor > 250 )
			sprintf ( buf, "tolerated" );
		else if ( ch->pcdata->favor > -250 )
			sprintf ( buf, "ignored" );
		else if ( ch->pcdata->favor > -750 )
			sprintf ( buf, "shunned" );
		else if ( ch->pcdata->favor > -1000 )
			sprintf ( buf, "disliked" );
		else if ( ch->pcdata->favor > -1250 )
			sprintf ( buf, "dishonored" );
		else if ( ch->pcdata->favor > -1500 )
			sprintf ( buf, "disowned" );
		else if ( ch->pcdata->favor > -1750 )
			sprintf ( buf, "abandoned" );
		else if ( ch->pcdata->favor > -2000 )
			sprintf ( buf, "despised" );
		else if ( ch->pcdata->favor > -2250 )
			sprintf ( buf, "hated" );
		else
			sprintf ( buf, "damned" );
		pager_printf ( ch, "Deity:  %-20s  Favor: %s&D\r\n", ch->pcdata->deity->name, buf );
	}


	if ( IS_IMMORTAL ( ch ) )
	{
		send_to_pager ( "&D-[&cImmortal Data&D]----------------------------------------------------------------&D\r\n", ch );
		pager_printf ( ch, "&D Wizinvis &D[&W%s&D]  &DWizlevel &D(&w%d&D)&D\r\n", xIS_SET ( ch->act, PLR_WIZINVIS ) ? "X" : " ", ch->pcdata->wizinvis );
		pager_printf ( ch, "&D Bamfin:  &w%s&D\r\n", ( ch->pcdata->bamfin[0] != STRING_NULL ) ? ch->pcdata->bamfin : "appears in a swirling mist." );
		pager_printf ( ch, "&D Bamfout: &w%s&D\r\n", ( ch->pcdata->bamfout[0] != STRING_NULL ) ? ch->pcdata->bamfout : "leaves in a swirling mist." );
		if ( ch->pcdata->area )
		{
			pager_printf ( ch, " Vnums:   Room (%-5.5d - %-5.5d)   Object (%-5.5d - %-5.5d)   Mob (%-5.5d - %-5.5d)&D\r\n",
			               ch->pcdata->area->low_r_vnum, ch->pcdata->area->hi_r_vnum,
			               ch->pcdata->area->low_o_vnum, ch->pcdata->area->hi_o_vnum, ch->pcdata->area->low_m_vnum, ch->pcdata->area->hi_m_vnum );
			pager_printf ( ch, "Area Loaded [%s]&D\r\n", ( IS_SET ( ch->pcdata->area->status, AREA_LOADED ) ) ? "yes" : "no" );
		}
	}


	if ( ch->first_affect )
	{
		int i;
		SKILLTYPE *sktmp;
		i = 0;
		send_to_pager ( "&D-[&cAffect Data&D]------------------------------------------------------------------&D\r\n", ch );
		for ( paf = ch->first_affect; paf; paf = paf->next )
		{
			if ( ( sktmp = get_skilltype ( paf->type ) ) == NULL )
				continue;
			if ( paf->modifier == 0 )
				pager_printf ( ch, "&D[&w%-24.24s&D;&w%5d rds&D]&D    ", sktmp->name, paf->duration );
			else if ( paf->modifier > 999 )
				pager_printf ( ch, "&D[&w%-15.15s&D;&w %7.7s&D;&w%5d rds&D]&D    ", sktmp->name, tiny_affect_loc_name ( paf->location ), paf->duration );
			else
				pager_printf ( ch, "&D[&w%-11.11s&D;&w%+-3.3d %7.7s&D;&w%5d rds&D]&D    ", sktmp->name, paf->modifier, tiny_affect_loc_name ( paf->location ), paf->duration );
			if ( i == 0 )
				i = 1;
			if ( ( ++i % 2 ) == 0 )
				send_to_pager ( "&D\r\n", ch );
		}
	}


	sprintf ( buf, "&D-[&cMagical Tattoo's&D]------------------------------------------------------------&W&D\r\n %s&D\r\n", tattoo_bit_name ( ch->tattoo ) );
	pager_printf ( ch, "&w %s", buf );


	if ( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != STRING_NULL )
	{
		send_to_pager ( "&D-[&c Bestowments &D]----------------------------------------------------------------&D\r\n", ch );
		pager_printf ( ch, "You are bestowed with the command(s): %s.&D\r\n", ch->pcdata->bestowments );
	}
	send_to_char ( "&D--------------------------------------------------------------------------------&D\r\n", ch );
	return;
}

/*
 * Return ascii name of an affect location.
 */
char *tiny_affect_loc_name ( int location )
{
	switch ( location )
	{
		case APPLY_NONE:
			return "NIL";
		case APPLY_STR:
			return " STR  ";
		case APPLY_DEX:
			return " DEX  ";
		case APPLY_INT:
			return " INT  ";
		case APPLY_WIS:
			return " WIS  ";
		case APPLY_CON:
			return " CON  ";
		case APPLY_CHA:
			return " CHA  ";
		case APPLY_LCK:
			return " LCK  ";
		case APPLY_SEX:
			return " SEX  ";
		case APPLY_CLASS:
			return " CLASS";
		case APPLY_LEVEL:
			return " LVL  ";
		case APPLY_AGE:
			return " AGE  ";
		case APPLY_MANA:
			return " MANA ";
		case APPLY_HIT:
			return " HV   ";
		case APPLY_MOVE:
			return " MOVE ";
		case APPLY_GOLD:
			return " GOLD ";
		case APPLY_EXP:
			return " EXP  ";
		case APPLY_AC:
			return " AC   ";
		case APPLY_HITROLL:
			return " HITRL";
		case APPLY_DAMROLL:
			return " DAMRL";
		case APPLY_SAVING_POISON:
			return "SV POI";
		case APPLY_SAVING_MENTAL:
			return "SV MENTAL";
		case APPLY_SAVING_PHYSICAL:
			return "SV PHYSICAL";
		case APPLY_SAVING_WEAPONS:
			return "SV WEAPONS";
		case APPLY_NOT_USED:
			return "SV NOT_USED";
		case APPLY_HEIGHT:
			return "HEIGHT";
		case APPLY_WEIGHT:
			return "WEIGHT";
		case APPLY_AFFECT:
			return "AFF BY";
		case APPLY_RESISTANT:
			return "RESIST";
		case APPLY_IMMUNE:
			return "IMMUNE";
		case APPLY_SUSCEPTIBLE:
			return "SUSCEPT";
		case APPLY_WEAPONSPELL:
			return " WEAPON";
		case APPLY_BACKSTAB:
			return "BACKSTB";
		case APPLY_PICK:
			return " PICK  ";
		case APPLY_TRACK:
			return " TRACK ";
		case APPLY_STEAL:
			return " STEAL ";
		case APPLY_SNEAK:
			return " SNEAK ";
		case APPLY_HIDE:
			return " HIDE  ";
		case APPLY_PALM:
			return " PALM  ";
		case APPLY_DETRAP:
			return " DETRAP";
		case APPLY_DODGE:
			return " DODGE ";
		case APPLY_PEEK:
			return " PEEK  ";
		case APPLY_SCAN:
			return " SCAN  ";
		case APPLY_GOUGE:
			return " GOUGE ";
		case APPLY_SEARCH:
			return " SEARCH";
		case APPLY_MOUNT:
			return " MOUNT ";
		case APPLY_DISARM:
			return " DISARM";
		case APPLY_KICK:
			return " KICK  ";
		case APPLY_PARRY:
			return " PARRY ";
		case APPLY_BASH:
			return " BASH  ";
		case APPLY_STUN:
			return " STUN  ";
		case APPLY_PUNCH:
			return " PUNCH ";
		case APPLY_CLIMB:
			return " CLIMB ";
		case APPLY_GRIP:
			return " GRIP  ";
		case APPLY_SCRIBE:
			return " SCRIBE";
		case APPLY_BREW:
			return " BREW  ";
		case APPLY_WEARSPELL:
			return " WEAR  ";
		case APPLY_REMOVESPELL:
			return " REMOVE";
		case APPLY_STRIPSN:
			return " DISPEL";
		case APPLY_REMOVE:
			return " REMOVE";
		case APPLY_DIG:
			return " DIG   ";
		case APPLY_FULL:
			return " HUNGER";
		case APPLY_THIRST:
			return " THIRST";
		case APPLY_DRUNK:
			return " DRUNK ";
		case APPLY_BLOOD:
			return " BLOOD ";
		case APPLY_COOK:
			return " COOK  ";
		case APPLY_RECURRINGSPELL:
			return " RECURR";
		case APPLY_CONTAGIOUS:
			return "CONTGUS";
		case APPLY_ODOR:
			return " ODOR  ";
		case APPLY_ROOMFLAG:
			return " RMFLG ";
		case APPLY_SECTORTYPE:
			return " SECTOR";
		case APPLY_ROOMLIGHT:
			return " LIGHT ";
		case APPLY_TELEVNUM:
			return " TELEVN";
		case APPLY_TELEDELAY:
			return " TELEDY";
	};
	bug ( "Affect_location_name: unknown location %d.", location );
	return "(?)";
}

char *get_Class ( CHAR_DATA * ch )
{
	if ( IS_NPC ( ch ) && ch->Class < MAX_NPC_CLASS && ch->Class >= 0 )
		return ( npc_Class[ch->Class] );
	else if ( !IS_NPC ( ch ) && ch->Class < MAX_PC_CLASS && ch->Class >= 0 )
		return Class_table[ch->Class]->who_name;
	return ( "Unknown" );
}

char *get_race ( CHAR_DATA * ch )
{
	if ( ch->race < MAX_PC_RACE && ch->race >= 0 )
		return ( race_table[ch->race]->race_name );
	if ( ch->race < MAX_NPC_RACE && ch->race >= 0 )
		return ( npc_race[ch->race] );
	return ( "Unknown" );
}

/*								-Thoric
 * Display your current exp, level, and surrounding level exp requirements
 */
void do_level ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int x, lowlvl, hilvl;
	if ( ch->level == 1 )
		lowlvl = 1;
	else
		lowlvl = UMAX ( 2, ch->level - 5 );
	hilvl = URANGE ( ch->level, ch->level + 5, MAX_LEVEL );
	set_char_color ( AT_LEVEL, ch );
	ch_printf ( ch, "\r\n&DExperience required, levels &w%d&D to&w %d&D:\r\n", lowlvl, hilvl );
	send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
	sprintf ( buf, " &DExp  (&cCurrent Experience               :&w %12s&D)", num_punct ( ch->exp ) );
	sprintf ( buf2, " &DExp  (&cExperience Needed Till Next Level:&w %12s&D)", num_punct ( exp_level ( ch, ch->level + 1 ) - ch->exp ) );
	for ( x = lowlvl; x <= hilvl; x++ )
		ch_printf ( ch, " (&c%2d&D) &w%12s%s&D\r\n", x, num_punct ( exp_level ( ch, x ) ), ( x == ch->level ) ? buf : ( x == ch->level + 1 ) ? buf2 : "&D Exp" );
	send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
}

/* Affects-at-a-glance, Blodkai */
void do_affected ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	SKILLTYPE *skill;
	if ( IS_NPC ( ch ) )
		return;
	set_char_color ( AT_SCORE, ch );
	argument = one_argument ( argument, arg );
	if ( !str_cmp ( arg, "by" ) )
	{
		send_to_char_color ( "\r\n&DImbued with:\r\n", ch );
		ch_printf_color ( ch, "&C%s\r\n", !xIS_EMPTY ( ch->affected_by ) ? affect_bit_name ( &ch->affected_by ) : "nothing" );
		if ( ch->level >= 20 )
		{
			send_to_char ( "\r\n", ch );
			if ( ch->resistant > 0 )
			{
				send_to_char_color ( "&DResistances:  ", ch );
				ch_printf_color ( ch, "&C%s\r\n", flag_string ( ch->resistant, ris_flags ) );
			}
			if ( ch->immune > 0 )
			{
				send_to_char_color ( "&DImmunities:   ", ch );
				ch_printf_color ( ch, "&C%s\r\n", flag_string ( ch->immune, ris_flags ) );
			}
			if ( ch->susceptible > 0 )
			{
				send_to_char_color ( "&DSuscepts:     ", ch );
				ch_printf_color ( ch, "&C%s\r\n", flag_string ( ch->susceptible, ris_flags ) );
			}
		}
		return;
	}
	if ( !ch->first_affect )
	{
		send_to_char_color ( "\r\n&CNo cantrip or skill affects you.\r\n", ch );
	}
	else
	{
		send_to_char ( "\r\n", ch );
		for ( paf = ch->first_affect; paf; paf = paf->next )
			if ( ( skill = get_skilltype ( paf->type ) ) != NULL )
			{
				set_char_color ( AT_BLUE, ch );
				send_to_char ( "Affected:  ", ch );
				set_char_color ( AT_SCORE, ch );
				if ( ch->level >= 20 || IS_PKILL ( ch ) )
				{
					if ( paf->duration < 25 )
						set_char_color ( AT_WHITE, ch );
					if ( paf->duration < 6 )
						set_char_color ( AT_WHITE + AT_BLINK, ch );
					ch_printf ( ch, "(%5d)   ", paf->duration );
				}
				ch_printf ( ch, "%-18s\r\n", skill->name );
			}
	}
	return;
}

void do_inventory ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_RED, ch );
	send_to_char ( "You are carrying:\r\n", ch );
	show_list_to_char ( ch->first_carrying, ch, TRUE, TRUE );
	return;
}

size_t mudstrlcpy ( char *dst, const char *src, size_t siz )
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	/*
	 * Copy as many bytes as will fit
	 */
	if ( n != 0 && --n != 0 )
	{
		do
		{
			if ( ( *d++ = *s++ ) == 0 )
				break;
		}
		while ( --n != 0 );
	}
	/*
	 * Not enough room in dst, add NUL and traverse rest of src
	 */
	if ( n == 0 )
	{
		if ( siz != 0 )
			*d = STRING_NULL; /* NUL-terminate dst */
		while ( *s++ )
			;
	}
	return ( s - src - 1 ); /* count does not include NUL */
}
char *condtxt ( int current, int base )
{
	static char text[30];
	current *= 1;
	base *= 1;
	if ( current == 0 )
		mudstrlcpy ( text, " }R[BROKEN!]&D", 30 );
	else if ( current == 1 )
		mudstrlcpy ( text, " }R[ALMOST BROKEN!]&D", 30 );
	else if ( current == 2 )
		mudstrlcpy ( text, " }R[Worthless!]&D", 30 );
	else if ( current == 3 )
		mudstrlcpy ( text, " }R[Worn!]&D", 30 );
	else if ( current == 4 )
		mudstrlcpy ( text, " &Y[Dire Repair]&D", 30 );
	else if ( current == 5 )
		mudstrlcpy ( text, " &Y[Urgent Repair]&D", 30 );
	else if ( current == 6 )
		mudstrlcpy ( text, " &Y[Needs Repair]&D", 30 );
	else if ( current == 7 )
		mudstrlcpy ( text, " &Y[Rundown]&D", 30 );
	else if ( current == 8 )
		mudstrlcpy ( text, " &g[Wearing]&D", 30 );
	else if ( current == 9 )
		mudstrlcpy ( text, " &g[Good]&D", 30 );
	else if ( current == 10 )
		mudstrlcpy ( text, " &g[Very Good]&D", 30 );
	else if ( current == 11 )
		mudstrlcpy ( text, " &g[Superb]&D", 30 );
	else
		mudstrlcpy ( text, " &G[Perfect]&D", 30 );
	return text;
}
void do_equipment ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	OBJ_DATA *obj, *obj2;
	bool found = FALSE;
	int iWear, count = 0;
	if ( !ch )
		return;
	if ( !argument || argument[0] == STRING_NULL || !IS_IMMORTAL ( ch ) )
		victim = ch;
	else
	{
		if ( ! ( victim = get_char_world ( ch, argument ) ) )
		{
			ch_printf ( ch, "There is nobody named %s online.\r\n", argument );
			return;
		}
	}
	if ( victim != ch )
		ch_printf ( ch, "&c%s is using:\r\n", IS_NPC ( victim ) ? victim->short_descr : victim->name );
	else
		send_to_char ( "&cYou are using:\r\n", ch );
	set_char_color ( AT_DGREY, ch );
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		count = 0;
		if ( iWear < ( MAX_WEAR - 3 ) )
		{
			if ( ( !IS_NPC ( victim ) ) && ( victim->race > 0 ) && ( victim->race < MAX_PC_RACE ) )
				send_to_char ( race_table[victim->race]->where_name[iWear], ch );
			else
				send_to_char ( where_name[iWear], ch );
		}
		if ( ( obj2 = get_eq_char ( victim, iWear ) ) == NULL && iWear < ( MAX_WEAR - 3 ) )
			send_to_char ( "&c<Nothing>&D", ch );
		for ( obj = victim->first_carrying; obj; obj = obj->next_content )
		{
			if ( obj->wear_loc == iWear )
			{
				count++;
				if ( iWear >= ( MAX_WEAR - 3 ) )
				{
					if ( ( !IS_NPC ( victim ) ) && ( victim->race > 0 ) && ( victim->race < MAX_PC_RACE ) )
						send_to_char ( race_table[victim->race]->where_name[iWear], ch );
					else
						send_to_char ( where_name[iWear], ch );
				}
				if ( count > 1 )
					send_to_char ( "&C<&W LAYERED &C>&D ", ch );
				if ( can_see_obj ( ch, obj ) )
				{
					send_to_char ( format_obj_to_char ( obj, ch, TRUE ), ch );
					if ( obj->item_type == ITEM_ARMOR )
						send_to_char ( condtxt ( obj->value[3], 1 ), ch );
					if ( obj->item_type == ITEM_LIGHT )
						send_to_char ( condtxt ( obj->value[0], 1 ), ch );
					if ( obj->item_type == ITEM_CONTAINER )
						send_to_char ( condtxt ( obj->value[3], 1 ), ch );
					if ( obj->item_type == ITEM_SHOVEL )
						send_to_char ( condtxt ( obj->value[3], 1 ), ch );
					if ( obj->item_type == ITEM_WEAPON || obj->item_type == ITEM_MISSILE_WEAPON )
						send_to_char ( condtxt ( obj->value[0], 12 ), ch );
					if ( obj->item_type == ITEM_PROJECTILE )
						send_to_char ( condtxt ( obj->value[5], obj->value[0] ), ch );
					send_to_char ( "\r\n", ch );
				}
				else
					send_to_char ( "something.\r\n", ch );
				found = TRUE;
			}
		}
		if ( count == 0 && iWear < ( MAX_WEAR ) )
			send_to_char ( "\r\n", ch );
	}
	return;
}
void set_title ( CHAR_DATA * ch, char *title )
{
	char buf[MAX_STRING_LENGTH];
	if ( IS_NPC ( ch ) )
	{
		bug ( "Set_title: NPC.", 0 );
		return;
	}
	if ( isalpha ( title[0] ) || isdigit ( title[0] ) )
	{
		buf[0] = ' ';
		mudstrlcpy ( buf + 1, title, MAX_STRING_LENGTH );
	}
	else
		//  strcpy( buf, title );
	{
		buf[0] = ' ';
		mudstrlcpy ( buf + 1, title, MAX_STRING_LENGTH );
	}
	STRFREE ( ch->pcdata->title );
	ch->pcdata->title = STRALLOC ( buf );
	return;
}
void do_title ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
		return;
	set_char_color ( AT_SCORE, ch );
	if ( IS_SET ( ch->pcdata->flags, PCFLAG_NOTITLE ) )
	{
		set_char_color ( AT_IMMORT, ch );
		send_to_char ( "The Gods prohibit you from changing your title.\r\n", ch );
		return;
	}
	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "Change your title to what?\r\n", ch );
		return;
	}
	if ( strlen_color ( argument ) > 27 )
		argument[27] = STRING_NULL;
	smash_tilde ( argument );
	set_title ( ch, argument );
	send_to_char ( "Ok.\r\n", ch );
}

/*
 * Set your personal description				-Thoric
 */
void do_description ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Monsters are too dumb to do that!\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		bug ( "do_description: no descriptor", 0 );
		return;
	}
	switch ( ch->substate )
	{
		default:
			bug ( "do_description: illegal substate", 0 );
			return;
		case SUB_RESTRICTED:
			send_to_char ( "You cannot use this command from within another command.\r\n", ch );
			return;
		case SUB_NONE:
			ch->substate = SUB_PERSONAL_DESC;
			ch->dest_buf = ch;
			start_editing ( ch, ch->description );
			return;
		case SUB_PERSONAL_DESC:
			STRFREE ( ch->description );
			ch->description = copy_buffer ( ch );
			stop_editing ( ch );
			return;
	}
}

/* Ripped off do_description for whois bio's -- Scryn*/
void do_bio ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mobs cannot set a bio.\r\n", ch );
		return;
	}
	if ( ch->level < 5 )
	{
		set_char_color ( AT_SCORE, ch );
		send_to_char ( "You must be at least level five to write your bio...\r\n", ch );
		return;
	}
	if ( !ch->desc )
	{
		bug ( "do_bio: no descriptor", 0 );
		return;
	}
	switch ( ch->substate )
	{
		default:
			bug ( "do_bio: illegal substate", 0 );
			return;
		case SUB_RESTRICTED:
			send_to_char ( "You cannot use this command from within another command.\r\n", ch );
			return;
		case SUB_NONE:
			ch->substate = SUB_PERSONAL_BIO;
			ch->dest_buf = ch;
			start_editing ( ch, ch->pcdata->bio );
			return;
		case SUB_PERSONAL_BIO:
			STRFREE ( ch->pcdata->bio );
			ch->pcdata->bio = copy_buffer ( ch );
			stop_editing ( ch );
			return;
	}
}


void do_statreport ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Huh?\r\n", ch );
		return;
	}
	sprintf ( buf, "&D--------------------------------------------------------------------------------\r\n"
	          "&c Stat Report for %s \r\n"
	          "&D--------------------------------------------------------------------------------\r\n"
	          "&D %d of %d hp\r\n"
	          "&D %d of %d mana\r\n"
	          "&D %d of %d mv\r\n"
	          "&D %d xp.\r\n"
	          "&D--------------------------------------------------------------------------------\r\n"
	          "&c Base         Current \r\n"
	          "&D %-2d str       %-2d str \r\n"
	          "&D %-2d wis       %-2d wis \r\n"
	          "&D %-2d int       %-2d int \r\n"
	          "&D %-2d dex       %-2d dex \r\n"
	          "&D %-2d con       %-2d con \r\n"
	          "&D %-2d cha       %-2d cha \r\n"
	          "&D %-2d lck       %-2d lck \r\n"
	          "&D--------------------------------------------------------------------------------\r\n",
	          ch->name, ch->hit, ch->max_hit , ch->mana, ch->max_mana, ch->move, ch->max_move, ch->exp,
	          ch->perm_str, get_curr_str ( ch ), ch->perm_wis, get_curr_wis ( ch ), ch->perm_int, get_curr_int ( ch ),
	          ch->perm_dex, get_curr_dex ( ch ), ch->perm_con, get_curr_con ( ch ), ch->perm_cha, get_curr_cha ( ch ),
	          ch->perm_lck, get_curr_lck ( ch ) );
	act ( AT_REPORT, buf, ch, NULL, NULL, TO_ROOM );
	act ( AT_REPORT, buf, ch, NULL, NULL, TO_CHAR );
	return;
}


void do_fprompt ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	set_char_color ( AT_GREY, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "NPC's can't change their prompt..\r\n", ch );
		return;
	}
	smash_tilde ( argument );
	one_argument ( argument, arg );
	if ( !*arg || !str_cmp ( arg, "display" ) )
	{
		send_to_char ( "Your current fighting prompt string:\r\n", ch );
		set_char_color ( AT_WHITE, ch );
		ch_printf ( ch, "%s\r\n", !str_cmp ( ch->pcdata->fprompt, "" ) ? "(default prompt)" : ch->pcdata->fprompt );
		set_char_color ( AT_GREY, ch );
		send_to_char ( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
		return;
	}
	send_to_char ( "Replacing old prompt of:\r\n", ch );
	set_char_color ( AT_WHITE, ch );
	ch_printf ( ch, "%s\r\n", !str_cmp ( ch->pcdata->fprompt, "" ) ? "(default fprompt)" : ch->pcdata->fprompt );
	if ( ch->pcdata->fprompt )
		STRFREE ( ch->pcdata->fprompt );
	if ( strlen ( argument ) > 128 )
		argument[128] = STRING_NULL;
	/*
	 * Can add a list of pre-set prompts here if wanted.. perhaps
	 * 'prompt 1' brings up a different, pre-set prompt
	 */
	if ( !str_cmp ( arg, "bprompt" ) )
		ch->pcdata->fprompt = STRALLOC ( "%h HP %m M %v MV %E" );
	else if ( !str_cmp ( arg, "default" ) )
		ch->pcdata->fprompt = STRALLOC ( "" );
	else
		ch->pcdata->fprompt = STRALLOC ( argument );
	return;
}
void do_prompt ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	set_char_color ( AT_GREY, ch );
	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "NPC's can't change their prompt..\r\n", ch );
		return;
	}
	smash_tilde ( argument );
	one_argument ( argument, arg );
	if ( !*arg || !str_cmp ( arg, "display" ) )
	{
		send_to_char ( "Your current prompt string:\r\n", ch );
		set_char_color ( AT_WHITE, ch );
		ch_printf ( ch, "%s\r\n", !str_cmp ( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
		set_char_color ( AT_GREY, ch );
		send_to_char ( "Type 'help prompt' for information on changing your prompt.\r\n", ch );
		return;
	}
	send_to_char ( "Replacing old prompt of:\r\n", ch );
	set_char_color ( AT_WHITE, ch );
	ch_printf ( ch, "%s\r\n", !str_cmp ( ch->pcdata->prompt, "" ) ? "(default prompt)" : ch->pcdata->prompt );
	if ( ch->pcdata->prompt )
		STRFREE ( ch->pcdata->prompt );
	if ( strlen ( argument ) > 128 )
		argument[128] = STRING_NULL;
	/*
	 * Can add a list of pre-set prompts here if wanted.. perhaps
	 * 'prompt 1' brings up a different, pre-set prompt
	 */
	if ( !str_cmp ( arg, "default" ) )
		ch->pcdata->prompt = STRALLOC ( "" );
	else
		ch->pcdata->prompt = STRALLOC ( argument );
	return;
}
