/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                                                                           *
 ****************************************************************************/
/*****************************************************
**     _________       __                           **
**     \_   ___ \_____|__| _____  ________  ___     **
**      /    \  \/_  __ \ |/     \/  ___/_ \/   \   **
**      \     \___|  | \/ |  | |  \___ \  / ) |  \  **
**       \______  /__| |__|__|_|  /____ \__/__|  /  **
**         ____\/____ _        \/ ___ \/      \/    **
**         \______   \ |_____  __| _/___            **
**          |    |  _/ |\__  \/ __ | __ \           **
**          |    |   \ |_/ __ \  / | ___/_          **
**          |_____  /__/____  /_  /___  /           **
**               \/Antipode\/  \/    \/             **
******************************************************
******************************************************
**       Copyright 2000-2003 Crimson Blade          **
******************************************************
** Contributors: Noplex, Krowe, Emberlyna, Lanthos  **
******************************************************/

/****************************************************************************
 * 	                       Version History                                *
 ****************************************************************************
 *  (v1.0) - Liquidtable converted into linked list, original 15 Smaug liqs *
 *           now read from a .dat file in /system                           *
 *  (v1.5) - OLC support added to create, edit, and delete liquids while    *
 *           the game is still running, automatic edit.                     *
 *  (v2.0) - Mixture support code added. Liquids can now be mixed with      *
 *           other liquids to form a result.                                *
 *  (v2.2) - Liquid statistics command added (liquids) shows all information*
 *           about the given liquid.                                        *
 *  (v2.3) - OLC addition for mixtures.                                     *
 *  (v2.4) - Mixtures are now saved into a seperate file and one linked list*
 *           because of some saving and loading issues. All the code has    *
 *           been modified to accept the new format. "liq_can_mix" function *
 *           introduced. "mix" command introduced to mix liquids.           *
 *  (v2.5) - Thanks to Samson for some polishing and bugfixing, we now have *
 *           a (hopefully) fully funcitonal copy =).                        *
 *  (v2.6) - "Fill" and "Empty" functions have been fixed to allow for the  *
 *           new liquidsystem.                                              *
 *  (v2.7) - Forgot to fix blood support... fixed.                          *
 *         - IS_VAMPIRE ifcheck placed in do_drink                          * 
 *  	   - Blood fix for blood on the ground.                             *
 *         - do_look/do_exam fix from Sirek.                                *
 *  (v2.8) - Ability to mix objects into liquids.                           *
 *	     (original code/concept -Sirek)                                 *
 ****************************************************************************/

/*
 * File: liquids.h
 * Name: Liquidtable Module (3.0b)
 * Author: John 'Noplex' Bellone (jbellone@comcast.net)
 * Terms:
 * If this file is to be re-disributed; you must send an email
 * to the author. All headers above the #include calls must be
 * kept intact. All license requirements must be met. License
 * can be found in the included license.txt document or on the
 * website.
 * Description:
 * This module is a rewrite of the original module which allowed for
 * a SMAUG mud to have a fully online editable liquidtable; adding liquids;
 * removing them; and editing them online. It allows an near-endless supply
 * of liquids for builder's to work with.
 * A second addition to this module allowed for builder's to create mixtures;
 * when two liquids were mixed together they would produce a different liquid.
 * Yet another adaptation to the above concept allowed for objects to be mixed
 * with liquids to produce a liquid.
 * This newest version offers a cleaner running code; smaller; and faster in
 * all ways around. Hopefully it'll knock out the old one ten fold ;)
 * Also in the upcoming 'testing' phase of this code; new additions will be added
 * including a better alchemey system for creating poitions as immortals; and as
 * mortals.
 */

#ifndef MIL
#define MIL MAX_INPUT_LENGTH
#endif

#ifndef MSL
#define MSL MAX_STRING_LENGTH
#endif

/* hard-coded max liquids */
#define MAX_LIQUIDS 100

typedef struct liquid_table LIQ_TABLE;
typedef struct mixture_list MIX_TABLE;

/* globals */
extern LIQ_TABLE *liquid_table[MAX_LIQUIDS];
extern MIX_TABLE *first_mixture;
extern MIX_TABLE *last_mixture;
extern int top_liquid;

typedef enum
{
   LIQTYPE_NORMAL, LIQTYPE_ALCOHOL, LIQTYPE_POISON, LIQTYPE_BLOOD, LIQTYPE_TOP
} liquid_struct_types;

/*
 * Conditions (moved from mud.h)
 */
typedef enum
{
  COND_DRUNK, COND_FULL, COND_THIRST, COND_BLOODTHIRST, MAX_CONDS
} conditions;

struct liquid_table
{
   char *name;
   char *shortdesc;
   char *color;
   int vnum;
   int type;
   int mod[MAX_CONDS];
};

struct mixture_list
{
   MIX_TABLE *next;
   MIX_TABLE *prev;
   char *name;
   int data[3];
   bool object;
};

DECLARE_DO_FUN( do_mix );
DECLARE_DO_FUN( do_setliquid );
DECLARE_DO_FUN( do_setmixture );
LIQ_TABLE *get_liq_vnum( int vnum );
void load_liquids( void );
void load_mixtures( void );
