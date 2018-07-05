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
 *   Player communication module       *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"

/*
 *  Externals
 */
void send_obj_page_to_char ( CHAR_DATA * ch, OBJ_INDEX_DATA * idx, char page );
void send_room_page_to_char ( CHAR_DATA * ch, ROOM_INDEX_DATA * idx, char page );
void send_page_to_char ( CHAR_DATA * ch, MOB_INDEX_DATA * idx, char page );
void send_control_page_to_char ( CHAR_DATA * ch, char page );
char *act_string ( const char *format, CHAR_DATA * to, CHAR_DATA * ch, const void *arg1, const void *arg2, int flags );
void mud_message ( CHAR_DATA * ch, int channel, char *arg );
void sportschan ( char * );
/*
 * Local functions.
 */
void talk_channel args ( ( CHAR_DATA * ch, char *argument, int channel, const char *verb ) );
char *scramble args ( ( const char *argument, int modifier ) );
/* Text scrambler -- Altrag */
char *scramble ( const char *argument, int modifier )
{
	static char arg[MAX_INPUT_LENGTH];
	short position;
	short conversion = 0;
	modifier %= number_range ( 80, 300 );  /* Bitvectors get way too large #s */

	for ( position = 0; position < MAX_INPUT_LENGTH; position++ )
	{
		if ( argument[position] == STRING_NULL )
		{
			arg[position] = STRING_NULL;
			return arg;
		}
		else if ( argument[position] >= 'A' && argument[position] <= 'Z' )
		{
			conversion = -conversion + position - modifier + argument[position] - 'A';
			conversion = number_range ( conversion - 5, conversion + 5 );

			while ( conversion > 25 )
				conversion -= 26;

			while ( conversion < 0 )
				conversion += 26;

			arg[position] = conversion + 'A';
		}
		else if ( argument[position] >= 'a' && argument[position] <= 'z' )
		{
			conversion = -conversion + position - modifier + argument[position] - 'a';
			conversion = number_range ( conversion - 5, conversion + 5 );

			while ( conversion > 25 )
				conversion -= 26;

			while ( conversion < 0 )
				conversion += 26;

			arg[position] = conversion + 'a';
		}
		else if ( argument[position] >= '0' && argument[position] <= '9' )
		{
			conversion = -conversion + position - modifier + argument[position] - '0';
			conversion = number_range ( conversion - 2, conversion + 2 );

			while ( conversion > 9 )
				conversion -= 10;

			while ( conversion < 0 )
				conversion += 10;

			arg[position] = conversion + '0';
		}
		else
			arg[position] = argument[position];
	}

	arg[position] = STRING_NULL;

	return arg;
}

/* I'll rewrite this later if its still needed.. -- Altrag
char *translate( CHAR_DATA *ch, CHAR_DATA *victim, const char *argument )
{
 return "";
}
*/
LANG_DATA *get_lang ( const char *name )
{
	LANG_DATA *lng;

	for ( lng = first_lang; lng; lng = lng->next )
		if ( !str_cmp ( lng->name, name ) )
			return lng;

	return NULL;
}

/* percent = percent knowing the language. */
char *translate ( int percent, const char *in, const char *name )
{
	LCNV_DATA *cnv;
	static char buf[256];
	char buf2[256];
	const char *pbuf;
	char *pbuf2 = buf2;
	LANG_DATA *lng;

	if ( percent > 99 || !str_cmp ( name, "common" ) )
	{
		strcpy ( log_buf, in );
		return log_buf;
	}

	/*
	 * If we don't know this language... use "default"
	 */
	if ( ! ( lng = get_lang ( name ) ) )
		if ( ! ( lng = get_lang ( "default" ) ) )
		{
			strcpy ( log_buf, in );
			return log_buf;
		}

	for ( pbuf = in; *pbuf; )
	{
		for ( cnv = lng->first_precnv; cnv; cnv = cnv->next )
		{
			if ( !str_prefix ( cnv->old, pbuf ) )
			{
				if ( percent && ( rand( ) % 100 ) < percent )
				{
					strncpy ( pbuf2, pbuf, cnv->olen );
					pbuf2[cnv->olen] = STRING_NULL;
					pbuf2 += cnv->olen;
				}
				else
				{
					strcpy ( pbuf2, cnv->new );
					pbuf2 += cnv->nlen;
				}

				pbuf += cnv->olen;

				break;
			}
		}

		if ( !cnv )
		{
			if ( isalpha ( *pbuf ) && ( !percent || ( rand( ) % 100 ) > percent ) )
			{
				*pbuf2 = lng->alphabet[LOWER ( *pbuf ) - 'a'];

				if ( isupper ( *pbuf ) )
					*pbuf2 = UPPER ( *pbuf2 );
			}
			else
				*pbuf2 = *pbuf;

			pbuf++;

			pbuf2++;
		}
	}

	*pbuf2 = STRING_NULL;

	for ( pbuf = buf2, pbuf2 = buf; *pbuf; )
	{
		for ( cnv = lng->first_cnv; cnv; cnv = cnv->next )
			if ( !str_prefix ( cnv->old, pbuf ) )
			{
				strcpy ( pbuf2, cnv->new );
				pbuf += cnv->olen;
				pbuf2 += cnv->nlen;
				break;
			}

		if ( !cnv )
			* ( pbuf2++ ) = * ( pbuf++ );
	}

	*pbuf2 = STRING_NULL;

#if 0

	for ( pbuf = in, pbuf2 = buf; *pbuf && *pbuf2; pbuf++, pbuf2++ )
		if ( isupper ( *pbuf ) )
			*pbuf2 = UPPER ( *pbuf2 );

	/*
	 * Attempt to align spacing..
	 */
		else if ( isspace ( *pbuf ) )
			while ( *pbuf2 && !isspace ( *pbuf2 ) )
				pbuf2++;

#endif
	return buf;
}

/*
 * Generic channel function.
 */
void talk_channel ( CHAR_DATA * ch, char *argument, int channel, const char *verb )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;
	int position;
	bool supress = FALSE;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	if ( IS_NPC ( ch ) && channel == CHANNEL_CLAN )
	{
		send_to_char ( "Mobs can't be in clans.\r\n", ch );
		return;
	}

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char ( "You can't do that here.\r\n", ch );
		return;
	}

	if ( IS_NPC ( ch ) && IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		if ( ch->master )
			send_to_char ( "I don't think so...\r\n", ch->master );

		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		sprintf ( buf, "%s what?\r\n", verb );
		buf[0] = UPPER ( buf[0] );
		send_to_char ( buf, ch );  /* where'd this line go? */
		return;
	}

	if ( argument[0] == '@' )
	{
		SOCIALTYPE *social;
		char cmd[MAX_STRING_LENGTH];
		char buf3[MAX_STRING_LENGTH];
		CHAR_DATA *victim;
		argument++;
		argument = one_argument ( argument, cmd );

		if ( ( social = find_social ( cmd ) ) == NULL )
		{
			ch_printf ( ch, "Couldn't find social %s\r\n", cmd );
			return;
		}

		supress = TRUE;

		argument = one_argument ( argument, cmd );

		if ( ( victim = get_char_world ( ch, cmd ) ) == NULL && social->others_no_arg )
		{
			strcpy ( buf2, ( char * ) act_string ( social->others_no_arg, ch, ch, NULL, victim, 0 ) );
			buf2[strlen ( buf2 ) - 2] = 0;
			sprintf ( buf3, "&c%s", buf2 );

			if ( social->char_no_arg )
			{
				strcpy ( buf2, ( char * ) act_string ( social->char_no_arg, ch, ch, NULL, victim, 0 ) );
				buf2[strlen ( buf2 ) - 2] = 0;
				ch_printf ( ch, "&c%s&W (%s)\r\n", buf2, verb );
			}
		}
		else
			if ( victim == ch && social->others_auto )
			{
				strcpy ( buf2, ( char * ) act_string ( social->others_auto, ch, ch, NULL, victim, 0 ) );
				buf2[strlen ( buf2 ) - 2] = 0;
				sprintf ( buf3, "&c%s", buf2 );

				if ( social->char_auto )
				{
					strcpy ( buf2, ( char * ) act_string ( social->char_auto, ch, ch, NULL, victim, 0 ) );
					buf2[strlen ( buf2 ) - 2] = 0;
					ch_printf ( ch, "&c%s&W (%s)\r\n", buf2, verb );
				}
			}
			else
				if ( victim && social->others_found )
				{
					strcpy ( buf2, ( char * ) act_string ( social->others_found, ch, ch, NULL, victim, 0 ) );
					buf2[strlen ( buf2 ) - 2] = 0;
					sprintf ( buf3, "&c%s", buf2 );

					if ( social->char_found )
					{
						strcpy ( buf2, ( char * ) act_string ( social->char_found, ch, ch, NULL, victim, 0 ) );
						buf2[strlen ( buf2 ) - 2] = 0;
						ch_printf ( ch, "&c%s&W (%s)\r\n", buf2, verb );
					}
				}
				else
				{
					ch_printf ( ch, "Couldn't perform social %s on %s\r\n", social->name, cmd );
					return;
				}

		if ( buf3[0] != STRING_NULL )
			argument = buf3;
	}

	if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_SILENCE ) )
	{
		ch_printf ( ch, "You can't %s.\r\n", verb );
		return;
	}

	REMOVE_BIT ( ch->deaf, channel );

	if ( supress )
		sprintf ( buf, "&c*$t&c*&W (%s)", verb );
	else
		switch ( channel )
		{

			default:
				set_char_color ( AT_GOSSIP, ch );
				ch_printf ( ch, "You %s '%s'\r\n", verb, argument );
				sprintf ( buf, "$n %ss '$t'", verb );
				send_mip_channel ( ch, "gossip", "[GOSSIP]", ch->name, argument );
				break;

			case CHANNEL_OOC:
				set_char_color ( AT_PINK, ch );
				ch_printf ( ch, "You %s '%s'\r\n", verb, argument );
				sprintf ( buf, "$n %ss '$t'", verb );
				send_mip_channel ( ch, "ooc", "[OOC]", ch->name, argument );
				break;

			case CHANNEL_NEWBIE:
				set_char_color ( AT_WHITE, ch );
				ch_printf ( ch, "You %s '%s'\r\n", verb, argument );
				sprintf ( buf, "$n %s '$t'", verb );
				send_mip_channel ( ch, "newbiechat", "[NEWBIECHAT]", ch->name, argument );
				break;

			case CHANNEL_RACETALK:
				set_char_color ( AT_RACETALK, ch );
				ch_printf ( ch, "You %s '%s'\r\n", verb, argument );
				sprintf ( buf, "$n %ss '$t'", verb );
				break;

			case CHANNEL_IMMTALK:
				set_char_color ( AT_YELLOW, ch );
				ch_printf ( ch, "%s> '%s'\r\n", ch->name, argument );
				sprintf ( buf, "$n%s '$t'", verb );
				send_mip_channel ( ch, "immtalk", "[IMMTALK]", ch->name, argument );
				break;

			case CHANNEL_AVTALK:
				set_char_color ( AT_BLUE, ch );
				ch_printf ( ch, "%s: '%s'\r\n", ch->name, argument );
				sprintf ( buf, "$n%s '$t'", verb );
				send_mip_channel ( ch, "avtalk", "[AVATAR]", ch->name, argument );
				break;
		}


	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf2, "%s: %s (%s)", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, verb );
		append_to_file ( LOG_FILE, buf2 );
	}

	for ( d = first_descriptor; d; d = d->next )
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;
		och = d->original ? d->original : d->character;
		vch = d->character;

		if ( d->connected == CON_PLAYING && vch != ch && !IS_SET ( och->deaf, channel ) )
		{
			char *sbuf = argument;
			char lbuf[MAX_INPUT_LENGTH + 4]; /* invis level string + buf */
			/*
			 * fix by Gorog os that players can ignore others' channel talk
			 */

			if ( is_ignoring ( och, ch ) && get_trust ( ch ) <= get_trust ( och ) )
				continue;

			if ( channel == CHANNEL_IMMTALK && !IS_IMMORTAL ( och ) )
				continue;

			if ( channel == CHANNEL_AVTALK && !IS_HERO ( och ) )
				continue;

			if ( xIS_SET ( vch->in_room->room_flags, ROOM_SILENCE ) )
				continue;

			if ( channel == CHANNEL_YELL && vch->in_room->area != ch->in_room->area )
				continue;

			if ( channel == CHANNEL_CLAN )
			{
				if ( IS_NPC ( vch ) )
					continue;

				if ( vch->pcdata->clan != ch->pcdata->clan )
					continue;
			}

			if ( channel == CHANNEL_RACETALK )
				if ( vch->race != ch->race )
					continue;

			if ( xIS_SET ( ch->act, PLR_WIZINVIS ) && can_see ( vch, ch, FALSE ) && IS_IMMORTAL ( vch ) )
			{
				sprintf ( lbuf, "(%d) ", ( !IS_NPC ( ch ) ) ? ch->pcdata->wizinvis : ch->mobinvis );
			}
			else
			{
				lbuf[0] = STRING_NULL;
			}

			position = vch->position;

			if ( channel != CHANNEL_YELL )
				vch->position = POS_STANDING;

#ifndef SCRAMBLE
			if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
			{
				int speakswell = UMIN ( knows_language ( vch, ch->speaking, ch ),
				                        knows_language ( ch, ch->speaking, vch ) );

				if ( speakswell < 85 && !supress )  /* Socials always clear, they're actions not words. */
					sbuf = translate ( speakswell, argument, lang_names[speaking] );
			}

#else
			if ( !knows_language ( vch, ch->speaking, ch ) && ( !IS_NPC ( ch ) || ch->speaking != 0 ) )
				sbuf = scramble ( argument, ch->speaking );

#endif
			
			MOBtrigger = FALSE;

			if ( channel == CHANNEL_IMMTALK )
			{
				act ( AT_IMMORT, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
				send_mip_channel ( vch, "immtalk", "[IMMTALK]", ch->name, argument );
			}
			else if ( channel == CHANNEL_AVTALK )
			{
				act ( AT_BLUE, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
				send_mip_channel ( vch, "avtalk", "[AVTALK]", ch->name, argument );
			}
			else if ( channel == CHANNEL_OOC )
			{
				act ( AT_PINK, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
				send_mip_channel ( vch, "ooc", "[OOC]", ch->name, argument );
			}
			else if ( channel == CHANNEL_NEWBIE )
			{
				act ( AT_WHITE, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
				send_mip_channel ( vch, "newbiechat", "[NEWBIECHAT]", ch->name, argument );
			}
			else  if ( channel == CHANNEL_RACETALK )
				act ( AT_RACETALK, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
			else
			{
				act ( AT_GOSSIP, strcat ( lbuf, buf ), ch, sbuf, vch, TO_VICT );
				send_mip_channel ( vch, "gossip", "[GOSSIP]", ch->name, argument );
			}
			vch->position = position;
		}
	}

	if ( ( ch->level < LEVEL_IMMORTAL ) && ( channel != CHANNEL_CLAN ) )
		WAIT_STATE ( ch, 6 );

	return;
}

void to_channel ( const char *argument, int channel, const char *verb, short level )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *d;

	if ( !first_descriptor || argument[0] == STRING_NULL )
		return;

	sprintf ( buf, "%s: %s\r\n", verb, argument );

	for ( d = first_descriptor; d; d = d->next )
	{
		CHAR_DATA *och;
		CHAR_DATA *vch;
		och = d->original ? d->original : d->character;
		vch = d->character;

		if ( !och || !vch )
			continue;

		if ( !IS_IMMORTAL ( vch )
		        || ( get_trust ( vch ) < sysdata.build_level && channel == CHANNEL_BUILD )
		        || ( get_trust ( vch ) < sysdata.log_level && ( channel == CHANNEL_LOG
		                || channel == CHANNEL_WARN || channel == CHANNEL_COMM ) ) )
			continue;

		if ( d->connected == CON_PLAYING && !IS_SET ( och->deaf, channel ) && get_trust ( vch ) >= level )
		{
			set_char_color ( AT_LOG, vch );
			send_to_char_color ( buf, vch );
		}
	}

	return;
}

void do_ooc ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_OOC, "ooc" );

	return;
}

void do_gossip ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_GOSSIP, "gossip" );
	return;
}

void do_clantalk ( CHAR_DATA * ch, char *argument )
{

	if ( IS_NPC ( ch ) || !ch->pcdata->clan )
	{
		send_to_char ( "Huh?\r\n", ch );
		return;
	}

	talk_channel ( ch, argument, CHANNEL_CLAN, "clantalk" );

	return;
}

void do_newbiechat ( CHAR_DATA * ch, char *argument )
{
	if ( IS_SET ( ch->pcdata->flags, PCFLAG_HELPER ) )
	{
		talk_channel ( ch, argument, CHANNEL_NEWBIE, "&Y*&R*&BHelper&R*&Y*&D" );
	}
	else
		talk_channel ( ch, argument, CHANNEL_NEWBIE, "newbiechat" );

	return;
}


void do_yell ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_YELL, "yell" );
	return;
}

void do_immtalk ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_IMMTALK, ">" );
	return;
}

void do_avtalk ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_AVTALK, ":" );
	return;
}

void do_say ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *vch;
	EXT_BV actflags;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "Say what?\r\n", ch );
		return;
	}

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char ( "You can't do that here.\r\n", ch );
		return;
	}

	actflags = ch->act;

	if ( IS_NPC ( ch ) )
		xREMOVE_BIT ( ch->act, ACT_SECRETIVE );

	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = argument;

		if ( vch == ch )
			continue;

		/*
		 * Check to see if a player on a map is at the same coords as the recipient
		 * don't need to verify the PLR_ONMAP flags here, it's a room occupants check
		 */
		if ( !is_same_map ( vch, ch ) )
			continue;

		/*
		 * Check to see if character is ignoring speaker
		 */
		if ( is_ignoring ( vch, ch ) )
		{
			/*
			 * continue unless speaker is an immortal
			 */
			if ( !IS_IMMORTAL ( ch ) || get_trust ( vch ) > get_trust ( ch ) )
				continue;
			else
			{
				set_char_color ( AT_IGNORE, vch );
				ch_printf ( vch, "You attempt to ignore %s, but" " are unable to do so.\r\n", ch->name );
			}
		}

#ifndef SCRAMBLE
		if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
		{
			int speakswell = UMIN ( knows_language ( vch, ch->speaking, ch ),
			                        knows_language ( ch, ch->speaking, vch ) );

			if ( speakswell < 75 )
				sbuf = translate ( speakswell, argument, lang_names[speaking] );
		}

#else
		if ( !knows_language ( vch, ch->speaking, ch ) && ( !IS_NPC ( ch ) || ch->speaking != 0 ) )
			sbuf = scramble ( argument, ch->speaking );

#endif

		MOBtrigger = FALSE;
		act ( AT_SAY, "$n says '$t'", ch, sbuf, vch, TO_VICT );
	}

	ch->act = actflags;
	MOBtrigger = FALSE;
	act ( AT_SAY, "You say '$T'", ch, NULL, argument, TO_CHAR );

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument );
		append_to_file ( LOG_FILE, buf );
	}

	mprog_speech_trigger ( argument, ch );

	if ( char_died ( ch ) )
		return;

	oprog_speech_trigger ( argument, ch );

	if ( char_died ( ch ) )
		return;

	rprog_speech_trigger ( argument, ch );

	return;
}

void do_whisper ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	int speaking = -1, lang;
#ifndef SCRAMBLE

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	REMOVE_BIT ( ch->deaf, CHANNEL_WHISPER );

	argument = one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
	{
		send_to_char ( "Whisper to whom what?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char ( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim->switched ) && !IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		send_to_char ( "That player is switched.\r\n", ch );
		return;
	}
	else if ( !IS_NPC ( victim ) && ( !victim->desc ) )
	{
		send_to_char ( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
	{
		send_to_char ( "That player is afk.\r\n", ch );
		return;
	}

	if ( IS_SET ( victim->deaf, CHANNEL_WHISPER ) && ( !IS_IMMORTAL ( ch ) || ( get_trust ( ch ) < get_trust ( victim ) ) ) )
	{
		act ( AT_PLAIN, "$E has $S whispers turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_SILENCE ) )
		send_to_char ( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );

	if ( victim->desc /* make sure desc exists first  -Thoric */
	        && victim->desc->connected == CON_EDITING && get_trust ( ch ) < LEVEL_GOD )
	{
		act ( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * Check to see if target of tell is ignoring the sender
	 */
	if ( is_ignoring ( victim, ch ) )
	{
		/*
		 * If the sender is an imm then they cannot be ignored
		 */
		if ( !IS_IMMORTAL ( ch ) || get_trust ( victim ) > get_trust ( ch ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "%s is ignoring you.\r\n", victim->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, victim );
			ch_printf ( victim, "You attempt to ignore %s, but " "are unable to do so.\r\n", ch->name );
		}
	}

	act ( AT_WHISPER, "You whisper to $N '$t'", ch, argument, victim, TO_CHAR );

	position = victim->position;
	victim->position = POS_STANDING;
#ifndef SCRAMBLE

	if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
	{
		int speakswell = UMIN ( knows_language ( victim, ch->speaking, ch ), knows_language ( ch, ch->speaking, victim ) );

		if ( speakswell < 85 )
			act ( AT_WHISPER, "$n whispers to you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );

#else
	if ( !knows_language ( vch, ch->speaking, ch ) && ( !IS_NPC ( ch ) || ch->speaking != 0 ) )
		act ( AT_WHISPER, "$n whispers to you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );

#endif
		else
			act ( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );
	}

	else
		act ( AT_WHISPER, "$n whispers to you '$t'", ch, argument, victim, TO_VICT );

	if ( !xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		act ( AT_WHISPER, "$n whispers something to $N.", ch, argument, victim, TO_NOTVICT );
	}

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s (whisper to) %s.", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, IS_NPC ( victim ) ? victim->short_descr : victim->name );
		append_to_file ( LOG_FILE, buf );
	}

	victim->position = position;
	mprog_speech_trigger ( argument, ch );

	return;
}

void do_tell ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	CHAR_DATA *switched_victim = NULL;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	REMOVE_BIT ( ch->deaf, CHANNEL_TELLS );

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char ( "You can't do that here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ( xIS_SET ( ch->act, PLR_SILENCE ) || xIS_SET ( ch->act, PLR_NO_TELL ) ) )
	{
		send_to_char ( "You can't do that.\r\n", ch );
		return;
	}

	argument = one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
	{
		send_to_char ( "Tell whom what?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_world ( ch, arg ) ) == NULL  || ( IS_NPC ( victim ) && victim->in_room != ch->in_room ) )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char ( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim->switched )
	        && ( get_trust ( ch ) > LEVEL_AVATAR ) && !IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		send_to_char ( "That player is switched.\r\n", ch );
		return;
	}
	else
		if ( !IS_NPC ( victim ) && ( victim->switched ) && IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
			switched_victim = victim->switched;
		else
			if ( !IS_NPC ( victim ) && ( !victim->desc ) )
			{
				send_to_char ( "That player is link-dead.\r\n", ch );
				return;
			}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
	{
		send_to_char ( "That player is afk.\r\n", ch );
		return;
	}

	if ( IS_SET ( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL ( ch ) || ( get_trust ( ch ) < get_trust ( victim ) ) ) )
	{
		act ( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_SILENCE ) )
		send_to_char ( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );

	if ( ( !IS_IMMORTAL ( ch ) && !IS_AWAKE ( victim ) ) )
	{
		act ( AT_PLAIN, "$E is too tired to discuss such matters with you now.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->in_room->room_flags, ROOM_SILENCE ) )
	{
		act ( AT_PLAIN, "A magic force prevents your message from being heard.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ( victim->desc /* make sure desc exists first  -Thoric */
	        && victim->desc->connected == CON_EDITING && get_trust ( ch ) < LEVEL_GOD )
	{
		act ( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * Check to see if target of tell is ignoring the sender
	 */
	if ( is_ignoring ( victim, ch ) )
	{
		/*
		 * If the sender is an imm then they cannot be ignored
		 */
		if ( !IS_IMMORTAL ( ch ) || get_trust ( victim ) > get_trust ( ch ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "%s is ignoring you.\r\n", victim->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, victim );
			ch_printf ( victim, "You attempt to ignore %s, but " "are unable to do so.\r\n", ch->name );
		}
	}

	ch->retell = victim;

	if ( switched_victim )
		victim = switched_victim;

	/*
	 * Bug fix by guppy@wavecomputers.net
	 */
	MOBtrigger = FALSE;
	act ( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );
	send_mip_tell ( victim, ch, argument, FALSE );
	position = victim->position;
	victim->position = POS_STANDING;

	if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
	{
		int speakswell = UMIN ( knows_language ( victim, ch->speaking, ch ),
		                        knows_language ( ch, ch->speaking, victim ) );

		if ( speakswell < 85 )
		{
			act ( AT_TELL, "$n tells you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );
			send_mip_tell ( victim, ch, argument, TRUE );
		}
		else
		{
			act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
			send_mip_tell ( victim, ch, argument, TRUE );
		}
	}
	else
	{
		act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
		send_mip_tell ( victim, ch, argument, TRUE );
	}

	MOBtrigger = TRUE;

	victim->position = position;
	victim->reply = ch;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s (tell to) %s.",
		          IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, IS_NPC ( victim ) ? victim->short_descr : victim->name );
		append_to_file ( LOG_FILE, buf );
	}

	mprog_speech_trigger ( argument, ch );

	return;
}

void do_reply ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *victim;
	int position;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	REMOVE_BIT ( ch->deaf, CHANNEL_TELLS );

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char ( "You can't do that here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_SILENCE ) )
	{
		send_to_char ( "Your message didn't get through.\r\n", ch );
		return;
	}

	if ( ( victim = ch->reply ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim->switched ) && can_see ( ch, victim, FALSE ) && ( get_trust ( ch ) > LEVEL_AVATAR ) )
	{
		send_to_char ( "That player is switched.\r\n", ch );
		return;
	}
	else if ( !IS_NPC ( victim ) && ( !victim->desc ) )
	{
		send_to_char ( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
	{
		send_to_char ( "That player is afk.\r\n", ch );
		return;
	}

	if ( IS_SET ( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL ( ch ) || ( get_trust ( ch ) < get_trust ( victim ) ) ) )
	{
		act ( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( ( !IS_IMMORTAL ( ch ) && !IS_AWAKE ( victim ) ) || ( !IS_NPC ( victim ) && xIS_SET ( victim->in_room->room_flags, ROOM_SILENCE ) ) )
	{
		act ( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ( victim->desc /* make sure desc exists first  -Thoric */
	        && victim->desc->connected == CON_EDITING && get_trust ( ch ) < LEVEL_GOD )
	{
		act ( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * Check to see if the receiver is ignoring the sender
	 */
	if ( is_ignoring ( victim, ch ) )
	{
		/*
		 * If the sender is an imm they cannot be ignored
		 */
		if ( !IS_IMMORTAL ( ch ) || get_trust ( victim ) > get_trust ( ch ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "%s is ignoring you.\r\n", victim->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, victim );
			ch_printf ( victim, "You attempt to ignore %s, but " "are unable to do so.\r\n", ch->name );
		}
	}

	act ( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );

	position = victim->position;
	victim->position = POS_STANDING;
#ifndef SCRAMBLE

	if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
	{
		int speakswell = UMIN ( knows_language ( victim, ch->speaking, ch ),
		                        knows_language ( ch, ch->speaking, victim ) );

		if ( speakswell < 85 )
		{
			act ( AT_TELL, "$n tells you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );
		}
		else
		{
			act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
		}
	}
	else
	{
		act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	}

#else
	if ( knows_language ( victim, ch->speaking, ch ) || ( IS_NPC ( ch ) && !ch->speaking ) )
		act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );

	send_mip_tell ( ch, victim, argument, FALSE );
	else
		act ( AT_TELL, "$n tells you '$t'", ch, scramble ( argument, ch->speaking ), victim, TO_VICT );

	send_mip_tell ( ch, victim, argument, FALSE );

#endif
	victim->position = position;

	victim->reply = ch;

	ch->retell = victim;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s (reply to) %s.", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, IS_NPC ( victim ) ? victim->short_descr : victim->name );
		append_to_file ( LOG_FILE, buf );
	}

	mprog_speech_trigger ( argument, ch );

	return;
}

void do_retell ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	CHAR_DATA *switched_victim = NULL;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	REMOVE_BIT ( ch->deaf, CHANNEL_TELLS );

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
	{
		send_to_char ( "You can't do that here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( ch ) && ( xIS_SET ( ch->act, PLR_SILENCE ) || xIS_SET ( ch->act, PLR_NO_TELL ) ) )
	{
		send_to_char ( "You can't do that.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		ch_printf ( ch, "What message do you wish to send?\r\n" );
		return;
	}

	victim = ch->retell;

	if ( !victim )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim->switched ) && ( get_trust ( ch ) > LEVEL_AVATAR ) && !IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		send_to_char ( "That player is switched.\r\n", ch );
		return;
	}
	else if ( !IS_NPC ( victim ) && ( victim->switched ) && IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		switched_victim = victim->switched;
	}
	else if ( !IS_NPC ( victim ) && ( !victim->desc ) )
	{
		send_to_char ( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
	{
		send_to_char ( "That player is afk.\r\n", ch );
		return;
	}

	if ( IS_SET ( victim->deaf, CHANNEL_TELLS ) && ( !IS_IMMORTAL ( ch ) || ( get_trust ( ch ) < get_trust ( victim ) ) ) )
	{
		act ( AT_PLAIN, "$E has $S tells turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_SILENCE ) )
		send_to_char ( "That player is silenced. They will receive your message, but can not respond.\r\n", ch );

	if ( ( !IS_IMMORTAL ( ch ) && !IS_AWAKE ( victim ) ) || ( !IS_NPC ( victim ) && xIS_SET ( victim->in_room->room_flags, ROOM_SILENCE ) ) )
	{
		act ( AT_PLAIN, "$E can't hear you.", ch, 0, victim, TO_CHAR );
		return;
	}

	if ( victim->desc && victim->desc->connected == CON_EDITING && get_trust ( ch ) < LEVEL_GOD )
	{
		act ( AT_PLAIN, "$E is currently in a writing buffer. Please " "try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * check to see if the target is ignoring the sender
	 */
	if ( is_ignoring ( victim, ch ) )
	{
		/*
		 * if the sender is an imm then they cannot be ignored
		 */
		if ( !IS_IMMORTAL ( ch ) || get_trust ( victim ) > get_trust ( ch ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "%s is ignoring you.\r\n", victim->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, victim );
			ch_printf ( victim, "You attempy to ignore %s, but " "are unable to do so.\r\n", ch->name );
		}
	}

	if ( switched_victim )
		victim = switched_victim;

	act ( AT_TELL, "You tell $N '$t'", ch, argument, victim, TO_CHAR );

	position = victim->position;

	victim->position = POS_STANDING;

#ifndef SCRAMBLE
	if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
	{
		int speakswell = UMIN ( knows_language ( victim, ch->speaking, ch ),
		                        knows_language ( ch, ch->speaking, victim ) );

		if ( speakswell < 85 )
			act ( AT_TELL, "$n tells you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );
		else
			act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	}
	else
		act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );

#else
	if ( knows_language ( victim, ch->speaking, ch ) || ( IS_NPC ( ch ) && !ch->speaking ) )
	{
		act ( AT_TELL, "$n tells you '$t'", ch, argument, victim, TO_VICT );
	}
	else
	{
		act ( AT_TELL, "$n tells you '$t'", ch, scramble ( argument, ch->speaking ), victim, TO_VICT );
	}

#endif
	victim->position = position;

	victim->reply = ch;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s (retell to) %s.", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, IS_NPC ( victim ) ? victim->short_descr : victim->name );
		append_to_file ( LOG_FILE, buf );
	}

	mprog_speech_trigger ( argument, ch );

	return;
}

void do_emote ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	CHAR_DATA *vch;
	EXT_BV actflags;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	if ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_NO_EMOTE ) )
	{
		send_to_char ( "You can't show your emotions.\r\n", ch );
		return;
	}

	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "Emote what?\r\n", ch );
		return;
	}

	actflags = ch->act;

	if ( IS_NPC ( ch ) )
		xREMOVE_BIT ( ch->act, ACT_SECRETIVE );

	for ( plast = argument; *plast != STRING_NULL; plast++ )
		;

	strcpy ( buf, argument );

	if ( isalpha ( plast[-1] ) )
		strcat ( buf, "." );

	for ( vch = ch->in_room->first_person; vch; vch = vch->next_in_room )
	{
		char *sbuf = buf;
		/*
		 * Check to see if a player on a map is at the same coords as the recipient
		 * don't need to verify the PLR_ONMAP flags here, it's a room occupants check
		 */

		if ( !is_same_map ( vch, ch ) )
			continue;

		/*
		 * Check to see if character is ignoring emoter
		 */
		if ( is_ignoring ( vch, ch ) )
		{
			/*
			 * continue unless emoter is an immortal
			 */
			if ( !IS_IMMORTAL ( ch ) || get_trust ( vch ) > get_trust ( ch ) )
				continue;
			else
			{
				set_char_color ( AT_IGNORE, vch );
				ch_printf ( vch, "You attempt to ignore %s, but" " are unable to do so.\r\n", ch->name );
			}
		}

#ifndef SCRAMBLE
		if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
		{
			int speakswell = UMIN ( knows_language ( vch, ch->speaking, ch ),
			                        knows_language ( ch, ch->speaking, vch ) );

			if ( speakswell < 85 )
				sbuf = translate ( speakswell, argument, lang_names[speaking] );
		}

#else
		if ( !knows_language ( vch, ch->speaking, ch ) && ( !IS_NPC ( ch ) && ch->speaking != 0 ) )
			sbuf = scramble ( buf, ch->speaking );

#endif
		MOBtrigger = FALSE;

		act ( AT_ACTION, "$n $t", ch, sbuf, vch, ( vch == ch ? TO_CHAR : TO_VICT ) );
	}

	/*
	 * MOBtrigger = FALSE;
	 * act( AT_ACTION, "$n $T", ch, NULL, buf, TO_ROOM );
	 * MOBtrigger = FALSE;
	 * act( AT_ACTION, "$n $T", ch, NULL, buf, TO_CHAR );
	 */
	ch->act = actflags;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s %s (emote)", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument );
		append_to_file ( LOG_FILE, buf );
	}

	return;
}

void do_bug ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];

	struct tm *t = localtime ( &current_time );
	set_char_color ( AT_PLAIN, ch );

	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "\r\nUsage:  'bug <message>'  (your location is automatically recorded)\r\n", ch );
		return;
	}

	sprintf ( buf, "(%-2.2d/%-2.2d):  %s", t->tm_mon + 1, t->tm_mday, argument );

	append_file ( ch, PBUG_FILE, buf );
	send_to_char ( "Thanks, your bug notice has been recorded.\r\n", ch );
	return;
}

void do_ide ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_PLAIN, ch );
	send_to_char ( "\r\nIf you want to send an idea, type 'idea <message>'.\r\n", ch );
	send_to_char ( "If you want to identify an object, use the identify spell.\r\n", ch );
	return;
}

void do_idea ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_PLAIN, ch );

	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "\r\nUsage:  'idea <message>'\r\n", ch );
		return;
	}

	append_file ( ch, IDEA_FILE, argument );

	send_to_char ( "Thanks, your idea has been recorded.\r\n", ch );
	return;
}

void do_typo ( CHAR_DATA * ch, char *argument )
{
	set_char_color ( AT_PLAIN, ch );

	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "\r\nUsage:  'typo <message>'  (your location is automatically recorded)\r\n", ch );

		if ( get_trust ( ch ) >= LEVEL_ASCENDANT )
			send_to_char ( "Usage:  'typo list' or 'typo clear now'\r\n", ch );

		return;
	}

	if ( !str_cmp ( argument, "clear now" ) && get_trust ( ch ) >= LEVEL_ASCENDANT )
	{
		FILE *fp = fopen ( TYPO_FILE, "w" );

		if ( fp )
			fclose ( fp );

		send_to_char ( "Typo file cleared.\r\n", ch );

		return;
	}

	if ( !str_cmp ( argument, "list" ) && get_trust ( ch ) >= LEVEL_ASCENDANT )
	{
		send_to_char ( "\r\n VNUM \r\n.......\r\n", ch );
		show_file ( ch, TYPO_FILE );
	}
	else
	{
		append_file ( ch, TYPO_FILE, argument );
		send_to_char ( "Thanks, your typo notice has been recorded.\r\n", ch );
	}

	return;
}

void do_quit ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_INPUT_LENGTH];
	char qbuf[MAX_INPUT_LENGTH];
	/*
	 * OBJ_DATA *obj;
	 *//*
 * Unused
 */
	int x, y;
	int level;

	if ( IS_NPC ( ch ) )
		return;

	if ( ch->position == POS_FIGHTING || ch->position == POS_EVASIVE || ch->position == POS_DEFENSIVE || ch->position == POS_AGGRESSIVE || ch->position == POS_BERSERK )
	{
		set_char_color ( AT_RED, ch );
		send_to_char ( "No way! You are fighting.\r\n", ch );
		return;
	}

	if ( ch->position < POS_STUNNED )
	{
		set_char_color ( AT_BLOOD, ch );
		send_to_char ( "You're not DEAD yet.\r\n", ch );
		return;
	}

	if ( get_timer ( ch, TIMER_RECENTFIGHT ) > 0 && !IS_IMMORTAL ( ch ) )
	{
		set_char_color ( AT_RED, ch );
		send_to_char ( "Your adrenaline is pumping too hard to quit now!\r\n", ch );
		return;
	}

	if ( auction->item != NULL && ( ( ch == auction->buyer ) || ( ch == auction->seller ) ) )
	{
		send_to_char ( "Wait until you have bought/sold the item on auction.\r\n", ch );
		return;
	}

	if ( ch->challenged )
	{
		sprintf ( qbuf, "%s has quit! Challenge is void. WHAT A WUSS!", ch->name );
		ch->challenged = NULL;
		sportschan ( qbuf );
	}

	if ( IS_PKILL ( ch ) && ch->wimpy > ( int ) ch->max_hit / 2.25 )
	{
		send_to_char ( "Your wimpy has been adjusted to the maximum level for deadlies.\r\n", ch );
		do_wimpy ( ch, "max" );
	}

	if ( xIS_SET ( ch->act, PLR_MIP ) )
	{
		xREMOVE_BIT ( ch->act, PLR_MIP );
	}

	/*
	 * Get 'em dismounted until we finish mount saving -- Blodkai, 4/97
	 */
	if ( ch->position == POS_MOUNTED )
		do_dismount ( ch, "" );

	set_char_color ( AT_WHITE, ch );

	send_to_char
	( "Your surroundings begin to fade as a mystical swirling vortex of colors\r\nenvelops your body... When you come to, things are not as they were.\r\n\r\n", ch );

	act ( AT_SAY, "A strange voice says, 'We await your return, $n...'", ch, NULL, NULL, TO_CHAR );

	if ( !IS_IMMORTAL ( ch ) )
	{
		sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c %s has left the game.\r\n", ch->name );
		talk_info ( AT_GREEN, buf, FALSE );
	}

	act ( AT_BYE, "$n has left the game.", ch, NULL, NULL, TO_CANSEE );

	set_char_color ( AT_GREY, ch );
	sprintf ( log_buf, "%s has quit (Room %d).", ch->name, ( ch->in_room ? ch->in_room->vnum : -1 ) );
	quitting_char = ch;
	save_char_obj ( ch );

	if ( sysdata.save_pets && ch->pcdata->pet )
	{
		act ( AT_BYE, "$N follows $S master into the Void.", ch, NULL, ch->pcdata->pet, TO_ROOM );
		extract_char ( ch->pcdata->pet, TRUE );
	}

	/*
	 * Synch clandata up only when clan member quits now. --Shaddai
	 */
	if ( ch->pcdata->clan )
		save_clan ( ch->pcdata->clan );

	saving_char = NULL;

	level = get_trust ( ch );

	/*
	 * After extract_char the ch is no longer valid!
	 */
	extract_char ( ch, TRUE );

	for ( x = 0; x < MAX_WEAR; x++ )
		for ( y = 0; y < MAX_LAYERS; y++ )
			save_equipment[x][y] = NULL;

	log_string_plus ( log_buf, LOG_COMM, level );

	return;
}


void do_ansi ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "ANSI ON or OFF?\r\n", ch );
		return;
	}

	if ( ( strcmp ( arg, "on" ) == 0 ) || ( strcmp ( arg, "ON" ) == 0 ) )
	{
		xSET_BIT ( ch->act, PLR_ANSI );
		set_char_color ( AT_WHITE + AT_BLINK, ch );
		send_to_char ( "ANSI ON!!!\r\n", ch );
		return;
	}

	if ( ( strcmp ( arg, "off" ) == 0 ) || ( strcmp ( arg, "OFF" ) == 0 ) )
	{
		xREMOVE_BIT ( ch->act, PLR_ANSI );
		send_to_char ( "Okay... ANSI support is now off\r\n", ch );
		return;
	}
}

void do_save ( CHAR_DATA * ch, char *argument )
{
	if ( IS_NPC ( ch ) )
		return;

	if ( ch->level < 2 )
	{
		send_to_char_color ( "&BYou must be at least second level to save.\r\n", ch );
		return;
	}

	WAIT_STATE ( ch, 2 ); /* For big muds with save-happy players, like RoD */

	update_aris ( ch );  /* update char affects and RIS */
	save_char_obj ( ch );
	saving_char = NULL;
	send_to_char ( "Saved...\r\n", ch );
	return;
}

/*
 * Something from original DikuMUD that Merc yanked out.
 * Used to prevent following loops, which can cause problems if people
 * follow in a loop through an exit leading back into the same room
 * (Which exists in many maze areas)   -Thoric
 */
bool circle_follow ( CHAR_DATA * ch, CHAR_DATA * victim )
{
	CHAR_DATA *tmp;

	for ( tmp = victim; tmp; tmp = tmp->master )
		if ( tmp == ch )
			return TRUE;

	return FALSE;
}

void do_dismiss ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Dismiss whom?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( ( IS_AFFECTED ( victim, AFF_CHARM ) ) && ( IS_NPC ( victim ) ) && ( victim->master == ch ) )
	{
		stop_follower ( victim );
		stop_hating ( victim );
		stop_hunting ( victim );
		stop_fearing ( victim );
		act ( AT_ACTION, "$n dismisses $N.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_ACTION, "You dismiss $N.", ch, NULL, victim, TO_CHAR );
	}
	else
	{
		send_to_char ( "You cannot dismiss them.\r\n", ch );
	}

	return;
}

void do_follow ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Follow whom?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( IS_AFFECTED ( ch, AFF_CHARM ) && ch->master )
	{
		act ( AT_PLAIN, "But you'd rather follow $N!", ch, NULL, ch->master, TO_CHAR );
		return;
	}

	if ( victim == ch )
	{
		if ( !ch->master )
		{
			send_to_char ( "You already follow yourself.\r\n", ch );
			return;
		}

		stop_follower ( ch );

		return;
	}

	if ( ( ch->level - victim->level < -10 || ch->level - victim->level > 10 )
	        && !IS_HERO ( ch ) && ! ( ch->level < 15 && !IS_NPC ( victim ) ) )
	{
		send_to_char ( "You are not of the right caliber to follow.\r\n", ch );
		return;
	}

	if ( circle_follow ( ch, victim ) )
	{
		send_to_char ( "Following in loops is not allowed... sorry.\r\n", ch );
		return;
	}

	if ( ch->master )
		stop_follower ( ch );

	add_follower ( ch, victim );

	return;
}

void add_follower ( CHAR_DATA * ch, CHAR_DATA * master )
{
	if ( ch->master )
	{
		bug ( "Add_follower: non-null master.", 0 );
		return;
	}

	ch->master = master;

	ch->leader = NULL;
	/*
	 * Support for saving pets --Shaddai
	 */

	if ( IS_NPC ( ch ) && xIS_SET ( ch->act, ACT_PET ) && !IS_NPC ( master ) )
		master->pcdata->pet = ch;

	if ( can_see ( master, ch, FALSE ) )
		act ( AT_ACTION, "$n now follows you.", ch, NULL, master, TO_VICT );

	act ( AT_ACTION, "You now follow $N.", ch, NULL, master, TO_CHAR );

	return;
}

void stop_follower ( CHAR_DATA * ch )
{
	if ( !ch->master )
	{
		bug ( "Stop_follower: null master.", 0 );
		return;
	}

	if ( IS_NPC ( ch ) && !IS_NPC ( ch->master ) && ch->master->pcdata->pet == ch )
		ch->master->pcdata->pet = NULL;

	if ( can_see ( ch->master, ch, FALSE ) )
		if ( ! ( !IS_NPC ( ch->master ) && IS_IMMORTAL ( ch ) && !IS_IMMORTAL ( ch->master ) ) )
			act ( AT_ACTION, "$n stops following you.", ch, NULL, ch->master, TO_VICT );

	act ( AT_ACTION, "You stop following $N.", ch, NULL, ch->master, TO_CHAR );

	ch->master = NULL;

	ch->leader = NULL;

	return;
}

void die_follower ( CHAR_DATA * ch )
{
	CHAR_DATA *fch;

	if ( ch->master )
		stop_follower ( ch );

	ch->leader = NULL;

	for ( fch = first_char; fch; fch = fch->next )
	{
		if ( fch->master == ch )
			stop_follower ( fch );

		if ( fch->leader == ch )
			fch->leader = fch;
	}

	return;
}

void do_order ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char argbuf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *och;
	CHAR_DATA *och_next;
	bool found;
	bool fAll;
	strcpy ( argbuf, argument );
	argument = one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
	{
		send_to_char ( "Order whom to do what?\r\n", ch );
		return;
	}

	if ( IS_AFFECTED ( ch, AFF_CHARM ) )
	{
		send_to_char ( "You feel like taking, not giving, orders.\r\n", ch );
		return;
	}

	if ( strstr ( argument, "mp" ) != NULL )
	{
		send_to_char ( "No.. I don't think so.\r\n", ch );
		return;
	}

	if ( !str_cmp ( arg, "all" ) )
	{
		fAll = TRUE;
		victim = NULL;
	}
	else
	{
		fAll = FALSE;

		if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
		{
			send_to_char ( "They aren't here.\r\n", ch );
			return;
		}

		if ( victim == ch )
		{
			send_to_char ( "Aye aye, right away!\r\n", ch );
			return;
		}

		if ( !IS_AFFECTED ( victim, AFF_CHARM ) || victim->master != ch )
		{
			send_to_char ( "Do it yourself!\r\n", ch );
			return;
		}
	}

	found = FALSE;

	for ( och = ch->in_room->first_person; och; och = och_next )
	{
		och_next = och->next_in_room;

		if ( IS_AFFECTED ( och, AFF_CHARM ) && och->master == ch && ( fAll || och == victim ) )
		{
			found = TRUE;
			act ( AT_ACTION, "$n orders you to '$t'.", ch, argument, och, TO_VICT );
			interpret ( och, argument );
		}
	}

	if ( found )
	{
		sprintf ( log_buf, "%s: order %s.", ch->name, argbuf );
		log_string_plus ( log_buf, LOG_NORMAL, ch->level );
		send_to_char ( "Ok.\r\n", ch );
		WAIT_STATE ( ch, 12 );
	}
	else
		send_to_char ( "You have no followers here.\r\n", ch );

	return;
}

/*
char *itoa(int foo)
{
  static char bar[256];
  sprintf(bar,"%d",foo);
  return(bar);
}
*/
/* Overhauled 2/97 -- Blodkai */
void do_group ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		CHAR_DATA *gch;
		CHAR_DATA *leader;
		leader = ch->leader ? ch->leader : ch;
		set_char_color ( AT_DGREEN, ch );
		ch_printf ( ch, "\r\nFollowing %-12.12s     [hitpnts]   [ magic ] [mst] [mvs] [race]%s\r\n", PERS ( leader, ch, TRUE ), ch->level < LEVEL_AVATAR ? " [to lvl]" : "" );

		for ( gch = first_char; gch; gch = gch->next )
		{
			if ( is_same_group ( gch, ch ) )
			{
				set_char_color ( AT_DGREEN, ch );

				if ( IS_AFFECTED ( gch, AFF_POSSESS ) )  /*reveal no information */
					ch_printf ( ch,
					            "[%2d %s] %-16s %4s/%4s hp %4s/%4s mana %4s/%4s mv %5s xp\r\n",
					            gch->level,
					            IS_NPC ( gch ) ? "Mob" : Class_table[gch->Class]->who_name,
					            capitalize ( PERS ( gch, ch, TRUE ) ), "????", "????", "????", "????", "????", "????", "?????" );
				else if ( gch->alignment > 750 )
					sprintf ( buf, " A" );
				else if ( gch->alignment > 350 )
					sprintf ( buf, "-A" );
				else if ( gch->alignment > 150 )
					sprintf ( buf, "+N" );
				else if ( gch->alignment > -150 )
					sprintf ( buf, " N" );
				else if ( gch->alignment > -350 )
					sprintf ( buf, "-N" );
				else if ( gch->alignment > -750 )
					sprintf ( buf, "+S" );
				else
					sprintf ( buf, " S" );

				set_char_color ( AT_DGREEN, ch );
				send_to_char ( "[", ch );
				set_char_color ( AT_GREEN, ch );
				ch_printf ( ch, "%-2d %2.2s %3.3s", gch->level, buf, IS_NPC ( gch ) ? "Mob" : Class_table[gch->Class]->who_name );
				set_char_color ( AT_DGREEN, ch );
				send_to_char ( "]  ", ch );
				set_char_color ( AT_GREEN, ch );
				ch_printf ( ch, "%-12.12s ", capitalize ( PERS ( gch, ch, TRUE ) ) );

				if ( gch->hit < gch->max_hit / 4 )
					set_char_color ( AT_DANGER, ch );
				else if ( gch->hit < gch->max_hit / 2.5 )
					set_char_color ( AT_YELLOW, ch );
				else
					set_char_color ( AT_GREY, ch );

				ch_printf ( ch, "%5d", gch->hit );
				set_char_color ( AT_GREY, ch );
				ch_printf ( ch, "/%-5d ", gch->max_hit );
				ch_printf ( ch, "%5d/%-5d ", gch->mana, gch->max_mana );
				set_char_color ( AT_DGREEN, ch );
				ch_printf ( ch, "%5d ", gch->move );
				ch_printf ( ch, "%6s ", gch->race == 0 ? "Angel" :
				            gch->race == 1 ? "Demon" :
				            gch->race == 2 ? "Nezumi":
				            gch->race == 3 ? "Vanara":
				            gch->race == 4 ? "Spirit":
				            gch->race == 6 ? "Undead" : "" );

				set_char_color ( AT_GREEN, ch );

				if ( gch->level < LEVEL_AVATAR )
					ch_printf ( ch, "%8d ", exp_level ( gch, gch->level + 1 ) - gch->exp );

				send_to_char ( "\r\n", ch );
			}
		}

		return;
	}

	if ( !strcmp ( arg, "disband" ) )
	{
		CHAR_DATA *gch;
		int count = 0;

		if ( ch->leader || ch->master )
		{
			send_to_char ( "You cannot disband a group if you're following someone.\r\n", ch );
			return;
		}

		for ( gch = first_char; gch; gch = gch->next )
		{
			if ( is_same_group ( ch, gch ) && ( ch != gch ) )
			{
				gch->leader = NULL;
				gch->master = NULL;
				count++;
				send_to_char ( "Your group is disbanded.\r\n", gch );
			}
		}

		if ( count == 0 )
			send_to_char ( "You have no group members to disband.\r\n", ch );
		else
			send_to_char ( "You disband your group.\r\n", ch );

		return;
	}

	if ( !strcmp ( arg, "all" ) )
	{
		CHAR_DATA *rch;
		int count = 0;

		for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
		{
			if ( ch != rch
			        && !IS_NPC ( rch )
			        && can_see ( ch, rch, FALSE )
			        && rch->master == ch && !ch->master && !ch->leader && abs ( ch->level - rch->level ) < 9 && !is_same_group ( rch, ch ) && IS_PKILL ( ch ) == IS_PKILL ( rch ) )
			{
				rch->leader = ch;
				count++;
			}
		}

		if ( count == 0 )
			send_to_char ( "You have no eligible group members.\r\n", ch );
		else
		{
			act ( AT_ACTION, "$n groups $s followers.", ch, NULL, NULL, TO_ROOM );
			send_to_char ( "You group your followers.\r\n", ch );
		}

		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch->master || ( ch->leader && ch->leader != ch ) )
	{
		send_to_char ( "But you are following someone else!\r\n", ch );
		return;
	}

	if ( victim->master != ch && ch != victim )
	{
		act ( AT_PLAIN, "$N isn't following you.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( victim == ch )
	{
		act ( AT_PLAIN, "You can't group yourself.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( is_same_group ( victim, ch ) && ch != victim )
	{
		victim->leader = NULL;
		act ( AT_ACTION, "$n removes $N from $s group.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_ACTION, "$n removes you from $s group.", ch, NULL, victim, TO_VICT );
		act ( AT_ACTION, "You remove $N from your group.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( ch->level - victim->level < -15 || ch->level - victim->level > 15 || ( IS_PKILL ( ch ) != IS_PKILL ( victim ) ) )
	{
		act ( AT_PLAIN, "$N cannot join $n's group.", ch, NULL, victim, TO_NOTVICT );
		act ( AT_PLAIN, "You cannot join $n's group.", ch, NULL, victim, TO_VICT );
		act ( AT_PLAIN, "$N cannot join your group.", ch, NULL, victim, TO_CHAR );
		return;
	}

	victim->leader = ch;

	act ( AT_ACTION, "$N joins $n's group.", ch, NULL, victim, TO_NOTVICT );
	act ( AT_ACTION, "You join $n's group.", ch, NULL, victim, TO_VICT );
	act ( AT_ACTION, "$N joins your group.", ch, NULL, victim, TO_CHAR );
	return;
}

/*
 * 'Split' originally by Gnort, God of Chaos.
 */
void do_split ( CHAR_DATA * ch, char *argument )
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *gch;
	int members;
	int amount;
	int share;
	int extra;
	one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Split how much?\r\n", ch );
		return;
	}

	amount = atoi ( arg );

	if ( amount < 0 )
	{
		send_to_char ( "Your group wouldn't like that.\r\n", ch );
		return;
	}

	if ( amount == 0 )
	{
		send_to_char ( "You hand out zero coins, but no one notices.\r\n", ch );
		return;
	}

	if ( ch->gold < amount )
	{
		send_to_char ( "You don't have that much gold.\r\n", ch );
		return;
	}

	members = 0;

	for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		if ( is_same_group ( gch, ch ) )
			members++;
	}

	if ( xIS_SET ( ch->act, PLR_AUTOGOLD ) && members < 2 )
		return;

	if ( members < 2 )
	{
		send_to_char ( "Just keep it all.\r\n", ch );
		return;
	}

	share = amount / members;

	extra = amount % members;

	if ( share == 0 )
	{
		send_to_char ( "Don't even bother, cheapskate.\r\n", ch );
		return;
	}

	ch->gold -= amount;

	ch->gold += share + extra;
	set_char_color ( AT_GOLD, ch );
	ch_printf ( ch, "You split %d gold coins.  Your share is %d gold coins.\r\n", amount, share + extra );
	sprintf ( buf, "$n splits %d gold coins.  Your share is %d gold coins.", amount, share );

	for ( gch = ch->in_room->first_person; gch; gch = gch->next_in_room )
	{
		if ( gch != ch && is_same_group ( gch, ch ) )
		{
			act ( AT_GOLD, buf, ch, NULL, gch, TO_VICT );
			gch->gold += share;
		}
	}

	return;
}

void do_gtell ( CHAR_DATA * ch, char *argument )
{
	CHAR_DATA *gch;
#ifndef SCRAMBLE
	int speaking = -1, lang;

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	if ( argument[0] == STRING_NULL )
	{
		send_to_char ( "Tell your group what?\r\n", ch );
		return;
	}

	if ( xIS_SET ( ch->act, PLR_NO_TELL ) )
	{
		send_to_char ( "Your message didn't get through!\r\n", ch );
		return;
	}

	/*
	 * Note use of send_to_char, so gtell works on sleepers.
	 */
	/*
	 * sprintf( buf, "%s tells the group '%s'.\r\n", ch->name, argument );
	 */
	for ( gch = first_char; gch; gch = gch->next )
	{
		if ( is_same_group ( gch, ch ) )
		{
			set_char_color ( AT_GTELL, gch );
			/*
			 * Groups unscrambled regardless of clan language.  Other languages
			 * still garble though. -- Altrag
			 */
#ifndef SCRAMBLE

			if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
			{
				int speakswell = UMIN ( knows_language ( gch, ch->speaking, ch ),
				                        knows_language ( ch, ch->speaking, gch ) );

				if ( speakswell < 85 )
					ch_printf ( gch, "%s tells the group '%s'.\r\n", ch->name, translate ( speakswell, argument, lang_names[speaking] ) );
				else
					ch_printf ( gch, "%s tells the group '%s'.\r\n", ch->name, argument );
			}
			else
				ch_printf ( gch, "%s tells the group '%s'.\r\n", ch->name, argument );

#else
			if ( knows_language ( gch, ch->speaking, gch ) || ( IS_NPC ( ch ) && !ch->speaking ) )
				ch_printf ( gch, "%s tells the group '%s'.\r\n", ch->name, argument );
			else
				ch_printf ( gch, "%s tells the group '%s'.\r\n", ch->name, scramble ( argument, ch->speaking ) );

#endif
		}
	}

	return;
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group ( CHAR_DATA * ach, CHAR_DATA * bch )
{
	if ( ach->leader )
		ach = ach->leader;

	if ( bch->leader )
		bch = bch->leader;

	return ach == bch;
}

/*
 * this function sends raw argument over the AUCTION: channel
 * I am not too sure if this method is right..
 */
void talk_auction ( char *argument )
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;
	sprintf ( buf, "Auction: %s", argument ); /* last %s to reset color */

	for ( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;  /* if switched */

		if ( ( d->connected == CON_PLAYING ) && !IS_SET ( original->deaf, CHANNEL_AUCTION )
		        && !xIS_SET ( original->in_room->room_flags, ROOM_SILENCE ) )
			act ( AT_GOSSIP, buf, original, NULL, NULL, TO_CHAR );
	}
}

/*
 * Language support functions. -- Altrag
 * 07/01/96
 *
 * Modified to return how well the language is known 04/04/98 - Thoric
 * Currently returns 100% for known languages... but should really return
 * a number based on player's wisdom (maybe 50+((25-wisdom)*2) ?)
 */
int knows_language ( CHAR_DATA * ch, int language, CHAR_DATA * cch )
{
	short sn;

	if ( !IS_NPC ( ch ) && IS_IMMORTAL ( ch ) )
		return 100;

	if ( IS_NPC ( ch ) && !ch->speaks ) /* No langs = knows all for npcs */
		return 100;

	if ( IS_NPC ( ch ) && IS_SET ( ch->speaks, ( language & ~LANG_CLAN ) ) )
		return 100;

	/*
	 * everyone KNOWS common tongue
	 */
	if ( IS_SET ( language, LANG_COMMON ) )
		return 100;

	if ( language & LANG_CLAN )
	{
		/*
		 * Clan = common for mobs.. snicker.. -- Altrag
		 */
		if ( IS_NPC ( ch ) || IS_NPC ( cch ) )
			return 100;

		if ( ch->pcdata->clan == cch->pcdata->clan && ch->pcdata->clan != NULL )
			return 100;
	}

	if ( !IS_NPC ( ch ) )
	{
		int lang;
		/*
		 * Racial languages for PCs
		 */

		if ( IS_SET ( race_table[ch->race]->language, language ) )
			return 100;

		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
			if ( IS_SET ( language, lang_array[lang] ) && IS_SET ( ch->speaks, lang_array[lang] ) )
			{
				if ( ( sn = skill_lookup ( lang_names[lang] ) ) != -1 )
					return ch->pcdata->learned[sn];
			}
	}

	return 0;
}

bool can_learn_lang ( CHAR_DATA * ch, int language )
{
	if ( language & LANG_CLAN )
		return FALSE;

	if ( IS_NPC ( ch ) || IS_IMMORTAL ( ch ) )
		return FALSE;

	if ( race_table[ch->race]->language & language )
		return FALSE;

	if ( ch->speaks & language )
	{
		int lang;

		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
			if ( language & lang_array[lang] )
			{
				int sn;

				if ( ! ( VALID_LANGS & lang_array[lang] ) )
					return FALSE;

				if ( ( sn = skill_lookup ( lang_names[lang] ) ) < 0 )
				{
					bug ( "Can_learn_lang: valid language without sn: %d", lang );
					continue;
				}

				if ( ch->pcdata->learned[sn] >= 99 )
					return FALSE;
			}
	}

	if ( VALID_LANGS & language )
		return TRUE;

	return FALSE;
}

int const lang_array[] =
{
	LANG_COMMON, LANG_CLAN, LANG_UNKNOWN
};
char *const lang_names[] =
{
	"common", "clan", "unknown"
};

/* Note: does not count racial language.  This is intentional (for now). */
int countlangs ( int languages )
{
	int numlangs = 0;
	int looper;

	for ( looper = 0; lang_array[looper] != LANG_UNKNOWN; looper++ )
	{
		if ( lang_array[looper] == LANG_CLAN )
			continue;

		if ( languages & lang_array[looper] )
			numlangs++;
	}

	return numlangs;
}

void do_speak ( CHAR_DATA * ch, char *argument )
{
	int langs;
	char arg[MAX_INPUT_LENGTH];
	argument = one_argument ( argument, arg );

	if ( !str_cmp ( arg, "all" ) && IS_IMMORTAL ( ch ) )
	{
		set_char_color ( AT_SAY, ch );
		ch->speaking = ~LANG_CLAN;
		send_to_char ( "Now speaking all languages.\r\n", ch );
		return;
	}

	for ( langs = 0; lang_array[langs] != LANG_UNKNOWN; langs++ )
		if ( !str_prefix ( arg, lang_names[langs] ) )
			if ( knows_language ( ch, lang_array[langs], ch ) )
			{
				if ( lang_array[langs] == LANG_CLAN && ( IS_NPC ( ch ) || !ch->pcdata->clan ) )
					continue;

				ch->speaking = lang_array[langs];

				set_char_color ( AT_SAY, ch );

				ch_printf ( ch, "You now speak %s.\r\n", lang_names[langs] );

				return;
			}

	set_char_color ( AT_SAY, ch );

	send_to_char ( "You do not know that language.\r\n", ch );
}

void do_languages ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	int lang;
	argument = one_argument ( argument, arg );

	if ( arg[0] != STRING_NULL && !str_prefix ( arg, "learn" ) && !IS_IMMORTAL ( ch ) && !IS_NPC ( ch ) )
	{
		CHAR_DATA *sch;
		char arg2[MAX_INPUT_LENGTH];
		int sn;
		int prct;
		int prac;
		argument = one_argument ( argument, arg2 );

		if ( arg2[0] == STRING_NULL )
		{
			send_to_char ( "Learn which language?\r\n", ch );
			return;
		}

		for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		{
			if ( lang_array[lang] == LANG_CLAN )
				continue;

			if ( !str_prefix ( arg2, lang_names[lang] ) )
				break;
		}

		if ( lang_array[lang] == LANG_UNKNOWN )
		{
			send_to_char ( "That is not a language.\r\n", ch );
			return;
		}

		if ( ! ( VALID_LANGS & lang_array[lang] ) )
		{
			send_to_char ( "You may not learn that language.\r\n", ch );
			return;
		}

		if ( ( sn = skill_lookup ( lang_names[lang] ) ) < 0 )
		{
			send_to_char ( "That is not a language.\r\n", ch );
			return;
		}

		if ( race_table[ch->race]->language & lang_array[lang] || lang_array[lang] == LANG_COMMON || ch->pcdata->learned[sn] >= 99 )
		{
			act ( AT_PLAIN, "You are already fluent in $t.", ch, lang_names[lang], NULL, TO_CHAR );
			return;
		}

		for ( sch = ch->in_room->first_person; sch; sch = sch->next_in_room )
			if ( IS_NPC ( sch ) && xIS_SET ( sch->act, ACT_SCHOLAR )
			        && knows_language ( sch, ch->speaking, ch ) && knows_language ( sch, lang_array[lang], sch ) && ( !sch->speaking || knows_language ( ch, sch->speaking, sch ) ) )
				break;

		if ( !sch )
		{
			send_to_char ( "There is no one who can teach that language here.\r\n", ch );
			return;
		}

		if ( countlangs ( ch->speaks ) >= ( ch->level / 10 ) && ch->pcdata->learned[sn] <= 0 )
		{
			act ( AT_TELL, "$n tells you 'You may not learn a new language yet.'", sch, NULL, ch, TO_VICT );
			return;
		}

		/*
		 * 0..16 cha = 2 pracs, 17..25 = 1 prac. -- Altrag
		 */
		prac = 2 - ( get_curr_cha ( ch ) / 17 );

		if ( ch->practice < prac )
		{
			act ( AT_TELL, "$n tells you 'You do not have enough practices.'", sch, NULL, ch, TO_VICT );
			return;
		}

		ch->practice -= prac;

		/*
		 * Max 12% (5 + 4 + 3) at 24+ int and 21+ wis. -- Altrag
		 */
		prct = 5 + ( get_curr_int ( ch ) / 6 ) + ( get_curr_wis ( ch ) / 7 );
		ch->pcdata->learned[sn] += prct;
		ch->pcdata->learned[sn] = UMIN ( ch->pcdata->learned[sn], 99 );
		SET_BIT ( ch->speaks, lang_array[lang] );

		if ( ch->pcdata->learned[sn] == prct )
			act ( AT_PLAIN, "You begin lessons in $t.", ch, lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 )
			act ( AT_PLAIN, "You continue lessons in $t.", ch, lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 60 + prct )
			act ( AT_PLAIN, "You feel you can start communicating in $t.", ch, lang_names[lang], NULL, TO_CHAR );
		else if ( ch->pcdata->learned[sn] < 99 )
			act ( AT_PLAIN, "You become more fluent in $t.", ch, lang_names[lang], NULL, TO_CHAR );
		else
			act ( AT_PLAIN, "You now speak perfect $t.", ch, lang_names[lang], NULL, TO_CHAR );

		return;
	}

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( knows_language ( ch, lang_array[lang], ch ) )
		{
			if ( ch->speaking & lang_array[lang] || ( IS_NPC ( ch ) && !ch->speaking ) )
				set_char_color ( AT_SAY, ch );
			else
				set_char_color ( AT_PLAIN, ch );

			send_to_char ( lang_names[lang], ch );

			send_to_char ( "\r\n", ch );
		}

	send_to_char ( "\r\n", ch );

	return;
}

void do_racetalk ( CHAR_DATA * ch, char *argument )
{
	talk_channel ( ch, argument, CHANNEL_RACETALK, "racetalk" );
	return;
}

/**
 * Allows public addressing from ch to ch
 * syntax: talk <to player> <message>
 * Ex.
 * talk Cynshard hey tard
 * > You say to Cynshard, 'tard'.
 */
void do_talk ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int position;
	int speaking = -1, lang;
#ifndef SCRAMBLE

	for ( lang = 0; lang_array[lang] != LANG_UNKNOWN; lang++ )
		if ( ch->speaking & lang_array[lang] )
		{
			speaking = lang;
			break;
		}

#endif
	REMOVE_BIT ( ch->deaf, CHANNEL_SAYTO );

	argument = one_argument ( argument, arg );

	if ( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
	{
		send_to_char ( "Say to whom what?\r\n", ch );
		return;
	}

	if ( ( victim = get_char_room ( ch, arg ) ) == NULL )
	{
		send_to_char ( "They aren't here.\r\n", ch );
		return;
	}

	if ( ch == victim )
	{
		send_to_char ( "You have a nice little chat with yourself.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && ( victim->switched ) && !IS_AFFECTED ( victim->switched, AFF_POSSESS ) )
	{
		send_to_char ( "That player is switched.\r\n", ch );
		return;
	}
	else if ( !IS_NPC ( victim ) && ( !victim->desc ) )
	{
		send_to_char ( "That player is link-dead.\r\n", ch );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_AFK ) )
	{
		send_to_char ( "That player is afk.\r\n", ch );
		return;
	}

	if ( IS_SET ( victim->deaf, CHANNEL_SAYTO ) && ( !IS_IMMORTAL ( ch ) || ( get_trust ( ch ) < get_trust ( victim ) ) ) )
	{
		act ( AT_PLAIN, "$E has $S says turned off.", ch, NULL, victim, TO_CHAR );
		return;
	}

	if ( !IS_NPC ( victim ) && xIS_SET ( victim->act, PLR_SILENCE ) )
		send_to_char ( "That player is silenced.  They will receive your message but can not respond.\r\n", ch );
	/*
	 * make sure desc exists first  -Thoric
	 */
	if ( victim->desc && victim->desc->connected == CON_EDITING && get_trust ( ch ) < LEVEL_GOD )
	{
		act ( AT_PLAIN, "$E is currently in a writing buffer.  Please try again in a few minutes.", ch, 0, victim, TO_CHAR );
		return;
	}

	/*
	 * Check to see if target of tell is ignoring the sender
	 */
	if ( is_ignoring ( victim, ch ) )
	{
		/*
		 * If the sender is an imm then they cannot be ignored
		 */
		if ( !IS_IMMORTAL ( ch ) || get_trust ( victim ) > get_trust ( ch ) )
		{
			set_char_color ( AT_IGNORE, ch );
			ch_printf ( ch, "%s is ignoring you.\r\n", victim->name );
			return;
		}
		else
		{
			set_char_color ( AT_IGNORE, victim );
			ch_printf ( victim, "You attempt to ignore %s, but " "are unable to do so.\r\n", ch->name );
		}
	}

	act ( AT_SAY, "You say to $N, '$t'", ch, argument, victim, TO_CHAR );

	position = victim->position;
	victim->position = POS_STANDING;
#ifndef SCRAMBLE

	if ( speaking != -1 && ( !IS_NPC ( ch ) || ch->speaking ) )
	{
		int speakswell = UMIN ( knows_language ( victim, ch->speaking, ch ),
		                        knows_language ( ch, ch->speaking, victim ) );

		if ( speakswell < 85 )
			act ( AT_SAY, "$n says to you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );

#else
	if ( !knows_language ( vch, ch->speaking, ch ) && ( !IS_NPC ( ch ) || ch->speaking != 0 ) )
		act ( AT_SAY, "$n says to you '$t'", ch, translate ( speakswell, argument, lang_names[speaking] ), victim, TO_VICT );

#endif
		else
			act ( AT_SAY, "$n says to you '$t'", ch, argument, victim, TO_VICT );
	}

	else
		act ( AT_SAY, "$n says to you '$t'", ch, argument, victim, TO_VICT );

	if ( !xIS_SET ( ch->in_room->room_flags, ROOM_SILENCE ) )
		act ( AT_SAY, "$n says '$t' to $N.", ch, argument, victim, TO_NOTVICT );

	victim->position = position;

	if ( xIS_SET ( ch->in_room->room_flags, ROOM_LOGSPEECH ) )
	{
		sprintf ( buf, "%s: %s (says to) %s.", IS_NPC ( ch ) ? ch->short_descr : ch->name, argument, IS_NPC ( victim ) ? victim->short_descr : victim->name );
		append_to_file ( LOG_FILE, buf );
	}

	mprog_speech_trigger ( argument, ch );

	return;
}
