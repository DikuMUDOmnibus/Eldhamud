/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ */

#include <string.h>
#include "./Headers/mud.h"
char *create_weather_string ( CHAR_DATA * ch, char *weather_string )
{
	char *combo, *single;
	char buf[MAX_INPUT_LENGTH];
	int temp, precip, wind;
	temp = ( ch->in_room->area->weather->temp + 3 * weath_unit - 1 ) / weath_unit;
	precip = ( ch->in_room->area->weather->precip + 3 * weath_unit - 1 ) / weath_unit;
	wind = ( ch->in_room->area->weather->wind + 3 * weath_unit - 1 ) / weath_unit;
	if ( xIS_OUTSIDE ( ch ) || IS_PLR_FLAG ( ch, PLR_ONMAP ) )
	{
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
		sprintf ( buf, "%s and %s. ", combo, single );
	}
	else
		strcpy ( buf, "" );
	weather_string = STRALLOC ( buf );
	return weather_string;
}
char *create_time_string ( CHAR_DATA * ch, char *time_string )
{
	char buf[MAX_INPUT_LENGTH];
	int n = number_bits( 2 );
	switch ( time_info.hour )
	{
		case 5:
		{
			char *echo_strings[4] = {
			"The fient yellow glow of the sun can be seen in the east, as the sun embarks on a new day. ",
			"The day has begun, the fient glow of the sun can be seen comming over the eastern horizon. ",
			"The sky slowly begins to glow to the east as the sun starts its accent into the sky. ",
			"The sun slowly embarks upon a new day, birds start to sing and crickets chirp as a new day begins. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		case 6:
		{
			char *echo_strings[4] = {
			"The firey glow of the sun is clearly visible as the sun rises over the horizon. ",
			"The sun rises in the east, continuing its steady climb into the sky. ",
			"The hazy sun rises over the horizon spreading its golden warmth across the land. ",
			"Day breaks as the sun lifts into the sky, the air erupts into noise as animals awake to the new day. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		case 7:
		case 8:
		case 9:
		case 10:
		case 11:
		{
			strcpy ( buf, "The sun moves ever so slowly towards its noonday zentih. " );
			break;
		}
		case 12:
		{
			char *echo_strings[4] = {
			"It's noon, shadows merge into nothingness as the full power of the sun can be felt beating down from above. ",
			"The intensity of the sun heralds the noon hour. ",
			"The sun's bright rays beat down upon your shoulders. ",
			"The sun's bright rays beat down upon your shoulders. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		case 13:
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		{
			char *echo_strings[4] = {
			"The sun continues on its westward journey accross the sky, increasing the length of shadows cast by its eternal warmth. ",
			"The reddish sun sets past the horizon. ",
			"The sky turns a reddish orange as the sun " "ends its journey. ",
			"The sun's radiance dims as it sinks in the sky. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		case 19:
		{
			char *echo_strings[4] = {
			"The sky turns a reddish orange as the sun ends its journey. ",
			"The reddish sun sets past the horizon. ",
			"The sky turns a reddish orange as the sun ends its journey. ",
			"The sun's radiance dims as it sinks in the sky. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		case 20:
		{
			char *echo_strings[4] = {
			"Day turns to night as twilight descends accross the lands. ",
			"Day turns to night as twilight descends accross the lands. ",
			"The night begins. ",
			"Twilight descends around you. "
			};
			strcpy ( buf, echo_strings[n] );
			break;			
		}
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		{
			char *echo_strings[4] = {
			"The sky it lit with a multitude of stars and heavenly wonders. The moon's makes its way accross the night sky illuminating the lands with its gentle diffused glow. ",
			"The moon's gentle glow diffuses through the night sky. ",
			"The moon's gentle glow diffuses through the night sky. ",
			"The night sky gleams with  glittering starlight. "
			};
			strcpy ( buf, echo_strings[n] );
			break;
		}
		break;
	}
	time_string = STRALLOC ( buf );
	return time_string;
}

char *create_sector_string ( CHAR_DATA * ch, char *sector_string )
{
	char buf[MAX_INPUT_LENGTH];
	short sector;

	if ( IS_PLR_FLAG( ch, PLR_ONMAP))
		sector = get_terrain ( ch->map, ch->x, ch->y );
	else
		sector = ch->in_room->sector_type;

	switch ( sector )
	{
		case 0:
		{
			strcpy ( buf, "Inside a great building. " );
			break;
		}
		case 1:
		{
			strcpy ( buf, "You are travelling along a smooth stretch of road. " );
			break;
		}
		case 2:
		{
			strcpy ( buf, "Rich farmland stretches out before you. " );
			break;
		}
		case 3:
		{
			strcpy ( buf, "Thick forest vegetation covers the ground all around. " );
			break;
		}
		case 4:
		{
			strcpy ( buf, "Gentle rolling hills stretch out all around. " );
			break;
		}
		case 5:
		{
			strcpy ( buf, "The rugged terrain of the mountains makes movement slow. " );
			break;
		}
		case 6:
		{
			strcpy ( buf, "The waters lap at your feet. " );
			break;
		}
		case 7:
		{
			strcpy ( buf, "The deep waters lap at your feet. " );
			break;
		}
		case 10:
		{
			strcpy ( buf, "The hot, dry desert sands seem to go on forever. " );
			break;
		}
		case 11:
		{
			strcpy ( buf, "The river churns and burbles beneath you. " );
			break;
		}
		case 14:
		{
			strcpy ( buf, "The jungle is extremely thick and humid. " );
			break;
		}
		case 15:
		{
			strcpy ( buf, "The swamps seem to surround everything. " );
			break;
		}
		case 16:
		{
			strcpy ( buf, "The frozen wastes seem to stretch on forever. " );
			break;
		}
		case 17:
		{
			strcpy ( buf, "The ice barely provides a stable footing. " );
			break;
		}
		case 20:
		{
			strcpy ( buf, "The soft sand makes for difficult walking. " );
			break;
		}
		case 27:
		{
			strcpy ( buf, "You are walking along a dusty trail. " );
			break;
		}
		case 28:
		{
			strcpy ( buf, "All around you the land has been scorched to ashes. " );
			break;
		}
		case 29:
		{
			strcpy ( buf, "Tall grass ripples in the wind. " );
			break;
		}
		case 30:
		{
			strcpy ( buf, "Scrub land stretches out as far as the eye can see. " );
			break;
		}
		case 31:
		{
			strcpy ( buf, "The land around you is dry and barren. " );
			break;
		}
		case 32:
		{
			strcpy ( buf, "A sturdy span of bridge passes over the water. " );
			break;
		}
		case 23:
		{
			strcpy ( buf, "A smooth strech of road. " );
			break;
		}
		break;
	}
	sector_string = STRALLOC ( buf );
	return sector_string;
}


char *random_description ( CHAR_DATA * ch, char *random_string )
{
	char desc_string[MAX_STRING_LENGTH];
	char temp[MAX_STRING_LENGTH];
	strcpy ( desc_string, create_sector_string ( ch, temp ) ); //sector specific
	strcat ( desc_string, create_time_string ( ch, temp ) ); //time specific
	strcat ( desc_string, create_weather_string ( ch, temp ) ); // wether specific
	random_string = STRALLOC ( desc_string );
	return random_string;
}
