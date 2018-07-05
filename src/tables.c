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
 * 			Table load/save Module				    *
 ****************************************************************************/
#include <ctype.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include "./Headers/mud.h"
#include <dlfcn.h>
#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )					\
				if ( !str_cmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}
bool load_race_file( char *fname );
void write_race_file( int ra );
/* global variables */
int top_sn;
int top_herb;
int MAX_PC_CLASS;
int MAX_PC_RACE;
SKILLTYPE *skill_table[MAX_SKILL];
struct Class_type *Class_table[MAX_CLASS];
RACE_TYPE *race_table[MAX_RACE];
char *title_table[MAX_CLASS][MAX_LEVEL + 1][2];
SKILLTYPE *herb_table[MAX_HERB];
SKILLTYPE *disease_table[MAX_DISEASE];
LANG_DATA *first_lang;
LANG_DATA *last_lang;
char *const skill_tname[] = { "unknown", "Spell", "Skill", "Weapon", "Tongue", "Herb", "Racial", "Disease"
};
SPELL_FUN *spell_function( char *name )
{
   void *funHandle;
   const char *error;
   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) != NULL )
   {
      bug( "Error locating %s in symbol table. %s", name, error );
      return spell_notfound;
   }
   return ( SPELL_FUN * ) funHandle;
}

DO_FUN *skill_function( char *name )
{
   void *funHandle;
   const char *error;
   funHandle = dlsym( sysdata.dlHandle, name );
   if( ( error = dlerror(  ) ) != NULL )
   {
      bug( "Error locating %s in symbol table. %s", name, error );
      return skill_notfound;
   }
   return ( DO_FUN * ) funHandle;
}

bool load_Class_file( char *fname )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   struct Class_type *Class;
   int cl = -1;
   FILE *fp;
   sprintf( buf, "%s%s", CLASS_DIR, fname );
   if( ( fp = fopen( buf, "r" ) ) == NULL )
   {
      perror( buf );
      return FALSE;
   }
   CREATE( Class, struct Class_type, 1 );
   /*
    * Setup defaults for additions to Class structure 
    */
  for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'C':
            KEY( "Class", cl, fread_number( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               fclose( fp );
               if( cl < 0 || cl >= MAX_CLASS )
               {
                  sprintf( buf, "Load_Class_file: Class (%s) bad/not found (%d)", Class->who_name ? Class->who_name : "name not found", cl );
                  bug( buf, 0 );
                  if( Class->who_name )
                     STRFREE( Class->who_name );
                  DISPOSE( Class );
                  return FALSE;
               }
               Class_table[cl] = Class;
               return TRUE;
            }
            KEY( "ExpBase", Class->exp_base, fread_number( fp ) );
            break;
         case 'H':
            KEY( "HpMax", Class->hp_max, fread_number( fp ) );
            KEY( "HpMin", Class->hp_min, fread_number( fp ) );
            break;
         case 'M':
            KEY( "ManaMax", Class->mana_max, fread_number( fp ) );
            KEY( "ManaMin", Class->mana_min, fread_number( fp ) );
            KEY( "MoveMax", Class->move_max, fread_number( fp ) );
            KEY( "MoveMin", Class->move_min, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", Class->who_name, fread_string( fp ) );
            break;
         case 'S':
            if( !str_cmp( word, "Skill" ) )
            {
               int sn, lev, adp;
               word = fread_word( fp );
               lev = fread_number( fp );
               adp = fread_number( fp );
               sn = skill_lookup( word );
               if( cl < 0 || cl >= MAX_CLASS )
               {
                  sprintf( buf, "load_Class_file: Skill %s -- Class bad/not found (%d)", word, cl );
                  bug( buf, 0 );
               }
               else if( !IS_VALID_SN( sn ) )
               {
                  sprintf( buf, "load_Class_file: Skill %s unknown", word );
                  bug( buf, 0 );
               }
               else
               {
                  skill_table[sn]->skill_level[cl] = lev;
                  skill_table[sn]->skill_adept[cl] = adp;
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Skilladept", Class->skill_adept, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Thac0", Class->thac0_00, fread_number( fp ) );
            KEY( "Thac32", Class->thac0_32, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "load_Class_file: no match: %s", word );
         bug( buf, 0 );
      }
   }
   return FALSE;
}

/*
 * Load in all the Class files.
 */
void load_Classes(  )
{
   FILE *fpList;
   char *filename;
   char Classlist[256];
   char buf[MAX_STRING_LENGTH];
   int i;
   MAX_PC_CLASS = 0;
   /*
    * Pre-init the Class_table with blank Classes
    */
   for( i = 0; i < MAX_CLASS; i++ )
      Class_table[i] = NULL;
   sprintf( Classlist, "%s%s", CLASS_DIR, CLASS_LIST );
   if( ( fpList = fopen( Classlist, "r" ) ) == NULL )
   {
      perror( Classlist );
      exit( 1 );
   }
   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;
      if( !load_Class_file( filename ) )
      {
         sprintf( buf, "Cannot load Class file: %s", filename );
         bug( buf, 0 );
      }
      else
         MAX_PC_CLASS++;
   }
   fclose( fpList );
   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( Class_table[i] == NULL )
      {
         CREATE( Class_table[i], struct Class_type, 1 );
         create_new_Class( i, "" );
      }
   }
   return;
}
void write_Class_file( int cl )
{
   FILE *fpout;
   char buf[MAX_STRING_LENGTH];
   char filename[MAX_INPUT_LENGTH];
   struct Class_type *Class = Class_table[cl];
   int x, y;
   sprintf( filename, "%s%s.class", CLASS_DIR, Class->who_name );
   if( ( fpout = fopen( filename, "w" ) ) == NULL )
   {
      sprintf( buf, "Cannot open: %s for writing", filename );
      bug( buf, 0 );
      return;
   }
   fprintf( fpout, "Name        %s~\n", Class->who_name );
   fprintf( fpout, "Class       %d\n", cl );
   fprintf( fpout, "Skilladept  %d\n", Class->skill_adept );
   fprintf( fpout, "Thac0       %d\n", Class->thac0_00 );
   fprintf( fpout, "Thac32      %d\n", Class->thac0_32 );
   fprintf( fpout, "Hpmin       %d\n", Class->hp_min );
   fprintf( fpout, "Hpmax       %d\n", Class->hp_max );
   fprintf( fpout, "Manamin     %d\n", Class->mana_min );
   fprintf( fpout, "Manamax     %d\n", Class->mana_max );
   fprintf( fpout, "Movemin     %d\n", Class->move_min );
   fprintf( fpout, "Movemax     %d\n", Class->move_max );
   fprintf( fpout, "Expbase     %d\n", Class->exp_base );
   
   for( x = 0; x < top_sn; x++ )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == STRING_NULL )
         break;
      if( ( y = skill_table[x]->skill_level[cl] ) < LEVEL_IMMORTAL )
         fprintf( fpout, "Skill '%s' %d %d\n", skill_table[x]->name, y, skill_table[x]->skill_adept[cl] );
   }
   fprintf( fpout, "End\n" );
   fclose( fpout );
}

/*
 * Load in all the race files.
 */
void load_races(  )
{
   FILE *fpList;
   char *filename;
   char racelist[256];
   char buf[MAX_STRING_LENGTH];
   int i;
   MAX_PC_RACE = 0;
   /*
    * Pre-init the race_table with blank races
    */
   for( i = 0; i < MAX_RACE; i++ )
      race_table[i] = NULL;
   sprintf( racelist, "%s%s", RACE_DIR, RACE_LIST );
   if( ( fpList = fopen( racelist, "r" ) ) == NULL )
   {
      perror( racelist );
      exit( 1 );
   }
   for( ;; )
   {
      filename = feof( fpList ) ? "$" : fread_word( fpList );
      if( filename[0] == '$' )
         break;
      if( !load_race_file( filename ) )
      {
         sprintf( buf, "Cannot load race file: %s", filename );
         bug( buf, 0 );
      }
      else
         MAX_PC_RACE++;
   }
   for( i = 0; i < MAX_RACE; i++ )
   {
      if( race_table[i] == NULL )
      {
         CREATE( race_table[i], struct race_type, 1 );
         sprintf( race_table[i]->race_name, "%s", "unused" );
      }
   }
   fclose( fpList );
   return;
}
void write_race_file( int ra )
{
   FILE *fpout;
   char buf[MAX_STRING_LENGTH];
   char filename[MAX_INPUT_LENGTH];
   struct race_type *race = race_table[ra];
   int x, y;
   if( !race->race_name )
   {
      sprintf( buf, "Race %d has null name, not writing .race file.", ra );
      bug( buf, 0 );
      return;
   }
   sprintf( filename, "%s%s.race", RACE_DIR, race->race_name );
   if( ( fpout = fopen( filename, "w+" ) ) == NULL )
   {
      sprintf( buf, "Cannot open: %s for writing", filename );
      bug( buf, 0 );
      return;
   }
   fprintf( fpout, "Name        %s~\n", race->race_name );
   fprintf( fpout, "Race        %d\n", ra );
   fprintf( fpout, "Classes     %d\n", race->Class_restriction );
   fprintf( fpout, "Str_Plus    %d\n", race->str_plus );
   fprintf( fpout, "Dex_Plus    %d\n", race->dex_plus );
   fprintf( fpout, "Wis_Plus    %d\n", race->wis_plus );
   fprintf( fpout, "Int_Plus    %d\n", race->int_plus );
   fprintf( fpout, "Con_Plus    %d\n", race->con_plus );
   fprintf( fpout, "Cha_Plus    %d\n", race->cha_plus );
   fprintf( fpout, "Lck_Plus    %d\n", race->lck_plus );
   fprintf( fpout, "Hit         %d\n", race->hit );
   fprintf( fpout, "Mana        %d\n", race->mana );
   fprintf( fpout, "Affected    %s\n", print_bitvector( &race->affected ) );
   fprintf( fpout, "Resist      %d\n", race->resist );
   fprintf( fpout, "Suscept     %d\n", race->suscept );
   fprintf( fpout, "Language    %d\n", race->language );
   fprintf( fpout, "Align       %d\n", race->alignment );
   fprintf( fpout, "Min_Align  %d\n", race->minalign );
   fprintf( fpout, "Max_Align	%d\n", race->maxalign );
   fprintf( fpout, "AC_Plus    %d\n", race->ac_plus );
   fprintf( fpout, "Exp_Mult   %d\n", race->exp_multiplier );
   fprintf( fpout, "Attacks    %s\n", print_bitvector( &race->attacks ) );
   fprintf( fpout, "Defenses   %s\n", print_bitvector( &race->defenses ) );
   fprintf( fpout, "Height     %d\n", race->height );
   fprintf( fpout, "Weight     %d\n", race->weight );
   fprintf( fpout, "Hunger_Mod  %d\n", race->hunger_mod );
   fprintf( fpout, "Thirst_mod  %d\n", race->thirst_mod );
   fprintf( fpout, "Mana_Regen  %d\n", race->mana_regen );
   fprintf( fpout, "HP_Regen    %d\n", race->hp_regen );
   fprintf( fpout, "Race_Recall %d\n", race->race_recall );
   for( x = 0; x < top_sn; x++ )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == STRING_NULL )
         break;
      if( ( y = skill_table[x]->race_level[ra] ) < LEVEL_IMMORTAL )
         fprintf( fpout, "Skill '%s' %d %d\n", skill_table[x]->name, y, skill_table[x]->race_adept[ra] );
   }
   fprintf( fpout, "End\n" );
   fclose( fpout );
}
bool load_race_file( char *fname )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   char *race_name = NULL;
   bool fMatch;
   struct race_type *race;
   int ra = -1;
   FILE *fp;
   int i, wear = 0;
   sprintf( buf, "%s%s", RACE_DIR, fname );
   if( ( fp = fopen( buf, "r" ) ) == NULL )
   {
      perror( buf );
      return FALSE;
   }
   CREATE( race, struct race_type, 1 );
   for( i = 0; i < MAX_WHERE_NAME; i++ )
      race->where_name[i] = where_name[i];
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'A':
            KEY( "Align", race->alignment, fread_number( fp ) );
            KEY( "AC_Plus", race->ac_plus, fread_number( fp ) );
            KEY( "Affected", race->affected, fread_bitvector( fp ) );
            KEY( "Attacks", race->attacks, fread_bitvector( fp ) );
            break;
         case 'C':
            KEY( "Con_Plus", race->con_plus, fread_number( fp ) );
            KEY( "Cha_Plus", race->cha_plus, fread_number( fp ) );
            KEY( "Classes", race->Class_restriction, fread_number( fp ) );
            break;
         case 'D':
            KEY( "Dex_Plus", race->dex_plus, fread_number( fp ) );
            KEY( "Defenses", race->defenses, fread_bitvector( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               fclose( fp );
               fp = NULL;
               if( ra < 0 || ra >= MAX_RACE )
               {
                  bug( "Load_race_file: Race (%s) bad/not found (%d)", race->race_name ? race->race_name : "name not found", ra );
                  STRFREE( race_name );
                  for( i = 0; i < MAX_WHERE_NAME; ++i )
                     DISPOSE( race->where_name[i] );
                  DISPOSE( race );
                  return FALSE;
               }
               race_table[ra] = race;
               if( race_name )
                  STRFREE( race_name );
               return TRUE;
            }
            KEY( "Exp_Mult", race->exp_multiplier, fread_number( fp ) );
            break;
         case 'I':
            KEY( "Int_Plus", race->int_plus, fread_number( fp ) );
            break;
         case 'H':
            KEY( "Height", race->height, fread_number( fp ) );
            KEY( "Hit", race->hit, fread_number( fp ) );
            KEY( "HP_Regen", race->hp_regen, fread_number( fp ) );
            KEY( "Hunger_Mod", race->hunger_mod, fread_number( fp ) );
            break;
         case 'L':
            KEY( "Language", race->language, fread_number( fp ) );
            KEY( "Lck_Plus", race->lck_plus, fread_number( fp ) );
            break;
         case 'M':
            KEY( "Mana", race->mana, fread_number( fp ) );
            KEY( "Mana_Regen", race->mana_regen, fread_number( fp ) );
            KEY( "Min_Align", race->minalign, fread_number( fp ) );
            race->minalign = -1000;
            KEY( "Max_Align", race->maxalign, fread_number( fp ) );
            race->maxalign = -1000;
            break;
         case 'N':
            KEY( "Name", race_name, fread_string( fp ) );
            break;
         case 'R':
            KEY( "Race", ra, fread_number( fp ) );
            KEY( "Race_Recall", race->race_recall, fread_number( fp ) );
            KEY( "Resist", race->resist, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Str_Plus", race->str_plus, fread_number( fp ) );
            KEY( "Suscept", race->suscept, fread_number( fp ) );
            if( !str_cmp( word, "Skill" ) )
            {
               int sn, lev, adp;
               word = fread_word( fp );
               lev = fread_number( fp );
               adp = fread_number( fp );
               sn = skill_lookup( word );
               if( ra < 0 || ra >= MAX_RACE )
               {
                  sprintf( buf, "load_race_file: Skill %s -- race bad/not found (%d)", word, ra );
                  bug( buf, 0 );
               }
               else if( !IS_VALID_SN( sn ) )
               {
                  sprintf( buf, "load_race_file: Skill %s unknown", word );
                  bug( buf, 0 );
               }
               else
               {
                  skill_table[sn]->race_level[ra] = lev;
                  skill_table[sn]->race_adept[ra] = adp;
               }
               fMatch = TRUE;
               break;
            }
            break;
         case 'T':
            KEY( "Thirst_Mod", race->thirst_mod, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Weight", race->weight, fread_number( fp ) );
            KEY( "Wis_Plus", race->wis_plus, fread_number( fp ) );
            if( !str_cmp( word, "WhereName" ) )
            {
               if( ra < 0 || ra >= MAX_RACE )
               {
                  char *tmp;
                  sprintf( buf, "load_race_file: Title -- race bad/not found (%d)", ra );
                  bug( buf, 0 );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
                  tmp = fread_string_nohash( fp );
                  DISPOSE( tmp );
               }
               else if( wear < MAX_WHERE_NAME )
               {
                  DISPOSE( race->where_name[wear] );
                  race->where_name[wear] = fread_string_nohash( fp );
                  ++wear;
               }
               else
                  bug( "load_race_file: Too many where_names" );
               fMatch = TRUE;
               break;
            }
            break;
      }
      if( race_name != NULL )
         sprintf( race->race_name, "%-.16s", race_name );
      if( !fMatch )
      {
         sprintf( buf, "load_race_file: no match: %s", word );
         bug( buf, 0 );
      }
   }
   return FALSE;
}

/*
 * Function used by qsort to sort skills
 */
int skill_comp( SKILLTYPE ** sk1, SKILLTYPE ** sk2 )
{
   SKILLTYPE *skill1 = ( *sk1 );
   SKILLTYPE *skill2 = ( *sk2 );
   if( !skill1 && skill2 )
      return 1;
   if( skill1 && !skill2 )
      return -1;
   if( !skill1 && !skill2 )
      return 0;
   if( skill1->type < skill2->type )
      return -1;
   if( skill1->type > skill2->type )
      return 1;
   return strcmp( skill1->name, skill2->name );
}

/*
 * Sort the skill table with qsort
 */
void sort_skill_table(  )
{
   log_string( "Sorting skill table..." );
   qsort( &skill_table[1], top_sn - 1, sizeof( SKILLTYPE * ), ( int ( * )( const void *, const void * ) )skill_comp );
}

/*
 * Write skill data to a file
 */
void fwrite_skill( FILE * fpout, SKILLTYPE * skill )
{
   SMAUG_AFF *aff;
   int modifier;
   fprintf( fpout, "Name         %s~\n", skill->name );
   fprintf( fpout, "Type         %s\n", skill_tname[skill->type] );
   fprintf( fpout, "Info         %d\n", skill->info );
   fprintf( fpout, "Flags        %d\n", skill->flags );
   if( skill->target )
      fprintf( fpout, "Target       %d\n", skill->target );
   if( skill->minimum_position )
      fprintf( fpout, "Minpos       %d\n", skill->minimum_position + 100 );
   if( skill->spell_sector )
      fprintf( fpout, "Ssector      %d\n", skill->spell_sector );
   if( skill->saves )
      fprintf( fpout, "Saves        %d\n", skill->saves );
   if( skill->min_mana )
      fprintf( fpout, "Mana         %d\n", skill->min_mana );
   if( skill->beats )
      fprintf( fpout, "Rounds       %d\n", skill->beats );
   if( skill->range )
      fprintf( fpout, "Range        %d\n", skill->range );
   if( skill->guild != -1 )
      fprintf( fpout, "Guild        %d\n", skill->guild );
   if( skill->skill_fun )
      fprintf( fpout, "Code         %s\n", skill->skill_fun_name );
   else if( skill->spell_fun )
      fprintf( fpout, "Code         %s\n", skill->spell_fun_name );
/*   fprintf( fpout, "Dammsg       %s~\n", skill->noun_damage );
   if( skill->msg_off && skill->msg_off[0] != STRING_NULL )
      fprintf( fpout, "Wearoff      %s~\n", skill->msg_off );
   if( skill->hit_char && skill->hit_char[0] != STRING_NULL )
      fprintf( fpout, "Hitchar      %s~\n", skill->hit_char );
   if( skill->hit_vict && skill->hit_vict[0] != STRING_NULL )
      fprintf( fpout, "Hitvict      %s~\n", skill->hit_vict );
   if( skill->hit_room && skill->hit_room[0] != STRING_NULL )
      fprintf( fpout, "Hitroom      %s~\n", skill->hit_room );
   if( skill->hit_dest && skill->hit_dest[0] != STRING_NULL )
      fprintf( fpout, "Hitdest      %s~\n", skill->hit_dest );
   if( skill->miss_char && skill->miss_char[0] != STRING_NULL )
      fprintf( fpout, "Misschar     %s~\n", skill->miss_char );
   if( skill->miss_vict && skill->miss_vict[0] != STRING_NULL )
      fprintf( fpout, "Missvict     %s~\n", skill->miss_vict );
   if( skill->miss_room && skill->miss_room[0] != STRING_NULL )
      fprintf( fpout, "Missroom     %s~\n", skill->miss_room );
   if( skill->die_char && skill->die_char[0] != STRING_NULL )
      fprintf( fpout, "Diechar      %s~\n", skill->die_char );
   if( skill->die_vict && skill->die_vict[0] != STRING_NULL )
      fprintf( fpout, "Dievict      %s~\n", skill->die_vict );
   if( skill->die_room && skill->die_room[0] != STRING_NULL )
      fprintf( fpout, "Dieroom      %s~\n", skill->die_room );
   if( skill->imm_char && skill->imm_char[0] != STRING_NULL )
      fprintf( fpout, "Immchar      %s~\n", skill->imm_char );
   if( skill->imm_vict && skill->imm_vict[0] != STRING_NULL )
      fprintf( fpout, "Immvict      %s~\n", skill->imm_vict );
   if( skill->imm_room && skill->imm_room[0] != STRING_NULL )
      fprintf( fpout, "Immroom      %s~\n", skill->imm_room );
   if( skill->dice && skill->dice[0] != STRING_NULL )
      fprintf( fpout, "Dice         %s~\n", skill->dice );
   if( skill->value )
      fprintf( fpout, "Value        %d\n", skill->value );
   if( skill->difficulty )
      fprintf( fpout, "Difficulty   %d\n", skill->difficulty );
   if( skill->participants )
      fprintf( fpout, "Participants %d\n", skill->participants );
   if( skill->components && skill->components[0] != STRING_NULL )
      fprintf( fpout, "Components   %s~\n", skill->components ); */
   if( skill->teachers && skill->teachers[0] != STRING_NULL )
      fprintf( fpout, "Teachers     %s~\n", skill->teachers );
   for( aff = skill->affects; aff; aff = aff->next )
   {
      fprintf( fpout, "Affect       '%s' %d ", aff->duration, aff->location );
      modifier = atoi( aff->modifier );
      if( ( aff->location == APPLY_WEAPONSPELL
            || aff->location == APPLY_WEARSPELL
            || aff->location == APPLY_REMOVESPELL || aff->location == APPLY_STRIPSN || aff->location == APPLY_RECURRINGSPELL ) && IS_VALID_SN( modifier ) )
         fprintf( fpout, "'%s' ", aff->modifier );

      fprintf( fpout, "%d\n", aff->bitvector );
   }

   fprintf( fpout, "End\n\n" );
}

/*
 * Save the skill table to disk
 */
void save_skill_table(  )
{
   int x;
   FILE *fpout;
   if( ( fpout = fopen( SKILL_FILE, "w" ) ) == NULL )
   {
      perror( SKILL_FILE );
      bug( "Cannot open skills.dat for writting", 0 );
      return;
   }
   for( x = 0; x < top_sn; x++ )
   {
      if( !skill_table[x]->name || skill_table[x]->name[0] == STRING_NULL )
         break;
      fprintf( fpout, "#SKILL\n" );
      fwrite_skill( fpout, skill_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the herb table to disk
 */
void save_herb_table(  )
{
   int x;
   FILE *fpout;
   if( !( fpout = fopen( HERB_FILE, "w" ) ) )
   {
      bug( "Cannot open %s for writting", HERB_FILE );
      perror( HERB_FILE );
      return;
   }
   for( x = 0; x < top_herb; x++ )
   {
      if( !herb_table[x]->name || herb_table[x]->name[0] == STRING_NULL )
         break;
      fprintf( fpout, "#HERB\n" );
      fwrite_skill( fpout, herb_table[x] );
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

/*
 * Save the socials to disk
 */
void save_socials(  )
{
   FILE *fpout;
   SOCIALTYPE *social;
   int x;
   if( ( fpout = fopen( SOCIAL_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open socials.dat for writting", 0 );
      perror( SOCIAL_FILE );
      return;
   }
   for( x = 0; x < 27; x++ )
   {
      for( social = social_index[x]; social; social = social->next )
      {
         if( !social->name || social->name[0] == STRING_NULL )
         {
            bug( "Save_socials: blank social in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#SOCIAL\n" );
         fprintf( fpout, "Name        %s~\n", social->name );
         if( social->char_no_arg )
            fprintf( fpout, "CharNoArg   %s~\n", social->char_no_arg );
         else
            bug( "Save_socials: NULL char_no_arg in hash bucket %d", x );
         if( social->others_no_arg )
            fprintf( fpout, "OthersNoArg %s~\n", social->others_no_arg );
         if( social->char_found )
            fprintf( fpout, "CharFound   %s~\n", social->char_found );
         if( social->others_found )
            fprintf( fpout, "OthersFound %s~\n", social->others_found );
         if( social->vict_found )
            fprintf( fpout, "VictFound   %s~\n", social->vict_found );
         if( social->char_auto )
            fprintf( fpout, "CharAuto    %s~\n", social->char_auto );
         if( social->others_auto )
            fprintf( fpout, "OthersAuto  %s~\n", social->others_auto );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}
int get_skill( char *skilltype )
{
   if( !str_cmp( skilltype, "Spell" ) )
      return SKILL_SPELL;
   if( !str_cmp( skilltype, "Skill" ) )
      return SKILL_SKILL;
   if( !str_cmp( skilltype, "Weapon" ) )
      return SKILL_WEAPON;
   if( !str_cmp( skilltype, "Tongue" ) )
      return SKILL_TONGUE;

   return SKILL_UNKNOWN;
}

/*
 * Save the commands to disk
 * Added flags Aug 25, 1997 --Shaddai
 */
void save_commands(  )
{
   FILE *fpout;
   CMDTYPE *command;
   int x;
   if( ( fpout = fopen( COMMAND_FILE, "w" ) ) == NULL )
   {
      bug( "Cannot open commands.dat for writing", 0 );
      perror( COMMAND_FILE );
      return;
   }
   for( x = 0; x < 126; x++ )
   {
      for( command = command_hash[x]; command; command = command->next )
      {
         if( !command->name || command->name[0] == STRING_NULL )
         {
            bug( "Save_commands: blank command in hash bucket %d", x );
            continue;
         }
         fprintf( fpout, "#COMMAND\n" );
         fprintf( fpout, "Name        %s~\n", command->name );
         fprintf( fpout, "Code        %s\n", command->fun_name ? command->fun_name : "" );   /*Modded to use new field - Trax */
         /*
          * Oops I think this may be a bad thing so I changed it -- Shaddai 
          */
         if( command->position < 100 )
            fprintf( fpout, "Position    %d\n", command->position + 100 );
         else
            fprintf( fpout, "Position    %d\n", command->position );
         fprintf( fpout, "Level       %d\n", command->level );
         fprintf( fpout, "Log         %d\n", command->log );
         if( command->flags )
            fprintf( fpout, "Flags       %d\n", command->flags );
         fprintf( fpout, "End\n\n" );
      }
   }
   fprintf( fpout, "#END\n" );
   fclose( fpout );
}

SKILLTYPE *fread_skill( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   bool got_info = FALSE;
   SKILLTYPE *skill;
   int x;
   CREATE( skill, SKILLTYPE, 1 );
   skill->min_mana = 0;
   for( x = 0; x < MAX_CLASS; x++ )
   {
      skill->skill_level[x] = LEVEL_IMMORTAL;
      skill->skill_adept[x] = 95;
   }
   for( x = 0; x < MAX_RACE; x++ )
   {
      skill->race_level[x] = LEVEL_IMMORTAL;
      skill->race_adept[x] = 95;
   }
   skill->guild = -1;
   skill->target = 0;
   skill->skill_fun = NULL;
   skill->spell_fun = NULL;
   skill->spell_sector = 0;
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'A':
            if( !str_cmp( word, "Affect" ) )
            {
               SMAUG_AFF *aff;
               CREATE( aff, SMAUG_AFF, 1 );
               aff->duration = str_dup( fread_word( fp ) );
               aff->location = fread_number( fp );
               aff->modifier = str_dup( fread_word( fp ) );
               aff->bitvector = fread_number( fp );
               if( !got_info )
               {
                  for( x = 0; x < 32; x++ )
                  {
                     if( IS_SET( aff->bitvector, 1 << x ) )
                     {
                        aff->bitvector = x;
                        break;
                     }
                  }
                  if( x == 32 )
                     aff->bitvector = -1;
               }
               aff->next = skill->affects;
               skill->affects = aff;
               fMatch = TRUE;
               break;
            }
            break;
         case 'C':
            if( !str_cmp( word, "Class" ) )
            {
               int Class = fread_number( fp );
               skill->skill_level[Class] = fread_number( fp );
               skill->skill_adept[Class] = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            if( !str_cmp( word, "Code" ) )
            {
               SPELL_FUN *spellfun;
               DO_FUN *dofun;
               char *w = fread_word( fp );
               fMatch = TRUE;
               if( !str_prefix( "do_", w ) && ( dofun = skill_function( w ) ) != skill_notfound )
               {
                  skill->skill_fun = dofun;
                  skill->spell_fun = NULL;
                  skill->skill_fun_name = str_dup( w );
               }
               else if( str_prefix( "do_", w ) && ( spellfun = spell_function( w ) ) != spell_notfound )
               {
                  skill->spell_fun = spellfun;
                  skill->skill_fun = NULL;
                  skill->spell_fun_name = str_dup( w );
               }
               else
               {
                  bug( "fread_skill: unknown skill/spell %s %s", w, skill->name );
                  skill->spell_fun = spell_null;
               }
               break;
            }
            KEY( "Code", skill->spell_fun, spell_function( fread_word( fp ) ) );
            KEY( "Components", skill->components, fread_string_nohash( fp ) );
            break;
         case 'D':
            KEY( "Dammsg", skill->noun_damage, fread_string_nohash( fp ) );
            KEY( "Dice", skill->dice, fread_string_nohash( fp ) );
            KEY( "Diechar", skill->die_char, fread_string_nohash( fp ) );
            KEY( "Dieroom", skill->die_room, fread_string_nohash( fp ) );
            KEY( "Dievict", skill->die_vict, fread_string_nohash( fp ) );
            KEY( "Difficulty", skill->difficulty, fread_number( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( skill->saves != 0 && SPELL_SAVE( skill ) == SE_NONE )
               {
                  bug( "fread_skill(%s):  Has saving throw (%d) with no saving effect.", skill->name, skill->saves );
                  SET_SSAV( skill, SE_NEGATE );
               }
               return skill;
            }
            break;
         case 'F':
            if( !str_cmp( word, "Flags" ) )
            {
               skill->flags = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;
         case 'G':
            KEY( "Guild", skill->guild, fread_number( fp ) );
            break;
         case 'H':
            KEY( "Hitchar", skill->hit_char, fread_string_nohash( fp ) );
            KEY( "Hitdest", skill->hit_dest, fread_string_nohash( fp ) );
            KEY( "Hitroom", skill->hit_room, fread_string_nohash( fp ) );
            KEY( "Hitvict", skill->hit_vict, fread_string_nohash( fp ) );
            break;
         case 'I':
            KEY( "Immchar", skill->imm_char, fread_string_nohash( fp ) );
            KEY( "Immroom", skill->imm_room, fread_string_nohash( fp ) );
            KEY( "Immvict", skill->imm_vict, fread_string_nohash( fp ) );
            if( !str_cmp( word, "Info" ) )
            {
               skill->info = fread_number( fp );
               got_info = TRUE;
               fMatch = TRUE;
               break;
            }
            break;
         case 'M':
            KEY( "Mana", skill->min_mana, fread_number( fp ) );
            if( !str_cmp( word, "Minlevel" ) )
            {
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }
            /*
             * KEY( "Minpos",   skill->minimum_position, fread_number( fp ) ); 
             */
            /*
             * 
             */
            if( !str_cmp( word, "Minpos" ) )
            {
               fMatch = TRUE;
               skill->minimum_position = fread_number( fp );
               if( skill->minimum_position < 100 )
               {
                  switch ( skill->minimum_position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        skill->minimum_position = 6;
                        break;
                     case 6:
                        skill->minimum_position = 8;
                        break;
                     case 7:
                        skill->minimum_position = 9;
                        break;
                     case 8:
                        skill->minimum_position = 12;
                        break;
                     case 9:
                        skill->minimum_position = 13;
                        break;
                     case 10:
                        skill->minimum_position = 14;
                        break;
                     case 11:
                        skill->minimum_position = 15;
                        break;
                  }
               }
               else
                  skill->minimum_position -= 100;
               break;
            }
            KEY( "Misschar", skill->miss_char, fread_string_nohash( fp ) );
            KEY( "Missroom", skill->miss_room, fread_string_nohash( fp ) );
            KEY( "Missvict", skill->miss_vict, fread_string_nohash( fp ) );
            break;
         case 'N':
            KEY( "Name", skill->name, fread_string_nohash( fp ) );
            break;
         case 'P':
            KEY( "Participants", skill->participants, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Range", skill->range, fread_number( fp ) );
            KEY( "Rounds", skill->beats, fread_number( fp ) );
            if( !str_cmp( word, "Race" ) )
            {
               int race = fread_number( fp );
               skill->race_level[race] = fread_number( fp );
               skill->race_adept[race] = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;
         case 'S':
            KEY( "Saves", skill->saves, fread_number( fp ) );
            KEY( "Ssector", skill->spell_sector, fread_number( fp ) );
            break;
         case 'T':
            KEY( "Target", skill->target, fread_number( fp ) );
            KEY( "Teachers", skill->teachers, fread_string_nohash( fp ) );
            KEY( "Type", skill->type, get_skill( fread_word( fp ) ) );
            break;
         case 'V':
            KEY( "Value", skill->value, fread_number( fp ) );
            break;
         case 'W':
            KEY( "Wearoff", skill->msg_off, fread_string_nohash( fp ) );
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_skill: no match: %s", word );
         bug( buf, 0 );
      }
   }
}
void load_skill_table(  )
{
   FILE *fp;
   if( ( fp = fopen( SKILL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;
         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }
         if( letter != '#' )
         {
            bug( "Load_skill_table: # not found.", 0 );
            break;
         }
         word = fread_word( fp );
         if( !str_cmp( word, "SKILL" ) )
         {
            if( top_sn >= MAX_SKILL )
            {
               bug( "load_skill_table: more skills than MAX_SKILL %d", MAX_SKILL );
               fclose( fp );
               return;
            }
            skill_table[top_sn++] = fread_skill( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_skill_table: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      perror( SKILL_FILE );
      bug( "Cannot open skills.dat", 0 );
      exit( 0 );
   }
}

void fread_social( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   SOCIALTYPE *social;
   CREATE( social, SOCIALTYPE, 1 );
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'C':
            KEY( "CharNoArg", social->char_no_arg, fread_string_nohash( fp ) );
            KEY( "CharFound", social->char_found, fread_string_nohash( fp ) );
            KEY( "CharAuto", social->char_auto, fread_string_nohash( fp ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !social->name )
               {
                  bug( "Fread_social: Name not found", 0 );
                  free_social( social );
                  return;
               }
               if( !social->char_no_arg )
               {
                  bug( "Fread_social: CharNoArg not found", 0 );
                  free_social( social );
                  return;
               }
               add_social( social );
               return;
            }
            break;
         case 'N':
            KEY( "Name", social->name, fread_string_nohash( fp ) );
            break;
         case 'O':
            KEY( "OthersNoArg", social->others_no_arg, fread_string_nohash( fp ) );
            KEY( "OthersFound", social->others_found, fread_string_nohash( fp ) );
            KEY( "OthersAuto", social->others_auto, fread_string_nohash( fp ) );
            break;
         case 'V':
            KEY( "VictFound", social->vict_found, fread_string_nohash( fp ) );
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_social: no match: %s", word );
         bug( buf, 0 );
      }
   }
}
void load_socials(  )
{
   FILE *fp;
   if( ( fp = fopen( SOCIAL_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;
         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }
         if( letter != '#' )
         {
            bug( "Load_socials: # not found.", 0 );
            break;
         }
         word = fread_word( fp );
         if( !str_cmp( word, "SOCIAL" ) )
         {
            fread_social( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_socials: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open %s", SOCIAL_FILE );
      exit( 0 );
   }
}

/*
 *  Added the flags Aug 25, 1997 --Shaddai
 */
void fread_command( FILE * fp )
{
   char buf[MAX_STRING_LENGTH];
   char *word;
   bool fMatch;
   CMDTYPE *command;
   CREATE( command, CMDTYPE, 1 );
   command->lag_count = 0; /* can't have caused lag yet... FB */
   command->flags = 0;  /* Default to no flags set */
   for( ;; )
   {
      word = feof( fp ) ? "End" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case 'C':
            KEY( "Code", command->fun_name, str_dup( fread_word( fp ) ) );
            break;
         case 'E':
            if( !str_cmp( word, "End" ) )
            {
               if( !command->name )
               {
                  bug( "%s", "Fread_command: Name not found" );
                  free_command( command );
                  return;
               }
               if( !command->fun_name )
               {
                  bug( "fread_command: No function name supplied for %s", command->name );
                  free_command( command );
                  return;
               }
               /*
                * Mods by Trax
                * Fread in code into char* and try linkage here then
                * deal in the "usual" way I suppose..
                */
               command->do_fun = skill_function( command->fun_name );
               if( command->do_fun == skill_notfound )
               {
                  bug( "Fread_command: Function %s not found for %s", command->fun_name, command->name );
                  free_command( command );
                  return;
               }
               add_command( command );
               return;
            }
            break;
         case 'F':
            KEY( "Flags", command->flags, fread_number( fp ) );
            break;
         case 'L':
            KEY( "Level", command->level, fread_number( fp ) );
            KEY( "Log", command->log, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", command->name, fread_string_nohash( fp ) );
            break;
         case 'P':
            /*
             * KEY( "Position", command->position,   fread_number(fp) ); 
             */
            if( !str_cmp( word, "Position" ) )
            {
               fMatch = TRUE;
               command->position = fread_number( fp );
               if( command->position < 100 )
               {
                  switch ( command->position )
                  {
                     default:
                     case 0:
                     case 1:
                     case 2:
                     case 3:
                     case 4:
                        break;
                     case 5:
                        command->position = 6;
                        break;
                     case 6:
                        command->position = 8;
                        break;
                     case 7:
                        command->position = 9;
                        break;
                     case 8:
                        command->position = 12;
                        break;
                     case 9:
                        command->position = 13;
                        break;
                     case 10:
                        command->position = 14;
                        break;
                     case 11:
                        command->position = 15;
                        break;
                  }
               }
               else
                  command->position -= 100;
               break;
            }
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_command: no match: %s", word );
         bug( buf, 0 );
      }
   }
}
void load_commands(  )
{
   FILE *fp;
   if( ( fp = fopen( COMMAND_FILE, "r" ) ) != NULL )
   {
      top_sn = 0;
      for( ;; )
      {
         char letter;
         char *word;
         letter = fread_letter( fp );
         if( letter == '*' )
         {
            fread_to_eol( fp );
            continue;
         }
         if( letter != '#' )
         {
            bug( "Load_commands: # not found.", 0 );
            break;
         }
         word = fread_word( fp );
         if( !str_cmp( word, "COMMAND" ) )
         {
            fread_command( fp );
            continue;
         }
         else if( !str_cmp( word, "END" ) )
            break;
         else
         {
            bug( "Load_commands: bad section.", 0 );
            continue;
         }
      }
      fclose( fp );
   }
   else
   {
      bug( "Cannot open %s", COMMAND_FILE );
      exit( 0 );
   }
}
void save_Classes(  )
{
   int x;
   for( x = 0; x < MAX_PC_CLASS; x++ )
      write_Class_file( x );
}

/*
 * Tongues / Languages loading/saving functions			-Altrag
 */
void fread_cnv( FILE * fp, LCNV_DATA ** first_cnv, LCNV_DATA ** last_cnv )
{
   LCNV_DATA *cnv;
   char letter;
   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == '~' || letter == EOF )
         break;
      ungetc( letter, fp );
      CREATE( cnv, LCNV_DATA, 1 );
      cnv->old = str_dup( fread_word( fp ) );
      cnv->olen = strlen( cnv->old );
      cnv->new = str_dup( fread_word( fp ) );
      cnv->nlen = strlen( cnv->new );
      fread_to_eol( fp );
      LINK( cnv, *first_cnv, *last_cnv, next, prev );
   }
}
void load_tongues(  )
{
   FILE *fp;
   LANG_DATA *lng;
   char *word;
   char letter;
   if( !( fp = fopen( TONGUE_FILE, "r" ) ) )
   {
      perror( "Load_tongues" );
      return;
   }
   for( ;; )
   {
      letter = fread_letter( fp );
      if( letter == EOF )
         return;
      else if( letter == '*' )
      {
         fread_to_eol( fp );
         continue;
      }
      else if( letter != '#' )
      {
         bug( "Letter '%c' not #.", letter );
         exit( 0 );
      }
      word = fread_word( fp );
      if( !str_cmp( word, "end" ) )
         break;
      fread_to_eol( fp );
      CREATE( lng, LANG_DATA, 1 );
      lng->name = STRALLOC( word );
      fread_cnv( fp, &lng->first_precnv, &lng->last_precnv );
      lng->alphabet = fread_string( fp );
      fread_cnv( fp, &lng->first_cnv, &lng->last_cnv );
      fread_to_eol( fp );
      LINK( lng, first_lang, last_lang, next, prev );
   }
   fclose( fp );
   fp = NULL;
   return;
}
void fwrite_langs( void )
{
   FILE *fp;
   LANG_DATA *lng;
   LCNV_DATA *cnv;
   if( !( fp = fopen( TONGUE_FILE, "w" ) ) )
   {
      perror( "fwrite_langs" );
      return;
   }
   for( lng = first_lang; lng; lng = lng->next )
   {
      fprintf( fp, "#%s\n", lng->name );
      for( cnv = lng->first_precnv; cnv; cnv = cnv->next )
         fprintf( fp, "'%s' '%s'\n", cnv->old, cnv->new );
      fprintf( fp, "~\n%s~\n", lng->alphabet );
      for( cnv = lng->first_cnv; cnv; cnv = cnv->next )
         fprintf( fp, "'%s' '%s'\n", cnv->old, cnv->new );
      fprintf( fp, "\n" );
   }
   fprintf( fp, "#end\n\n" );
   fclose( fp );
   return;
}

void copy_files_contents( FILE * fsource, FILE * fdestination )
{
   int ch;
   int cnt = 1;
   for( ;; )
   {
      ch = fgetc( fsource );
      if( !feof( fsource ) )
      {
         fputc( ch, fdestination );
         if( ch == '\n' )
         {
            cnt++;
            if( cnt >= LAST_FILE_SIZE )   /*limit size of this file please :-) */
               break;
         }
      }
      else
         break;
   }
}
void write_last_file( char *entry )
{
   FILE *fpout;
   FILE *fptemp;
   char filename[MAX_INPUT_LENGTH];
   char tempname[MAX_INPUT_LENGTH];
   sprintf( filename, "%s", LAST_LIST );
   sprintf( tempname, "%s", LAST_TEMP_LIST );
   if( ( fptemp = fopen( tempname, "w" ) ) == NULL )
   {
      bug( "Cannot open: %s for writing", tempname );
      return;
   }
   fprintf( fptemp, "%s\n", entry );   /*adds new entry to top of the file */
   if( ( fpout = fopen( filename, "r" ) ) != NULL )
   {
      copy_files_contents( fpout, fptemp );  /*copy the rest to the file */
      fclose( fpout );  /*close the files since writing is done */
   }
   fclose( fptemp );
   if( remove( filename ) != 0 && fopen( filename, "r" ) != NULL )
   {
      bug( "Do not have permission to delete the %s file", filename );
      return;
   }
   if( rename( tempname, filename ) != 0 )
   {
      bug( "Do not have permission to rename the %s file", tempname );
      return;
   }
   return;
}
void read_last_file( CHAR_DATA * ch, int count, char *name )
{
   FILE *fpout;
   char filename[MAX_INPUT_LENGTH];
   char charname[100];
   int cnt = 0;
   int letter = 0;
   char *ln;
   char *c;
   char d, e;
   struct tm *tme;
   time_t now;
   char day[MAX_INPUT_LENGTH];
   char sday[5];
   int fnd = 0;
   sprintf( filename, "%s", LAST_LIST );
   if( ( fpout = fopen( filename, "r" ) ) == NULL )
   {
      send_to_char( "There is no last file to look at.\r\n", ch );
      return;
   }
   for( ;; )
   {
      if( feof( fpout ) )
      {
         fclose( fpout );
         ch_printf( ch, "--------------------------------------------------------------------------------\r\n%d Entries Listed.\r\n", cnt );
         return;
      }
      else
      {
         if( count == -2 || ++cnt <= count || count == -1 )
         {
            ln = fread_line( fpout );
            strcpy( charname, "" );
            if( name )  /*looking for a certain name */
            {
               c = ln;
               for( ;; )
               {
                  if( isalpha( *c ) && !isspace( *c ) )
                  {
                     charname[letter] = *c;
                     letter++;
                     c++;
                  }
                  else
                  {
                     charname[letter] = STRING_NULL;
                     if( !str_cmp( charname, name ) )
                     {
                        ch_printf( ch, "%s", ln );
                        letter = 0;
                        strcpy( charname, "" );
                        break;
                     }
                     else
                     {
                        if( !feof( fpout ) )
                        {
                           fread_line( fpout );
                           c = ln;
                           letter = 0;
                           strcpy( charname, "" );
                           continue;
                        }
                        else
                        {
                           cnt--;
                           break;
                        }
                     }
                  }
               }
            }
            else if( count == -2 )  /*only today's entries */
            {
               c = ln;
               now = time( 0 );
               tme = localtime( &now );
               strftime( day, 10, "%d", tme );
               for( ;; )
               {
                  if( !isdigit( *c ) )
                  {
                     c++;
                  }
                  else
                  {
                     d = *c;
                     c++;
                     e = *c;
                     sprintf( sday, "%c%c", d, e );
                     if( !str_cmp( sday, day ) )
                     {
                        fnd = 1;
                        cnt++;
                        ch_printf( ch, "%s", ln );
                        break;
                     }
                     else
                     {
                        if( fnd == 1 )
                        {
                           fclose( fpout );
                           ch_printf( ch, "--------------------------------------------------------------------------------\r\n%d Entries Listed.\r\n", cnt );
                           return;
                        }
                        else
                           break;
                     }
                  }
               }
            }
            else
            {
               ch_printf( ch, "%s", ln );
            }
         }
         else
         {
            fclose( fpout );
            ch_printf( ch, "--------------------------------------------------------------------------------\r\n%d Entries Listed.\r\n", count );
            return;
         }
      }
   }
}
