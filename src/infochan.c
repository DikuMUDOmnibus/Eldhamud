/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ */

#include <stdio.h>
#include "./Headers/mud.h"
void talk_info ( short AT_COLOR, char *argument, bool sound)
{
	DESCRIPTOR_DATA *d;
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *original;
	int position;
	sprintf ( buf, "%s", argument );
	for ( d = first_descriptor; d; d = d->next )
	{
		original = d->original ? d->original : d->character;
		if ( ( d->connected == CON_PLAYING ) && !IS_SET ( original->deaf, CHANNEL_ANNOUNCE )
		        && !xIS_SET ( original->in_room->room_flags, ROOM_SILENCE ) )
		{
			position = original->position;
			original->position = POS_STANDING;
			act ( AT_COLOR, buf, original, NULL, NULL, TO_CHAR );
			original->position = position;
			if (sound)
				send_mip_sound ( d->character, "fury_ambiance_fanfare3.wav" );
		}
	}
}
void do_announce ( CHAR_DATA * ch, char *argument )
{
	char arg[MAX_INPUT_LENGTH];
	char buf[MAX_STRING_LENGTH];
	one_argument ( argument, arg );
	if ( arg[0] == STRING_NULL )
	{
		send_to_char ( "Syntax: Announce <message>\r\n", ch );
		return;
	}
	sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c: %s", argument);
	talk_info ( AT_PURPLE, buf, TRUE );
	return;
}
