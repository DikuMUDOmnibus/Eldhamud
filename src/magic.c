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
 *			     Spell handling module			    *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
/*
 * Local functions.
 */
void say_spell args( ( CHAR_DATA * ch, int sn ) );
ch_ret spell_affect args( ( int sn, int level, CHAR_DATA * ch, void *vo ) );
ch_ret spell_affectchar args( ( int sn, int level, CHAR_DATA * ch, void *vo ) );
int dispel_casting( AFFECT_DATA * paf, CHAR_DATA * ch, CHAR_DATA * victim, int affect, bool dispel );
bool can_charm( CHAR_DATA * ch );
/*
 * Is immune to a damage type
 */
bool is_immune( CHAR_DATA * ch, short damtype )
{
   switch ( damtype )
   {
      case SD_FIRE:
         if( IS_SET( ch->immune, RIS_FIRE ) )
            return TRUE;
      case SD_COLD:
         if( IS_SET( ch->immune, RIS_COLD ) )
            return TRUE;
      case SD_ELECTRICITY:
         if( IS_SET( ch->immune, RIS_ELECTRICITY ) )
            return TRUE;
      case SD_ENERGY:
         if( IS_SET( ch->immune, RIS_ENERGY ) )
            return TRUE;
      case SD_ACID:
         if( IS_SET( ch->immune, RIS_ACID ) )
            return TRUE;
      case SD_POISON:
         if( IS_SET( ch->immune, RIS_POISON ) )
            return TRUE;
      case SD_DRAIN:
         if( IS_SET( ch->immune, RIS_DRAIN ) )
            return TRUE;
   }
   return FALSE;
}

/*
 * Lookup a skill by name, only stopping at skills the player has.
 */
int ch_slookup( CHAR_DATA * ch, const char *name )
{
   int sn;
   if( IS_NPC( ch ) )
      return skill_lookup( name );
   for( sn = 0; sn < top_sn; sn++ )
   {
      if( !skill_table[sn]->name )
         break;
      if( ch->pcdata->learned[sn] > 0
          && ch->level >= skill_table[sn]->skill_level[ch->Class] && LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
         return sn;
   }
   return -1;
}

/*
 * Lookup an herb by name.
 */
int herb_lookup( const char *name )
{
   int sn;
   for( sn = 0; sn < top_herb; sn++ )
   {
      if( !herb_table[sn] || !herb_table[sn]->name )
         return -1;
      if( LOWER( name[0] ) == LOWER( herb_table[sn]->name[0] ) && !str_prefix( name, herb_table[sn]->name ) )
         return sn;
   }
   return -1;
}

/*
 * Lookup a personal skill
 * Unused for now.  In place to allow a player to have a custom spell/skill.
 * When this is put in make sure you put in cleanup code if you do any
 * sort of allocating memory in free_char --Shaddai
 */
int personal_lookup( CHAR_DATA * ch, const char *name )
{
   int sn;
   if( !ch->pcdata )
      return -1;
   for( sn = 0; sn < MAX_PERSONAL; sn++ )
   {
      if( !ch->pcdata->special_skills[sn] || !ch->pcdata->special_skills[sn]->name )
         return -1;
      if( LOWER( name[0] ) == LOWER( ch->pcdata->special_skills[sn]->name[0] ) && !str_prefix( name, ch->pcdata->special_skills[sn]->name ) )
         return sn;
   }
   return -1;
}

/*
 * Lookup a skill by name.
 */
int skill_lookup( const char *name )
{
   int sn;
   if( ( sn = bsearch_skill_exact( name, gsn_first_spell, gsn_first_skill - 1 ) ) == -1 )
      if( ( sn = bsearch_skill_exact( name, gsn_first_skill, gsn_first_weapon - 1 ) ) == -1 )
         if( ( sn = bsearch_skill_exact( name, gsn_first_weapon, gsn_first_tongue - 1 ) ) == -1 )
            if( ( sn = bsearch_skill_exact( name, gsn_first_tongue, gsn_top_sn - 1 ) ) == -1 )
               if( ( sn = bsearch_skill_prefix( name, gsn_first_spell, gsn_first_skill - 1 ) ) == -1 )
                  if( ( sn = bsearch_skill_prefix( name, gsn_first_skill, gsn_first_weapon - 1 ) ) == -1 )
                     if( ( sn = bsearch_skill_prefix( name, gsn_first_weapon, gsn_first_tongue - 1 ) ) == -1 )
                        if( ( sn = bsearch_skill_prefix( name, gsn_first_tongue, gsn_top_sn - 1 ) ) == -1 && gsn_top_sn < top_sn )
                        {
                           for( sn = gsn_top_sn; sn < top_sn; sn++ )
                           {
                              if( !skill_table[sn] || !skill_table[sn]->name )
                                 return -1;
                              if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
                                 return sn;
                           }
                           return -1;
                        }
   return sn;
}

/*
 * Return a skilltype pointer based on sn			-Thoric
 * Returns NULL if bad, unused or personal sn.
 */
SKILLTYPE *get_skilltype( int sn )
{
   if( sn >= TYPE_PERSONAL )
      return NULL;
   if( sn >= TYPE_HERB )
      return IS_VALID_HERB( sn - TYPE_HERB ) ? herb_table[sn - TYPE_HERB] : NULL;
   if( sn >= TYPE_HIT )
      return NULL;
   return IS_VALID_SN( sn ) ? skill_table[sn] : NULL;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for prefix matches
 */
int bsearch_skill_prefix( const char *name, int first, int top )
{
   int sn;
   for( ;; )
   {
      sn = ( first + top ) >> 1;
      if( !IS_VALID_SN( sn ) )
         return -1;
      if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] ) && !str_prefix( name, skill_table[sn]->name ) )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check for exact matches only
 */
int bsearch_skill_exact( const char *name, int first, int top )
{
   int sn;
   for( ;; )
   {
      sn = ( first + top ) >> 1;
      if( !IS_VALID_SN( sn ) )
         return -1;
      if( !str_cmp( name, skill_table[sn]->name ) )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}

/*
 * Perform a binary search on a section of the skill table	-Thoric
 * Each different section of the skill table is sorted alphabetically
 *
 * Check exact match first, then a prefix match
 */
int bsearch_skill( const char *name, int first, int top )
{
   int sn = bsearch_skill_exact( name, first, top );
   return ( sn == -1 ) ? bsearch_skill_prefix( name, first, top ) : sn;
}

/*
 * Perform a binary search on a section of the skill table
 * Each different section of the skill table is sorted alphabetically
 * Only match skills player knows				-Thoric
 */
int ch_bsearch_skill_prefix( CHAR_DATA * ch, const char *name, int first, int top )
{
   int sn;
   for( ;; )
   {
      sn = ( first + top ) >> 1;
      if( LOWER( name[0] ) == LOWER( skill_table[sn]->name[0] )
          && !str_prefix( name, skill_table[sn]->name ) && ch->pcdata->learned[sn] > 0 && ch->level >= skill_table[sn]->skill_level[ch->Class] )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}
int ch_bsearch_skill_exact( CHAR_DATA * ch, const char *name, int first, int top )
{
   int sn;
   for( ;; )
   {
      sn = ( first + top ) >> 1;
      if( !str_cmp( name, skill_table[sn]->name ) && ch->pcdata->learned[sn] > 0 && ch->level >= skill_table[sn]->skill_level[ch->Class] )
         return sn;
      if( first >= top )
         return -1;
      if( strcmp( name, skill_table[sn]->name ) < 1 )
         top = sn - 1;
      else
         first = sn + 1;
   }
   return -1;
}
int ch_bsearch_skill( CHAR_DATA * ch, const char *name, int first, int top )
{
   int sn = ch_bsearch_skill_exact( ch, name, first, top );
   return ( sn == -1 ) ? ch_bsearch_skill_prefix( ch, name, first, top ) : sn;
}
int find_spell( CHAR_DATA * ch, const char *name, bool know )
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_spell, gsn_first_skill - 1 );
   else
      return ch_bsearch_skill( ch, name, gsn_first_spell, gsn_first_skill - 1 );
}
int find_skill( CHAR_DATA * ch, const char *name, bool know )
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_skill, gsn_first_weapon - 1 );
   else
      return ch_bsearch_skill( ch, name, gsn_first_skill, gsn_first_weapon - 1 );
}
int find_weapon( CHAR_DATA * ch, const char *name, bool know )
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_weapon, gsn_first_tongue - 1 );
   else
      return ch_bsearch_skill( ch, name, gsn_first_weapon, gsn_first_tongue - 1 );
}
int find_tongue( CHAR_DATA * ch, const char *name, bool know )
{
   if( IS_NPC( ch ) || !know )
      return bsearch_skill( name, gsn_first_tongue, gsn_top_sn - 1 );
   else
      return ch_bsearch_skill( ch, name, gsn_first_tongue, gsn_top_sn - 1 );
}

/*
 * Handler to tell the victim which spell is being affected.
 * Shaddai
 */
int dispel_casting( AFFECT_DATA * paf, CHAR_DATA * ch, CHAR_DATA * victim, int affect, bool dispel )
{
   char buf[MAX_STRING_LENGTH];
   char *spell;
   SKILLTYPE *sktmp;
   bool is_mage = FALSE, has_detect = FALSE;
   EXT_BV ext_bv = meb( affect );
   is_mage = TRUE;
   if( IS_AFFECTED( ch, AFF_DETECT_MAGIC ) )
      has_detect = TRUE;
   if( paf )
   {
      if( ( sktmp = get_skilltype( paf->type ) ) == NULL )
         return 0;
      spell = sktmp->name;
   }
   else
      spell = affect_bit_name( &ext_bv );
   set_char_color( AT_MAGIC, ch );
   set_char_color( AT_HITME, victim );
   if( !can_see( ch, victim, FALSE ) )
      strcpy( buf, "Someone" );
   else
   {
      strcpy( buf, ( IS_NPC( victim ) ? victim->short_descr : victim->name ) );
      buf[0] = toupper( buf[0] );
   }
   if( dispel )
   {
      ch_printf( victim, "Your %s vanishes.\r\n", spell );
      if( is_mage && has_detect )
         ch_printf( ch, "%s's %s vanishes.\r\n", buf, spell );
      else
         return 0;   /* So we give the default Ok. Message */
   }
   else
   {
      if( is_mage && has_detect )
         ch_printf( ch, "%s's %s wavers but holds.\r\n", buf, spell );
      else
         return 0;   /* The wonderful Failed. Message */
   }
   return 1;
}

/*
 * Fancy message handling for a successful casting		-Thoric
 */
void successful_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
   {
      if( skill->hit_char && skill->hit_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->hit_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->type == SKILL_SPELL )
         act( AT_MAGIC, "Ok.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->hit_room && skill->hit_room[0] != STRING_NULL && str_cmp( skill->hit_room, SPELL_SILENT_MARKER ) )
      act( AT_MAGIC, skill->hit_room, ch, obj, victim, TO_NOTVICT );
   if( ch && victim && skill->hit_vict && skill->hit_vict[0] != STRING_NULL )
   {
      if( str_cmp( skill->hit_vict, SPELL_SILENT_MARKER ) )
      {
         if( ch != victim )
            act( AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_VICT );
         else
            act( AT_MAGIC, skill->hit_vict, ch, obj, victim, TO_CHAR );
      }
   }
   else if( ch && ch == victim && skill->type == SKILL_SPELL )
      act( AT_MAGIC, "Ok.", ch, NULL, NULL, TO_CHAR );
}

/*
 * Fancy message handling for a failed casting			-Thoric
 */
void failed_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
   {
      if( skill->miss_char && skill->miss_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->miss_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->type == SKILL_SPELL )
         act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->miss_room && skill->miss_room[0] != STRING_NULL && str_cmp( skill->miss_room, SPELL_SILENT_MARKER ) && str_cmp( skill->miss_room, "supress" ) )  /* Back Compat -- Alty */
      act( AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOTVICT );
   if( ch && victim && skill->miss_vict && skill->miss_vict[0] != STRING_NULL )
   {
      if( str_cmp( skill->miss_vict, SPELL_SILENT_MARKER ) )
      {
         if( ch != victim )
            act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT );
         else
            act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR );
      }
   }
   else if( ch && ch == victim )
   {
      if( skill->miss_char && skill->miss_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->miss_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->miss_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->type == SKILL_SPELL )
         act( chitme, "You failed.", ch, NULL, NULL, TO_CHAR );
   }
}

/*
 * Fancy message handling for being immune to something		-Thoric
 */
void immune_casting( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   short chitroom = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_ACTION );
   short chit = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HIT );
   short chitme = ( skill->type == SKILL_SPELL ? AT_MAGIC : AT_HITME );
   if( skill->target != TAR_CHAR_OFFENSIVE )
   {
      chit = chitroom;
      chitme = chitroom;
   }
   if( ch && ch != victim )
   {
      if( skill->imm_char && skill->imm_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->imm_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->miss_char && skill->miss_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->miss_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
         act( chit, "That appears to have no effect.", ch, NULL, NULL, TO_CHAR );
   }
   if( ch && skill->imm_room && skill->imm_room[0] != STRING_NULL )
   {
      if( str_cmp( skill->imm_room, SPELL_SILENT_MARKER ) )
         act( AT_MAGIC, skill->imm_room, ch, obj, victim, TO_NOTVICT );
   }
   else if( ch && skill->miss_room && skill->miss_room[0] != STRING_NULL )
   {
      if( str_cmp( skill->miss_room, SPELL_SILENT_MARKER ) )
         act( AT_MAGIC, skill->miss_room, ch, obj, victim, TO_NOTVICT );
   }
   if( ch && victim && skill->imm_vict && skill->imm_vict[0] != STRING_NULL )
   {
      if( str_cmp( skill->imm_vict, SPELL_SILENT_MARKER ) )
      {
         if( ch != victim )
            act( AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_VICT );
         else
            act( AT_MAGIC, skill->imm_vict, ch, obj, victim, TO_CHAR );
      }
   }
   else if( ch && victim && skill->miss_vict && skill->miss_vict[0] != STRING_NULL )
   {
      if( str_cmp( skill->miss_vict, SPELL_SILENT_MARKER ) )
      {
         if( ch != victim )
            act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_VICT );
         else
            act( AT_MAGIC, skill->miss_vict, ch, obj, victim, TO_CHAR );
      }
   }
   else if( ch && ch == victim )
   {
      if( skill->imm_char && skill->imm_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->imm_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->imm_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->miss_char && skill->miss_char[0] != STRING_NULL )
      {
         if( str_cmp( skill->hit_char, SPELL_SILENT_MARKER ) )
            act( AT_MAGIC, skill->hit_char, ch, obj, victim, TO_CHAR );
      }
      else if( skill->type == SKILL_SPELL || skill->type == SKILL_SKILL )
         act( chit, "That appears to have no affect.", ch, NULL, NULL, TO_CHAR );
   }
}

/*
 * Utter mystical words for an sn.
 */
void say_spell( CHAR_DATA * ch, int sn )
{
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CHAR_DATA *rch;
   char *pName;
   int iSyl;
   int length;
   SKILLTYPE *skill = get_skilltype( sn );
   struct syl_type
   {
      char *old;
      char *new;
   };
   static const struct syl_type syl_table[] = {
      {" ", " "},
      {"ar", "abra"},
      {"au", "kada"},
      {"bless", "fido"},
      {"blind", "nose"},
      {"bur", "mosa"},
      {"cu", "judi"},
      {"de", "oculo"},
      {"en", "unso"},
      {"light", "dies"},
      {"lo", "hi"},
      {"mor", "zak"},
      {"move", "sido"},
      {"ness", "lacri"},
      {"ning", "illa"},
      {"per", "duda"},
      {"palys", "iaddahs"},
      {"ra", "gru"},
      {"re", "candus"},
      {"son", "sabru"},
      {"tect", "infra"},
      {"tri", "cula"},
      {"ven", "nofo"},
      {"a", "a"}, {"b", "b"}, {"c", "q"}, {"d", "e"},
      {"e", "z"}, {"f", "y"}, {"g", "o"}, {"h", "p"},
      {"i", "u"}, {"j", "y"}, {"k", "t"}, {"l", "r"},
      {"m", "w"}, {"n", "i"}, {"o", "a"}, {"p", "s"},
      {"q", "d"}, {"r", "f"}, {"s", "g"}, {"t", "h"},
      {"u", "j"}, {"v", "z"}, {"w", "x"}, {"x", "n"},
      {"y", "l"}, {"z", "k"},
      {"", ""}
   };
   buf[0] = STRING_NULL;
   for( pName = skill->name; *pName != STRING_NULL; pName += length )
   {
      for( iSyl = 0; ( length = strlen( syl_table[iSyl].old ) ) != 0; iSyl++ )
      {
         if( !str_prefix( syl_table[iSyl].old, pName ) )
         {
            strcat( buf, syl_table[iSyl].new );
            break;
         }
      }
      if( length == 0 )
         length = 1;
   }
   sprintf( buf2, "$n utters the words, '%s'.", buf );
   sprintf( buf, "$n utters the words, '%s'.", skill->name );
   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch != ch )
      {
         if( is_same_map( ch, rch ) )
            act( AT_MAGIC, ch->Class == rch->Class ? buf : buf2, ch, NULL, rch, TO_VICT );
      }
   }
   return;
}

/*								    -Thoric
 * Fancy dice expression parsing complete with order of operations,
 * simple exponent support, dice support as well as a few extra
 * variables: L = level, H = hp, M = mana, V = move, S = str, X = dex
 *            I = int, W = wis, C = con, A = cha, U = luck, A = age
 *
 * Used for spell dice parsing, ie: 3d8+L-6
 *
 */
int rd_parse( CHAR_DATA * ch, int level, char *texp )
{
   int x, lop = 0, gop = 0, eop = 0;
   char operation;
   char *sexp[2];
   int total = 0, len = 0;
   /*
    * take care of nulls coming in
    */
   if( !texp || !strlen( texp ) )
      return 0;
   if( ( *texp == '(' ) && texp[strlen( texp ) - 1] == ')' )
   {
      texp[strlen( texp ) - 1] = STRING_NULL;
      texp++;
   }
   /*
    * check if the expresion is just a number
    */
   len = strlen( texp );
   if( len == 1 && isalpha( texp[0] ) )
   {
      switch ( texp[0] )
      {
         case 'L':
         case 'l':
            return level;
         case 'H':
         case 'h':
            return ch->hit;
         case 'M':
         case 'm':
            return ch->mana;
         case 'V':
         case 'v':
            return ch->move;
         case 'S':
         case 's':
            return get_curr_str( ch );
         case 'I':
         case 'i':
            return get_curr_int( ch );
         case 'W':
         case 'w':
            return get_curr_wis( ch );
         case 'X':
         case 'x':
            return get_curr_dex( ch );
         case 'C':
         case 'c':
            return get_curr_con( ch );
         case 'A':
         case 'a':
            return get_curr_cha( ch );
         case 'U':
         case 'u':
            return get_curr_lck( ch );
         case 'Y':
         case 'y':
            return get_age( ch );
      }
   }
   for( x = 0; x < len; ++x )
      if( !isdigit( texp[x] ) && !isspace( texp[x] ) )
         break;
   if( x == len )
      return atoi( texp );
   /*
    * break it into 2 parts
    */
   for( x = 0; x < strlen( texp ); ++x )
      switch ( texp[x] )
      {
         case '^':
            if( !total )
               eop = x;
            break;
         case '-':
         case '+':
            if( !total )
               lop = x;
            break;
         case '*':
         case '/':
         case '%':
         case 'd':
         case 'D':
         case '<':
         case '>':
         case '{':
         case '}':
         case '=':
            if( !total )
               gop = x;
            break;
         case '(':
            ++total;
            break;
         case ')':
            --total;
            break;
      }
   if( lop )
      x = lop;
   else if( gop )
      x = gop;
   else
      x = eop;
   operation = texp[x];
   texp[x] = STRING_NULL;
   sexp[0] = texp;
   sexp[1] = ( char * )( texp + x + 1 );
   /*
    * work it out
    */
   total = rd_parse( ch, level, sexp[0] );
   switch ( operation )
   {
      case '-':
         total -= rd_parse( ch, level, sexp[1] );
         break;
      case '+':
         total += rd_parse( ch, level, sexp[1] );
         break;
      case '*':
         total *= rd_parse( ch, level, sexp[1] );
         break;
      case '/':
         total /= rd_parse( ch, level, sexp[1] );
         break;
      case '%':
         total %= rd_parse( ch, level, sexp[1] );
         break;
      case 'd':
      case 'D':
         total = dice( total, rd_parse( ch, level, sexp[1] ) );
         break;
      case '<':
         total = ( total < rd_parse( ch, level, sexp[1] ) );
         break;
      case '>':
         total = ( total > rd_parse( ch, level, sexp[1] ) );
         break;
      case '=':
         total = ( total == rd_parse( ch, level, sexp[1] ) );
         break;
      case '{':
         total = UMIN( total, rd_parse( ch, level, sexp[1] ) );
         break;
      case '}':
         total = UMAX( total, rd_parse( ch, level, sexp[1] ) );
         break;
      case '^':
      {
         int y = rd_parse( ch, level, sexp[1] ), z = total;
         for( x = 1; x < y; ++x, z *= total )
            ;
         total = z;
         break;
      }
   }
   return total;
}

/* wrapper function so as not to destroy exp */
int dice_parse( CHAR_DATA * ch, int level, char *texp )
{
   char buf[MAX_INPUT_LENGTH];
   strcpy( buf, texp );
   return rd_parse( ch, level, buf );
}

/*
 * Make adjustments to saving throw based in RIS		-Thoric
 */
int ris_save( CHAR_DATA * ch, int schance, int ris )
{
   short modifier;
   modifier = 10;
   if( IS_SET( ch->immune, ris ) )
      modifier -= 10;
   if( IS_SET( ch->resistant, ris ) )
      modifier -= 2;
   if( IS_SET( ch->susceptible, ris ) )
   {
      if( IS_NPC( ch ) && IS_SET( ch->immune, ris ) )
         modifier += 0;
      else
         modifier += 2;
   }
   if( modifier <= 0 )
      return 1000;
   if( modifier == 10 )
      return schance;
   return ( schance * modifier ) / 10;
}

/*
 * Compute a saving throw.
 * Negative apply's make saving throw better.
 */
bool saves_poison_death( CHAR_DATA * victim )
{
   return chance( victim, victim->saving_poison_death );
}

bool saves_mental( CHAR_DATA * victim )
{
   return chance( victim, victim->saving_mental );
}

bool saves_physical( CHAR_DATA * victim )
{
   return chance( victim, victim->saving_physical );
}

bool saves_weapons( CHAR_DATA * victim )
{
   return chance( victim, victim->saving_weapons );
}

/*
 * Process the spell's required components, if any		-Thoric
 * -----------------------------------------------
 * T###		check for item of type ###
 * V#####	check for item of vnum #####
 * Kword	check for item with keyword 'word'
 * G#####	check if player has ##### amount of gold
 * H####	check if player has #### amount of hitpoints
 *
 * Special operators:
 * ! spell fails if player has this
 * + don't consume this component
 * @ decrease component's value[0], and extract if it reaches 0
 * # decrease component's value[1], and extract if it reaches 0
 * $ decrease component's value[2], and extract if it reaches 0
 * % decrease component's value[3], and extract if it reaches 0
 * ^ decrease component's value[4], and extract if it reaches 0
 * & decrease component's value[5], and extract if it reaches 0
 */
bool process_spell_components( CHAR_DATA * ch, int sn )
{
   SKILLTYPE *skill = get_skilltype( sn );
   char *comp = skill->components;
   char *check;
   char arg[MAX_INPUT_LENGTH];
   bool consume, fail, found;
   int val, value;
   OBJ_DATA *obj;
   /*
    * if no components necessary, then everything is cool
    */
   if( !comp || comp[0] == STRING_NULL )
      return TRUE;
   while( comp[0] != STRING_NULL )
   {
      comp = one_argument( comp, arg );
      consume = TRUE;
      fail = found = FALSE;
      val = -1;
      switch ( arg[1] )
      {
         default:
            check = arg + 1;
            break;
         case '!':
            check = arg + 2;
            fail = TRUE;
            break;
         case '+':
            check = arg + 2;
            consume = FALSE;
            break;
         case '@':
            check = arg + 2;
            val = 0;
            break;
         case '#':
            check = arg + 2;
            val = 1;
            break;
         case '$':
            check = arg + 2;
            val = 2;
            break;
         case '%':
            check = arg + 2;
            val = 3;
            break;
         case '^':
            check = arg + 2;
            val = 4;
            break;
         case '&':
            check = arg + 2;
            val = 5;
            break;
            /*
             * reserve '*', '(' and ')' for v6, v7 and v8
             */
      }
      value = atoi( check );
      obj = NULL;
      switch ( UPPER( arg[0] ) )
      {
         case 'T':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( obj->item_type == value )
               {
                  if( fail )
                  {
                     send_to_char( "Something disrupts the casting of this spell...\r\n", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'V':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( obj->pIndexData->vnum == value )
               {
                  if( fail )
                  {
                     send_to_char( "Something disrupts the casting of this spell...\r\n", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'K':
            for( obj = ch->first_carrying; obj; obj = obj->next_content )
               if( nifty_is_name( check, obj->name ) )
               {
                  if( fail )
                  {
                     send_to_char( "Something disrupts the casting of this spell...\r\n", ch );
                     return FALSE;
                  }
                  found = TRUE;
                  break;
               }
            break;
         case 'G':
            if( ch->gold >= value )
            {
               if( fail )
               {
                  send_to_char( "Something disrupts the casting of this spell...\r\n", ch );
                  return FALSE;
               }
               else
               {
                  if( consume )
                  {
                     set_char_color( AT_GOLD, ch );
                     send_to_char( "You feel a little lighter...\r\n", ch );
                     ch->gold -= value;
                  }
                  continue;
               }
            }
            break;
         case 'H':
            if( ch->hit >= value )
            {
               if( fail )
               {
                  send_to_char( "Something disrupts the casting of this spell...\r\n", ch );
                  return FALSE;
               }
               else
               {
                  if( consume )
                  {
                     set_char_color( AT_BLOOD, ch );
                     send_to_char( "You feel a little weaker...\r\n", ch );
                     ch->hit -= value;
                     update_pos( ch );
                  }
                  continue;
               }
            }
            break;
      }
      /*
       * having this component would make the spell fail... if we get
       * here, then the caster didn't have that component
       */
      if( fail )
         continue;
      if( !found )
      {
         send_to_char( "Something is missing...\r\n", ch );
         return FALSE;
      }
      if( obj )
      {
         if( val >= 0 && val < 6 )
         {
            separate_obj( obj );
            if( obj->value[val] <= 0 )
            {
               act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
               act( AT_MAGIC, "$p disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
               extract_obj( obj );
               return FALSE;
            }
            else if( --obj->value[val] == 0 )
            {
               act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
               act( AT_MAGIC, "$p glows briefly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
               extract_obj( obj );
            }
            else
               act( AT_MAGIC, "$p glows briefly and a whisp of smoke rises from it.", ch, obj, NULL, TO_CHAR );
         }
         else if( consume )
         {
            separate_obj( obj );
            act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_CHAR );
            act( AT_MAGIC, "$p glows brightly, then disappears in a puff of smoke!", ch, obj, NULL, TO_ROOM );
            extract_obj( obj );
         }
         else
         {
            int count = obj->count;
            obj->count = 1;
            act( AT_MAGIC, "$p glows briefly.", ch, obj, NULL, TO_CHAR );
            obj->count = count;
         }
      }
   }
   return TRUE;
}

int pAbort;
/*
 * Locate targets.
 */
/* Turn off annoying message and just abort if needed */
bool silence_locate_targets;
void *locate_targets( CHAR_DATA * ch, char *arg, int sn, CHAR_DATA ** victim, OBJ_DATA ** obj )
{
   SKILLTYPE *skill = get_skilltype( sn );
   void *vo = NULL;
   *victim = NULL;
   *obj = NULL;
   switch ( skill->target )
   {
      default:
         bug( "Do_cast: bad target for sn %d.", sn );
         return &pAbort;
      case TAR_IGNORE:
         break;
      case TAR_CHAR_OFFENSIVE:
      {
         if( arg[0] == STRING_NULL )
         {
            if( ( *victim = who_fighting( ch ) ) == NULL )
            {
               if( !silence_locate_targets )
                  send_to_char( "Cast the spell on whom?\r\n", ch );
               return &pAbort;
            }
         }
         else
         {
            if( ( *victim = get_char_room( ch, arg ) ) == NULL )
            {
               if( !silence_locate_targets )
                  send_to_char( "They aren't here.\r\n", ch );
               return &pAbort;
            }
         }
      }
         if( !IS_NPC( ch ) )
         {
            if( !IS_NPC( *victim ) )
            {
               if( get_timer( ch, TIMER_PKILLED ) > 0 )
               {
                  if( !silence_locate_targets )
                     send_to_char( "You have been killed in the last 5 minutes.\r\n", ch );
                  return &pAbort;
               }
               if( get_timer( *victim, TIMER_PKILLED ) > 0 )
               {
                  if( !silence_locate_targets )
                     send_to_char( "This player has been killed in the last 5 minutes.\r\n", ch );
                  return &pAbort;
               }
               if( xIS_SET( ch->act, PLR_NICE ) && ch != *victim )
               {
                  if( !silence_locate_targets )
                     send_to_char( "You are too nice to attack another player.\r\n", ch );
                  return &pAbort;
               }
               if( *victim != ch )
               {
                  if( !silence_locate_targets )
                     send_to_char( "You really shouldn't do this to another player...\r\n", ch );
                  else if( who_fighting( *victim ) != ch )
                  {
                     /*
                      * Only auto-attack those that are hitting you.
                      */
                     return &pAbort;
                  }
               }
            }
            if( IS_AFFECTED( ch, AFF_CHARM ) && ch->master == *victim )
            {
               if( !silence_locate_targets )
                  send_to_char( "You can't do that on your own follower.\r\n", ch );
               return &pAbort;
            }
         }
         check_illegal_pk( ch, *victim );
         vo = ( void * )*victim;
         break;
      case TAR_CHAR_DEFENSIVE:
      {
         if( arg[0] == STRING_NULL )
            *victim = ch;
         else
         {
            if( ( *victim = get_char_room( ch, arg ) ) == NULL )
            {
               if( !silence_locate_targets )
                  send_to_char( "They aren't here.\r\n", ch );
               return &pAbort;
            }
         }
      }
         if( ch == *victim && SPELL_FLAG( get_skilltype( sn ), SF_NOSELF ) )
         {
            if( !silence_locate_targets )
               send_to_char( "You can't cast this on yourself!\r\n", ch );
            return &pAbort;
         }
         vo = ( void * )*victim;
         break;
      case TAR_CHAR_SELF:
         if( arg[0] != STRING_NULL && !nifty_is_name( arg, ch->name ) )
         {
            if( !silence_locate_targets )
               send_to_char( "You cannot cast this spell on another.\r\n", ch );
            return &pAbort;
         }
         vo = ( void * )ch;
         break;
      case TAR_OBJ_INV:
      {
         if( arg[0] == STRING_NULL )
         {
            if( !silence_locate_targets )
               send_to_char( "What should the spell be cast upon?\r\n", ch );
            return &pAbort;
         }
         if( ( *obj = get_obj_carry( ch, arg ) ) == NULL )
         {
            if( !silence_locate_targets )
               send_to_char( "You are not carrying that.\r\n", ch );
            return &pAbort;
         }
      }
         vo = ( void * )*obj;
         break;
   }
   return vo;
}

/*
 * The kludgy global is for spells who want more stuff from command line.
 */
char *target_name;
char *ranged_target_name = NULL;
/*
 * Cast a spell.  Multi-caster and component support by Thoric
 */
void do_cast( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   static char staticbuf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   void *vo = NULL;
   int mana;
   int blood;
   int sn;
   ch_ret retcode;
   bool dont_wait = FALSE;
   SKILLTYPE *skill = NULL;
   struct timeval time_used;
   retcode = rNONE;
   switch ( ch->substate )
   {
      default:
         /*
          * no ordering charmed mobs to cast spells
          */
         if( IS_NPC( ch ) && ( IS_AFFECTED( ch, AFF_CHARM ) || IS_AFFECTED( ch, AFF_POSSESS ) ) )
         {
            send_to_char( "You can't seem to do that right now...\r\n", ch );
            return;
         }
         if( xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "You failed.\r\n", ch );
            return;
         }
         target_name = one_argument( argument, arg1 );
         one_argument( target_name, arg2 );
         if( ranged_target_name )
            DISPOSE( ranged_target_name );
         ranged_target_name = str_dup( target_name );
         if( arg1[0] == STRING_NULL )
         {
            send_to_char( "Cast which what where?\r\n", ch );
            return;
         }
         /*
          * Regular mortal spell casting
          */
         if( get_trust( ch ) < LEVEL_GOD )
         {
            if( ( sn = find_spell( ch, arg1, TRUE ) ) < 0 || ( !IS_NPC( ch ) && ch->level < skill_table[sn]->skill_level[ch->Class] ) )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return;
            }
            if( ( skill = get_skilltype( sn ) ) == NULL )
            {
               send_to_char( "You can't do that right now...\r\n", ch );
               return;
            }
         }
         else
            /*
             * Godly "spell builder" spell casting with debugging messages
             */
         {
            if( ( sn = skill_lookup( arg1 ) ) < 0 )
            {
               send_to_char( "We didn't create that yet...\r\n", ch );
               return;
            }
            if( sn >= MAX_SKILL )
            {
               send_to_char( "Hmm... that might hurt.\r\n", ch );
               return;
            }
            if( ( skill = get_skilltype( sn ) ) == NULL )
            {
               send_to_char( "Something is severely wrong with that one...\r\n", ch );
               return;
            }
            if( skill->type != SKILL_SPELL )
            {
               send_to_char( "That isn't a spell.\r\n", ch );
               return;
            }
            if( !skill->spell_fun )
            {
               send_to_char( "We didn't finish that one yet...\r\n", ch );
               return;
            }
         }
         /*
          * Something else removed by Merc         -Thoric
          */
         /*
          * Band-aid alert!  !IS_NPC check -- Blod
          */
         if( ch->position < skill->minimum_position && !IS_NPC( ch ) )
         {
            switch ( ch->position )
            {
               default:
                  send_to_char( "You can't concentrate enough.\r\n", ch );
                  break;
               case POS_SITTING:
                  send_to_char( "You can't summon enough energy sitting down.\r\n", ch );
                  break;
               case POS_RESTING:
                  send_to_char( "You're too relaxed to cast that spell.\r\n", ch );
                  break;
               case POS_FIGHTING:
                  if( skill->minimum_position <= POS_EVASIVE )
                  {
                     send_to_char( "This fighting style is too demanding for that!\r\n", ch );
                  }
                  else
                  {
                     send_to_char( "No way!  You are still fighting!\r\n", ch );
                  }
                  break;
               case POS_DEFENSIVE:
                  if( skill->minimum_position <= POS_EVASIVE )
                  {
                     send_to_char( "This fighting style is too demanding for that!\r\n", ch );
                  }
                  else
                  {
                     send_to_char( "No way!  You are still fighting!\r\n", ch );
                  }
                  break;
               case POS_AGGRESSIVE:
                  if( skill->minimum_position <= POS_EVASIVE )
                  {
                     send_to_char( "This fighting style is too demanding for that!\r\n", ch );
                  }
                  else
                  {
                     send_to_char( "No way!  You are still fighting!\r\n", ch );
                  }
                  break;
               case POS_BERSERK:
                  if( skill->minimum_position <= POS_EVASIVE )
                  {
                     send_to_char( "This fighting style is too demanding for that!\r\n", ch );
                  }
                  else
                  {
                     send_to_char( "No way!  You are still fighting!\r\n", ch );
                  }
                  break;
               case POS_EVASIVE:
                  send_to_char( "No way!  You are still fighting!\r\n", ch );
                  break;
               case POS_SLEEPING:
                  send_to_char( "You dream about great feats of magic.\r\n", ch );
                  break;
            }
            return;
         }
         if( skill->spell_fun == spell_null )
         {
            send_to_char( "That's not a spell!\r\n", ch );
            return;
         }
         if( !skill->spell_fun )
         {
            send_to_char( "You cannot cast that... yet.\r\n", ch );
            return;
         }
         if( !IS_NPC( ch ) /* fixed by Thoric */
             && !IS_IMMORTAL( ch ) && skill->guild != CLASS_NONE && ( !ch->pcdata->clan || skill->guild != ch->pcdata->clan->Class ) )
         {
            send_to_char( "That is only available to members of a certain guild.\r\n", ch );
            return;
         }
         /*
          * Mystaric, 980908 - Added checks for spell sector type
          */
         if( !ch->in_room || ( skill->spell_sector && !IS_SET( skill->spell_sector, ( 1 << ch->in_room->sector_type ) ) ) )
         {
            send_to_char( "You can not cast that here.\r\n", ch );
            return;
         }
         mana = IS_NPC( ch ) ? 0 : UMAX( skill->min_mana, 100 / ( 2 + ch->level - skill->skill_level[ch->Class] ) );
         /*
          * Locate targets.
          */
         vo = locate_targets( ch, arg2, sn, &victim, &obj );
         if( vo == &pAbort )
            return;
         if( !IS_NPC( ch ) && victim && !IS_NPC( victim ) && CAN_PKILL( victim ) && !CAN_PKILL( ch ) )
         {
            set_char_color( AT_MAGIC, ch );
            send_to_char( "The gods will not permit you to cast spells on that character.\r\n", ch );
            return;
         }
         if( !IS_NPC( ch ) && ch->mana < mana )
         {
            send_to_char( "You don't have enough mana.\r\n", ch );
            return;
         }
         if( skill->participants <= 1 )
            break;
         /*
          * multi-participant spells         -Thoric
          */
         add_timer( ch, TIMER_DO_FUN, UMIN( skill->beats / 10, 3 ), do_cast, 1 );
         act( AT_MAGIC, "You begin to chant...", ch, NULL, NULL, TO_CHAR );
         act( AT_MAGIC, "$n begins to chant...", ch, NULL, NULL, TO_ROOM );
         sprintf( staticbuf, "%s %s", arg2, target_name );
         ch->alloc_ptr = str_dup( staticbuf );
         ch->tempnum = sn;
         return;
      case SUB_TIMER_DO_ABORT:
         DISPOSE( ch->alloc_ptr );
         if( IS_VALID_SN( ( sn = ch->tempnum ) ) )
         {
            if( ( skill = get_skilltype( sn ) ) == NULL )
            {
               send_to_char( "Something went wrong...\r\n", ch );
               bug( "do_cast: SUB_TIMER_DO_ABORT: bad sn %d", sn );
               return;
            }
            mana = IS_NPC( ch ) ? 0 : UMAX( skill->min_mana, 100 / ( 2 + ch->level - skill->skill_level[ch->Class] ) );
            if( ch->level < LEVEL_IMMORTAL ) /* so imms dont lose mana */
               ch->mana -= mana / 3;
         }
         set_char_color( AT_MAGIC, ch );
         send_to_char( "You stop chanting...\r\n", ch );
         /*
          * should add chance of backfire here
          */
         return;
      case 1:
         sn = ch->tempnum;
         if( ( skill = get_skilltype( sn ) ) == NULL )
         {
            send_to_char( "Something went wrong...\r\n", ch );
            bug( "do_cast: substate 1: bad sn %d", sn );
            return;
         }
         if( !ch->alloc_ptr || !IS_VALID_SN( sn ) || skill->type != SKILL_SPELL )
         {
            send_to_char( "Something cancels out the spell!\r\n", ch );
            bug( "do_cast: ch->alloc_ptr NULL or bad sn (%d)", sn );
            return;
         }
         mana = IS_NPC( ch ) ? 0 : UMAX( skill->min_mana, 100 / ( 2 + ch->level - skill->skill_level[ch->Class] ) );
         blood = UMAX( 1, ( mana + 4 ) / 8 );
         strcpy( staticbuf, ch->alloc_ptr );
         target_name = one_argument( staticbuf, arg2 );
         DISPOSE( ch->alloc_ptr );
         ch->substate = SUB_NONE;
         if( skill->participants > 1 )
         {
            int cnt = 1;
            CHAR_DATA *tmp;
            TIMER *t;
            for( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
               if( tmp != ch
                   && ( t = get_timerptr( tmp, TIMER_DO_FUN ) ) != NULL
                   && t->count >= 1 && t->do_fun == do_cast && tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp( tmp->alloc_ptr, staticbuf ) )
                  ++cnt;
            if( cnt >= skill->participants )
            {
               for( tmp = ch->in_room->first_person; tmp; tmp = tmp->next_in_room )
                  if( tmp != ch
                      && ( t = get_timerptr( tmp, TIMER_DO_FUN ) ) != NULL
                      && t->count >= 1 && t->do_fun == do_cast && tmp->tempnum == sn && tmp->alloc_ptr && !str_cmp( tmp->alloc_ptr, staticbuf ) )
                  {
                     extract_timer( tmp, t );
                     act( AT_MAGIC, "Channeling your energy into $n, you help cast the spell!", ch, NULL, tmp, TO_VICT );
                     act( AT_MAGIC, "$N channels $S energy into you!", ch, NULL, tmp, TO_CHAR );
                     act( AT_MAGIC, "$N channels $S energy into $n!", ch, NULL, tmp, TO_NOTVICT );
                     learn_from_success( tmp, sn );
                     tmp->mana -= mana;
                     tmp->substate = SUB_NONE;
                     tmp->tempnum = -1;
                     DISPOSE( tmp->alloc_ptr );
                  }
               dont_wait = TRUE;
               send_to_char( "You concentrate all the energy into a burst of mystical words!\r\n", ch );
               vo = locate_targets( ch, arg2, sn, &victim, &obj );
               if( vo == &pAbort )
                  return;
            }
            else
            {
               set_char_color( AT_MAGIC, ch );
               send_to_char( "There was not enough power for the spell to succeed...\r\n", ch );
               if( ch->level < LEVEL_IMMORTAL ) /* so imms dont lose mana */
                  ch->mana -= mana / 2;
               learn_from_failure( ch, sn );
               return;
            }
         }
   }
   /*
    * uttering those magic words unless casting "ventriloquate"
    */
   if( str_cmp( skill->name, "ventriloquate" ) )
      say_spell( ch, sn );
   if( !dont_wait )
      WAIT_STATE( ch, skill->beats );
   /*
    * Getting ready to cast... check for spell components -Thoric
    */
   if( !process_spell_components( ch, sn ) )
   {
      if( ch->level < LEVEL_IMMORTAL ) /* so imms dont lose mana */
         ch->mana -= mana / 2;
      learn_from_failure( ch, sn );
      return;
   }
   if( !IS_NPC( ch ) && ( number_percent(  ) + skill->difficulty * 5 ) > ch->pcdata->learned[sn] )
   {
      /*
       * Some more interesting loss of concentration messages  -Thoric
       */
      switch ( number_bits( 2 ) )
      {
         case 0: /* too busy */
            if( ch->fighting )
               send_to_char( "This round of battle is too hectic to concentrate properly.\r\n", ch );
            else
               send_to_char( "You lost your concentration.\r\n", ch );
            break;
         case 1: /* irritation */
            if( number_bits( 2 ) == 0 )
            {
               switch ( number_bits( 2 ) )
               {
                  case 0:
                     send_to_char( "A tickle in your nose prevents you from keeping your concentration.\r\n", ch );
                     break;
                  case 1:
                     send_to_char( "An itch on your leg keeps you from properly casting your spell.\r\n", ch );
                     break;
                  case 2:
                     send_to_char( "Something in your throat prevents you from uttering the proper phrase.\r\n", ch );
                     break;
                  case 3:
                     send_to_char( "A twitch in your eye disrupts your concentration for a moment.\r\n", ch );
                     break;
               }
            }
            else
               send_to_char( "Something distracts you, and you lose your concentration.\r\n", ch );
            break;
         case 2: /* not enough time */
            if( ch->fighting )
               send_to_char( "There wasn't enough time this round to complete the casting.\r\n", ch );
            else
               send_to_char( "You lost your concentration.\r\n", ch );
            break;
         case 3:
            send_to_char( "You get a mental block mid-way through the casting.\r\n", ch );
            break;
      }
      if( ch->level < LEVEL_IMMORTAL ) /* so imms dont lose mana */
         ch->mana -= mana / 2;
      learn_from_failure( ch, sn );
      return;
   }
   else
   {
      ch->mana -= mana;
      /*
       * check for immunity to magic if victim is known...
       * and it is a TAR_CHAR_DEFENSIVE/SELF spell
       * otherwise spells will have to check themselves
       */
      if( ( ( skill->target == TAR_CHAR_DEFENSIVE || skill->target == TAR_CHAR_SELF ) && victim && IS_SET( victim->immune, RIS_MAGIC ) ) )
      {
         immune_casting( skill, ch, victim, NULL );
         retcode = rSPELL_FAILED;
      }
      else
      {
         start_timer( &time_used );
         if( !IS_IMMORTAL( ch ) )
            ch->mana -= mana;
         retcode = ( *skill->spell_fun ) ( sn, ch->level, ch, vo );
         end_timer( &time_used );
         update_userec( &time_used, &skill->userec );
      }
   }
   if( ch->in_room && IS_SET( ch->in_room->area->flags, AFLAG_SPELLLIMIT ) )
      ch->in_room->area->curr_spell_count++;
   if( retcode == rCHAR_DIED || retcode == rERROR || char_died( ch ) )
      return;
   /*
    * learning
    */
   if( retcode != rSPELL_FAILED )
      learn_from_success( ch, sn );
   else
      learn_from_failure( ch, sn );
   /*
    * favor adjustments
    */
   if( victim && victim != ch && !IS_NPC( victim ) && skill->target == TAR_CHAR_DEFENSIVE )
      adjust_favor( ch, 7, 1 );
   if( victim && victim != ch && !IS_NPC( ch ) && skill->target == TAR_CHAR_DEFENSIVE )
      adjust_favor( victim, 13, 1 );
   if( victim && victim != ch && !IS_NPC( ch ) && skill->target == TAR_CHAR_OFFENSIVE )
      adjust_favor( ch, 4, 1 );
   /*
    * Fixed up a weird mess here, and added double safeguards   -Thoric
    */
   if( skill->target == TAR_CHAR_OFFENSIVE && victim && !char_died( victim ) && victim != ch )
   {
      CHAR_DATA *vch, *vch_next;
      for( vch = ch->in_room->first_person; vch; vch = vch_next )
      {
         vch_next = vch->next_in_room;
         if( vch == victim )
         {
            if( vch->master != ch && !vch->fighting )
               retcode = multi_hit( vch, ch, TYPE_UNDEFINED );
            break;
         }
      }
   }
   return;
}

/*
 * Cast spells at targets using a magical object.
 */
ch_ret obj_cast_spell( int sn, int level, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj )
{
   void *vo;
   ch_ret retcode = rNONE;
   int levdiff = ch->level - level;
   SKILLTYPE *skill = get_skilltype( sn );
   struct timeval time_used;
   if( sn == -1 )
      return retcode;
   if( !skill || !skill->spell_fun )
   {
      bug( "Obj_cast_spell: bad sn %d.", sn );
      return rERROR;
   }
   if( xIS_SET( ch->in_room->room_flags, ROOM_NO_MAGIC ) )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "Nothing seems to happen...\r\n", ch );
      return rNONE;
   }
   if( xIS_SET( ch->in_room->room_flags, ROOM_SAFE ) && skill->target == TAR_CHAR_OFFENSIVE )
   {
      set_char_color( AT_MAGIC, ch );
      send_to_char( "Nothing seems to happen...\r\n", ch );
      return rNONE;
   }
   /*
    * Basically this was added to cut down on level 5 players using level
    * 40 scrolls in battle too often ;)    -Thoric
    */
   if( ( skill->target == TAR_CHAR_OFFENSIVE || number_bits( 7 ) == 1 ) /* 1/128 chance if non-offensive */
       && !chance( ch, 95 + levdiff ) )
   {
      switch ( number_bits( 2 ) )
      {
         case 0:
            failed_casting( skill, ch, victim, NULL );
            break;
         case 1:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if( victim )
               act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
            return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
         case 2:
            failed_casting( skill, ch, victim, NULL );
            break;
         case 3:
            act( AT_MAGIC, "The $t spell backfires!", ch, skill->name, victim, TO_CHAR );
            if( victim )
               act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_VICT );
            act( AT_MAGIC, "$n's $t spell backfires!", ch, skill->name, victim, TO_NOTVICT );
            return damage( ch, ch, number_range( 1, level ), TYPE_UNDEFINED );
      }
      return rNONE;
   }
   target_name = "";
   switch ( skill->target )
   {
      default:
         bug( "Obj_cast_spell: bad target for sn %d.", sn );
         return rERROR;
      case TAR_IGNORE:
         vo = NULL;
         if( victim )
            target_name = victim->name;
         else if( obj )
            target_name = obj->name;
         break;
      case TAR_CHAR_OFFENSIVE:
         if( victim != ch )
         {
            if( !victim )
               victim = who_fighting( ch );
            if( !victim || ( !IS_NPC( victim ) && !in_arena( victim ) ) )
            {
               send_to_char( "You can't do that.\r\n", ch );
               return rNONE;
            }
         }
         if( ch != victim && is_safe( ch, victim, TRUE ) )
            return rNONE;
         vo = ( void * )victim;
         break;
      case TAR_CHAR_DEFENSIVE:
         if( victim == NULL )
            victim = ch;
         vo = ( void * )victim;
         if( IS_SET( victim->immune, RIS_MAGIC ) )
         {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
         }
         break;
      case TAR_CHAR_SELF:
         vo = ( void * )ch;
         if( IS_SET( ch->immune, RIS_MAGIC ) )
         {
            immune_casting( skill, ch, victim, NULL );
            return rNONE;
         }
         break;
      case TAR_OBJ_INV:
         if( obj == NULL )
         {
            send_to_char( "You can't do that.\r\n", ch );
            return rNONE;
         }
         vo = ( void * )obj;
         break;
   }
   start_timer( &time_used );
   retcode = ( *skill->spell_fun ) ( sn, level, ch, vo );
   end_timer( &time_used );
   update_userec( &time_used, &skill->userec );
   if( retcode == rSPELL_FAILED )
      retcode = rNONE;
   if( retcode == rCHAR_DIED || retcode == rERROR )
      return retcode;
   if( char_died( ch ) )
      return rCHAR_DIED;
   if( skill->target == TAR_CHAR_OFFENSIVE && victim != ch && !char_died( victim ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;
      for( vch = ch->in_room->first_person; vch; vch = vch_next )
      {
         vch_next = vch->next_in_room;
         if( victim == vch && !vch->fighting && vch->master != ch )
         {
            retcode = multi_hit( vch, ch, TYPE_UNDEFINED );
            break;
         }
      }
   }
   return retcode;
}

ch_ret spell_null( int sn, int level, CHAR_DATA * ch, void *vo )
{
   send_to_char( "That's not a spell!\r\n", ch );
   return rNONE;
}

/* don't remove, may look redundant, but is important */
ch_ret spell_notfound( int sn, int level, CHAR_DATA * ch, void *vo )
{
   send_to_char( "That's not a spell!\r\n", ch );
   return rNONE;
}

