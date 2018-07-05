/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                  Creation Selections Module                              *
 ****************************************************************************/
#include "./Headers/mud.h"



void do_select_gender ( CHAR_DATA * ch, char *argument )
{

	if ( ch->in_room->vnum != 4049 )
	{
		send_to_char ( "You can only select your gender while in the tutorial zone\r\n", ch );
		return;
	}
	if ( !str_cmp ( argument, "male" ) )
	{
		send_to_char ( "You have selected male.\r\n", ch );
		ch->sex = 1;
		return;
	}
	if ( !str_cmp ( argument, "female" ) )
	{
		send_to_char ( "You have selected female.\r\n", ch );
		ch->sex = 2;
		return;
	}
	if ( !str_cmp ( argument, "done" ) )
	{
		if ( ch->sex == 0 )
		{
			send_to_char ( "You must select a valid gender before you can select done.\r\n", ch );
			return;
		}
		else
		{
			char_from_room ( ch );
			char_to_room ( ch, get_room_index ( 4021 ) );
			do_look ( ch, "auto" );
			return;
		}
	}
	else
	{
		send_to_char ( "Valid selections are male and female, see help <gender> for more details.\r\n", ch );
		return;
	}
}

void do_select_fstyle ( CHAR_DATA * ch, char *argument )
{

	if ( ch->in_room->vnum != 4048 )
	{
		send_to_char ( "You can only select your fighting style while in the tutorial zone\r\n", ch );
		return;
	}
	if ( !str_cmp ( argument, "bushi" ) )
	{
		send_to_char ( "You have selected male.\r\n", ch );
		ch->Class = 1;
		return;
	}
	if ( !str_cmp ( argument, "shugenja" ) )
	{
		send_to_char ( "You have selected female.\r\n", ch );
		ch->Class = 2;
		return;
	}
	if ( !str_cmp ( argument, "done" ) )
	{
		if ( ch->Class == 0 )
		{
			send_to_char ( "You must select a valid fighting style before you can select done.\r\n", ch );
			return;
		}
		else
		{
		char_from_room ( ch );
		char_to_room ( ch, get_room_index ( 4049 ) );
		do_look ( ch, "auto" );
		return;
		}
	}
	else
	{
		send_to_char ( "Valid selections are bushi and shugenja, see help <style name> for mor details.\r\n", ch );
		return;
	}
}

void do_select_clan ( CHAR_DATA * ch, char *argument )
{

	if ( ch->in_room->vnum != 4047 )
	{
		send_to_char ( "You can only select your clan while in the tutorial zone\r\n", ch );
		return;
	}
	if ( !str_cmp ( argument, "badger" ) )
	{
		send_to_char ( "You have selected badger clan.\r\n", ch );
		ch->race = 1;
		return;
	}
	if ( !str_cmp ( argument, "crane" ) )
	{
		send_to_char ( "You have selected crane clan.\r\n", ch );
		ch->race = 2;
		return;
	}
	if ( !str_cmp ( argument, "dragon" ) )
	{
		send_to_char ( "You have selected dragon clan.\r\n", ch );
		ch->race = 3;
		return;
	}
	if ( !str_cmp ( argument, "tiger" ) )
	{
		send_to_char ( "You have selected tiger clan.\r\n", ch );
		ch->race = 4;
		return;
	}
	if ( !str_cmp ( argument, "naga" ) )
	{
		send_to_char ( "You have selected naga clan.\r\n", ch );
		ch->race = 5;
		char_from_room ( ch );
		char_to_room ( ch, get_room_index ( 4048 ) );
		do_look ( ch, "auto" );
		return;
	}
	if ( !str_cmp ( argument, "phoenix" ) )
	{
		send_to_char ( "You have selected phoenix clan.\r\n", ch );
		ch->race = 6;
		return;
	}
	if ( !str_cmp ( argument, "scorpion" ) )
	{
		send_to_char ( "You have selected scorpion clan.\r\n", ch );
		ch->race = 7;
		return;
	}
	if ( !str_cmp ( argument, "unicorn" ) )
	{
		send_to_char ( "You have selected unicorn clan.\r\n", ch );
		ch->race = 8;
		return;
	}
	if ( !str_cmp ( argument, "done" ) )
	{
		if ( ch->race == 0 )
		{
			send_to_char ( "You must select a valid clan before you can select done.\r\n", ch );
			return;
		}
		else
		{
		char_from_room ( ch );
		char_to_room ( ch, get_room_index ( 4048 ) );
		do_look ( ch, "auto" );
		return;
		}
	}
	else
	{
		send_to_char ( "Valid selections are: dragon,  naga,    scorpion, tiger,\r\n", ch );
		send_to_char ( "                      unicorn, phoenix, crane,    badger.\r\n", ch );
		send_to_char ( "See help <clan name> for more details.\r\n", ch );
		return;
	}
}

