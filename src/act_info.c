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
 *        Informational module       *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include "./Headers/mud.h"
char *get_exits ( CHAR_DATA * ch );
/*
* Needed in the do_ignore function. -Orion
*/
/* bool    check_parse_name        args( ( char *name, bool newchar ) );*/
/*
 * Keep players from defeating examine progs -Druid
 * False = do not trigger
 * True = Trigger
 */
bool EXA_prog_trigger = TRUE;
/* Had to add unknowns because someone added new positions and didn't
 * update them.  Just a band-aid till I have time to fix it right.
 * This was found thanks to mud@mini.axcomp.com pointing it out :)
 * --Shaddai
 */
char *const where_name[] =
{
	"<used as light>     ",
	"<worn on head>      ",
	"<worn on body>      ",
	"<worn on back>      ",
	"<worn on arms>      ",
	"<worn on hands>     ",
	"<wielded>           ",
	"<worn as shield>    ",
	"<held>              ",
	"<worn on legs>      ",
	"<worn on feet>      ",
	"<lodged in a rib>   ",
	"<lodged in an arm>  ",
	"<lodged in a leg>   "
};

/*
 * Local functions.
 */
void show_char_to_char_0 args ( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char_1 args ( ( CHAR_DATA * victim, CHAR_DATA * ch ) );
void show_char_to_char args ( ( CHAR_DATA * list, CHAR_DATA * ch ) );
bool check_blind args ( ( CHAR_DATA * ch ) );
void show_condition args ( ( CHAR_DATA * ch, CHAR_DATA * victim ) );
/*Similar Helpfile Snippet Declarations*/
short str_similarity ( const char *astr, const char *bstr );
short str_prefix_level ( const char *astr, const char *bstr );
void similar_help_files ( CHAR_DATA * ch, char *argument );

char *format_obj_to_char ( OBJ_DATA * obj, CHAR_DATA * ch, bool fShort )
{
	static char buf[MAX_STRING_LENGTH];
	static char buf2[MAX_STRING_LENGTH];
	bool glowsee = FALSE;

	if ( !IS_NPC ( ch ) && ch->pcdata->questobj > 0 && obj->pIndexData->vnum == ch->pcdata->questobj )
		strcat ( buf, "[TARGET] " );

	/*
	 * can see glowing invis items in the dark
	 */
	if ( IS_OBJ_STAT ( obj, ITEM_GLOW ) && IS_OBJ_STAT ( obj, ITEM_INVIS ) && !IS_AFFECTED ( ch, AFF_TRUESIGHT ) && !IS_AFFECTED ( ch, AFF_DETECT_INVIS ) )
		glowsee = TRUE;

	buf[0] = STRING_NULL;

	if ( IS_OBJ_STAT ( obj, ITEM_INVIS ) )
		strcat ( buf, "(Invis) " );

	if ( ( IS_AFFECTED ( ch, AFF_DETECT_EVIL ) && IS_OBJ_STAT ( obj, ITEM_EVIL ) ) )
		strcat ( buf, "(Red Aura) " );

	if ( IS_AFFECTED ( ch, AFF_DETECT_MAGIC ) && IS_OBJ_STAT ( obj, ITEM_MAGIC ) )
		strcat ( buf, "&Y(&OMagical&Y)&D " );

	if ( !glowsee && IS_OBJ_STAT ( obj, ITEM_GLOW ) )
		strcat ( buf, "&Y(&WGlowing&Y)&D " );

	if ( IS_OBJ_STAT ( obj, ITEM_HUM ) )
		strcat ( buf, "&Y(&pHumming&Y)&D " );

	if ( IS_OBJ_STAT ( obj, ITEM_REFINED ) )
		strcat ( buf, "&Y(&CRefined&Y)&D " );

	if ( IS_OBJ_STAT ( obj, ITEM_UNIQUE ) )
		strcat ( buf, "&Y(&GUnique&Y)&D " );

	if ( IS_OBJ_STAT ( obj, ITEM_ELITE ) )
		strcat ( buf, "&Y(&BElite&Y)&D " );

	if ( IS_OBJ_STAT ( obj, ITEM_HIDDEN ) )
		strcat ( buf, "(Hidden) " );

	if ( IS_OBJ_STAT ( obj, ITEM_BURIED ) )
		strcat ( buf, "(Buried) " );

	if ( IS_IMMORTAL ( ch ) && IS_OBJ_STAT ( obj, ITEM_PROTOTYPE ) )
		strcat ( buf, "&Y(&RPROTO&Y)&D " );

	if ( fShort )
	{
		if ( glowsee && !IS_IMMORTAL ( ch ) )
			strcat ( buf, "the faint glow of something" );
		else if ( obj->short_descr )
			strcat ( buf, obj->short_descr );
	}
	else
	{
		if ( glowsee )
			strcat ( buf, "You see the faint glow of something nearby." );

		if ( obj->description )
			strcat ( buf, obj->description );
	}
	if ( obj->item_type == ITEM_ARMOR || obj->item_type == ITEM_WEAPON )
	{
		sprintf ( buf2, "[%d]", obj->level );
		strcat ( buf, buf2 );
	}
	return buf;
}

/*
 * Some increasingly freaky hallucinated objects  -Thoric
 * (Hats off to Albert Hoffman's "problem child")
 */
char *hallucinated_object ( int ms, bool fShort )
{
	int sms = URANGE ( 1, ( ms + 10 ) / 5, 20 );

	if ( fShort )
		switch ( number_range ( 6 - URANGE ( 1, sms / 2, 5 ), sms ) )
		{

			case 1:
				return "a sword";

			case 2:
				return "a stick";

			case 3:
				return "something shiny";

			case 4:
				return "something";

			case 5:
				return "something interesting";

			case 6:
				return "something colorful";

			case 7:
				return "something that looks cool";

			case 8:
				return "a nifty thing";

			case 9:
				return "a cloak of flowing colors";

			case 10:
				return "a mystical flaming sword";

			case 11:
				return "a swarm of insects";

			case 12:
				return "a deathbane";

			case 13:
				return "a figment of your imagination";

			case 14:
				return "your gravestone";

			case 15:
				return "the long lost boots of Ranger Thoric";

			case 16:
				return "a glowing tome of arcane knowledge";

			case 17:
				return "a long sought secret";

			case 18:
				return "the meaning of it all";

			case 19:
				return "the answer";

			case 20:
				return "the key to life, the universe and everything";
		}

	switch ( number_range ( 6 - URANGE ( 1, sms / 2, 5 ), sms ) )
	{

		case 1:
			return "A nice looking sword catches your eye.";

		case 2:
			return "The ground is covered in small sticks.";

		case 3:
			return "Something shiny catches your eye.";

		case 4:
			return "Something catches your attention.";

		case 5:
			return "Something interesting catches your eye.";

		case 6:
			return "Something colorful flows by.";

		case 7:
			return "Something that looks cool calls out to you.";

		case 8:
			return "A nifty thing of great importance stands here.";

		case 9:
			return "A cloak of flowing colors asks you to wear it.";

		case 10:
			return "A mystical flaming sword awaits your grasp.";

		case 11:
			return "A swarm of insects buzzes in your face!";

		case 12:
			return "The extremely rare Deathbane lies at your feet.";

		case 13:
			return "A figment of your imagination is at your command.";

		case 14:
			return "You notice a gravestone here... upon closer examination, it reads your name.";

		case 15:
			return "The long lost boots of Ranger Thoric lie off to the side.";

		case 16:
			return "A glowing tome of arcane knowledge hovers in the air before you.";

		case 17:
			return "A long sought secret of all mankind is now clear to you.";

		case 18:
			return "The meaning of it all, so simple, so clear... of course!";

		case 19:
			return "The answer.  One.  It's always been One.";

		case 20:
			return "The key to life, the universe and everything awaits your hand.";
	}

	return "Whoa!!!";
}

/* This is the punct snippet from Desden el Chaman Tibetano - Nov 1998
   Email: jlalbatros@mx2.redestb.es
*/
char *num_punct ( int foo )
{
	int nindex, index_new, rest;
	char buf[16];
	static char buf_new[16];
	sprintf ( buf, "%d", foo );
	rest = strlen ( buf ) % 3;

	for ( nindex = index_new = 0; nindex < strlen ( buf ); nindex++, index_new++ )
	{
		if ( nindex != 0 && ( nindex - rest ) % 3 == 0 )
		{
			buf_new[index_new] = ',';
			index_new++;
			buf_new[index_new] = buf[nindex];
		}
		else
			buf_new[index_new] = buf[nindex];
	}

	buf_new[index_new] = STRING_NULL;

	return buf_new;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char ( OBJ_DATA * list, CHAR_DATA * ch, bool fShort, bool fShowNothing )
{
	char **prgpstrShow;
	int *prgnShow;
	int *pitShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count, offcount, tmp, ms, cnt;
	bool fCombine;

	if ( !ch->desc )
		return;

	/*
	 * if there's no list... then don't do all this crap!  -Thoric
	 */
	if ( !list )
	{
		if ( fShowNothing )
		{
			if ( IS_NPC ( ch ) || xIS_SET ( ch->act, PLR_COMBINE ) )
				send_to_char ( "     ", ch );

			set_char_color ( AT_OBJECT, ch );

			send_to_char ( "Nothing.\r\n", ch );
		}

		return;
	}

	/*
	 * Alloc space for output lines.
	 */
	count = 0;
	offcount = 0;
	for ( obj = list; obj; obj = obj->next_content )
		count++;

	if ( count + offcount <= 0 )
	{
		if ( fShowNothing )
		{
			if ( IS_NPC ( ch ) || xIS_SET ( ch->act, PLR_COMBINE ) )
				send_to_char ( "     ", ch );

			set_char_color ( AT_OBJECT, ch );

			send_to_char ( "Nothing.\r\n", ch );
		}

		return;
	}

	CREATE ( prgpstrShow, char *, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	CREATE ( prgnShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	CREATE ( pitShow, int, count + ( ( offcount > 0 ) ? offcount : 0 ) );
	nShow = 0;
	tmp = ( offcount > 0 ) ? offcount : 0;
	cnt = 0;
	/*
	 * Format the list of objects.
	 */

	for ( obj = list; obj; obj = obj->next_content )
	{
		if ( offcount < 0 && ++cnt > ( count + offcount ) )
			break;

		if ( tmp > 0 && number_bits ( 1 ) == 0 )
		{
			prgpstrShow[nShow] = str_dup ( hallucinated_object ( ms, fShort ) );
			prgnShow[nShow] = 1;
			pitShow[nShow] = number_range ( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
			--tmp;
		}

		if ( obj->wear_loc == WEAR_NONE && can_see_obj ( ch, obj ) && ( obj->item_type != ITEM_TRAP || IS_AFFECTED ( ch, AFF_DETECTTRAPS ) ) )
		{
			pstrShow = format_obj_to_char ( obj, ch, fShort );
			fCombine = FALSE;

			if ( IS_NPC ( ch ) || xIS_SET ( ch->act, PLR_COMBINE ) )
			{
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop backwords.
				 */
				for ( iShow = nShow - 1; iShow >= 0; iShow-- )
				{
					if ( !strcmp ( prgpstrShow[iShow], pstrShow ) )
					{
						prgnShow[iShow] += obj->count;
						fCombine = TRUE;
						break;
					}
				}
			}

			pitShow[nShow] = obj->item_type;

			/*
			 * Couldn't combine, or didn't want to.
			 */

			if ( !fCombine )
			{
				prgpstrShow[nShow] = str_dup ( pstrShow );
				prgnShow[nShow] = obj->count;
				nShow++;
			}
		}
	}

	if ( tmp > 0 )
	{
		int x;

		for ( x = 0; x < tmp; x++ )
		{
			prgpstrShow[nShow] = str_dup ( hallucinated_object ( ms, fShort ) );
			prgnShow[nShow] = 1;
			pitShow[nShow] = number_range ( ITEM_LIGHT, ITEM_BOOK );
			nShow++;
		}
	}

	/*
	 * Output the formatted list.     -Color support by Thoric
	 */
	for ( iShow = 0; iShow < nShow; iShow++ )
	{
		switch ( pitShow[iShow] )
		{

			default:
				set_char_color ( AT_OBJECT, ch );
				break;

			case ITEM_BLOOD:
				set_char_color ( AT_BLOOD, ch );
				break;

			case ITEM_MONEY:

			case ITEM_TREASURE:
				set_char_color ( AT_YELLOW, ch );
				break;

			case ITEM_COOK:

			case ITEM_FOOD:
				set_char_color ( AT_HUNGRY, ch );
				break;

			case ITEM_DRINK_CON:

			case ITEM_FOUNTAIN:
				set_char_color ( AT_THIRSTY, ch );
				break;

			case ITEM_FIRE:
				set_char_color ( AT_FIRE, ch );
				break;

			case ITEM_SCROLL:

			case ITEM_WAND:

			case ITEM_STAFF:
				set_char_color ( AT_MAGIC, ch );
				break;
		}

		if ( fShowNothing )
			send_to_char ( "     ", ch );

		send_to_char ( prgpstrShow[iShow], ch );


		/*
		 * if ( IS_NPC(ch) || xIS_SET(ch->act, PLR_COMBINE) )
		 */
		{
			if ( prgnShow[iShow] != 1 )
				ch_printf ( ch, " (%d)", prgnShow[iShow] );
		}

		send_to_char ( "\r\n", ch );

		DISPOSE ( prgpstrShow[iShow] );
	}

	if ( fShowNothing && nShow == 0 )
	{
		if ( IS_NPC ( ch ) || xIS_SET ( ch->act, PLR_COMBINE ) )
			send_to_char ( "     ", ch );

		set_char_color ( AT_OBJECT, ch );

		send_to_char ( "Nothing.\r\n", ch );
	}

	/*
	 * Clean up.
	 */
	DISPOSE ( prgpstrShow );

	DISPOSE ( prgnShow );

	DISPOSE ( pitShow );

	return;
}

/*
 * Show fancy descriptions for certain spell affects  -Thoric
 */
void show_visible_affects_to_char ( CHAR_DATA * victim, CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char name[MAX_STRING_LENGTH];

	if ( IS_NPC ( victim ) )
		strcpy ( name, victim->short_descr );
	else
		strcpy ( name, victim->name );

	name[0] = toupper ( name[0] );

	if ( IS_AFFECTED ( victim, AFF_SANCTUARY ) )
	{
		set_char_color ( AT_WHITE, ch );

		if ( IS_GOOD ( victim ) )
			ch_printf ( ch, "%s glows with an aura of divine radiance.\r\n", name );
		else if ( IS_EVIL ( victim ) )
			ch_printf ( ch, "%s shimmers beneath an aura of dark energy.\r\n", name );
		else
			ch_printf ( ch, "%s is shrouded in flowing shadow and light.\r\n", name );
	}

	/*
	 * if( IS_AFFECTED( victim, AFF_FIRESHIELD ) )
	 * {
	 * set_char_color( AT_FIRE, ch );
	 * ch_printf( ch, "%s is engulfed within a blaze of mystical flame.\r\n", name );
	 * }
	 * if( IS_AFFECTED( victim, AFF_SHOCKSHIELD ) )
	 * {
	 * set_char_color( AT_BLUE, ch );
	 * ch_printf( ch, "%s is surrounded by cascading torrents of energy.\r\n", name );
	 * }
	 * if( IS_AFFECTED( victim, AFF_ACIDMIST ) )
	 * {
	 * set_char_color( AT_GREEN, ch );
	 * ch_printf( ch, "%s is visible through a cloud of churning mist.\r\n", name );
	 * }
	 *
	 * * Scryn 8/13
	 *
	 * if( IS_AFFECTED( victim, AFF_ICESHIELD ) )
	 * {
	 * set_char_color( AT_LBLUE, ch );
	 * ch_printf( ch, "%s is ensphered by shards of glistening ice.\r\n", name );
	 * }
	 */
	if ( IS_AFFECTED ( victim, AFF_CHARM ) )
	{
		set_char_color ( AT_MAGIC, ch );
		ch_printf ( ch, "%s wanders in a dazed, zombie-like state.\r\n", name );
	}

	if ( !IS_NPC ( victim ) && !victim->desc && victim->switched && IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		set_char_color ( AT_MAGIC, ch );
		strcpy ( buf, PERS ( victim, ch, FALSE ) );
		strcat ( buf, " appears to be in a deep trance...\r\n" );
	}
}

void show_char_to_char_0 ( CHAR_DATA * victim, CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf1[MAX_STRING_LENGTH];
	char message[MAX_STRING_LENGTH];
	buf[0] = STRING_NULL;
	set_char_color ( AT_PERSON, ch );

	if ( !IS_NPC ( victim ) && !victim->desc )
	{
		if ( !victim->switched )
			send_to_char_color ( "&P[(Link Dead)] ", ch );
		else if ( !IS_AFFECTED ( victim, AFF_POSSESS ) )
			strcat ( buf, "(Switched) " );
	}

	if ( IS_NPC ( victim ) && IS_AFFECTED ( victim, AFF_POSSESS ) && IS_IMMORTAL ( ch ) && victim->desc )
	{
		sprintf ( buf1, "(%s)", victim->desc->original->name );
		strcat ( buf, buf1 );
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
		strcat ( buf, "[AFK] " );

	if ( ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_WIZINVIS ) ) || ( IS_NPC ( victim ) && xIS_SET ( victim->act, ACT_MOBINVIS ) ) )
	{
		if ( !IS_NPC ( victim ) )
			sprintf ( buf1, "(Invis %d) ", victim->pcdata->wizinvis );
		else
			sprintf ( buf1, "(Mobinvis %d) ", victim->mobinvis );

		strcat ( buf, buf1 );
	}

	if ( IS_NPC ( victim ) && !IS_NPC ( ch ) && ch->pcdata->questmob > 0 && victim->pIndexData->vnum == ch->pcdata->questmob )
		strcat ( buf, "[TARGET] " );

	if ( IS_AFFECTED ( victim, AFF_FIRESHIELD ) )
		strcat ( buf, "&R(Fire)&D" );

	if ( IS_AFFECTED ( victim, AFF_SHOCKSHIELD ) )
		strcat ( buf, "&B(Shock)&D" );

	if ( IS_AFFECTED ( victim, AFF_ACIDMIST ) )
		strcat ( buf, "&Y(Acid)&D" );

	if ( IS_AFFECTED ( victim, AFF_ICESHIELD ) )
		strcat ( buf, "&W(Ice)&D" );

	if ( !IS_NPC ( victim ) )
	{
		if ( IS_IMMORTAL ( victim ) && victim->level > LEVEL_AVATAR )
			send_to_char_color ( "&P(&WImmortal&P) ", ch );

		if ( victim->pcdata->clan  && IS_SET ( victim->pcdata->flags, PCFLAG_DEADLY ) && victim->pcdata->clan->badge )
			ch_printf_color ( ch, "%s ", victim->pcdata->clan->badge );
		else if ( CAN_PKILL ( victim ) && victim->level < LEVEL_IMMORTAL )
			send_to_char_color ( "&P(&wUnclanned&P) ", ch );
	}

	set_char_color ( AT_PERSON, ch );

	if ( IS_AFFECTED ( victim, AFF_INVISIBLE ) )
		strcat ( buf, "(Invis) " );

	if ( IS_AFFECTED ( victim, AFF_HIDE ) )
		strcat ( buf, "(Hide) " );

	if ( IS_AFFECTED ( victim, AFF_PASS_DOOR ) )
		strcat ( buf, "(Translucent) " );

	if ( IS_AFFECTED ( victim, AFF_FAERIE_FIRE ) )
		strcat ( buf, "(Pink Aura) " );

	if ( IS_EVIL ( victim ) && ( IS_AFFECTED ( ch, AFF_DETECT_EVIL ) ) )
		strcat ( buf, "(Red Aura) " );

	if ( IS_NEUTRAL ( victim ) && ( IS_AFFECTED ( ch, AFF_DETECT_EVIL ) ) )
		strcat ( buf, "(Grey Aura) " );

	if ( IS_GOOD ( victim ) && ( IS_AFFECTED ( ch, AFF_DETECT_EVIL ) ) )
		strcat ( buf, "(White Aura) " );

	if ( IS_AFFECTED ( victim, AFF_BERSERK ) )
		strcat ( buf, "(Berserk) " );

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_ATTACKER ) )
		strcat ( buf, "(Attacker) " );

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_KILLER ) )
		strcat ( buf, "(Killer) " );

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_THIEF ) )
		strcat ( buf, "(Theif) " );

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_LITTERBUG ) )
		strcat ( buf, "(Litterbug) " );

	if ( IS_NPC ( victim ) && IS_IMMORTAL ( ch ) && xIS_SET ( victim->act, ACT_PROTOTYPE ) )
		strcat ( buf, "(P) " );

	if ( IS_NPC ( victim ) && ch->mount && ch->mount == victim && ch->in_room == ch->mount->in_room )
		strcat ( buf, "(Mount) " );

	if ( victim->desc && victim->desc->connected == CON_EDITING )
		strcat ( buf, "(Writing) " );

	set_char_color ( AT_PERSON, ch );

	if ( ( victim->position == victim->defposition && victim->long_descr[0] != STRING_NULL ) )
	{
		strcat ( buf, victim->long_descr );
		send_to_char ( buf, ch );
		show_visible_affects_to_char ( victim, ch );
		return;
	}
	else
	{
		strcat ( buf, PERS ( victim, ch, FALSE ) );
	}

	if ( !IS_NPC ( victim ) && !xIS_SET ( ch->act, PLR_BRIEF ) )
		strcat ( buf, victim->pcdata->title );

	switch ( victim->position )
	{

		case POS_DEAD:
			strcat ( buf, " is DEAD!!" );
			break;

		case POS_MORTAL:
			strcat ( buf, " is mortally wounded." );
			break;

		case POS_INCAP:
			strcat ( buf, " is incapacitated." );
			break;

		case POS_STUNNED:
			strcat ( buf, " is lying here stunned." );
			break;
			/*
			 * Furniture ideas taken from ROT
			 * Furniture 1.01 is provided by Xerves
			 * Info rewrite for sleeping/resting/standing/sitting on Objects -- Xerves
			 */

		case POS_SLEEPING:

			if ( victim->on != NULL )
			{
				if ( IS_SET ( victim->on->value[2], SLEEP_AT ) )
				{
					sprintf ( message, "&P is sleeping at %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else if ( IS_SET ( victim->on->value[2], SLEEP_ON ) )
				{
					sprintf ( message, "&P is sleeping on %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else
				{
					sprintf ( message, "&P is sleeping in %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
			}
			else
			{
				if ( ch->position == POS_SITTING || ch->position == POS_RESTING )
					strcat ( buf, "&P is sleeping nearby.&G" );
				else
					strcat ( buf, "&P is deep in slumber here.&G" );
			}

			break;

		case POS_RESTING:

			if ( victim->on != NULL )
			{
				if ( IS_SET ( victim->on->value[2], REST_AT ) )
				{
					sprintf ( message, "&P is resting at %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else if ( IS_SET ( victim->on->value[2], REST_ON ) )
				{
					sprintf ( message, "&P is resting on %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else
				{
					sprintf ( message, "&P is resting in %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
			}
			else
			{
				if ( ch->position == POS_RESTING )
					strcat ( buf, "&P is sprawled out alongside you.&G" );
				else if ( ch->position == POS_MOUNTED )
					strcat ( buf, "&P is sprawled out at the foot of your mount.&G" );
				else
					strcat ( buf, "&P is sprawled out here.&G" );
			}

			break;

		case POS_SITTING:

			if ( victim->on != NULL )
			{
				if ( IS_SET ( victim->on->value[2], SIT_AT ) )
				{
					sprintf ( message, "&P is sitting at %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else if ( IS_SET ( victim->on->value[2], SIT_ON ) )
				{
					sprintf ( message, "&P is sitting on %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else
				{
					sprintf ( message, "&P is sitting in %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
			}
			else
				strcat ( buf, "&P is sitting here." );

			break;

		case POS_STANDING:
			if ( victim->on != NULL )
			{
				if ( IS_SET ( victim->on->value[2], STAND_AT ) )
				{
					sprintf ( message, "&P is standing at %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else if ( IS_SET ( victim->on->value[2], STAND_ON ) )
				{
					sprintf ( message, "&P is standing on %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
				else
				{
					sprintf ( message, "&P is standing in %s.", victim->on->short_descr );
					strcat ( buf, message );
				}
			}
			else if ( IS_IMMORTAL ( victim ) )
				strcat ( buf, "&P is here before you.&G" );
			else if ( ( victim->in_room->sector_type == SECT_UNDERWATER ) && !IS_AFFECTED ( victim, AFF_AQUA_BREATH ) && !IS_NPC ( victim ) )
				strcat ( buf, "&P is drowning here.&G" );
			else if ( victim->in_room->sector_type == SECT_UNDERWATER )
				strcat ( buf, "&P is here in the water.&G" );
			else if ( ( victim->in_room->sector_type == SECT_OCEANFLOOR ) && !IS_AFFECTED ( victim, AFF_AQUA_BREATH ) && !IS_NPC ( victim ) )
				strcat ( buf, "&P is drowning here.&G" );
			else if ( victim->in_room->sector_type == SECT_OCEANFLOOR )
				strcat ( buf, "&P is standing here in the water.&G" );
			else if ( IS_AFFECTED ( victim, AFF_FLOATING ) || IS_AFFECTED ( victim, AFF_FLYING ) )
				strcat ( buf, "&P is hovering here.&G" );
			else
				strcat ( buf, "&P is standing here.&G" );

			break;

		case POS_SHOVE:
			strcat ( buf, " is being shoved around." );

			break;

		case POS_DRAG:
			strcat ( buf, " is being dragged around." );

			break;

		case POS_MOUNTED:
			strcat ( buf, " is here, upon " );

			if ( !victim->mount )
				strcat ( buf, "thin air???" );
			else if ( victim->mount == ch )
				strcat ( buf, "your back." );
			else if ( victim->in_room == victim->mount->in_room )
			{
				strcat ( buf, PERS ( victim->mount, ch, FALSE ) );
				strcat ( buf, "." );
			}
			else
				strcat ( buf, "someone who left??" );

			break;

		case POS_FIGHTING:

		case POS_EVASIVE:

		case POS_DEFENSIVE:

		case POS_AGGRESSIVE:

		case POS_BERSERK:
			strcat ( buf, " is here, fighting " );

			if ( !victim->fighting )
			{
				strcat ( buf, "thin air???" );
				/*
				 * some bug somewhere.... kinda hackey fix -h
				 */

				if ( !victim->mount )
					victim->position = POS_STANDING;
				else
					victim->position = POS_MOUNTED;
			}
			else if ( who_fighting ( victim ) == ch )
				strcat ( buf, "YOU!" );
			else if ( victim->in_room == victim->fighting->who->in_room )
			{
				strcat ( buf, PERS ( victim->fighting->who, ch, FALSE ) );
				strcat ( buf, "." );
			}
			else
				strcat ( buf, "someone who left??" );

			break;
	}

	strcat ( buf, "\r\n" );

	buf[0] = UPPER ( buf[0] );
	send_to_char ( buf, ch );
	show_visible_affects_to_char ( victim, ch );
	return;
}

void show_char_to_char_1 ( CHAR_DATA * victim, CHAR_DATA * ch )
{
	OBJ_DATA *obj;
	int iWear;
	bool found;

	if ( can_see ( victim, ch, FALSE ) && !IS_NPC ( ch ) && !xIS_SET ( ch->act, PLR_WIZINVIS ) )
	{
		act ( AT_ACTION, "$n looks at you.", ch, NULL, victim, TO_VICT );

		if ( victim != ch )
			act ( AT_ACTION, "$n looks at $N.", ch, NULL, victim, TO_NOTVICT );
		else
			act ( AT_ACTION, "$n looks at $mself.", ch, NULL, victim, TO_NOTVICT );
	}

	if ( victim->description[0] != STRING_NULL )
	{
		send_to_char ( victim->description, ch );
	}

	show_race_line ( ch, victim );
	show_condition ( ch, victim );
	found = FALSE;

	for ( iWear = 0; iWear < MAX_WEAR; iWear++ )
	{
		if ( ( obj = get_eq_char ( victim, iWear ) ) != NULL && can_see_obj ( ch, obj ) )
		{
			if ( !found )
			{
				send_to_char ( "\r\n", ch );

				if ( victim != ch )
					act ( AT_PLAIN, "$N is using:", ch, NULL, victim, TO_CHAR );
				else
					act ( AT_PLAIN, "You are using:", ch, NULL, NULL, TO_CHAR );

				found = TRUE;
			}

			if ( ( !IS_NPC ( victim ) ) && ( victim->race > 0 ) && ( victim->race < MAX_PC_RACE ) )
				send_to_char ( race_table[victim->race]->where_name[iWear], ch );
			else
				send_to_char ( where_name[iWear], ch );

			send_to_char ( format_obj_to_char ( obj, ch, TRUE ), ch );

			send_to_char ( "\r\n", ch );
		}
	}

	/*
	 * Crash fix here by Thoric
	 */
	if ( IS_NPC ( ch ) || victim == ch )
		return;

	if ( IS_IMMORTAL ( ch ) )
	{
		if ( IS_NPC ( victim ) )
			ch_printf ( ch, "\r\nMobile #%d '%s' ", victim->pIndexData->vnum, victim->name );
		else
			ch_printf ( ch, "\r\n%s ", victim->name );
	}
	ch_printf ( ch, "is a level %d %s of the %s clan.\r\n",
	            victim->level,
            IS_NPC ( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ? npc_Class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS
	            && Class_table[victim->Class]->who_name && Class_table[victim->Class]->who_name[0] != STRING_NULL ? Class_table[victim->Class]->who_name :
	            "unknown",
            IS_NPC ( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ? npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE
	            && race_table[victim->race]->race_name && race_table[victim->race]->race_name[0] != STRING_NULL ? race_table[victim->race]->race_name :
	            "unknown" );


	ch_printf ( ch, "\r\nYou peek at %s inventory:\r\n", victim->sex == 1 ? "his" : victim->sex == 2 ? "her" : "its" );
	show_list_to_char ( victim->first_carrying, ch, TRUE, TRUE );

	return;
}

void show_char_to_char ( CHAR_DATA * list, CHAR_DATA * ch )
{
	CHAR_DATA *rch;

	for ( rch = list; rch; rch = rch->next_in_room )
	{
		if ( rch == ch )
			continue;

		if ( can_see ( ch, rch, FALSE ) )
		{
			show_char_to_char_0 ( rch, ch );
		}
		else if ( room_is_dark ( ch->in_room ) && IS_AFFECTED ( ch, AFF_INFRARED ) && ! ( !IS_NPC ( rch ) && IS_IMMORTAL ( rch ) ) )
		{
			set_char_color ( AT_BLOOD, ch );
			send_to_char ( "The red form of a living creature is here.\r\n", ch );
		}
	}

	return;
}

bool check_blind ( CHAR_DATA * ch )
{
	if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_HOLYLIGHT ) )
		return TRUE;

	if ( IS_AFFECTED ( ch, AFF_TRUESIGHT ) )
		return TRUE;

	if ( IS_AFFECTED ( ch, AFF_BLIND ) )
	{
		send_to_char ( "You can't see a thing!\r\n", ch );
		return FALSE;
	}

	return TRUE;
}

/*
 * Returns Classical DIKU door direction based on text in arg -Thoric
 */
int get_door ( char *arg )
{
	int door;

	if ( !str_cmp ( arg, "n" ) || !str_cmp ( arg, "north" ) )
		door = 0;
	else if ( !str_cmp ( arg, "e" ) || !str_cmp ( arg, "east" ) )
		door = 1;
	else if ( !str_cmp ( arg, "s" ) || !str_cmp ( arg, "south" ) )
		door = 2;
	else if ( !str_cmp ( arg, "w" ) || !str_cmp ( arg, "west" ) )
		door = 3;
	else if ( !str_cmp ( arg, "u" ) || !str_cmp ( arg, "up" ) )
		door = 4;
	else if ( !str_cmp ( arg, "d" ) || !str_cmp ( arg, "down" ) )
		door = 5;
	else if ( !str_cmp ( arg, "ne" ) || !str_cmp ( arg, "northeast" ) )
		door = 6;
	else if ( !str_cmp ( arg, "nw" ) || !str_cmp ( arg, "northwest" ) )
		door = 7;
	else if ( !str_cmp ( arg, "se" ) || !str_cmp ( arg, "southeast" ) )
		door = 8;
	else if ( !str_cmp ( arg, "sw" ) || !str_cmp ( arg, "southwest" ) )
		door = 9;
	else
		door = -1;

	return door;
}

void display_imm_toggle ( CHAR_DATA * ch )
{
	/*
	 * Room flag display installed by Samson 12-10-97
	 */
	if ( !IS_NPC ( ch ) && IS_IMMORTAL ( ch ) && IS_SET ( ch->pcdata->flags, PCFLAG_AUTOFLAGS ) )
	{
		ch_printf ( ch, "\r\n\r\n&B[&wArea Flags:&W %s&B]&W\r\n", flag_string ( ch->in_room->area->flags, area_flags ) );
		ch_printf ( ch, "&B[&wRoom Flags:&W %s&B]\r\n", ext_flag_string ( &ch->in_room->room_flags, r_flags ) );
	}

	/*
	 * Room Sector display written and installed by Samson 12-10-97
	 */
	if ( !IS_NPC ( ch ) && IS_IMMORTAL ( ch ) && IS_SET ( ch->pcdata->flags, PCFLAG_SECTORD ) )
	{
		ch_printf ( ch, "&B[&wSector Type: &W%s&B]\r\n", sec_flags[ch->in_room->sector_type] );
	}

	/*
	 * Area name and filename display installed by Samson 12-13-97
	 */
	if ( !IS_NPC ( ch ) && IS_IMMORTAL ( ch ) && IS_SET ( ch->pcdata->flags, PCFLAG_ANAME ) )
	{
		ch_printf ( ch, "&B[&wArea name: &W%s&B]", ch->in_room->area->name );

		if ( ch->level >= LEVEL_CREATOR )
			ch_printf ( ch, "&B[&wArea filename:&W %s&B]\r\n\r\n\r\n", ch->in_room->area->filename );
		else
			send_to_char ( "\r\n\r\n\r\n", ch );
	}

	return;
}

void do_look ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *original;
	LIQ_TABLE *liq;
	char *pdesc;
	short door;
	int number, cnt;


	if ( !ch->desc )
		return;

	if ( ch->position < POS_SLEEPING )
	{
		send_to_char ( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if ( ch->position == POS_SLEEPING )
	{
		send_to_char ( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if ( !check_blind ( ch ) )
		return;

	if ( !IS_NPC ( ch ) && !xIS_SET ( ch->act, PLR_HOLYLIGHT ) && !IS_AFFECTED ( ch, AFF_TRUESIGHT ) && room_is_dark ( ch->in_room ) )
	{
		set_char_color ( AT_DGREY, ch );
		send_to_char ( "It is pitch black ... \r\n", ch );
		show_char_to_char ( ch->in_room->first_person, ch );
		return;
	}

	argument = one_argument ( argument, arg1 );

	argument = one_argument ( argument, arg2 );
	argument = one_argument ( argument, arg3 );
	send_mip_room ( ch, ch->in_room->name );
	if ( arg1[0] == STRING_NULL || !str_cmp ( arg1, "auto" ) )
	{
		if ( IS_PLR_FLAG ( ch, PLR_ONMAP ) || IS_ACT_FLAG ( ch, ACT_ONMAP ) )
		{
			display_map ( ch );
			show_list_to_char ( ch->in_room->first_content, ch, FALSE, FALSE );
			show_char_to_char ( ch->in_room->first_person, ch );
			return;
		}

		display_imm_toggle ( ch );
		//	set_char_color ( AT_RMNAME, ch );
		send_to_char ( "&z--------------------------------------------------------------------------------\r\n", ch );
		if ( xIS_SET ( ch->in_room->room_flags, ROOM_SAFE ) )
			ch_printf ( ch, "&z-{ &WSAFE ROOM &z}-&R %s&D\r\n", ch->in_room->name );
		else
			ch_printf ( ch, "&z-{  &rPK ROOM  &z}-&R %s&D\r\n", ch->in_room->name );
		send_to_char ( "&z--------------------------------------------------------------------------------\r\n", ch );

		if ( !IS_NPC ( ch ) && ( xIS_SET ( ch->act, PLR_BRIEF ) || xIS_SET ( ch->in_room->room_flags, ROOM_NOMAP ) ) )
		{
			set_char_color ( AT_RMDESC, ch );
			send_to_char ( ch->in_room->description, ch );
			send_to_char ( "&z--------------------------------------------------------------------------------\r\n", ch );

		}

		else if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_AUTOMAP ) )   /* maps */
		{
			draw_map ( ch, ch->in_room->description );
			return;
		}


		if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_AUTOEXIT ) )
		{
			sprintf ( buf, "%s%s", color_str ( AT_EXITS, ch ), get_exits ( ch ) );
			send_to_char ( buf, ch );
			send_mip_exits ( ch );
		}

		show_list_to_char ( ch->in_room->first_content, ch, FALSE, FALSE );

		show_char_to_char ( ch->in_room->first_person, ch );
		return;
	}

	if ( !str_cmp ( arg1, "under" ) )
	{
		int count;
		/*
		 * 'look under'
		 */

		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Look beneath what?\r\n", ch );
			return;
		}

		if ( ( obj = get_obj_here ( ch, arg2 ) ) == NULL )
		{
			send_to_char ( "You do not see that here.\r\n", ch );
			return;
		}

		if ( !CAN_WEAR ( obj, ITEM_TAKE ) && ch->level < sysdata.level_getobjnotake )
		{
			send_to_char ( "You can't seem to get a grip on it.\r\n", ch );
			return;
		}

		if ( ch->carry_weight + obj->weight > can_carry_w ( ch ) )
		{
			send_to_char ( "It's too heavy for you to look under.\r\n", ch );
			return;
		}

		count = obj->count;

		obj->count = 1;
		act ( AT_PLAIN, "You lift $p and look beneath it:", ch, obj, NULL, TO_CHAR );
		act ( AT_PLAIN, "$n lifts $p and looks beneath it:", ch, obj, NULL, TO_ROOM );
		obj->count = count;

		if ( IS_OBJ_STAT ( obj, ITEM_COVERING ) )
			show_list_to_char ( obj->first_content, ch, TRUE, TRUE );
		else
			send_to_char ( "Nothing.\r\n", ch );

		if ( EXA_prog_trigger )
			oprog_examine_trigger ( ch, obj );

		return;
	}

	if ( !str_cmp ( arg1, "i" ) || !str_cmp ( arg1, "in" ) )
	{
		int count;
		/*
		 * 'look in'
		 */

		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Look in what?\r\n", ch );
			return;
		}

		if ( ( obj = get_obj_here ( ch, arg2 ) ) == NULL )
		{
			send_to_char ( "You do not see that here.\r\n", ch );
			return;
		}

		switch ( obj->item_type )
		{

			default:
				send_to_char ( "That is not a container.\r\n", ch );
				break;

			case ITEM_DRINK_CON:

				if ( obj->value[1] <= 0 )
				{
					send_to_char ( "It is empty.\r\n", ch );

					if ( EXA_prog_trigger )
						oprog_examine_trigger ( ch, obj );

					break;
				}

				/*
				 * exam addition for the new liquidtable  -Nopey
				 */
				{
					if ( obj->value[2] > 17 )
						obj->value[2] = 17;

					liq = get_liq_vnum ( obj->value[2] );

					ch_printf ( ch, "It's %s full of a %s liquid.",
					            obj->value[1] < obj->value[0] / 4 ? "less than" : obj->value[1] < 3 * obj->value[0] / 4 ? "about" : "more than", liq->color );
				}

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				break;

			case ITEM_PORTAL:
				for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
				{
					if ( pexit->vdir == DIR_PORTAL && IS_SET ( pexit->exit_info, EX_PORTAL ) )
					{
						if ( room_is_private ( pexit->to_room ) && ch->level < sysdata.level_override_private )
						{
							set_char_color ( AT_WHITE, ch );
							send_to_char ( "The room ahead is private!\r\n", ch );
							return;
						}

						if ( IS_EXIT_FLAG ( pexit, EX_OVERLAND ) )
						{
							original = ch->in_room;
							enter_map ( ch, pexit->x, pexit->y, -1 );
							leave_map ( ch, NULL, original );
						}
						else
						{
							original = ch->in_room;
							char_from_room ( ch );
							char_to_room ( ch, pexit->to_room );
							do_look ( ch, "auto" );
							char_from_room ( ch );
							char_to_room ( ch, original );
						}

						return;
					}
				}

				send_to_char ( "You see swirling chaos...\r\n", ch );

				break;

			case ITEM_CONTAINER:

			case ITEM_QUIVER:

			case ITEM_CORPSE_NPC:

			case ITEM_CORPSE_PC:

				if ( IS_SET ( obj->value[1], CONT_CLOSED ) )
				{
					send_to_char ( "It is closed.\r\n", ch );
					break;
				}

			case ITEM_KEYRING:

				count = obj->count;
				obj->count = 1;

				if ( obj->item_type == ITEM_CONTAINER )
					act ( AT_PLAIN, "$p contains:", ch, obj, NULL, TO_CHAR );
				else
					act ( AT_PLAIN, "$p holds:", ch, obj, NULL, TO_CHAR );

				obj->count = count;

				show_list_to_char ( obj->first_content, ch, TRUE, TRUE );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				break;
		}

		return;
	}

	if ( ( pdesc = get_extra_descr ( arg1, ch->in_room->first_extradesc ) ) != NULL )
	{
		send_to_char_color ( pdesc, ch );
		return;
	}

	door = get_door ( arg1 );

	if ( ( pexit = find_door ( ch, arg1, TRUE ) ) != NULL )
	{
		if ( IS_SET ( pexit->exit_info, EX_CLOSED ) && !IS_SET ( pexit->exit_info, EX_WINDOW ) )
		{
			if ( ( IS_SET ( pexit->exit_info, EX_SECRET ) || IS_SET ( pexit->exit_info, EX_DIG ) ) && door != -1 )
				send_to_char ( "Nothing special there.\r\n", ch );
			else
				act ( AT_PLAIN, "The $d is closed.", ch, NULL, pexit->keyword, TO_CHAR );

			return;
		}

		if ( IS_SET ( pexit->exit_info, EX_BASHED ) )
			act ( AT_RED, "The $d has been bashed from its hinges!", ch, NULL, pexit->keyword, TO_CHAR );

		if ( pexit->description && pexit->description[0] != STRING_NULL )
			send_to_char ( pexit->description, ch );
		else
			send_to_char ( "Nothing special there.\r\n", ch );

		/*
		 * Ability to look into the next room       -Thoric
		 */
		if ( pexit->to_room && ( IS_AFFECTED ( ch, AFF_SCRYING ) || IS_SET ( pexit->exit_info, EX_xLOOK ) || get_trust ( ch ) >= LEVEL_IMMORTAL ) )
		{
			if ( !IS_SET ( pexit->exit_info, EX_xLOOK ) && get_trust ( ch ) < LEVEL_IMMORTAL )
			{
				set_char_color ( AT_MAGIC, ch );
				send_to_char ( "You attempt to scry...\r\n", ch );
				/*
				 * Change by Narn, Sept 96 to allow characters who don't have the
				 * scry spell to benefit from objects that are affected by scry.
				 */

				if ( !IS_NPC ( ch ) )
				{
					int percent = LEARNED ( ch, skill_lookup ( "scry" ) );

					if ( !percent )
					{
						percent = 95;
					}

					if ( number_percent( ) > percent )
					{
						send_to_char ( "You fail.\r\n", ch );
						return;
					}
				}
			}

			if ( room_is_private ( pexit->to_room ) && ch->level < sysdata.level_override_private )
			{
				set_char_color ( AT_WHITE, ch );
				send_to_char ( "The room ahead is private!\r\n", ch );
				return;
			}

			if ( IS_EXIT_FLAG ( pexit, EX_OVERLAND ) )
			{
				original = ch->in_room;
				enter_map ( ch, pexit->x, pexit->y, -1 );
				leave_map ( ch, NULL, original );
			}
			else
			{
				original = ch->in_room;
				char_from_room ( ch );
				char_to_room ( ch, pexit->to_room );
				do_look ( ch, "auto" );
				char_from_room ( ch );
				char_to_room ( ch, original );
			}
		}

		return;
	}
	else if ( door != -1 )
	{
		send_to_char ( "Nothing special there.\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg1 ) ) != NULL )
	{
		show_char_to_char_1 ( victim, ch );
		return;
	}

	/*
	 * finally fixed the annoying look 2.obj desc bug   -Thoric
	 */
	number = number_argument ( arg1, arg );

	for ( cnt = 0, obj = ch->last_carrying; obj; obj = obj->prev_content )
	{
		if ( can_see_obj ( ch, obj ) )
		{
			if ( ( pdesc = get_extra_descr ( arg, obj->first_extradesc ) ) != NULL )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}

			if ( ( pdesc = get_extra_descr ( arg, obj->pIndexData->first_extradesc ) ) != NULL )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}

			if ( nifty_is_name_prefix ( arg, obj->name ) )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				pdesc = get_extra_descr ( obj->name, obj->pIndexData->first_extradesc );

				if ( !pdesc )
					pdesc = get_extra_descr ( obj->name, obj->first_extradesc );

				if ( !pdesc )
					send_to_char_color ( "You see nothing special.\r\n", ch );
				else
					send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}
		}
	}

	for ( obj = ch->in_room->last_content; obj; obj = obj->prev_content )
	{
		if ( can_see_obj ( ch, obj ) )
		{
			if ( ( pdesc = get_extra_descr ( arg, obj->first_extradesc ) ) != NULL )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}

			if ( ( pdesc = get_extra_descr ( arg, obj->pIndexData->first_extradesc ) ) != NULL )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}

			if ( nifty_is_name_prefix ( arg, obj->name ) )
			{
				if ( ( cnt += obj->count ) < number )
					continue;

				pdesc = get_extra_descr ( obj->name, obj->pIndexData->first_extradesc );

				if ( !pdesc )
					pdesc = get_extra_descr ( obj->name, obj->first_extradesc );

				if ( !pdesc )
					send_to_char ( "You see nothing special.\r\n", ch );
				else
					send_to_char_color ( pdesc, ch );

				if ( EXA_prog_trigger )
					oprog_examine_trigger ( ch, obj );

				return;
			}
		}
	}

	send_to_char ( "You do not see that here.\r\n", ch );

	return;
}

void show_race_line ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	char buf[MAX_STRING_LENGTH];
	int feet, inches;

	if ( !IS_NPC ( victim ) && ( victim != ch ) )
	{
		feet = victim->height / 12;
		inches = victim->height % 12;
		sprintf ( buf, "%s is %d'%d\" and weighs %d pounds.\r\n", PERS ( victim, ch, FALSE ), feet, inches, victim->weight );
		send_to_char ( buf, ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim == ch ) )
	{
		feet = victim->height / 12;
		inches = victim->height % 12;
		sprintf ( buf, "You are %d'%d\" and weigh %d pounds.\r\n", feet, inches, victim->weight );
		send_to_char ( buf, ch );
		return;
	}
}

void show_condition ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	char buf[MAX_STRING_LENGTH];
	int percent;

	if ( victim->max_hit > 0 )
		percent = ( 100 * victim->hit ) / victim->max_hit;
	else
		percent = -1;

	if ( victim != ch )
	{
		strcpy ( buf, PERS ( victim, ch, FALSE ) );

		if ( percent >= 100 )
			strcat ( buf, " is in perfect health.\r\n" );
		else if ( percent >= 90 )
			strcat ( buf, " is slightly scratched.\r\n" );
		else if ( percent >= 80 )
			strcat ( buf, " has a few bruises.\r\n" );
		else if ( percent >= 70 )
			strcat ( buf, " has some cuts.\r\n" );
		else if ( percent >= 60 )
			strcat ( buf, " has several wounds.\r\n" );
		else if ( percent >= 50 )
			strcat ( buf, " has many nasty wounds.\r\n" );
		else if ( percent >= 40 )
			strcat ( buf, " is bleeding freely.\r\n" );
		else if ( percent >= 30 )
			strcat ( buf, " is covered in blood.\r\n" );
		else if ( percent >= 20 )
			strcat ( buf, " is leaking guts.\r\n" );
		else if ( percent >= 10 )
			strcat ( buf, " is almost dead.\r\n" );
		else
			strcat ( buf, " is DYING.\r\n" );
	}
	else
	{
		strcpy ( buf, "You" );

		if ( percent >= 100 )
			strcat ( buf, " are in perfect health.\r\n" );
		else if ( percent >= 90 )
			strcat ( buf, " are slightly scratched.\r\n" );
		else if ( percent >= 80 )
			strcat ( buf, " have a few bruises.\r\n" );
		else if ( percent >= 70 )
			strcat ( buf, " have some cuts.\r\n" );
		else if ( percent >= 60 )
			strcat ( buf, " have several wounds.\r\n" );
		else if ( percent >= 50 )
			strcat ( buf, " have many nasty wounds.\r\n" );
		else if ( percent >= 40 )
			strcat ( buf, " are bleeding freely.\r\n" );
		else if ( percent >= 30 )
			strcat ( buf, " are covered in blood.\r\n" );
		else if ( percent >= 20 )
			strcat ( buf, " are leaking guts.\r\n" );
		else if ( percent >= 10 )
			strcat ( buf, " are almost dead.\r\n" );
		else
			strcat ( buf, " are DYING.\r\n" );
	}

	buf[0] = UPPER ( buf[0] );

	send_to_char ( buf, ch );
	return;
}

/* A much simpler version of look, this function will show you only
the condition of a mob or pc, or if used without an argument, the
same you would see if you enter the room and have config +brief.
-- Narn, winter '96
*/
void do_glance ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool brief;

	if ( !ch->desc )
		return;

	if ( ch->position < POS_SLEEPING )
	{
		send_to_char ( "You can't see anything but stars!\r\n", ch );
		return;
	}

	if ( ch->position == POS_SLEEPING )
	{
		send_to_char ( "You can't see anything, you're sleeping!\r\n", ch );
		return;
	}

	if ( !check_blind ( ch ) )
		return;

	set_char_color ( AT_ACTION, ch );

	argument = one_argument ( argument, arg1 );

	if ( arg1[0] == STRING_NULL )
	{
		if ( xIS_SET ( ch->act, PLR_BRIEF ) )
			brief = TRUE;
		else
			brief = FALSE;

		xSET_BIT ( ch->act, PLR_BRIEF );

		do_look ( ch, "auto" );

		if ( !brief )
			xREMOVE_BIT ( ch->act, PLR_BRIEF );

		return;
	}

	if ( ( victim = get_char_room ( ch, arg1 ) ) == NULL )
	{
		send_to_char ( "They're not here.\r\n", ch );
		return;
	}
	else
	{
		if ( can_see ( victim, ch, FALSE ) )
		{
			act ( AT_ACTION, "$n glances at you.", ch, NULL, victim, TO_VICT );
			act ( AT_ACTION, "$n glances at $N.", ch, NULL, victim, TO_NOTVICT );
		}

		if ( IS_IMMORTAL ( ch ) && victim != ch )
		{
			if ( IS_NPC ( victim ) )
				ch_printf ( ch, "Mobile #%d '%s' ", victim->pIndexData->vnum, victim->name );
			else
				ch_printf ( ch, "%s ", victim->name );

			ch_printf ( ch, "is a level %d %s of the %s clan.\r\n",
			            victim->level,
		            IS_NPC ( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ? npc_Class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS
			            && Class_table[victim->Class]->who_name && Class_table[victim->Class]->who_name[0] != STRING_NULL ? Class_table[victim->Class]->who_name :
			            "unknown",
		            IS_NPC ( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ? npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE
			            && race_table[victim->race]->race_name && race_table[victim->race]->race_name[0] != STRING_NULL ? race_table[victim->race]->race_name :
			            "unknown" );

		}

		show_condition ( ch, victim );

		return;
	}

	return;
}

void do_examine ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	short dam;

	if ( !argument )
	{
		bug ( "do_examine: null argument.", 0 );
		return;
	}

	if ( !ch )
	{
		bug ( "do_examine: null ch.", 0 );
		return;
	}

	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Examine what?\r\n", ch );
		return;
	}

	EXA_prog_trigger = FALSE;

	do_look ( ch, arg );
	EXA_prog_trigger = TRUE;
	/*
	 * Support for looking at boards, checking equipment conditions,
	 * and support for trigger positions by Thoric
	 */

	if ( ( obj = get_obj_here ( ch, arg ) ) != NULL )
	{
		switch ( obj->item_type )
		{

			default:
				break;

			case ITEM_ARMOR:

				if ( obj->value[1] == 0 )
					obj->value[1] = obj->value[0];

				if ( obj->value[1] == 0 )
					obj->value[1] = 1;

				dam = ( short ) ( ( obj->value[0] * 10 ) / obj->value[1] );

				strcpy ( buf, "As you look more closely, you notice that it is " );

				if ( dam >= 10 )
					strcat ( buf, "in superb condition." );
				else if ( dam == 9 )
					strcat ( buf, "in very good condition." );
				else if ( dam == 8 )
					strcat ( buf, "in good shape." );
				else if ( dam == 7 )
					strcat ( buf, "showing a bit of wear." );
				else if ( dam == 6 )
					strcat ( buf, "a little run down." );
				else if ( dam == 5 )
					strcat ( buf, "in need of repair." );
				else if ( dam == 4 )
					strcat ( buf, "in great need of repair." );
				else if ( dam == 3 )
					strcat ( buf, "in dire need of repair." );
				else if ( dam == 2 )
					strcat ( buf, "very badly worn." );
				else if ( dam == 1 )
					strcat ( buf, "practically worthless." );
				else if ( dam <= 0 )
					strcat ( buf, "broken." );

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );

				break;

			case ITEM_WEAPON:
				dam = INIT_WEAPON_CONDITION - obj->value[0];

				strcpy ( buf, "As you look more closely, you notice that it is " );

				if ( dam == 0 )
					strcat ( buf, "in superb condition." );
				else if ( dam == 1 )
					strcat ( buf, "in excellent condition." );
				else if ( dam == 2 )
					strcat ( buf, "in very good condition." );
				else if ( dam == 3 )
					strcat ( buf, "in good shape." );
				else if ( dam == 4 )
					strcat ( buf, "showing a bit of wear." );
				else if ( dam == 5 )
					strcat ( buf, "a little run down." );
				else if ( dam == 6 )
					strcat ( buf, "in need of repair." );
				else if ( dam == 7 )
					strcat ( buf, "in great need of repair." );
				else if ( dam == 8 )
					strcat ( buf, "in dire need of repair." );
				else if ( dam == 9 )
					strcat ( buf, "very badly worn." );
				else if ( dam == 10 )
					strcat ( buf, "practically worthless." );
				else if ( dam == 11 )
					strcat ( buf, "almost broken." );
				else if ( dam == 12 )
					strcat ( buf, "broken." );

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );

				break;

			case ITEM_COOK:
				strcpy ( buf, "As you examine it carefully you notice that it " );

				dam = obj->value[2];

				if ( dam >= 3 )
					strcat ( buf, "is burned to a crisp." );
				else if ( dam == 2 )
					strcat ( buf, "is a little over cooked." );
				else if ( dam == 1 )
					strcat ( buf, "is perfectly roasted." );
				else
					strcat ( buf, "is raw." );

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );

			case ITEM_FOOD:
				if ( obj->timer > 0 && obj->value[1] > 0 )
					dam = ( obj->timer * 10 ) / obj->value[1];
				else
					dam = 10;

				if ( obj->item_type == ITEM_FOOD )
					strcpy ( buf, "As you examine it carefully you notice that it " );
				else
					strcpy ( buf, "Also it " );

				if ( dam >= 10 )
					strcat ( buf, "is fresh." );
				else if ( dam == 9 )
					strcat ( buf, "is nearly fresh." );
				else if ( dam == 8 )
					strcat ( buf, "is perfectly fine." );
				else if ( dam == 7 )
					strcat ( buf, "looks good." );
				else if ( dam == 6 )
					strcat ( buf, "looks ok." );
				else if ( dam == 5 )
					strcat ( buf, "is a little stale." );
				else if ( dam == 4 )
					strcat ( buf, "is a bit stale." );
				else if ( dam == 3 )
					strcat ( buf, "smells slightly off." );
				else if ( dam == 2 )
					strcat ( buf, "smells quite rank." );
				else if ( dam == 1 )
					strcat ( buf, "smells revolting!" );
				else if ( dam <= 0 )
					strcat ( buf, "is crawling with maggots!" );

				strcat ( buf, "\r\n" );

				send_to_char ( buf, ch );

				break;

			case ITEM_SWITCH:

			case ITEM_LEVER:

			case ITEM_PULLCHAIN:
				if ( IS_SET ( obj->value[0], TRIG_UP ) )
					send_to_char ( "You notice that it is in the up position.\r\n", ch );
				else
					send_to_char ( "You notice that it is in the down position.\r\n", ch );

				break;

			case ITEM_BUTTON:
				if ( IS_SET ( obj->value[0], TRIG_UP ) )
					send_to_char ( "You notice that it is depressed.\r\n", ch );
				else
					send_to_char ( "You notice that it is not depressed.\r\n", ch );

				break;

			case ITEM_CORPSE_PC:

			case ITEM_CORPSE_NPC:
			{
				short timerfrac = obj->timer;

				if ( obj->item_type == ITEM_CORPSE_PC )
					timerfrac = ( int ) obj->timer / 8 + 1;

				switch ( timerfrac )
				{

					default:
						send_to_char ( "This corpse has recently been slain.\r\n", ch );
						break;

					case 4:
						send_to_char ( "This corpse was slain a little while ago.\r\n", ch );
						break;

					case 3:
						send_to_char ( "A foul smell rises from the corpse, and it is covered in flies.\r\n", ch );
						break;

					case 2:
						send_to_char ( "A writhing mass of maggots and decay, you can barely go near this corpse.\r\n", ch );
						break;

					case 1:

					case 0:
						send_to_char ( "Little more than bones, there isn't much left of this corpse.\r\n", ch );
						break;
				}
			}

			case ITEM_CONTAINER:

				if ( IS_OBJ_STAT ( obj, ITEM_COVERING ) )
					break;

			case ITEM_DRINK_CON:

			case ITEM_QUIVER:
				send_to_char ( "When you look inside, you see:\r\n", ch );

			case ITEM_KEYRING:
				EXA_prog_trigger = FALSE;

				sprintf ( buf, "in %s", arg );

				do_look ( ch, buf );

				EXA_prog_trigger = TRUE;

				break;
		}

		if ( IS_OBJ_STAT ( obj, ITEM_COVERING ) )
		{
			EXA_prog_trigger = FALSE;
			sprintf ( buf, "under %s", arg );
			do_look ( ch, buf );
			EXA_prog_trigger = TRUE;
		}

		oprog_examine_trigger ( ch, obj );

		if ( char_died ( ch ) || obj_extracted ( obj ) )
			return;
	}
	return;
}

void do_exits ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	EXIT_DATA *pexit;
	bool found, closed = FALSE, locked = FALSE, DT = FALSE;
	bool fAuto;
	int spaces;
	set_char_color ( AT_EXITS, ch );
	fAuto = !str_cmp ( argument, "auto" );

	if ( !check_blind ( ch ) )
		return;

	strcpy ( buf, fAuto ? "&WExits:" : "&WObvious exits:\r\n" );

	found = FALSE;

	for ( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
	{
		if ( pexit->to_room
		        && !IS_SET ( pexit->exit_info, EX_SECRET )
		        && ( !IS_SET ( pexit->exit_info, EX_WINDOW ) || IS_SET ( pexit->exit_info, EX_ISDOOR ) ) && !IS_SET ( pexit->exit_info, EX_HIDDEN ) )
		{
			found = TRUE;
			closed = IS_SET ( pexit->exit_info, EX_CLOSED );
			locked = ( IS_SET ( pexit->exit_info, EX_LOCKED ) || IS_SET ( pexit->exit_info, EX_BOLTED ) );
			DT = ( ( IS_IMMORTAL ( ch ) || IS_AFFECTED ( ch, AFF_DETECTTRAPS ) ) && xIS_SET ( pexit->to_room->room_flags, ROOM_DEATH ) );

			if ( fAuto )
			{
				sprintf ( buf, "%s %s" "%s" "&W" "%s", buf,
				          DT ? "&R***" : !closed ? "" : locked ? "&R[&W" : "[",
				          DT ? strupper ( dir_name[pexit->vdir] ) : dir_name[pexit->vdir], DT ? "&R***&W" : !closed ? "" : locked ? "&R]&W" : "]" );
			}
			else
			{
				/*
				 * I don't want to underline spaces, so I'll calculate the number we need
				 */
				spaces = 5 - strlen ( dir_name[pexit->vdir] );

				if ( spaces < 0 )
					spaces = 0;

				sprintf ( buf + strlen ( buf ), "%s" "%*s - %s\r\n", capitalize ( dir_name[pexit->vdir] ), spaces, /* number of spaces */
				          "",
				          DT ? "&RYou sense an aura of imminent doom this way&W" : locked ? "&R[&WClosed and Locked/Barred&R]&W" :
				          closed ? "[Closed]" : room_is_dark ( pexit->to_room ) ? "Too dark to tell" : pexit->to_room->name );
			}
		}
	}

	if ( !found )
		strcat ( buf, fAuto ? " none.\r\n" : "None.\r\n" );
	else if ( fAuto )
		strcat ( buf, ".\r\n" );

	send_to_char ( buf, ch );

	return;
}

char *const day_name[] =
{
	"The Jade Emperor", "The Amber Tiger Lord", "The Onyx Dragon King", "The Emerald Prince", "The Obsidian Guardian",
	"The Saphire Princess", "The Jasper Buddha"
};
char *const month_name[] =
{
	"Tiger", "Rabbit", "Dragon",  //Nature
	"Snake", "Horse", "Sheep",     //Fire
	"Monkey", "Rooster", "Dog",   //Air
	"Pig", "Rat", "Ox"            //Water
};
void do_time ( CHAR_DATA * ch, char *argument )
{
	extern char str_boot_time[];
	char *suf;
	int day;
	day = time_info.day + 1;

	if ( day > 4 && day < 20 )
		suf = "th";
	else if ( day % 10 == 1 )
		suf = "st";
	else if ( day % 10 == 2 )
		suf = "nd";
	else if ( day % 10 == 3 )
		suf = "rd";
	else
		suf = "th";

	set_char_color ( AT_TIME, ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( " &cCurrent Game Time                                                              \r\n", ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	ch_printf ( ch,"&D Time:&w %d o'clock %s \r\n", ( time_info.hour % 12 == 0 ) ? 12 : time_info.hour % 12, time_info.hour >= 12 ? "pm" : "am" );
	ch_printf ( ch,"&D Day : &wThe Day of %s \r\n", day_name[day % 7] );
	ch_printf ( ch,"&D       &w%d%s day in the month of %ss\r\n", day, suf, month_name[time_info.month] );
	ch_printf ( ch,"&D Year: &w%d.\r\n", time_info.year );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	ch_printf ( ch,"&D The mud started up at:    &w%s\r", str_boot_time );
	ch_printf ( ch,"&D The system time (E.S.T.): &w%s\r", ( char * ) ctime ( &current_time ) );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	if ( sysdata.CLEANPFILES )
		ch_printf ( ch, "Next pfile cleanup is scheduled for: %s\r\n", ( char * ) ctime ( &new_pfile_time_t ) );

	return;
}

/*
 * Produce a description of the weather based on area weather using
 * the following sentence format:
 *  <combo-phrase> and <single-phrase>.
 * Where the combo-phrase describes either the precipitation and
 * temperature or the wind and temperature. The single-phrase
 * describes either the wind or precipitation depending upon the
 * combo-phrase.
 * Last Modified: July 31, 1997
 * Fireblade - Under Construction
 */
void do_weather ( CHAR_DATA * ch, char *argument )
{
	char *combo, *single;
	char buf[MAX_INPUT_LENGTH];
	int temp, precip, wind;

	if ( !IS_PLR_FLAG ( ch, PLR_ONMAP ) )
	{
		if ( !xIS_OUTSIDE ( ch ) || INDOOR_SECTOR ( ch->in_room->sector_type ) )
		{
			send_to_char ( "You can't see the sky from here.\r\n", ch );
			return;
		}
	}

	if ( !xIS_OUTSIDE ( ch ) )
	{
		ch_printf ( ch, "You can't see the sky from here.\r\n" );
		return;
	}

	temp = ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit;

	precip = ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit;
	wind = ( ch->in_room->area->weather->wind + 3 * weath_unit - 1 ) / weath_unit;

	if ( precip >= 3 )
	{
		combo = preciptemp_msg[precip][temp];
		single = wind_msg[wind];
	}
	else
	{
		combo = windtemp_msg[wind][temp];
		single = precip_msg[precip];
	}

	sprintf ( buf, "%s and %s.\r\n", combo, single );

	set_char_color ( AT_BLUE, ch );
	ch_printf ( ch, "%s", buf );
}

/*
 * Moved into a separate function so it can be used for other things
 * ie: online help editing    -Thoric
 */
HELP_DATA *get_help ( CHAR_DATA * ch, char *argument )
{
	char argall[MAX_INPUT_LENGTH];
	char argone[MAX_INPUT_LENGTH];
	char argnew[MAX_INPUT_LENGTH];
	HELP_DATA *pHelp;
	int lev;

	if ( argument[0] == STRING_NULL )
		argument = "summary";

	if ( isdigit ( argument[0] ) )
	{
		lev = number_argument ( argument, argnew );
		argument = argnew;
	}
	else
		lev = -2;

	/*
	 * Tricky argument handling so 'help a b' doesn't match a.
	 */
	argall[0] = STRING_NULL;

	while ( argument[0] != STRING_NULL )
	{
		argument = one_argument ( argument, argone );

		if ( argall[0] != STRING_NULL )
			strcat ( argall, " " );

		strcat ( argall, argone );
	}

	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		if ( pHelp->level > get_trust ( ch ) )
			continue;

		if ( lev != -2 && pHelp->level != lev )
			continue;

		if ( is_name ( argall, pHelp->keyword ) )
			return pHelp;
	}

	return NULL;
}

/*
 * LAWS command
 */
void do_laws ( CHAR_DATA * ch, char *argument )
{
	char buf[1024];

	if ( argument == NULL )
		do_help ( ch, "laws" );
	else
	{
		sprintf ( buf, "law %s", argument );
		do_help ( ch, buf );
	}
}

/*  Ranks by number of matches between two whole words. Coded for the Similar Helpfiles
  Snippet by Senir. */
short str_similarity ( const char *astr, const char *bstr )
{
	short matches = 0;

	if ( !astr || !bstr )
		return matches;

	for ( ; *astr; astr++ )
	{
		if ( LOWER ( *astr ) == LOWER ( *bstr ) )
			matches++;

		if ( ++bstr == STRING_NULL )
			return matches;
	}

	return matches;
}

/*  Ranks by number of matches until there's a nonmatching character between two words.
  Coded for the Similar Helpfiles Snippet by Senir.*/
short str_prefix_level ( const char *astr, const char *bstr )
{
	short matches = 0;

	if ( !astr || !bstr )
		return matches;

	for ( ; *astr; astr++ )
	{
		if ( LOWER ( *astr ) == LOWER ( *bstr ) )
			matches++;
		else
			return matches;

		if ( ++bstr == STRING_NULL )
			return matches;
	}

	return matches;
}

/* Main function of Similar Helpfiles Snippet by Senir. It loops through all of the
 helpfiles, using the string matching function defined to find the closest matching
 helpfiles to the argument. It then checks for singles. Then, if matching helpfiles
 are found at all, it loops through and prints out the closest matching helpfiles.
 If its a single(there's only one), it opens the helpfile.*/
void similar_help_files ( CHAR_DATA * ch, char *argument )
{
	HELP_DATA *pHelp = NULL;
	char buf[MAX_STRING_LENGTH];
	char *extension;
	short lvl = 0;
	bool single = FALSE;
	send_to_pager_color ( "&C&BSimilar Help Files:\r\n", ch );

	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		buf[0] = STRING_NULL;
		extension = pHelp->keyword;

		if ( pHelp->level > get_trust ( ch ) )
			continue;

		while ( extension[0] != STRING_NULL )
		{
			extension = one_argument ( extension, buf );

			if ( str_similarity ( argument, buf ) > lvl )
			{
				lvl = str_similarity ( argument, buf );
				single = TRUE;
			}
			else if ( str_similarity ( argument, buf ) == lvl && lvl > 0 )
			{
				single = FALSE;
			}
		}
	}

	if ( lvl == 0 )
	{
		send_to_pager_color ( "&C&GNo similar help files.\r\n", ch );
		return;
	}

	for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
	{
		buf[0] = STRING_NULL;
		extension = pHelp->keyword;

		while ( extension[0] != STRING_NULL )
		{
			extension = one_argument ( extension, buf );

			if ( str_similarity ( argument, buf ) >= lvl && pHelp->level <= get_trust ( ch ) )
			{
				if ( single )
				{
					send_to_pager_color ( "&C&GOpening only similar helpfile.&C\r\n", ch );
					do_help ( ch, buf );
					return;
				}

				pager_printf_color ( ch, "&C&G   %s\r\n", pHelp->keyword );

				break;
			}
		}
	}

	return;
}

/*
 * Now this is cleaner
 */
void do_help ( CHAR_DATA * ch, char *argument )
{
	HELP_DATA *pHelp;
	set_pager_color ( AT_HELP, ch );

	if ( ( pHelp = get_help ( ch, argument ) ) == NULL )
	{
		/*
		 * Looks better printing out the missed argument before going to similar helpfiles. - Senir
		 */
		pager_printf_color ( ch, "&CNo help on \'%s\' found.&D\r\n", argument );
		similar_help_files ( ch, argument );
		return;
	}

	/*
	 * Make newbies do a help start. --Shaddai
	 */
	if ( !IS_NPC ( ch ) && !str_cmp ( argument, "start" ) )
		SET_BIT ( ch->pcdata->flags, PCFLAG_HELPSTART );

	set_pager_color ( AT_HELP, ch );

	if ( pHelp->level >= 0 && str_cmp ( argument, "imotd" ) )
	{
		send_to_pager ( "--------------------------------------------------------------------------------\r\n", ch );
		pager_printf_color ( ch, "&R %s &D\r\n", pHelp->keyword );
		send_to_pager ( "--------------------------------------------------------------------------------\r\n", ch );
		/*
		 * Strip leading '.' to allow initial blanks.
		 */

		if ( pHelp->text[0] == '.' )
			send_to_pager_color ( pHelp->text + 1, ch );
		else
			send_to_pager_color ( pHelp->text, ch );

		send_to_pager ( "--------------------------------------------------------------------------------&D\r\n", ch );
		return;
	}
	else if ( pHelp->text[0] == '.' )
		send_to_pager_color ( pHelp->text + 1, ch );
	else
		send_to_pager_color ( pHelp->text, ch );
	return;
}

void do_news ( CHAR_DATA * ch, char *argument )
{
	set_pager_color ( AT_NOTE, ch );
	do_help ( ch, "news" );
}

extern char *help_greeting;   /* so we can edit the greeting online */
/*
 * Help editor       -Thoric
 */
void do_hedit ( CHAR_DATA * ch, char *argument )
{
	HELP_DATA *pHelp;

	if ( !ch->desc )
	{
		send_to_char ( "You have no descriptor.\r\n", ch );
		return;
	}

	switch ( ch->substate )
	{

		default:
			break;

		case SUB_HELP_EDIT:

			if ( ( pHelp = ch->dest_buf ) == NULL )
			{
				bug ( "hedit: sub_help_edit: NULL ch->dest_buf", 0 );
				stop_editing ( ch );
				return;
			}

			if ( help_greeting == pHelp->text )
				help_greeting = NULL;

			STRFREE ( pHelp->text );

			pHelp->text = copy_buffer ( ch );

			if ( !help_greeting )
				help_greeting = pHelp->text;

			stop_editing ( ch );

			return;
	}

	if ( ( pHelp = get_help ( ch, argument ) ) == NULL ) /* new help */
	{
		HELP_DATA *tHelp;
		char argnew[MAX_INPUT_LENGTH];
		int lev;
		bool new_help = TRUE;

		for ( tHelp = first_help; tHelp; tHelp = tHelp->next )
			if ( !str_cmp ( argument, tHelp->keyword ) )
			{
				pHelp = tHelp;
				new_help = FALSE;
				break;
			}

		if ( new_help )
		{
			if ( isdigit ( argument[0] ) )
			{
				lev = number_argument ( argument, argnew );
				argument = argnew;
			}
			else
				lev = get_trust ( ch );

			CREATE ( pHelp, HELP_DATA, 1 );

			pHelp->keyword = STRALLOC ( strupper ( argument ) );

			pHelp->text = STRALLOC ( "" );

			pHelp->level = lev;

			add_help ( pHelp );
		}
	}

	ch->substate = SUB_HELP_EDIT;

	ch->dest_buf = pHelp;
	start_editing ( ch, pHelp->text );
}

/*
 * Stupid leading space muncher fix    -Thoric
 */
char *help_fix ( char *text )
{
	char *fixed;

	if ( !text )
		return "";

	fixed = strip_cr ( text );

	if ( fixed[0] == ' ' )
		fixed[0] = '.';

	return fixed;
}

void do_hset ( CHAR_DATA * ch, char *argument )
{
	HELP_DATA *pHelp;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	smash_tilde ( argument );
	argument = one_argument ( argument, arg1 );

	if ( arg1[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: hset <field> [value] [help page]\r\n", ch );
		send_to_char ( "\r\n", ch );
		send_to_char ( "Field being one of:\r\n", ch );
		send_to_char ( "  level keyword remove save\r\n", ch );
		return;
	}

	if ( !str_cmp ( arg1, "save" ) )
	{
		FILE *fpout;
		log_string_plus ( "Saving help.are...", LOG_NORMAL, LEVEL_GREATER );
		rename ( "help.are", "help.are.bak" );
		fclose ( fpReserve );

		if ( ( fpout = fopen ( "help.are", "w" ) ) == NULL )
		{
			bug ( "hset save: fopen", 0 );
			perror ( "help.are" );
			fpReserve = fopen ( NULL_FILE, "r" );
			return;
		}

		fprintf ( fpout, "#HELPS\n\n" );

		for ( pHelp = first_help; pHelp; pHelp = pHelp->next )
			fprintf ( fpout, "%d %s~\n%s~\n\n", pHelp->level, pHelp->keyword, help_fix ( pHelp->text ) );

		fprintf ( fpout, "0 $~\n\n\n#$\n" );

		fclose ( fpout );

		fpReserve = fopen ( NULL_FILE, "r" );

		send_to_char ( "Saved.\r\n", ch );

		return;
	}

	if ( str_cmp ( arg1, "remove" ) )
		argument = one_argument ( argument, arg2 );

	if ( ( pHelp = get_help ( ch, argument ) ) == NULL )
	{
		send_to_char ( "Cannot find help on that subject.\r\n", ch );
		return;
	}

	if ( !str_cmp ( arg1, "remove" ) )
	{
		UNLINK ( pHelp, first_help, last_help, next, prev );
		STRFREE ( pHelp->text );
		STRFREE ( pHelp->keyword );
		DISPOSE ( pHelp );
		send_to_char ( "Removed.\r\n", ch );
		return;
	}

	if ( !str_cmp ( arg1, "level" ) )
	{
		pHelp->level = atoi ( arg2 );
		send_to_char ( "Done.\r\n", ch );
		return;
	}

	if ( !str_cmp ( arg1, "keyword" ) )
	{
		STRFREE ( pHelp->keyword );
		pHelp->keyword = STRALLOC ( strupper ( arg2 ) );
		send_to_char ( "Done.\r\n", ch );
		return;
	}

	do_hset ( ch, "" );
}

void do_hl ( CHAR_DATA * ch, char *argument )
{
	send_to_char ( "If you want to use HLIST, spell it out.\r\n", ch );
	return;
}

/*
 * Show help topics in a level range    -Thoric
 * Idea suggested by Gorog
 * prefix keyword indexing added by Fireblade
 */
void do_hlist ( CHAR_DATA * ch, char *argument )
{
	int min, max, minlimit, maxlimit, cnt;
	char arg[MAX_INPUT_LENGTH];
	HELP_DATA *help;
	bool minfound, maxfound;
	char *idx;
	maxlimit = get_trust ( ch );
	minlimit = maxlimit >= LEVEL_GREATER ? -1 : 0;
	min = minlimit;
	max = maxlimit;
	idx = NULL;
	minfound = FALSE;
	maxfound = FALSE;

	for ( argument = one_argument ( argument, arg ); arg[0] != STRING_NULL; argument = one_argument ( argument, arg ) )
	{
		if ( !isdigit ( arg[0] ) )
		{
			if ( idx )
			{
				set_char_color ( AT_GREEN, ch );
				ch_printf ( ch, "You may only use a single keyword to index the list.\r\n" );
				return;
			}

			idx = STRALLOC ( arg );
		}
		else
		{
			if ( !minfound )
			{
				min = URANGE ( minlimit, atoi ( arg ), maxlimit );
				minfound = TRUE;
			}
			else if ( !maxfound )
			{
				max = URANGE ( minlimit, atoi ( arg ), maxlimit );
				maxfound = TRUE;
			}
			else
			{
				set_char_color ( AT_GREEN, ch );
				ch_printf ( ch, "You may only use two level limits.\r\n" );
				return;
			}
		}
	}

	if ( min > max )
	{
		int temp = min;
		min = max;
		max = temp;
	}

	set_pager_color ( AT_GREEN, ch );

	pager_printf ( ch, "Help Topics in level range %d to %d:\r\n\r\n", min, max );

	for ( cnt = 0, help = first_help; help; help = help->next )
		if ( help->level >= min && help->level <= max && ( !idx || nifty_is_name_prefix ( idx, help->keyword ) ) )
		{
			pager_printf ( ch, "  %3d %s\r\n", help->level, help->keyword );
			++cnt;
		}

	if ( cnt )
		pager_printf ( ch, "\r\n%d pages found.\r\n", cnt );
	else
		send_to_char ( "None found.\r\n", ch );

	if ( idx )
		STRFREE ( idx );

	return;
}

/*
 * New do_who with WHO REQUEST, clan, race and homepage support.  -Thoric
 *
 * Latest version of do_who eliminates redundant code by using linked lists.
 * Shows imms separately, indicates guest and retired immortals.
 * Narn, Oct/96
 *
 * Who group by Altrag, Feb 28/97
 */

struct whogr_s
{

	struct whogr_s *next;

	struct whogr_s *follower;

	struct whogr_s *l_follow;
	DESCRIPTOR_DATA *d;
	int indent;
}

*first_whogr, *last_whogr;

struct whogr_s *find_whogr ( DESCRIPTOR_DATA * d, struct whogr_s *first )
{

	struct whogr_s *whogr, *whogr_t;

	for ( whogr = first; whogr; whogr = whogr->next )
		if ( whogr->d == d )
			return whogr;
		else if ( whogr->follower && ( whogr_t = find_whogr ( d, whogr->follower ) ) )
			return whogr_t;

	return NULL;
}

void indent_whogr ( CHAR_DATA * looker, struct whogr_s *whogr, int ilev )
{
	for ( ; whogr; whogr = whogr->next )
	{
		if ( whogr->follower )
		{
			int nlev = ilev;
			CHAR_DATA *wch = ( whogr->d->original ? whogr->d->original : whogr->d->character );

			if ( can_see ( looker, wch, TRUE ) && !IS_IMMORTAL ( wch ) )
				nlev += 3;

			indent_whogr ( looker, whogr->follower, nlev );
		}

		whogr->indent = ilev;
	}
}

/* This a great big mess to backwards-structure the ->leader character
   fields */
void create_whogr ( CHAR_DATA * looker )
{
	DESCRIPTOR_DATA *d;
	CHAR_DATA *wch;

	struct whogr_s *whogr, *whogr_t;
	int dc = 0, wc = 0;

	while ( ( whogr = first_whogr ) != NULL )
	{
		first_whogr = whogr->next;
		DISPOSE ( whogr );
	}

	first_whogr = last_whogr = NULL;

	/*
	 * Link in the ones without leaders first
	 */

	for ( d = last_descriptor; d; d = d->prev )
	{
		if ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
			continue;

		++dc;

		wch = ( d->original ? d->original : d->character );

		if ( !wch->leader || wch->leader == wch || !wch->leader->desc || IS_NPC ( wch->leader ) || IS_IMMORTAL ( wch ) || IS_IMMORTAL ( wch->leader ) )
		{
			CREATE ( whogr, struct whogr_s, 1 );

			if ( !last_whogr )
				first_whogr = last_whogr = whogr;
			else
			{
				last_whogr->next = whogr;
				last_whogr = whogr;
			}

			whogr->next = NULL;

			whogr->follower = whogr->l_follow = NULL;
			whogr->d = d;
			whogr->indent = 0;
			++wc;
		}
	}

	/*
	 * Now for those who have leaders..
	 */
	while ( wc < dc )
		for ( d = last_descriptor; d; d = d->prev )
		{
			if ( d->connected != CON_PLAYING && d->connected != CON_EDITING )
				continue;

			if ( find_whogr ( d, first_whogr ) )
				continue;

			wch = ( d->original ? d->original : d->character );

			if ( wch->leader && wch->leader != wch && wch->leader->desc &&
			        !IS_NPC ( wch->leader ) && !IS_IMMORTAL ( wch ) && !IS_IMMORTAL ( wch->leader ) && ( whogr_t = find_whogr ( wch->leader->desc, first_whogr ) ) )
			{
				CREATE ( whogr, struct whogr_s, 1 );

				if ( !whogr_t->l_follow )
					whogr_t->follower = whogr_t->l_follow = whogr;
				else
				{
					whogr_t->l_follow->next = whogr;
					whogr_t->l_follow = whogr;
				}

				whogr->next = NULL;

				whogr->follower = whogr->l_follow = NULL;
				whogr->d = d;
				whogr->indent = 0;
				++wc;
			}
		}

	/*
	 * Set up indentation levels
	 */
	indent_whogr ( looker, first_whogr, 0 );

	/*
	 * And now to linear link them..
	 */
	for ( whogr_t = NULL, whogr = first_whogr; whogr; )
		if ( whogr->l_follow )
		{
			whogr->l_follow->next = whogr;
			whogr->l_follow = NULL;

			if ( whogr_t )
				whogr_t->next = whogr = whogr->follower;
			else
				first_whogr = whogr = whogr->follower;
		}
		else
		{
			whogr_t = whogr;
			whogr = whogr->next;
		}
}

void do_who ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char invis_str[MAX_INPUT_LENGTH];
	char char_name[MAX_INPUT_LENGTH];
	char Class_text[MAX_INPUT_LENGTH];

	struct whogr_s *whogr, *whogr_p;
	DESCRIPTOR_DATA *d;
	int iClass, iRace;
	int iLevelLower;
	int iLevelUpper;
	int nNumber;
	int nMatch;
	bool rgfClass[MAX_CLASS];
	bool rgfRace[MAX_RACE];
	bool fClassRestrict;
	bool fRaceRestrict;
	bool fImmortalOnly;
	bool fLeader;
	bool fPkill;
	bool fClanMatch;  /* SB who clan*/
	bool fCouncilMatch;
	bool fDeityMatch;
	bool fGroup;
	CLAN_DATA *pClan = NULL;
	DEITY_DATA *pDeity = NULL;
	FILE *whoout = NULL;
	WHO_DATA *cur_who = NULL;
	WHO_DATA *next_who = NULL;
	WHO_DATA *first_mortal = NULL;
	WHO_DATA *first_imm = NULL;
	/*
	 * Set default arguments.
	 */
	iLevelLower = 0;
	iLevelUpper = MAX_LEVEL;
	fClassRestrict = FALSE;
	fRaceRestrict = FALSE;
	fImmortalOnly = FALSE;
	fPkill = FALSE;
	fCouncilMatch = FALSE;
	fDeityMatch = FALSE;
	fGroup = FALSE;   /* Alty who group */
	fLeader = FALSE;

	for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
		rgfClass[iClass] = FALSE;

	for ( iRace = 0; iRace < MAX_RACE; iRace++ )
		rgfRace[iRace] = FALSE;

	set_char_color ( AT_WHO, ch );
#ifdef REQWHOARG
	/*
	 * The who command must have at least one argument because we often
	 * have up to 500 players on. Too much spam if a player accidentally
	 * types "who" with no arguments.           --Gorog
	 */
	if ( ch && argument[0] == STRING_NULL )
	{
		send_to_pager_color ( "\r\n&DYou must specify at least one argument.\r\nUse 'who 1' to view the entire who list.\r\n", ch );
		return;
	}

#endif
	/*
	 * Parse arguments.
	 */
	nNumber = 0;

	for ( ;; )
	{
		char arg[MAX_STRING_LENGTH];
		argument = one_argument ( argument, arg );

		if ( arg[0] == STRING_NULL )
			break;

		if ( is_number ( arg ) )
		{
			switch ( ++nNumber )
			{

				case 1:
					iLevelLower = atoi ( arg );
					break;

				case 2:
					iLevelUpper = atoi ( arg );
					break;

				default:
					send_to_char ( "&DOnly two level numbers allowed.\r\n", ch );
					return;
			}
		}
		else
		{
			if ( strlen ( arg ) < 3 )
			{
				send_to_char ( "&DArguments must be longer than that.\r\n", ch );
				return;
			}

			/*
			 * Look for Classes to turn on.
			 */
			if ( !str_cmp ( arg, "imm" ) || !str_cmp ( arg, "gods" ) )
				fImmortalOnly = TRUE;
			else if ( !str_cmp ( arg, "leader" ) )
				fLeader = TRUE;
			else if ( ( pClan = get_clan ( arg ) ) )
				fClanMatch = TRUE;
			else if ( ( pDeity = get_deity ( arg ) ) )
				fDeityMatch = TRUE;
			else
			{
				for ( iClass = 0; iClass < MAX_CLASS; iClass++ )
				{
					if ( !str_cmp ( arg, Class_table[iClass]->who_name ) )
					{
						rgfClass[iClass] = TRUE;
						break;
					}
				}

				if ( iClass != MAX_CLASS )
					fClassRestrict = TRUE;

				for ( iRace = 0; iRace < MAX_RACE; iRace++ )
				{
					if ( !str_cmp ( arg, race_table[iRace]->race_name ) )
					{
						rgfRace[iRace] = TRUE;
						break;
					}
				}

				if ( iRace != MAX_RACE )
					fRaceRestrict = TRUE;

				if ( iClass == MAX_CLASS && iRace == MAX_RACE && fClanMatch == FALSE && fDeityMatch == FALSE )
				{
					send_to_char ( "&DThat's not a Class, race, order, guild, council or deity.\r\n", ch );
					return;
				}
			}
		}
	}

	/*
	 * Now find matching chars.
	 */
	nMatch = 0;

	buf[0] = STRING_NULL;
	/*
	 * start from last to first to get it in the proper order
	 */
	if ( fGroup )
	{
		create_whogr ( ch );
		whogr = first_whogr;
		d = whogr->d;
	}
	else
	{
		whogr = NULL;
		d = last_descriptor;
	}

	whogr_p = NULL;

	for ( ; d; whogr_p = whogr, whogr = ( fGroup ? whogr->next : NULL ), d = ( fGroup ? ( whogr ? whogr->d : NULL ) : d->prev ) )
	{
		CHAR_DATA *wch;
		char const *Class;

		if ( ( d->connected != CON_PLAYING && d->connected != CON_EDITING ) || !can_see ( ch, d->character, TRUE ) || d->original )
			continue;

		wch = d->original ? d->original : d->character;

		if ( wch->level < iLevelLower || wch->level > iLevelUpper || ( fPkill && !CAN_PKILL ( wch ) ) || ( fImmortalOnly && wch->level < LEVEL_IMMORTAL ) || ( fClassRestrict && !rgfClass[wch->Class] ) || ( fRaceRestrict && !rgfRace[wch->race] ) || ( fClanMatch && ( pClan != wch->pcdata->clan ) )  /* SB */
		        || ( fDeityMatch && ( pDeity != wch->pcdata->deity ) ) )
			continue;

		if ( fLeader && ! ( wch->pcdata->clan && ( ( wch->pcdata->clan->deity && !str_cmp ( wch->pcdata->clan->deity, wch->name ) )
		                    || ( wch->pcdata->clan->leader && !str_cmp ( wch->pcdata->clan->leader, wch->name ) )
		                    || ( wch->pcdata->clan->number1 && !str_cmp ( wch->pcdata->clan->number1, wch->name ) )
		                    || ( wch->pcdata->clan->number2 && !str_cmp ( wch->pcdata->clan->number2, wch->name ) ) ) ) )
			continue;

		if ( fGroup && !wch->leader && !IS_SET ( wch->pcdata->flags, PCFLAG_GROUPWHO ) && ( !whogr_p || !whogr_p->indent ) )
			continue;
		nMatch++;
		strcpy ( char_name, wch->name );

		sprintf ( Class_text, "&D%-4d", wch->level );
		Class = Class_text;
		switch ( wch->level )
		{

			default:
				break;

			case MAX_LEVEL - 0:
				Class = "&r[&WGM&r]&D";
				break;
			case MAX_LEVEL - 1:
			case MAX_LEVEL - 2:
			case MAX_LEVEL - 3:
			case MAX_LEVEL - 4:
			case MAX_LEVEL - 5:
			case MAX_LEVEL - 6:
			case MAX_LEVEL - 7:
			case MAX_LEVEL - 8:
			case MAX_LEVEL - 9:
			case MAX_LEVEL - 10:
			case MAX_LEVEL - 11:
			case MAX_LEVEL - 12:
			case MAX_LEVEL - 13:
			case MAX_LEVEL - 14:
				Class = "&p[&wGS&p]&D";
				break;
			case MAX_LEVEL - 15:
				Class = "&p[&wAV&p]&D";
				break;
		}

		if ( xIS_SET ( wch->act, PLR_WIZINVIS ) )
			sprintf ( invis_str, "Invis(%d) ", wch->pcdata->wizinvis );
		else
			invis_str[0] = STRING_NULL;

		sprintf ( buf, "&w%-11s &c%-4s   &w%-8s &w%-8s   &rR &w%-2d  &D%-8s%-8s%-9s%-5s&D\r\n",
		          char_name,
		          Class,
		          get_race ( wch ),
		          get_Class ( wch ),
		          wch->remorts,
		          IS_SET ( wch->pcdata->flags, PCFLAG_HELPER ) ? "&R[&WHELPER&R]&D" : "",
		          IS_SET ( wch->pcdata->flags, PCFLAG_CODER ) ? "&R[&WCODER&R]&D" : "",
		          IS_SET ( wch->pcdata->flags, PCFLAG_BUILDER ) ? "&R[&WBUILDER&R]&D" : "",
		          xIS_SET ( wch->act, PLR_AFK ) ? "&R[&WAFK&R]&D" : "" );


		/*
		 * This is where the old code would display the found player to the ch.
		 * What we do instead is put the found data into a linked list
		 */
		/*
		 * First make the structure.
		 */
		CREATE ( cur_who, WHO_DATA, 1 );

		cur_who->text = str_dup ( buf );

		if ( wch->level > LEVEL_AVATAR && IS_IMMORTAL ( wch ) )
			cur_who->type = WT_IMM;
		else
			cur_who->type = WT_MORTAL;

		/*
		 * Then put it into the appropriate list.
		 */
		switch ( cur_who->type )
		{

			case WT_MORTAL:
				cur_who->next = first_mortal;
				first_mortal = cur_who;
				break;

			case WT_IMM:
				cur_who->next = first_imm;
				first_imm = cur_who;
				break;
		}
	}

	/*
	 * Ok, now we have three separate linked lists and what remains is to
	 * * display the information and clean up.
	 */

	send_to_pager ( "&D--------------------------------------------------------------------------------&D\r\n", ch );
	send_to_pager ( "&D    &cName   &D|&cLevel&D| &c Clan&D  | &c Style&D   |&cReborn&D| &cFlags&D     \r\n", ch );
	send_to_pager ( "&D--------------------------------------------------------------------------------&D\r\n", ch );

	for ( cur_who = first_mortal; cur_who; cur_who = next_who )
	{
		if ( !ch )
			fprintf ( whoout, "%s", cur_who->text );
		else
			send_to_pager ( cur_who->text, ch );

		next_who = cur_who->next;
		DISPOSE ( cur_who->text );
		DISPOSE ( cur_who );
	}

	for ( cur_who = first_imm; cur_who; cur_who = next_who )
	{
		if ( !ch )
			fprintf ( whoout, "%s", cur_who->text );
		else
			send_to_pager_color ( cur_who->text, ch );

		next_who = cur_who->next;
		DISPOSE ( cur_who->text );
		DISPOSE ( cur_who );
	}

	send_to_pager ( "--------------------------------------------------------------------------------\r\n", ch );

	if ( !ch )
	{
		fprintf ( whoout, "%d player%s.\r\n", nMatch, nMatch == 1 ? "" : "s" );
		fclose ( whoout );
		return;
	}

	set_char_color ( AT_YELLOW, ch );

	ch_printf ( ch, "%d player%s.\r\n", nMatch, nMatch == 1 ? "" : "s" );
	return;
}

void do_compare ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *msg;
	argument = one_argument ( argument, arg1 );
	argument = one_argument ( argument, arg2 );

	if ( arg1[0] == STRING_NULL )
	{
		send_to_char ( "Compare what to what?\r\n", ch );
		return;
	}

	if ( ( obj1 = get_obj_carry ( ch, arg1 ) ) == NULL )
	{
		send_to_char ( "You do not have that item.\r\n", ch );
		return;
	}

	if ( arg2[0] == STRING_NULL )
	{
		for ( obj2 = ch->first_carrying; obj2; obj2 = obj2->next_content )
		{
			if ( obj2->wear_loc != WEAR_NONE && can_see_obj ( ch, obj2 ) && obj1->item_type == obj2->item_type && ( obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE ) != 0 )
				break;
		}

		if ( !obj2 )
		{
			send_to_char ( "You aren't wearing anything comparable.\r\n", ch );
			return;
		}
	}
	else
	{
		if ( ( obj2 = get_obj_carry ( ch, arg2 ) ) == NULL )
		{
			send_to_char ( "You do not have that item.\r\n", ch );
			return;
		}
	}

	msg = NULL;

	value1 = 0;
	value2 = 0;

	if ( obj1 == obj2 )
	{
		msg = "You compare $p to itself.  It looks about the same.";
	}
	else if ( obj1->item_type != obj2->item_type )
	{
		msg = "You can't compare $p and $P.";
	}
	else
	{
		switch ( obj1->item_type )
		{

			default:
				msg = "You can't compare $p and $P.";
				break;

			case ITEM_ARMOR:
				value1 = obj1->value[0];
				value2 = obj2->value[0];
				break;

			case ITEM_WEAPON:
				value1 = obj1->value[1] + obj1->value[2];
				value2 = obj2->value[1] + obj2->value[2];
				break;
		}
	}

	if ( !msg )
	{
		if ( value1 == value2 )
			msg = "$p and $P look about the same.";
		else if ( value1 > value2 )
			msg = "$p looks better than $P.";
		else
			msg = "$p looks worse than $P.";
	}

	act ( AT_PLAIN, msg, ch, obj1, obj2, TO_CHAR );

	return;
}

void do_where ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA *d;
	bool found;
	one_argument ( argument, arg );

	if ( arg[0] != STRING_NULL
	        && ( victim = get_char_world ( ch, arg ) ) && !IS_NPC ( victim ) && IS_SET ( victim->pcdata->flags, PCFLAG_DND ) && get_trust ( ch ) < get_trust ( victim ) )
	{
		act ( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
		return;
	}

	set_pager_color ( AT_DGREEN, ch );

	if ( arg[0] == STRING_NULL )
	{
		pager_printf ( ch, "&D--------------------------------------------------------------------------------\r\n" );
		pager_printf ( ch, "&cPlayers near you in&w %s:\r\n", ch->in_room->area->name );
		pager_printf ( ch, "&D--------------------------------------------------------------------------------\r\n" );

		found = FALSE;

		for ( d = first_descriptor; d; d = d->next )
			if ( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
			        && ( victim = d->character ) != NULL && !IS_NPC ( victim ) && victim->in_room
			        && victim->in_room->area == ch->in_room->area && can_see ( ch, victim, TRUE )
			        && ( get_trust ( ch ) >= get_trust ( victim ) || !IS_SET ( victim->pcdata->flags, PCFLAG_DND ) ) )  /* if victim has the DND flag ch must outrank them */
			{
				found = TRUE;
				pager_printf_color ( ch, "&P%-13s", victim->name );
				if ( CAN_PKILL ( victim ) && victim->pcdata->clan )
					pager_printf_color ( ch, "%-18s", victim->pcdata->clan->badge );
				else if ( CAN_PKILL ( victim ) )
					send_to_pager_color ( "(&wUnclanned&P)", ch );
				else
					send_to_pager ( "\t", ch );

				pager_printf_color ( ch, "&P%s", victim->in_room->name );
				if ( IS_PLR_FLAG ( victim, PLR_ONMAP ) )
					pager_printf ( ch, "        Coords: %dx %dy\r\n", victim->x, victim->y );
				else
					pager_printf ( ch, "\r\n" );
			}

		if ( !found )
			send_to_char ( "None\r\n", ch );
	}
	else
	{
		found = FALSE;

		for ( victim = first_char; victim; victim = victim->next )
			if ( victim->in_room
			        && victim->in_room->area == ch->in_room->area
			        && !IS_AFFECTED ( victim, AFF_HIDE ) && !IS_AFFECTED ( victim, AFF_SNEAK ) && can_see ( ch, victim, TRUE ) && is_name ( arg, victim->name ) )
			{
				found = TRUE;
				pager_printf ( ch, "%-28s %s\r\n", PERS ( victim, ch, TRUE ), victim->in_room->name );
				break;
			}

		if ( !found )
			act ( AT_PLAIN, "You didn't find any $T.", ch, NULL, arg, TO_CHAR );
	}
	pager_printf ( ch, "&D--------------------------------------------------------------------------------\r\n" );

	return;
}

void do_consider ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *msg;
	int diff;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Consider killing whom?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They're not here.\r\n", ch );
		return;
	}

	if ( victim == ch )
	{
		send_to_char ( "You decide you're pretty sure you could take yourself in a fight.\r\n", ch );
		return;
	}

	diff = victim->level - ch->level;

	if ( diff <= -10 )
		msg = "You are far more experienced than $N.";
	else if ( diff <= -5 )
		msg = "$N is not nearly as experienced as you.";
	else if ( diff <= -2 )
		msg = "You are more experienced than $N.";
	else if ( diff <= 1 )
		msg = "You are just about as experienced as $N.";
	else if ( diff <= 4 )
		msg = "You are not nearly as experienced as $N.";
	else if ( diff <= 9 )
		msg = "$N is far more experienced than you!";
	else
		msg = "$N would make a great teacher for you!";

	act ( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

	diff = ( int ) ( victim->max_hit - ch->max_hit ) / 6;

	if ( diff <= -200 )
		msg = "$N looks like a feather!";
	else if ( diff <= -150 )
		msg = "You could kill $N with your hands tied!";
	else if ( diff <= -100 )
		msg = "Hey! Where'd $N go?";
	else if ( diff <= -50 )
		msg = "$N is a wimp.";
	else if ( diff <= 0 )
		msg = "$N looks weaker than you.";
	else if ( diff <= 50 )
		msg = "$N looks about as strong as you.";
	else if ( diff <= 100 )
		msg = "It would take a bit of luck...";
	else if ( diff <= 150 )
		msg = "It would take a lot of luck, and equipment!";
	else if ( diff <= 200 )
		msg = "Why don't you dig a grave for yourself first?";
	else
		msg = "$N is built like a TANK!";

	act ( AT_CONSIDER, msg, ch, NULL, victim, TO_CHAR );

	return;
}

/*
 * Place any skill types you don't want them to be able to practice
 * normally in this list.  Separate each with a space.
 * (Uses an is_name check). -- Altrag
 */
#define CANT_PRAC "Tongue"
void do_practice ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	int sn;

	if ( IS_NPC ( ch ) )
		return;

	if ( argument[0] == STRING_NULL )
	{
		int col;
		short lasttype, cnt;
		col = cnt = 0;
		lasttype = SKILL_SPELL;
		set_pager_color ( AT_PRACTICE, ch );

		for ( sn = 0; sn < top_sn; sn++ )
		{
			if ( !skill_table[sn]->name )
				break;

			if ( strcmp ( skill_table[sn]->name, "reserved" ) == 0 && ( IS_IMMORTAL ( ch ) || CAN_CAST ( ch ) ) )
			{
				if ( col % 3 != 0 )
					send_to_pager ( "\r\n", ch );
				send_to_char ( "&D -------------------------------------------------------------------------------\r\n", ch );
				send_to_char ( " &c Skill and Spell List                                                      &D\r\n", ch );
				send_to_char ( " &D-----------------------------------[&cSpells&D]------------------------------------&D\r\n", ch );
				col = 0;
			}

			if ( skill_table[sn]->type != lasttype )
			{
				if ( !cnt )
				{
					send_to_char ( "                                     (none)\r\n", ch );
				}
				else if ( col % 3 != 0 )
					send_to_pager ( "&D\r\n", ch );

				pager_printf_color ( ch, " &D-----------------------------------[&c%ss&D]-----------------------------------&D\r\n",
				                     skill_tname[skill_table[sn]->type] );

				col = cnt = 0;
			}

			lasttype = skill_table[sn]->type;

			if ( !IS_IMMORTAL ( ch ) && ( skill_table[sn]->guild != CLASS_NONE && ( ch->pcdata->clan->Class != skill_table[sn]->guild ) ) )
				continue;

			if ( ch->level < skill_table[sn]->skill_level[ch->Class] || ( !IS_IMMORTAL ( ch ) && skill_table[sn]->skill_level[ch->Class] == 0 ) )
				continue;

			if ( ch->pcdata->learned[sn] <= 0 && SPELL_FLAG ( skill_table[sn], SF_SECRETSKILL ) )
				continue;

			++cnt;

			pager_printf ( ch, "&D%18.18s&D", skill_table[sn]->name );
			pager_printf ( ch, "&c [&w%3d%%&c]&D", ch->pcdata->learned[sn] );

			if ( ++col % 3 == 0 )
				send_to_pager ( "\r\n", ch );
		}

		if ( col % 3 != 0 )
			send_to_pager ( "&D\r\n", ch );

		send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
		set_pager_color ( AT_GREY, ch );
		pager_printf ( ch, "\r\n&D You have %d practice sessions left.\r\n", ch->practice );
	}
	else
	{
		CHAR_DATA *mob;
		int adept;
		bool can_prac = TRUE;

		if ( !IS_AWAKE ( ch ) )
		{
			send_to_char ( "You are unable to do that while asleep!\r\n", ch );
			return;
		}

		for ( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
			if ( IS_NPC ( mob ) && xIS_SET ( mob->act, ACT_PRACTICE ) )
				break;

		if ( !mob )
		{
			send_to_char ( "You can't do that here.\r\n", ch );
			return;
		}

		if ( ch->practice <= 0 )
		{
			act ( AT_TELL, "$n tells you 'You must earn some more practice sessions.'", mob, NULL, ch, TO_VICT );
			return;
		}

		sn = skill_lookup ( argument );

		if ( ch->pcdata->learned[sn] <= 0 && SPELL_FLAG ( skill_table[sn], SF_SECRETSKILL ) )
		{
			send_to_char ( "$n tells you 'You cannot practice that untill someone teaches you...'", ch );
			return;
		}

		if ( can_prac && ( ( sn == -1 ) || ( !IS_NPC ( ch ) && ch->level < skill_table[sn]->skill_level[ch->Class] ) ) )
		{
			act ( AT_TELL, "$n tells you 'You're not ready to learn that yet...'", mob, NULL, ch, TO_VICT );
			return;
		}

		if ( is_name ( skill_tname[skill_table[sn]->type], CANT_PRAC ) )
		{
			act ( AT_TELL, "$n tells you 'I do not know how to teach that.'", mob, NULL, ch, TO_VICT );
			return;
		}

		/*
		 * Skill requires a special teacher
		 */
		if ( skill_table[sn]->teachers && skill_table[sn]->teachers[0] != STRING_NULL )
		{
			sprintf ( buf, "%d", mob->pIndexData->vnum );

			if ( !is_name ( buf, skill_table[sn]->teachers ) )
			{
				act ( AT_TELL, "$n tells you, 'I know not know how to teach that.'", mob, NULL, ch, TO_VICT );
				return;
			}
		}

		if ( !IS_NPC ( ch ) && skill_table[sn]->guild != CLASS_NONE )
		{
			act ( AT_TELL, "$n tells you 'That is only for members of guilds...'", mob, NULL, ch, TO_VICT );
			return;
		}

		adept = Class_table[ch->Class]->skill_adept * 0.55;

		if ( ch->pcdata->learned[sn] >= adept )
		{
			act ( AT_TELL, "$n, You'll have to practice it on your own now...", ch, NULL, ch, TO_CHAR );
			sprintf ( buf, "$n tells you, 'I've taught you everything I can about %s.'", skill_table[sn]->name );
			act ( AT_TELL, buf, mob, NULL, ch, TO_VICT );
		}
		else
		{
			ch->practice--;
			ch->pcdata->learned[sn] += int_app[get_curr_int ( ch ) ].learn;
			act ( AT_ACTION, "You practice $T.", ch, NULL, skill_table[sn]->name, TO_CHAR );
			act ( AT_ACTION, "$n practices $T.", ch, NULL, skill_table[sn]->name, TO_ROOM );

			if ( ch->pcdata->learned[sn] >= adept )
			{
				ch->pcdata->learned[sn] = adept;
				act ( AT_TELL, "$n tells you. 'You'll have to practice it on your own now...'", mob, NULL, ch, TO_VICT );
			}
		}
	}

	return;
}

void do_wimpy ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int wimpy;
	set_char_color ( AT_YELLOW, ch );
	one_argument ( argument, arg );

	if ( !str_cmp ( arg, "max" ) )
	{
		if ( IS_PKILL ( ch ) )
			wimpy = ( int ) ch->max_hit / 2.25;
		else
			wimpy = ( int ) ch->max_hit / 1.2;
	}
	else if ( arg[0] == STRING_NULL )
		wimpy = ( int ) ch->max_hit / 5;
	else
		wimpy = atoi ( arg );

	if ( wimpy < 0 )
	{
		send_to_char ( "Your courage exceeds your wisdom.\r\n", ch );
		return;
	}

	if ( IS_PKILL ( ch ) && wimpy > ( int ) ch->max_hit / 2.25 )
	{
		send_to_char ( "Such cowardice ill becomes you.\r\n", ch );
		return;
	}
	else if ( wimpy > ( int ) ch->max_hit / 1.2 )
	{
		send_to_char ( "Such cowardice ill becomes you.\r\n", ch );
		return;
	}

	ch->wimpy = wimpy;

	ch_printf ( ch, "Wimpy set to %d hit points.\r\n", wimpy );
	return;
}

void do_password ( CHAR_DATA * ch, char *argument )
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pArg;
	char *pwdnew;
	char *p;
	char cEnd;

	if ( IS_NPC ( ch ) )
		return;

	/*
	 * Can't use one_argument here because it smashes case.
	 * So we just steal all its code.  Bleagh.
	 */
	pArg = arg1;

	while ( isspace ( *argument ) )
		argument++;

	cEnd = ' ';

	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != STRING_NULL )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}

		*pArg++ = *argument++;
	}

	*pArg = STRING_NULL;

	pArg = arg2;

	while ( isspace ( *argument ) )
		argument++;

	cEnd = ' ';

	if ( *argument == '\'' || *argument == '"' )
		cEnd = *argument++;

	while ( *argument != STRING_NULL )
	{
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}

		*pArg++ = *argument++;
	}

	*pArg = STRING_NULL;

	if ( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: password <new> <again>.\r\n", ch );
		send_to_char ( "Syntax: password <new> <again>.\r\n", ch );
		return;
	}

	/*
	 * This should stop all the mistyped password problems --Shaddai
	 */
	if ( strcmp ( arg1, arg2 ) )
	{
		send_to_char ( "Passwords don't match try again.\r\n", ch );
		return;
	}

	if ( strlen ( arg2 ) < 5 )
	{
		send_to_char ( "New password must be at least five characters long.\r\n", ch );
		return;
	}

	if ( arg1[0] == '!' || arg2[0] == '!' )
	{
		send_to_char ( "New password cannot begin with the '!' character.", ch );
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
	pwdnew = sha256_crypt ( arg2 );  /* SHA-256 Encryption */

	for ( p = pwdnew; *p != STRING_NULL; p++ )
	{
		if ( *p == '~' )
		{
			send_to_char ( "New password not acceptable, try again.\r\n", ch );
			return;
		}
	}

	DISPOSE ( ch->pcdata->pwd );

	ch->pcdata->pwd = str_dup ( pwdnew );

	if ( IS_SET ( sysdata.save_flags, SV_PASSCHG ) )
		save_char_obj ( ch );

	if ( ch->desc && ch->desc->host[0] != STRING_NULL )
		sprintf ( log_buf, "%s changing password from site %s\n", ch->name, ch->desc->host );
	else
		sprintf ( log_buf, "%s changing thier password with no descriptor!", ch->name );

	log_string ( log_buf );

	send_to_char ( "Ok.\r\n", ch );

	return;
}

void do_socials ( CHAR_DATA * ch, char *argument )
{
	int iHash;
	int col = 0;
	SOCIALTYPE *social;
	set_pager_color ( AT_PLAIN, ch );

	for ( iHash = 0; iHash < 27; iHash++ )
		for ( social = social_index[iHash]; social; social = social->next )
		{
			pager_printf ( ch, "%-12s", social->name );

			if ( ++col % 6 == 0 )
				send_to_pager ( "\r\n", ch );
		}

	if ( col % 6 != 0 )
		send_to_pager ( "\r\n", ch );

	return;
}

void do_commands ( CHAR_DATA * ch, char *argument )
{
	int col;
	bool found;
	int hash;
	CMDTYPE *command;
	col = 0;
	set_pager_color ( AT_PLAIN, ch );

	if ( argument[0] == STRING_NULL )
	{
		for ( hash = 0; hash < 126; hash++ )
			for ( command = command_hash[hash]; command; command = command->next )
				if ( command->level < LEVEL_HERO && command->level <= get_trust ( ch ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
				{
					pager_printf ( ch, "%-12s", command->name );

					if ( ++col % 6 == 0 )
						send_to_pager ( "\r\n", ch );
				}

		if ( col % 6 != 0 )
			send_to_pager ( "\r\n", ch );
	}
	else
	{
		found = FALSE;

		for ( hash = 0; hash < 126; hash++ )
			for ( command = command_hash[hash]; command; command = command->next )
				if ( command->level < LEVEL_HERO
				        && command->level <= get_trust ( ch ) && !str_prefix ( argument, command->name ) && ( command->name[0] != 'm' || command->name[1] != 'p' ) )
				{
					pager_printf ( ch, "%-12s", command->name );
					found = TRUE;

					if ( ++col % 6 == 0 )
						send_to_pager ( "\r\n", ch );
				}

		if ( col % 6 != 0 )
			send_to_pager ( "\r\n", ch );

		if ( !found )
			ch_printf ( ch, "No command found under %s.\r\n", argument );
	}

	return;
}

void do_channels ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	one_argument ( argument, arg );

	if ( IS_NPC ( ch ) )
		return;

	if ( arg[0] == STRING_NULL )
	{
		if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_SILENCE ) )
		{
			set_char_color ( AT_GREEN, ch );
			send_to_char ( "You are silenced.\r\n", ch );
			return;
		}

		/*
		 * Channels everyone sees regardless of affiliation --Blodkai
		 */
		send_to_char_color ( "\r\n &gPublic channels  (severe penalties for abuse)&G:\r\n  ", ch );

		/*
		 * (    ch_printf_color( ch, "%s",   !IS_SET( ch->deaf, CHANNEL_RACETALK )?
		 * " &G+RACETALK" :
		 * " &g-racetalk" );
		 */
		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_GOSSIP ) ? " &G+GOSSIP" : " &g-gossip" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_OOC ) ? " &G+OOC" : " &g-ooc" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_SAYTO ) ? " &G+SAYTO" : " &g-sayto" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_YELL ) ? " &G+YELL" : " &g-yell" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_NEWBIE ) ? " &G+NEWBIE" : " &g-newbie" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_ANNOUNCE ) ? " &G+ANNOUNCE" : " &g-announce" );

		if ( get_trust ( ch ) > 2 )
			ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_AUCTION ) ? " &G+AUCTION" : " &g-auction" );

		if ( IS_HERO ( ch ) )
			ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_AVTALK ) ? " &G+AVATAR" : " &g-avatar" );

		/*
		 * For organization channels (orders, clans, guilds, councils)
		 */
		send_to_char_color ( "\r\n &gPrivate channels (severe penalties for abuse)&G:\r\n ", ch );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_TELLS ) ? " &G+TELLS" : " &g-tells" );

		ch_printf_color ( ch, "%s", !IS_SET ( ch->deaf, CHANNEL_WHISPER ) ? " &G+WHISPER" : " &g-whisper" );

		if ( !IS_NPC ( ch ) && ch->pcdata->clan )
		{
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_CLAN ) ? " &G+CLAN" : " &g-clan", ch );
		}

		/*
		 * Immortal channels
		 */
		if ( IS_IMMORTAL ( ch ) )
		{
			send_to_char_color ( "\r\n &gImmortal Channels&G:\r\n  ", ch );
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_IMMTALK ) ? " &G+IMMTALK" : " &g-immtalk", ch );

			if ( get_trust ( ch ) >= sysdata.muse_level )
				send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_MONITOR ) ? " &G+MONITOR" : " &g-monitor", ch );
		}

		if ( get_trust ( ch ) >= sysdata.log_level )
		{
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_LOG ) ? " &G+LOG" : " &g-log", ch );
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_BUILD ) ? " &G+BUILD" : " &g-build", ch );
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_COMM ) ? " &G+COMM" : " &g-comm", ch );
			send_to_char_color ( !IS_SET ( ch->deaf, CHANNEL_WARN ) ? " &G+WARN" : " &g-warn", ch );
		}

		send_to_char ( "\r\n", ch );
	}
	else
	{
		bool fClear;
		bool ClearAll;
		int bit;
		bit = 0;
		ClearAll = FALSE;

		if ( arg[0] == '+' )
			fClear = TRUE;
		else if ( arg[0] == '-' )
			fClear = FALSE;
		else
		{
			send_to_char ( "Channels -channel or +channel?\r\n", ch );
			return;
		}

		if ( !str_cmp ( arg + 1, "auction" ) )
			bit = CHANNEL_AUCTION;
		else if ( !str_cmp ( arg + 1, "gossip" ) )
			bit = CHANNEL_GOSSIP;
		else if ( !str_cmp ( arg + 1, "clan" ) )
			bit = CHANNEL_CLAN;
		else if ( !str_cmp ( arg + 1, "ooc" ) )
			bit = CHANNEL_OOC;
		else if ( !str_cmp ( arg + 1, "tells" ) )
			bit = CHANNEL_TELLS;
		else if ( !str_cmp ( arg + 1, "immtalk" ) )
			bit = CHANNEL_IMMTALK;
		else if ( !str_cmp ( arg + 1, "log" ) )
			bit = CHANNEL_LOG;
		else if ( !str_cmp ( arg + 1, "build" ) )
			bit = CHANNEL_BUILD;
		else if ( !str_cmp ( arg + 1, "avatar" ) )
			bit = CHANNEL_AVTALK;
		else if ( !str_cmp ( arg + 1, "monitor" ) )
			bit = CHANNEL_MONITOR;
		else if ( !str_cmp ( arg + 1, "newbie" ) )
			bit = CHANNEL_NEWBIE;
		else if ( !str_cmp ( arg + 1, "yell" ) )
			bit = CHANNEL_YELL;
		else if ( !str_cmp ( arg + 1, "comm" ) )
			bit = CHANNEL_COMM;
		else if ( !str_cmp ( arg + 1, "warn" ) )
			bit = CHANNEL_WARN;
		else if ( !str_cmp ( arg + 1, "sayto" ) )
			bit = CHANNEL_SAYTO;
		else if ( !str_cmp ( arg + 1, "whisper" ) )
			bit = CHANNEL_WHISPER;
		else if ( !str_cmp ( arg + 1, "racetalk" ) )
			bit = CHANNEL_RACETALK;
		else if ( !str_cmp ( arg + 1, "announce" ) )
			bit = CHANNEL_ANNOUNCE;
		else if ( !str_cmp ( arg + 1, "all" ) )
			ClearAll = TRUE;
		else
		{
			send_to_char ( "Set or clear which channel?\r\n", ch );
			return;
		}

		if ( ( fClear ) && ( ClearAll ) )
		{
			REMOVE_BIT ( ch->deaf, CHANNEL_RACETALK );
			REMOVE_BIT ( ch->deaf, CHANNEL_AUCTION );
			REMOVE_BIT ( ch->deaf, CHANNEL_GOSSIP );
			REMOVE_BIT ( ch->deaf, CHANNEL_OOC );
			REMOVE_BIT ( ch->deaf, CHANNEL_YELL );
			REMOVE_BIT ( ch->deaf, CHANNEL_ANNOUNCE );
			if ( ch->pcdata->clan )
				REMOVE_BIT ( ch->deaf, CHANNEL_CLAN );
			if ( ch->level >= LEVEL_IMMORTAL )
				REMOVE_BIT ( ch->deaf, CHANNEL_AVTALK );
		}
		else if ( ( !fClear ) && ( ClearAll ) )
		{
			SET_BIT ( ch->deaf, CHANNEL_RACETALK );
			SET_BIT ( ch->deaf, CHANNEL_AUCTION );
			SET_BIT ( ch->deaf, CHANNEL_GOSSIP );
			SET_BIT ( ch->deaf, CHANNEL_OOC );
			SET_BIT ( ch->deaf, CHANNEL_YELL );
			SET_BIT ( ch->deaf, CHANNEL_ANNOUNCE );
			if ( ch->pcdata->clan )
				SET_BIT ( ch->deaf, CHANNEL_CLAN );
			if ( ch->level >= LEVEL_IMMORTAL )
				SET_BIT ( ch->deaf, CHANNEL_AVTALK );
		}
		else if ( fClear )
		{
			REMOVE_BIT ( ch->deaf, bit );
		}
		else
		{
			SET_BIT ( ch->deaf, bit );
		}

		send_to_char ( "Ok.\r\n", ch );
	}

	return;
}

/*
 * display WIZLIST file      -Thoric
 */
void do_wizlist ( CHAR_DATA * ch, char *argument )
{
	set_pager_color ( AT_IMMORT, ch );
	show_file ( ch, WIZLIST_FILE );
}

/*
 * Contributed by Grodyn.
 * Display completely overhauled, 2/97 -- Blodkai
 */
void do_config ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	if ( IS_NPC ( ch ) )
		return;

	one_argument ( argument, arg );

	set_char_color ( AT_GREEN, ch );

	if ( arg[0] == STRING_NULL )
	{
		set_char_color ( AT_DGREEN, ch );
		send_to_char ( "\r\nConfigurations ", ch );
		set_char_color ( AT_GREEN, ch );
		send_to_char ( "(use 'config +/- <keyword>' to toggle, see 'help config')\r\n\r\n", ch );
		set_char_color ( AT_DGREEN, ch );
		send_to_char ( "Display:   ", ch );
		set_char_color ( AT_GREY, ch );
		ch_printf ( ch, "%-12s   %-12s   %-12s   %-12s\r\n           %-12s   %-12s   %-12s\r\n           %-12s   %-12s",
		            IS_SET ( ch->pcdata->flags, PCFLAG_PAGERON ) ? "[+] PAGER": "[-] pager",
		            IS_SET ( ch->pcdata->flags, PCFLAG_GAG ) ? "[+] GAG": "[-] gag",
		            xIS_SET ( ch->act, PLR_BRIEF ) ? "[+] BRIEF": "[-] brief",
		            xIS_SET ( ch->act, PLR_COMBINE ) ? "[+] COMBINE": "[-] combine",
		            xIS_SET ( ch->act, PLR_BLANK ) ? "[+] BLANK": "[-] blank",
		            xIS_SET ( ch->act, PLR_PROMPT ) ? "[+] PROMPT": "[-] prompt",
		            xIS_SET ( ch->act, PLR_ANSI ) ? "[+] ANSI": "[-] ansi",
		            xIS_SET ( ch->act, PLR_RIP ) ? "[+] RIP": "[-] rip",
		            xIS_SET ( ch->act, PLR_AUTOMAP ) ? "[+] MAP" : "[-] map" );
		set_char_color ( AT_DGREEN, ch );
		send_to_char ( "\r\n\r\nAuto:      ", ch );
		set_char_color ( AT_GREY, ch );
		ch_printf ( ch, "%-12s   %-12s   %-12s   %-12s",
		            xIS_SET ( ch->act, PLR_AUTOSAC ) ? "[+] AUTOSAC": "[-] autosac",
		            xIS_SET ( ch->act, PLR_AUTOGOLD ) ? "[+] AUTOGOLD": "[-] autogold",
		            xIS_SET ( ch->act, PLR_AUTOLOOT ) ? "[+] AUTOLOOT" : "[-] autoloot",
		            xIS_SET ( ch->act, PLR_AUTOEXIT ) ? "[+] AUTOEXIT" : "[-] autoexit" );
		set_char_color ( AT_DGREEN, ch );
		send_to_char ( "\r\n\r\nSafeties:  ", ch );
		set_char_color ( AT_GREY, ch );
		ch_printf ( ch, "%-12s   %-12s",
		            IS_SET ( ch->pcdata->flags, PCFLAG_NORECALL ) ? "[+] NORECALL": "[-] norecall",
		            IS_SET ( ch->pcdata->flags, PCFLAG_NOSUMMON ) ? "[+] NOSUMMON" : "[-] nosummon" );

		if ( !IS_SET ( ch->pcdata->flags, PCFLAG_DEADLY ) )
			ch_printf ( ch, "   %-12s   %-12s",
			            xIS_SET ( ch->act, PLR_SHOVEDRAG ) ? "[+] DRAG" : "[-] drag",
			            xIS_SET ( ch->act, PLR_NICE ) ? "[+] NICE" : "[-] nice" );

		set_char_color ( AT_DGREEN, ch );

		send_to_char ( "\r\n\r\nMisc:      ", ch );

		set_char_color ( AT_GREY, ch );

		ch_printf ( ch, "%-12s   %-12s   %-12s",
		            xIS_SET ( ch->act, PLR_TELNET_GA ) ? "[+] TELNETGA"
		            : "[-] telnetga",
		            IS_SET ( ch->pcdata->flags, PCFLAG_GROUPWHO ) ? "[+] GROUPWHO" : "[-] groupwho", IS_SET ( ch->pcdata->flags, PCFLAG_NOINTRO ) ? "[+] NOINTRO" : "[-] nointro" );

		set_char_color ( AT_DGREEN, ch );

		send_to_char ( "\r\n\r\nSettings:  ", ch );

		set_char_color ( AT_GREY, ch );

		ch_printf_color ( ch, "Pager Length (%d)    Wimpy (&W%d&w)", ch->pcdata->pagerlen, ch->wimpy );

		/*
		 * Config option for Room Flag display added by Samson 12-10-97
		 */
		/*
		 * Config option for Sector Type display added by Samson 12-10-97
		 */
		/*
		 * Config option Area name and filename added by Samson 12-13-97
		 */
		/*
		 * changed how all this was setup sept 2005 tommi
		 */
		if ( IS_IMMORTAL ( ch ) )
		{
			set_char_color ( AT_DGREEN, ch );
			send_to_char ( "\r\n\r\nToggles:  ", ch );
			set_char_color ( AT_GREY, ch );
			ch_printf ( ch, "%-12s   %-12s   %-12s   %-12s\r\n",
			            xIS_SET ( ch->act, PLR_ROOMVNUM ) ? "[+] VNUM"
			            : "[-] vnum",
			            IS_SET ( ch->pcdata->flags, PCFLAG_AUTOFLAGS ) ? "[+] ROOMFLAGS"
			            : "[-] roomflags",
			            IS_SET ( ch->pcdata->flags, PCFLAG_SECTORD ) ? "[+] SECTORTYPE"
			            : "[-] sectortype ", IS_SET ( ch->pcdata->flags, PCFLAG_ANAME ) ? "[+] FILENAME" : "[-] filename " );
		}

		set_char_color ( AT_DGREEN, ch );

		send_to_char ( "\r\n\r\nSentences imposed on you (if any):", ch );
		set_char_color ( AT_YELLOW, ch );
		ch_printf ( ch, "\r\n%s%s%s%s%s%s",
		            xIS_SET ( ch->act, PLR_SILENCE ) ?
		            " For your abuse of channels, you are currently silenced.\r\n" : "",
		            xIS_SET ( ch->act, PLR_NO_EMOTE ) ?
		            " The gods have removed your emotes.\r\n" : "",
		            xIS_SET ( ch->act, PLR_NO_TELL ) ?
		            " You are not permitted to send 'tells' to others.\r\n" : "",
		            xIS_SET ( ch->act, PLR_LITTERBUG ) ?
		            " A convicted litterbug.  You cannot drop anything.\r\n" : "",
		            xIS_SET ( ch->act, PLR_THIEF ) ?
		            " A proven thief, you will be hunted by the authorities.\r\n" : "",
		            xIS_SET ( ch->act, PLR_KILLER ) ? " For the crime of murder you are sentenced to death...\r\n" : "" );
	}
	else
	{
		bool fSet;
		int bit = 0;

		if ( arg[0] == '+' )
			fSet = TRUE;
		else if ( arg[0] == '-' )
			fSet = FALSE;
		else
		{
			send_to_char ( "Config -option or +option?\r\n", ch );
			return;
		}

		if ( !str_prefix ( arg + 1, "autoexit" ) )
			bit = PLR_AUTOEXIT;
		else if ( !str_prefix ( arg + 1, "autoloot" ) )
			bit = PLR_AUTOLOOT;
		else if ( !str_prefix ( arg + 1, "autosac" ) )
			bit = PLR_AUTOSAC;
		else if ( !str_prefix ( arg + 1, "autogold" ) )
			bit = PLR_AUTOGOLD;
		else if ( !str_prefix ( arg + 1, "blank" ) )
			bit = PLR_BLANK;
		else if ( !str_prefix ( arg + 1, "brief" ) )
			bit = PLR_BRIEF;
		else if ( !str_prefix ( arg + 1, "combine" ) )
			bit = PLR_COMBINE;
		else if ( !str_prefix ( arg + 1, "prompt" ) )
			bit = PLR_PROMPT;
		else if ( !str_prefix ( arg + 1, "telnetga" ) )
			bit = PLR_TELNET_GA;
		else if ( !str_prefix ( arg + 1, "ansi" ) )
			bit = PLR_ANSI;
		else if ( !str_prefix ( arg + 1, "rip" ) )
			bit = PLR_RIP;
		else if ( !str_prefix ( arg + 1, "map" ) )
			bit = PLR_AUTOMAP;   /* maps */
		else if ( !str_prefix ( arg + 1, "nice" ) )
			bit = PLR_NICE;
		else if ( !str_prefix ( arg + 1, "drag" ) )
			bit = PLR_SHOVEDRAG;
		else if ( IS_IMMORTAL ( ch ) && !str_prefix ( arg + 1, "vnum" ) )
			bit = PLR_ROOMVNUM;

		if ( bit )
		{
			if ( ( bit == PLR_FLEE || bit == PLR_NICE || bit == PLR_SHOVEDRAG ) && IS_SET ( ch->pcdata->flags, PCFLAG_DEADLY ) )
			{
				send_to_char ( "Pkill characters can not config that option.\r\n", ch );
				return;
			}

			if ( fSet )
				xSET_BIT ( ch->act, bit );
			else
				xREMOVE_BIT ( ch->act, bit );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}
		else
		{
			if ( !str_prefix ( arg + 1, "norecall" ) )
				bit = PCFLAG_NORECALL;
			else if ( !str_prefix ( arg + 1, "nointro" ) )
				bit = PCFLAG_NOINTRO;
			else if ( !str_prefix ( arg + 1, "nosummon" ) )
				bit = PCFLAG_NOSUMMON;
			else if ( !str_prefix ( arg + 1, "gag" ) )
				bit = PCFLAG_GAG;
			else if ( !str_prefix ( arg + 1, "pager" ) )
				bit = PCFLAG_PAGERON;
			else if ( IS_IMMORTAL ( ch ) && ( !str_prefix ( arg + 1, "roomflags" ) ) )
				bit = PCFLAG_AUTOFLAGS;
			else if ( IS_IMMORTAL ( ch ) && ( !str_prefix ( arg + 1, "sectortypes" ) ) )
				bit = PCFLAG_SECTORD;
			else if ( IS_IMMORTAL ( ch ) && ( !str_prefix ( arg + 1, "filename" ) ) )
				bit = PCFLAG_ANAME;
			else if ( !str_prefix ( arg + 1, "groupwho" ) )
				bit = PCFLAG_GROUPWHO;
			else if ( !str_prefix ( arg + 1, "@hgflag_" ) )
				bit = PCFLAG_HIGHGAG;
			else
			{
				send_to_char ( "Config which option?\r\n", ch );
				return;
			}

			if ( fSet )
				SET_BIT ( ch->pcdata->flags, bit );
			else
				REMOVE_BIT ( ch->pcdata->flags, bit );

			send_to_char ( "Ok.\r\n", ch );

			return;
		}
	}

	return;
}

void do_credits ( CHAR_DATA * ch, char *argument )
{
	do_help ( ch, "credits" );
}

extern int top_area;
/*
 * New do_areas, written by Fireblade, last modified - 4/27/97
 *
 *   Syntax: area            ->      lists areas in alphanumeric order
 *           area <a>        ->      lists areas with soft max less than
 *                                                    parameter a
 *           area <a> <b>    ->      lists areas with soft max bewteen
 *                                                    numbers a and b
 *           area old        ->      list areas in order loaded
 *
 */
void do_areas ( CHAR_DATA * ch, char *argument )
{
	char *print_string = "&D| &w%-13s &D| &w%-33s &D| &w%4d - %-4d &D| &w%3d - %-4d &D|\r\n";
	AREA_DATA *pArea;
	int lower_bound = 0;
	int upper_bound = MAX_LEVEL + 1;
	/*
	 * make sure is to init. > max area level
	 */
	char arg[MAX_STRING_LENGTH];
	argument = one_argument ( argument, arg );

	if ( arg[0] != STRING_NULL )
	{
		if ( !is_number ( arg ) )
		{
			if ( !strcmp ( arg, "old" ) )
			{
				set_pager_color ( AT_PLAIN, ch );

				for ( pArea = first_area; pArea; pArea = pArea->next )
				{
					pager_printf ( ch, print_string, pArea->author, pArea->name, pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
				}

				return;
			}
			else
			{
				send_to_char ( "Area may only be followed by numbers, or 'old'.\r\n", ch );
				return;
			}
		}

		upper_bound = atoi ( arg );

		lower_bound = upper_bound;
		argument = one_argument ( argument, arg );

		if ( arg[0] != STRING_NULL )
		{
			if ( !is_number ( arg ) )
			{
				send_to_char ( "Area may only be followed by numbers.\r\n", ch );
				return;
			}

			upper_bound = atoi ( arg );

			argument = one_argument ( argument, arg );

			if ( arg[0] != STRING_NULL )
			{
				send_to_char ( "Only two level numbers allowed.\r\n", ch );
				return;
			}
		}
	}

	if ( lower_bound > upper_bound )
	{
		int swap = lower_bound;
		lower_bound = upper_bound;
		upper_bound = swap;
	}

	set_pager_color ( AT_AREA, ch );

	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "   &cAuthor                     Area                   Recommended    Enforced   \r\n", ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );

	for ( pArea = first_area_name; pArea; pArea = pArea->next_sort_name )
	{
		if ( pArea->hi_soft_range >= lower_bound && pArea->low_soft_range <= upper_bound )
		{
			pager_printf ( ch, print_string, pArea->author, pArea->name, pArea->low_soft_range, pArea->hi_soft_range, pArea->low_hard_range, pArea->hi_hard_range );
		}
	}

	send_to_char ( "--------------------------------------------------------------------------------\r\n", ch );
	return;
}

void do_afk ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
		return;

	if ( xIS_SET ( ch->act, PLR_AFK ) )
	{
		xREMOVE_BIT ( ch->act, PLR_AFK );
		send_to_char ( "You are no longer afk.\r\n", ch );
		act ( AT_GREY, "$n is no longer afk.", ch, NULL, NULL, TO_CANSEE );
	}
	else
	{
		xSET_BIT ( ch->act, PLR_AFK );
		send_to_char ( "You are now afk.\r\n", ch );
		act ( AT_GREY, "$n is now afk.", ch, NULL, NULL, TO_CANSEE );
		return;
	}
}

void do_slist ( CHAR_DATA * ch, char *argument )
{
	int sn, i, lFound;
	char skn[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int lowlev, hilev;
	short lasttype = SKILL_SPELL;

	if ( IS_NPC ( ch ) )
		return;

	argument = one_argument ( argument, arg1 );

	argument = one_argument ( argument, arg2 );

	lowlev = 1;

	hilev = 100;

	if ( arg1[0] != STRING_NULL )
		lowlev = atoi ( arg1 );

	if ( ( lowlev < 1 ) || ( lowlev > LEVEL_IMMORTAL ) )
		lowlev = 1;

	if ( arg2[0] != STRING_NULL )
		hilev = atoi ( arg2 );

	if ( ( hilev < 0 ) || ( hilev >= LEVEL_IMMORTAL ) )
		hilev = LEVEL_HERO;

	if ( lowlev > hilev )
		lowlev = hilev;

	set_pager_color ( AT_SLIST, ch );
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	send_to_char ( "  &cSkill and Spell List                                                      &D\r\n", ch );

	for ( i = lowlev; i <= hilev; i++ )
	{
		lFound = 0;
		sprintf ( skn, "Spell" );

		for ( sn = 0; sn < top_sn; sn++ )
		{
			if ( !skill_table[sn]->name )
				break;

			if ( skill_table[sn]->type != lasttype )
			{
				lasttype = skill_table[sn]->type;
				strcpy ( skn, skill_tname[lasttype] );
			}

			if ( ch->pcdata->learned[sn] <= 0 && SPELL_FLAG ( skill_table[sn], SF_SECRETSKILL ) )
				continue;

			if ( i == skill_table[sn]->skill_level[ch->Class] )
			{
				if ( !lFound )
				{
					lFound = 1;
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
					pager_printf ( ch, "&wLevel &W%d\r\n", i );
					send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
				}

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

				pager_printf ( ch, "%7s&c: &w%-18s&D Current&c: &w%-3d&D Max&c:&w %-3d&D Position&c:&w %s&D \r\n", 
				skn, skill_table[sn]->name, ch->pcdata->learned[sn], skill_table[sn]->skill_adept[ch->Class], buf );
			}
		}
	}
	send_to_char ( "&D--------------------------------------------------------------------------------\r\n", ch );
	return;
}

void do_whois ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *victim;
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	buf[0] = STRING_NULL;

	if ( IS_NPC ( ch ) )
		return;

	if ( argument[0] == STRING_NULL )
	{
		send_to_pager ( "You must input the name of an online character.\r\n", ch );
		return;
	}

	strcat ( buf, "0." );

	strcat ( buf, argument );

	if ( ( ( victim = get_char_world ( ch, buf ) ) == NULL ) )
	{
		send_to_pager ( "No such character online.\r\n", ch );
		return;
	}

	if ( IS_NPC ( victim ) )
	{
		send_to_pager ( "That's not a player!\r\n", ch );
		return;
	}

	set_pager_color ( AT_GREY, ch );

	pager_printf ( ch, "\r\n'%s%s.'\r\n %s is a %s level %d %s %s, %d years of age.\r\n",
	               victim->name,
	               victim->pcdata->title,
	               victim->sex == SEX_MALE ? "He" :
	               victim->sex == SEX_FEMALE ? "She" : "It",
	               victim->sex == SEX_MALE ? "male" :
	               victim->sex == SEX_FEMALE ? "female" : "neutral",
	               victim->level, capitalize ( race_table[victim->race]->race_name ), Class_table[victim->Class]->who_name, get_age ( victim ) );
	pager_printf ( ch, " %s is a %sdeadly player",
	               victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", IS_SET ( victim->pcdata->flags, PCFLAG_DEADLY ) ? "" : "non-" );

	if ( victim->pcdata->clan )
	{
		send_to_pager ( ", and belongs to Clan ", ch );
		send_to_pager ( victim->pcdata->clan->name, ch );
	}

	send_to_pager ( ".\r\n", ch );

	if ( victim->pcdata->deity )
		pager_printf ( ch, " %s has found succor in the deity %s.\r\n", victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->deity->name );

	if ( victim->pcdata->homepage && victim->pcdata->homepage[0] != STRING_NULL )
		pager_printf ( ch, " %s homepage can be found at %s\r\n",
		               victim->sex == SEX_MALE ? "His" : victim->sex == SEX_FEMALE ? "Her" : "Its", show_tilde ( victim->pcdata->homepage ) );

	if ( victim->pcdata->bio && victim->pcdata->bio[0] != STRING_NULL )
		pager_printf ( ch, " %s's personal bio:\r\n%s", victim->name, victim->pcdata->bio );
	else
		pager_printf ( ch, " %s has yet to create a bio.\r\n", victim->name );

	if ( IS_IMMORTAL ( ch ) )
	{
		send_to_pager ( "-------------------\r\n", ch );
		send_to_pager ( "Info for immortals:\r\n", ch );

		if ( mip_enabled ( victim ) )
			pager_printf ( ch, "%s's client is: %s (sec_code: %s)\r\n",
			               victim->name, victim->pcdata->mip_ver, victim->pcdata->sec_code );

		if ( victim->pcdata->authed_by && victim->pcdata->authed_by[0] != STRING_NULL )
			pager_printf ( ch, "%s was authorized by %s.\r\n", victim->name, victim->pcdata->authed_by );

		pager_printf ( ch, "%s has killed %d mobiles, and been killed by a mobile %d times.\r\n", victim->name, victim->pcdata->mkills, victim->pcdata->mdeaths );

		if ( victim->pcdata->pkills || victim->pcdata->pdeaths )
			pager_printf ( ch, "%s has killed %d players, and been killed by a player %d times.\r\n", victim->name, victim->pcdata->pkills, victim->pcdata->pdeaths );

		if ( victim->pcdata->illegal_pk )
			pager_printf ( ch, "%s has committed %d illegal player kills.\r\n", victim->name, victim->pcdata->illegal_pk );

		pager_printf ( ch, "%s is %shelled at the moment.\r\n", victim->name, ( victim->pcdata->release_date == 0 ) ? "not " : "" );

		if ( victim->pcdata->release_date != 0 )
			pager_printf ( ch, "%s was helled by %s, and will be released on %24.24s.\r\n",
			               victim->sex == SEX_MALE ? "He" : victim->sex == SEX_FEMALE ? "She" : "It", victim->pcdata->helled_by, ctime ( &victim->pcdata->release_date ) );

		if ( xIS_SET ( victim->act, PLR_SILENCE ) || xIS_SET ( victim->act, PLR_NO_EMOTE )
		        || xIS_SET ( victim->act, PLR_NO_TELL ) || xIS_SET ( victim->act, PLR_THIEF ) || xIS_SET ( victim->act, PLR_KILLER ) )
		{
			sprintf ( buf2, "This player has the following flags set:" );

			if ( xIS_SET ( victim->act, PLR_SILENCE ) )
				strcat ( buf2, " silence" );

			if ( xIS_SET ( victim->act, PLR_NO_EMOTE ) )
				strcat ( buf2, " noemote" );

			if ( xIS_SET ( victim->act, PLR_NO_TELL ) )
				strcat ( buf2, " notell" );

			if ( xIS_SET ( victim->act, PLR_THIEF ) )
				strcat ( buf2, " thief" );

			if ( xIS_SET ( victim->act, PLR_KILLER ) )
				strcat ( buf2, " killer" );

			strcat ( buf2, ".\r\n" );

			send_to_pager ( buf2, ch );
		}

		if ( victim->desc && victim->desc->host[0] != STRING_NULL ) /* added by Gorog */
		{
			sprintf ( buf2, "%s's IP info: %s ", victim->name, victim->desc->host );
			strcat ( buf2, "\r\n" );
			send_to_pager ( buf2, ch );
		}
	}
}

void do_pager ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];

	if ( IS_NPC ( ch ) )
		return;

	set_char_color ( AT_NOTE, ch );

	argument = one_argument ( argument, arg );

	if ( !*arg )
	{
		if ( IS_SET ( ch->pcdata->flags, PCFLAG_PAGERON ) )
		{
			send_to_char ( "Pager disabled.\r\n", ch );
			do_config ( ch, "-pager" );
		}
		else
		{
			ch_printf ( ch, "Pager is now enabled at %d lines.\r\n", ch->pcdata->pagerlen );
			do_config ( ch, "+pager" );
		}

		return;
	}

	if ( !is_number ( arg ) )
	{
		send_to_char ( "Set page pausing to how many lines?\r\n", ch );
		return;
	}

	ch->pcdata->pagerlen = atoi ( arg );

	if ( ch->pcdata->pagerlen < 5 )
		ch->pcdata->pagerlen = 5;

	ch_printf ( ch, "Page pausing set to %d lines.\r\n", ch->pcdata->pagerlen );

	return;
}

/*
 * The ignore command allows players to ignore up to MAX_IGN
 * other players. Players may ignore other characters whether
 * they are online or not. This is to prevent people from
 * spamming someone and then logging off quickly to evade
 * being ignored.
 * Syntax:
 * ignore  - lists players currently ignored
 * ignore none - sets it so no players are ignored
 * ignore <player> - start ignoring player if not already
 *    ignored otherwise stop ignoring player
 * ignore reply - start ignoring last player to send a
 *    tell to ch, to deal with invis spammers
 * Last Modified: June 26, 1997
 * - Fireblade
 */
void do_ignore ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	IGNORE_DATA *temp, *next;
	char fname[1024];

	struct stat fst;
	CHAR_DATA *victim;

	if ( IS_NPC ( ch ) )
		return;

	argument = one_argument ( argument, arg );

	sprintf ( fname, "%s%c/%s", PLAYER_DIR, tolower ( arg[0] ), capitalize ( arg ) );

	victim = NULL;

	/*
	 * If no arguements, then list players currently ignored
	 */
	if ( arg[0] == STRING_NULL )
	{
		set_char_color ( AT_DIVIDER, ch );
		ch_printf ( ch, "\r\n----------------------------------------\r\n" );
		set_char_color ( AT_DGREEN, ch );
		ch_printf ( ch, "You are currently ignoring:\r\n" );
		set_char_color ( AT_DIVIDER, ch );
		ch_printf ( ch, "----------------------------------------\r\n" );
		set_char_color ( AT_IGNORE, ch );

		if ( !ch->pcdata->first_ignored )
		{
			ch_printf ( ch, "\t    no one\r\n" );
			return;
		}

		for ( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
		{
			ch_printf ( ch, "\t  - %s\r\n", temp->name );
		}

		return;
	}

	/*
	 * Clear players ignored if given arg "none"
	 */
	else if ( !strcmp ( arg, "none" ) )
	{
		for ( temp = ch->pcdata->first_ignored; temp; temp = next )
		{
			next = temp->next;
			UNLINK ( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
			STRFREE ( temp->name );
			DISPOSE ( temp );
		}

		set_char_color ( AT_IGNORE, ch );

		ch_printf ( ch, "You now ignore no one.\r\n" );
		return;
	}

	/*
	 * Prevent someone from ignoring themself...
	 */
	else if ( !strcmp ( arg, "self" ) || nifty_is_name ( arg, ch->name ) )
	{
		set_char_color ( AT_IGNORE, ch );
		ch_printf ( ch, "Did you type something?\r\n" );
		return;
	}
	else
	{
		int i;
		/*
		 * get the name of the char who last sent tell to ch
		 */

		if ( !strcmp ( arg, "reply" ) )
		{
			if ( !ch->reply )
			{
				set_char_color ( AT_IGNORE, ch );
				ch_printf ( ch, "They're not here.\r\n" );
				return;
			}
			else
			{
				strcpy ( arg, ch->reply->name );
			}
		}

		/*
		 * Loop through the linked list of ignored players
		 */
		/*
		 * keep track of how many are being ignored
		 */
		for ( temp = ch->pcdata->first_ignored, i = 0; temp; temp = temp->next, i++ )
		{
			/*
			 * If the argument matches a name in list remove it
			 */
			if ( !strcmp ( temp->name, capitalize ( arg ) ) )
			{
				UNLINK ( temp, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
				set_char_color ( AT_IGNORE, ch );
				ch_printf ( ch, "You no longer ignore %s.\r\n", temp->name );
				STRFREE ( temp->name );
				DISPOSE ( temp );
				return;
			}
		}

		/*
		 * if there wasn't a match check to see if the name
		 */
		/*
		 * is valid. This if-statement may seem like overkill
		 */
		/*
		 * but it is intended to prevent people from doing the
		 */
		/*
		 * spam and log thing while still allowing ya to
		 */
		/*
		 * ignore new chars without pfiles yet...
		 */
		if ( stat ( fname, &fst ) == -1 && ( ! ( victim = get_char_world ( ch, arg ) ) || IS_NPC ( victim ) || strcmp ( capitalize ( arg ), victim->name ) != 0 ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "No player exists by that" " name.\r\n" );
			return;
		}

		if ( victim )
		{
			strcpy ( capitalize ( arg ), victim->name );
		}

		if ( !check_parse_name ( capitalize ( arg ), TRUE ) )
		{
			set_char_color ( AT_IGNORE, ch );
			send_to_char ( "No player exists by that name.\r\n", ch );
			return;
		}

		/*
		 * If its valid and the list size limit has not been
		 */
		/*
		 * reached create a node and at it to the list
		 */
		if ( i < MAX_IGN )
		{
			IGNORE_DATA *new;
			CREATE ( new, IGNORE_DATA, 1 );
			new->name = STRALLOC ( capitalize ( arg ) );
			new->next = NULL;
			new->prev = NULL;
			LINK ( new, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "You now ignore %s.\r\n", new->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "You may only ignore %d players.\r\n", MAX_IGN );
			return;
		}
	}
}

/*
 * This function simply checks to see if ch is ignoring ign_ch.
 * Last Modified: October 10, 1997
 * - Fireblade
 */
bool is_ignoring ( CHAR_DATA * ch, CHAR_DATA * ign_ch )
{
	IGNORE_DATA *temp;

	if ( IS_NPC ( ch ) || IS_NPC ( ign_ch ) )
		return FALSE;

	for ( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
	{
		if ( nifty_is_name ( temp->name, ign_ch->name ) )
			return TRUE;
	}

	return FALSE;
}

/* Version info -- Scryn */
void do_version ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
		return;

	set_char_color ( AT_YELLOW, ch );

	ch_printf ( ch, "%s%s\r\n", SMAUG_VERSION_MAJOR, SMAUG_VERSION_MINOR );

	if ( IS_IMMORTAL ( ch ) )
		ch_printf ( ch, "Compiled on %s at %s.\r\n", __DATE__, __TIME__ );

	return;
}

/***************************************************************************
*     Shows mana and blood (if vampire) requirements to cast a spell.      *
*          Created by Desden, el Chaman Tibetano - Jun 1999                *
* Snippets page: http://luisso.net/smaug_snippets.htm       *
*                Email: jose@luisso.net                              *
***************************************************************************/
void do_mana ( CHAR_DATA * ch, char *argument )
{
	SKILLTYPE *skill = NULL;
	char arg1[MAX_INPUT_LENGTH];
	int sn;
	int col = 0;
	argument = one_argument ( argument, arg1 );

	if ( IS_NPC ( ch ) )
	{
		send_to_char ( "Mobs cannot use this command.\r\n", ch );
		return;
	}

	if ( arg1[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: mana all\r\n        mana <spell>\r\n", ch );
		return;
	}

	if ( !strcmp ( arg1, "all" ) )
	{
		set_pager_color ( AT_MANA_C, ch );
		send_to_pager ( "&D--------------------------------------------------------------------------------\r\n", ch );
		send_to_pager ( "&c                         MANA REQUIRED TO CAST					\r\n", ch );
		send_to_pager ( "&D--------------------------------------------------------------------------------\r\n", ch );

		for ( sn = 0; sn < top_sn; sn++ )
		{
			skill = get_skilltype ( sn );

			if ( ch->pcdata->learned[sn] < 1 || !skill->name || !skill->min_mana )
				continue;

			if ( ch->level >= skill->skill_level[ch->Class] )
			{
				pager_printf_color ( ch, "&c%-15.15s:&w%4d    &D", skill->name, MANA );

				if ( ++col % 3 == 0 )
					pager_printf ( ch, "\r\n" );
			}
		}

		pager_printf ( ch, "\r\n&D--------------------------------------------------------------------------------\r\n" );
	}
	else
	{
		if ( ( sn = skill_lookup ( arg1 ) ) > 0 )
		{
			skill = get_skilltype ( sn );

			if ( !skill->min_mana )
			{
				ch_printf ( ch, "%s, '%s' doesn't use mana points.\r\n", ch->name, skill->name );
				return;
			}

			if ( ch->pcdata->learned[sn] < 1 )
			{
				if ( ch->level < skill->skill_level[ch->Class] )
				{
					send_to_char ( "You have not enough level to cast it.\r\n", ch );
					return;
				}
				else
				{
					send_to_char ( "You must practice a spell before using it.\r\n", ch );
					return;
				}
			}

			pager_printf ( ch, "You need %d mana points to cast '%s' with success.\r\n", MANA, skill->name );
		}
		else
			ch_printf ( ch, "That is not a spell or a skill.\r\n" );
	}

	return;
}
