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
 *		      "Special procedure" module			    *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
/*
 * The following special functions are available for mobiles.
 */
DECLARE_SPEC_FUN ( spec_mental );
DECLARE_SPEC_FUN ( spec_physical );
DECLARE_SPEC_FUN ( spec_leader );
DECLARE_SPEC_FUN ( spec_general );
/*
 * Given a name, return the appropriate spec fun.
 */
SPEC_FUN *spec_lookup ( const char *name )
{
	if ( !str_cmp ( name, "spec_mental" ) )
		return spec_mental;
	if ( !str_cmp ( name, "spec_physical" ) )
		return spec_physical;
	if ( !str_cmp ( name, "spec_leader" ) )
		return spec_leader;
	if ( !str_cmp ( name, "spec_general" ) )
		return spec_general;
	return 0;
}

/*
 * Given a pointer, return the appropriate spec fun text.
 */
char *lookup_spec ( SPEC_FUN * special )
{
	if ( special == spec_mental )
		return "spec_mental";
	if ( special == spec_physical )
		return "spec_physical";
	if ( special == spec_leader )
		return "spec_leader";
	if ( special == spec_general )
		return "spec_general";
	return "";
}

bool spec_mental ( CHAR_DATA * ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POS_FIGHTING || ( victim = who_fighting ( ch ) ) == NULL )
		return FALSE;

	return TRUE;
}

bool spec_physical ( CHAR_DATA * ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POS_FIGHTING || ( victim = who_fighting ( ch ) ) == NULL )
		return FALSE;

	return TRUE;
}

bool spec_leader ( CHAR_DATA * ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POS_FIGHTING || ( victim = who_fighting ( ch ) ) == NULL )
		return FALSE;

	return TRUE;
}

bool spec_general ( CHAR_DATA * ch )
{
	CHAR_DATA *victim;

	if ( ch->position != POS_FIGHTING || ( victim = who_fighting ( ch ) ) == NULL )
		return FALSE;

	return TRUE;
}
