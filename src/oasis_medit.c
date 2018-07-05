/****************************************************************************
 *			Eldhamud Codebase V2.0				    *
 * ------------------------------------------------------------------------ *
 * EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)                     *
 * EldhaMUD Team: Celest, Altere and Krelowyn                               *
 * ------------------------------------------------------------------------ *
 *                                                                          *
 *     OasisOLC II for Smaug 1.40 written by Evan Cortens(Tagith)           *
 *                                                                          *
 *   Based on OasisOLC for CircleMUD3.0bpl9 written by Harvey Gilpin        *
 *                                                                          *
 ****************************************************************************
 *                                                                          *
 *               Mobile/Player editing module (medit.c)                     *
 *                                                                          *
\**************************************************************************/
#include <stdio.h>
#include <string.h>
#include <crypt.h>
#include "./Headers/mud.h"
#include "./Headers/olc.h"
/*-------------------------------------------------------------------*/
/* Externals */
extern int top_mob_index;
MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
void olc_log( DESCRIPTOR_DATA * d, char *format, ... );
/* Global Variables */
char *olc_clan_list[50];
char *olc_deity_list[50];
char *olc_council_list[50];
int olc_top_clan;
int olc_top_guild;
int olc_top_order;
int olc_top_council;
int olc_top_deity;
char *const position_names[] = {
   "dead", "mortallywounded", "incapacitated", "stunned", "sleeping",
   "berserk", "resting", "aggressive", "sitting", "fighting", "defensive",
   "evasive", "standing", "mounted", "shoved", "dragged"
};
/*-------------------------------------------------------------------*/
/*. Function prototypes .*/
int get_actflag args( ( char *flag ) );
int get_risflag args( ( char *flag ) );
int get_partflag args( ( char *flag ) );
int get_attackflag args( ( char *flag ) );
int get_defenseflag args( ( char *flag ) );
DECLARE_DO_FUN( do_medit_reset );
void medit_disp_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_npc_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_pc_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_positions args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_mob_flags args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_plr_flags args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_pcdata_flags args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_aff_flags args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_attack_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_defense_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_ris args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_spec args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_clans args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_deities args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_councils args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_parts args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_Classes args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_races args( ( DESCRIPTOR_DATA * d ) );
void medit_disp_saving_menu args( ( DESCRIPTOR_DATA * d ) );
void medit_setup_arrays args( ( void ) );
/*-------------------------------------------------------------------*\
  initialization functions
\*-------------------------------------------------------------------*/
void do_omedit( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   if( IS_NPC( ch ) )
   {
      send_to_char( "I don't think so...\n\r", ch );
      return;
   }
   one_argument( argument, arg );
   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Medit what?\n\r", ch );
      return;
   }
   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\n\r", ch );
      return;
   }
   if( !IS_NPC( victim ) && get_trust( ch ) < sysdata.level_modify_proto )
   {
      send_to_char( "Huh?\n\r", ch );
      return;
   }
   /*
    * Make sure the object isnt already being edited 
    */
   for( d = first_descriptor; d; d = d->next )
      if( d->connected == CON_MEDIT )
         if( d->olc && OLC_VNUM( d ) == victim->pIndexData->vnum )
         {
            ch_printf( ch, "That %s is currently being edited by %s.\n\r",
                       IS_NPC( victim ) ? "mobile" : "character", d->character->name );
            return;
         }
   if( !can_mmodify( ch, victim ) )
      return;
   d = ch->desc;
   CREATE( d->olc, OLC_DATA, 1 );
   if( IS_NPC( victim ) )
   {
      OLC_VNUM( d ) = victim->pIndexData->vnum;
   }
   else
      medit_setup_arrays(  );
   do_immtalk( ch, "BrB, Going into Oasis OLC" );
   act( AT_ACTION, "$n starts using OLC.", ch, NULL, NULL, TO_ROOM );
   d->character->dest_buf = victim;
   d->connected = CON_MEDIT;
   OLC_CHANGE( d ) = FALSE;
   medit_disp_menu( d );
   return;
}

void do_mcopy( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   int ovnum, cvnum;
   MOB_INDEX_DATA *orig;
   MOB_INDEX_DATA *copy;
   MPROG_DATA *mprog, *cprog;
   int iHash;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Usage: mcopy <original> <new>\n\r", ch );
      return;
   }
   ovnum = atoi( arg1 );
   cvnum = atoi( arg2 );
   if( get_trust( ch ) < LEVEL_NEOPHYTE )
   {
      AREA_DATA *pArea;
      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to copy objects.\n\r", ch );
         return;
      }
      if( cvnum < pArea->low_o_vnum || cvnum > pArea->hi_o_vnum )
      {
         send_to_char( "That number is not in your allocated range.\n\r", ch );
         return;
      }
   }
   if( get_mob_index( cvnum ) )
   {
      send_to_char( "That object already exists.\n\r", ch );
      return;
   }
   if( ( orig = get_mob_index( ovnum ) ) == NULL )
   {
      send_to_char( "How can you copy something that doesnt exist?\n\r", ch );
      return;
   }
   CREATE( copy, MOB_INDEX_DATA, 1 );
   copy->vnum = cvnum;
   copy->player_name = QUICKLINK( orig->player_name );
   copy->short_descr = QUICKLINK( orig->short_descr );
   copy->long_descr = QUICKLINK( orig->long_descr );
   copy->description = QUICKLINK( orig->description );
   copy->act = orig->act;
   copy->affected_by = orig->affected_by;
   copy->pShop = NULL;
   copy->rShop = NULL;
   copy->spec_fun = orig->spec_fun;
   copy->mudprogs = NULL;
   xCLEAR_BITS( copy->progtypes );
   copy->alignment = orig->alignment;
   copy->level = orig->level;
   copy->mobthac0 = orig->mobthac0;
   copy->ac = orig->ac;
   copy->hitnodice = orig->hitnodice;
   copy->hitsizedice = orig->hitsizedice;
   copy->hitplus = orig->hitplus;
   copy->damnodice = orig->damnodice;
   copy->damsizedice = orig->damsizedice;
   copy->damplus = orig->damplus;
   copy->gold = orig->gold;
   copy->exp = orig->exp;
   copy->position = orig->position;
   copy->defposition = orig->defposition;
   copy->sex = orig->sex;
   copy->perm_str = orig->perm_str;
   copy->perm_dex = orig->perm_dex;
   copy->perm_int = orig->perm_int;
   copy->perm_wis = orig->perm_wis;
   copy->perm_cha = orig->perm_cha;
   copy->perm_con = orig->perm_con;
   copy->perm_lck = orig->perm_lck;
   copy->race = orig->race;
   copy->Class = orig->Class;
   copy->xflags = orig->xflags;
   copy->resistant = orig->resistant;
   copy->immune = orig->immune;
   copy->susceptible = orig->susceptible;
   copy->numattacks = orig->numattacks;
   copy->attacks = orig->attacks;
   copy->defenses = orig->defenses;
   copy->height = orig->height;
   copy->weight = orig->weight;
   copy->saving_poison_death = orig->saving_poison_death;
   copy->saving_mental = orig->saving_mental;
   copy->saving_physical = orig->saving_physical;
   copy->saving_weapons = orig->saving_weapons;
   if( orig->mudprogs )
   {
      CREATE( mprog, MPROG_DATA, 1 );
      copy->mudprogs = mprog;
      for( cprog = orig->mudprogs; cprog; cprog = cprog->next )
      {
         mprog->type = cprog->type;
         xSET_BIT( copy->progtypes, mprog->type );
         mprog->arglist = QUICKLINK( cprog->arglist );
         mprog->comlist = QUICKLINK( cprog->comlist );
         if( cprog->next )
         {
            CREATE( mprog->next, MPROG_DATA, 1 );
            mprog = mprog->next;
         }
         else
            mprog->next = NULL;
      }
   }
   copy->count = 0;
   iHash = cvnum % MAX_KEY_HASH;
   copy->next = mob_index_hash[iHash];
   mob_index_hash[iHash] = copy;
   top_mob_index++;
   set_char_color( AT_PLAIN, ch );
   send_to_char( "Mobile copied.\n\r", ch );
   return;
}
/*
 * Quite the hack here :P
 */
void medit_setup_arrays( void )
{
   CLAN_DATA *clan;
   DEITY_DATA *deity;
   COUNCIL_DATA *council;
   int count;
   count = 0;
   for( clan = first_clan; clan; clan = clan->next )
   {
      olc_clan_list[count] = clan->name;
         count++;
   }
   olc_top_clan = count - 1;
   count = 0;
   for( deity = first_deity; deity; deity = deity->next )
   {
      olc_deity_list[count] = QUICKLINK( deity->name );
      count++;
   }
   olc_top_deity = count;
   count = 0;
   for( council = first_council; council; council = council->next )
   {
      olc_council_list[count] = council->name;
      count++;
   }
   olc_top_council = count;
}
/**************************************************************************
 Menu Displaying Functions
 **************************************************************************/
/*
 * Display poistions (sitting, standing etc), same for pos and defpos
 */
void medit_disp_positions( DESCRIPTOR_DATA * d )
{
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Positions Menu                                                    EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   send_to_char_color( " &P0&w) Dead\n\r", d->character );
   send_to_char_color( " &P1&w) Mortally Wounded\n\r", d->character );
   send_to_char_color( " &P2&w) Incapacitated\n\r", d->character );
   send_to_char_color( " &P3&w) Stunned\n\r", d->character );
   send_to_char_color( " &P4&w) Sleeping\n\r", d->character );
   send_to_char_color( " &P5&w) Berserk\n\r", d->character );
   send_to_char_color( " &P6&w) Resting\n\r", d->character );
   send_to_char_color( " &P7&w) Aggressive\n\r", d->character );
   send_to_char_color( " &P8&w) Sitting\n\r", d->character );
   send_to_char_color( " &P9&w) Fighting\n\r", d->character );
   send_to_char_color( "&P10&w) Defensive\n\r", d->character );
   send_to_char_color( "&P11&w) Evasive\n\r", d->character );
   send_to_char_color( "&P12&w) Standing\n\r", d->character );
   /*
    * for (i = 0; i < POS_MOUNTED; i++)
    * {
    * sprintf(buf, "&P%2d&w) %s\r\n", i, capitalize(position_names[i]));
    * send_to_char_color(buf, d->character);
    * } 
    */
   send_to_char( "Enter position number : ", d->character );
}
/*
 * Display mobile sexes, this is hard coded cause it just works that way :)
 */
void medit_disp_sex( DESCRIPTOR_DATA * d )
{
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Sex Menu                                                          EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   send_to_char_color( " &P0&w) Neutral\n\r", d->character );
   send_to_char_color( " &P1&w) Male\n\r", d->character );
   send_to_char_color( " &P2&w) Female\n\r", d->character );
   send_to_char( "\n\rEnter gender number : ", d->character );
}
void medit_disp_spec( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Special Functions Menu                                            EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( ch, " &P 7&w) Spec_guard\n\r" );
   send_to_char( "Enter number of special: ", ch );
}
/*
 * Used for both mob affected_by and object affect bitvectors
 */
void medit_disp_ris( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int counter;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Resistance Menu                                                   EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( counter = 0; counter < 24; counter++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s\n\r", counter + 1, ris_flags[counter] );
   }
   switch ( OLC_MODE( d ) )
   {
      case OEDIT_AFFECT_MODIFIER:
         ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( d->character->tempnum, ris_flags ) );
         break;
      case MEDIT_RESISTANT:
         ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->resistant, ris_flags ) );
         break;
      case MEDIT_IMMUNE:
         ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->immune, ris_flags ) );
         break;
      case MEDIT_SUSCEPTIBLE:
         ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\r", flag_string( victim->susceptible, ris_flags ) );
         break;
   }
   send_to_char( "Enter flag (0 to quit): ", d->character );
}
/*-------------------------------------------------------------------*/
/*. Display mob-flags menu .*/
void medit_disp_mob_flags( DESCRIPTOR_DATA * d )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim = d->character->dest_buf;
   int i, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Act Flags Menu                                                    EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( i = 0; i < MAX_ACT; i++ )
   {
      sprintf( buf, "&P%2d&w) %-20.20s  ", i + 1, act_flags[i] );
      if( !( ++columns % 2 ) )
         strcat( buf, "\n\r" );
      send_to_char_color( buf, d->character );
   }
   sprintf( buf, "\r\n"
            "Current flags : &c%s&w\r\n" "Enter mob flags (0 to quit) : ", ext_flag_string( &victim->act, act_flags ) );
   send_to_char_color( buf, d->character );
}
/*
 * Special handing for PC only flags
 */
void medit_disp_plr_flags( DESCRIPTOR_DATA * d )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim = d->character->dest_buf;
   int i, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Player Flags Menu                                                 EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( i = 0; i < PLR_INVISPROMPT; i++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s   ", i + 1, plr_flags[i] );
      if( ++columns % 2 == 0 )
         send_to_char( "\n\r", d->character );
   }
   sprintf( buf, "\n\rCurrent flags: &c%s&w\n\rEnter flags (0 to quit): ", ext_flag_string( &victim->act, plr_flags ) );
   send_to_char_color( buf, d->character );
}
void medit_disp_pcdata_flags( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int i;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Player Flags Menu                                                 EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( i = 0; i < 15; i++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s   \n\r", i + 1, pc_flags[i] );
   }
   ch_printf_color( d->character, "\n\rCurrent flags: &c%s&w\n\rEnter flags (0 to quit): ",
                    flag_string( victim->pcdata->flags, pc_flags ) );
}
/*-------------------------------------------------------------------*/
/*. Display aff-flags menu .*/
void medit_disp_aff_flags( DESCRIPTOR_DATA * d )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim = d->character->dest_buf;
   int i, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Display Affects Menu                                              EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( i = 0; i < MAX_AFFECTED_BY; i++ )
   {
      sprintf( buf, "&P%2d&w) %-20.20s  ", i + 1, a_flags[i] );
      if( !( ++columns % 2 ) )
         strcat( buf, "\r\n" );
      send_to_char_color( buf, d->character );
   }
   /*
    * sprintf(buf, "\r\n"
    * "Current flags   : &c%s&w\r\n"
    * "Enter affected flags (0 to quit) : ",
    * affect_bit_name( &victim->affected_by )
    * ); 
    */
   if( OLC_MODE( d ) == OEDIT_AFFECT_MODIFIER )
   {
      buf[0] = STRING_NULL;
      for( i = 0; i < 36; i++ )
         if( IS_SET( d->character->tempnum, 1 << i ) )
         {
            strcat( buf, " " );
            strcat( buf, a_flags[i] );
         }
      ch_printf_color( d->character, "\n\rCurrent flags   : &c%s&w\n\r", buf );
   }
   else
      ch_printf_color( d->character, "\n\rCurrent flags   : &c%s&w\n\r", affect_bit_name( &victim->affected_by ) );
   send_to_char_color( "Enter affected flags (0 to quit) : ", d->character );
}
void medit_disp_clans( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   int count;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Clans Menu                                                        EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   /*
    * ch_printf( ch, "%d %d %d\n\r", olc_top_clan, olc_top_guild, olc_top_order ); 
    */
   send_to_char( "Clans:\n\r", d->character );
   for( count = 0; count <= olc_top_clan; count++ )
   {
      ch_printf_color( ch, "&P%2d&w) %-20.20s\n\r", count + 1, olc_clan_list[count] );
   }
   send_to_char( "\n\rGuilds:\n\r", d->character );
   for( count = olc_top_clan + 1; count <= olc_top_guild; count++ )
   {
      ch_printf_color( ch, "&P%2d&w) %-20.20s\n\r", count + 1, olc_clan_list[count] );
   }
   send_to_char( "\n\rOrders:\n\r", d->character );
   for( count = olc_top_guild + 1; count <= olc_top_order; count++ )
   {
      ch_printf_color( ch, "&P%2d&w) %-20.20s\n\r", count + 1, olc_clan_list[count] );
   }
   send_to_char( "Enter choice (0 for none): ", d->character );
}
void medit_disp_deities( DESCRIPTOR_DATA * d )
{
   int count;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Deities Menu                                                      EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf( d->character, "%d\n\r", olc_top_deity );
   ch_printf_color( d->character, "&P%2d&w) %-20.20s\n\r", 0, "None" );
   for( count = 0; count < olc_top_deity; count++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s\n\r", count + 1, olc_deity_list[count] );
   }
   send_to_char( "Enter choice: ", d->character );
}
void medit_disp_councils( DESCRIPTOR_DATA * d )
{
   int count;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Councils Menu                                                     EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf( d->character, "%d\n\r", olc_top_council );
   ch_printf_color( d->character, "&P%2d&w) %-20.20s\n\r", 0, "None" );
   for( count = 0; count < olc_top_council; count++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s\n\r", count + 1, olc_council_list[count] );
   }
   send_to_char( "Enter choice: ", d->character );
}
void medit_disp_parts( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int count, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Parts Menu                                                        EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   for( count = 0; count < 29; count++ )
   {
      ch_printf_color( d->character, "&P%2d&w) %-20.20s    ", count + 1, part_flags[count] );
      if( ++columns % 2 == 0 )
         send_to_char( "\n\r", d->character );
   }
   ch_printf_color( d->character, "\n\rCurrent flags: %s\n\rEnter flag or 0 to exit: ",
                    flag_string( victim->xflags, part_flags ) );
}
void medit_disp_Classes( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int iClass, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Classes Menu                                                      EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   if( IS_NPC( victim ) )
   {
      for( iClass = 0; iClass < MAX_NPC_CLASS; iClass++ )
      {
         ch_printf_color( d->character, "&P%2d&w) %-20.20s     ", iClass, npc_Class[iClass] );
         if( ++columns % 2 == 0 )
            send_to_char( "\n\r", d->character );
      }
   }
   else
   {
      for( iClass = 0; iClass < MAX_CLASS; iClass++ )
      {
         ch_printf_color( d->character, "&P%2d&w) %-20.20s     \n\r", iClass, Class_table[iClass]->who_name );
         /*
          * if ( ++columns % 2 == 0 )
          * send_to_char( "\n\r", d->character ); 
          */
      }
   }
   send_to_char( "\n\rEnter Class: ", d->character );
}
void medit_disp_races( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int iRace, columns = 0;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Races Menu                                                        EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   if( IS_NPC( victim ) )
   {
      for( iRace = 0; iRace < MAX_NPC_RACE; iRace++ )
      {
         ch_printf_color( d->character, "&P%2d&w) %-20.20s  ", iRace, npc_race[iRace] );
         if( ++columns % 3 == 0 )
            send_to_char( "\n\r", d->character );
      }
   }
   else
   {
      for( iRace = 0; iRace < MAX_RACE; iRace++ )
      {
         ch_printf_color( d->character, "&P%2d&w) %-20.20s     \n\r", iRace, race_table[iRace]->race_name );
         /*
          * if ( ++columns % 2 == 0 )
          * send_to_char( "\n\r", d->character ); 
          */
      }
   }
   send_to_char( "\n\rEnter race: ", d->character );
}
void medit_disp_saving_menu( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Saving Throws Menu                                                EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character, "&P1&w) %-30.30s: %2d\n\r", "Saving vs. poison", victim->saving_poison_death );
   ch_printf_color( d->character, "&P2&w) %-30.30s: %2d\n\r", "Saving vs. mental", victim->saving_mental );
   ch_printf_color( d->character, "&P3&w) %-30.30s: %2d\n\r", "Saving vs. physical", victim->saving_physical );
   ch_printf_color( d->character, "&P4&w) %-30.30s: %2d\n\r", "Saving vs. weapons", victim->saving_weapons );
   send_to_char( "\n\rModify which saving throw: ", d->character );
   OLC_MODE( d ) = MEDIT_SAVE_MENU;
}
void medit_disp_menu( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *victim = d->character->dest_buf;
   if( !IS_NPC( victim ) )
      medit_disp_pc_menu( d );
   else
      medit_disp_npc_menu( d );
}
/*
 * Display main menu for NPCs
 */
void medit_disp_npc_menu( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *mob = d->character->dest_buf;
   int hitestimate, damestimate;
   if( !mob->pIndexData->hitnodice )
      hitestimate = mob->level * 8 + number_range( mob->level * mob->level / 4, mob->level * mob->level );
   else
      hitestimate = mob->pIndexData->hitnodice * number_range( 1, mob->pIndexData->hitsizedice ) + mob->pIndexData->hitplus;
   damestimate = number_range( mob->pIndexData->damnodice, mob->pIndexData->damsizedice * mob->pIndexData->damnodice );
   damestimate += GET_DAMROLL( mob ); 
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c NPC Main Menu                                                    EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   set_char_color( AT_PLAIN, d->character );
   ch_printf_color( ch, "-- Mob VNum   : &c%d\n\r", mob->pIndexData->vnum );
   ch_printf_color( ch, "&P1&w) Sex        : &O%s \n\r",  mob->sex == SEX_MALE ? "male" : mob->sex == SEX_FEMALE ? "female" : "neutral" );
   ch_printf_color( ch, "&P2&w) Name       : &O%s\n\r", mob->name); 
   ch_printf_color( ch, "&P3&w) Shortdesc  : &O%s\n\r", mob->short_descr[0] == STRING_NULL ? "(none set)" : mob->short_descr );
   ch_printf_color( ch, "&P4&w) Longdesc   : &O%s", mob->long_descr[0] == STRING_NULL ? "(none set)" : mob->long_descr );
   ch_printf_color( ch, "&P5&w) Description:-\n\r&O%-74.74s\n\r", mob->description );
   ch_printf_color( ch, "&P6&w) Class      : &c%s\n\r", npc_Class[mob->Class]); 
   ch_printf_color( ch, "&P7&w) Race       : &c%s\n\r", npc_race[mob->race] );
   ch_printf_color( ch, "&P8&w) Level      : &c%d\n\r", mob->level);
   ch_printf_color( ch, "&P9&w) Alignment  : &c%d\n\r", mob->alignment );
   ch_printf_color( ch, "&PJ&w) DamPlus    : &c%d\n\r", mob->pIndexData->damplus );
   ch_printf_color( ch, "&PM&w) HitPlus    : &c%d\r\n", mob->pIndexData->hitplus );
   ch_printf_color( ch, "&PN&w) Gold       : &c%d\n\r", mob->gold );
   ch_printf_color( ch, "&PR&w) Resistant  : &O%s\n\r", flag_string( mob->resistant, ris_flags ) );
   ch_printf_color( ch, "&PS&w) Immune     : &O%s\n\r", flag_string( mob->immune, ris_flags ) );
   ch_printf_color( ch, "&PT&w) Susceptible: &O%s\n\r", flag_string( mob->susceptible, ris_flags ) );
   ch_printf_color( ch, "&PU&w) Position   : &O%s\n\r", position_names[( int )mob->position] );
   ch_printf_color( ch, "&PW&w) Defenses   : &c%s\n\r", ext_flag_string( &mob->defenses, defense_flags ) );
   ch_printf_color( ch, "&PY&w) Act Flags  : &c%s\n\r", ext_flag_string( &mob->act, act_flags ) );
   ch_printf_color( ch, "&PO&w) Spec: &O%-22.22s\n\r", lookup_spec( mob->spec_fun ) );
   ch_printf_color( ch, "&PP&w) Saving Throws\n\r" );
   ch_printf_color( ch, "&PQ&w) Quit\n\r" );
   ch_printf_color( ch, "Enter choice : " );
   OLC_MODE( d ) = MEDIT_NPC_MAIN_MENU;
}
void medit_disp_pc_menu( DESCRIPTOR_DATA * d )
{
   CHAR_DATA *ch = d->character;
   CHAR_DATA *victim = d->character->dest_buf;
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( d->character,"&c Player Main Menu                                                  EldhaMUD OLC\n\r", 0 );
   ch_printf_color( d->character,"&D--------------------------------------------------------------------------------\n\r");
   ch_printf_color( ch, "&P1&w) Sex: &O%-7.7s           &P2&w) Name: &O%s\n\r",
                    victim->sex == SEX_MALE ? "male" : victim->sex == SEX_FEMALE ? "female" : "neutral", victim->name );
   ch_printf_color( ch, "&P3&w) Description:-\n\r&O%-74.74s\n\r", victim->description );
   ch_printf_color( ch, "&P4&w) Class: [&c%-11.11s&w],  &P5&w) Race:   [&c%-11.11s&w]\n\r",
                    Class_table[victim->Class]->who_name, race_table[victim->race]->race_name );
   ch_printf_color( ch, "&P6&w) Level:       [&c%5d&w],  &P7&w) Alignment:    [&c%5d&w],  &P8&w) Strength:  [&c%5d&w]\n\r",
                    victim->level, victim->alignment, get_curr_str( victim ) );
   ch_printf_color( ch, "&P9&w) Intelligence:[&c%5d&w],  &PA&w) Widsom:       [&c%5d&w],  &PB&w) Dexterity: [&c%5d&w]\n\r",
                    get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ) );
   ch_printf_color( ch, "&PC&w) Constitution:[&c%5d&w],  &PD&w) Charisma:     [&c%5d&w],  &PE&w) Luck:      [&c%5d&w]\n\r",
                    get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ) );
   ch_printf_color( ch,
                    "&PF&w) Hps:   [&c%5d&w/&c%5d&w],  &PG&w) Mana:   [&c%5d&w/&c%5d&w],  &PH&w) Move:[&c%5d&w/&c%-5d&w]\n\r",
                    victim->hit, victim->max_hit, victim->mana, victim->max_mana, victim->move, victim->max_move );
   ch_printf_color( ch, "&PI&w) Gold:  [&c%11d&w]\n\r",       victim->gold);
   ch_printf_color( ch, "&PL&w) Thirst:      [&c%5d&w],  &PM&w) Full:         [&c%5d&w],  &PN&w) Drunk:     [&c%5d&w]\n\r",
                    victim->pcdata->condition[COND_THIRST], victim->pcdata->condition[COND_FULL],
                    victim->pcdata->condition[COND_DRUNK] );
   ch_printf_color( ch, "&PO&w) Favor:       [&c%5d&w]\n\r", victim->pcdata->favor );
   ch_printf_color( ch, "&PP&w) Saving Throws\n\r" );
   ch_printf_color( ch, "&PR&w) Resistant   : &O%s\n\r", flag_string( victim->resistant, ris_flags ) );
   ch_printf_color( ch, "&PS&w) Immune      : &O%s\n\r", flag_string( victim->immune, ris_flags ) );
   ch_printf_color( ch, "&PT&w) Susceptible : &O%s\n\r", flag_string( victim->susceptible, ris_flags ) );
   ch_printf_color( ch, "&PU&w) Position    : &O%s\n\r", position_names[( int )victim->position] );
   ch_printf_color( ch, "&PV&w) Act Flags   : &c%s\n\r", ext_flag_string( &victim->act, plr_flags ) );
   ch_printf_color( ch, "&PW&w) PC Flags    : &c%s\n\r", flag_string( victim->pcdata->flags, pc_flags ) );
   ch_printf_color( ch, "&PX&w) Affected    : &c%s\n\r", affect_bit_name( &victim->affected_by ) );
   ch_printf_color( ch, "&PY&w) Deity       : &O%s\n\r", victim->pcdata->deity ? victim->pcdata->deity->name : "None" );
   if( get_trust( ch ) >= LEVEL_GOD && victim->pcdata->clan )
      ch_printf_color( ch, "&PZ&w) CLan: &O%s\n\r", victim->pcdata->clan->name );
   else if( get_trust( ch ) >= LEVEL_GOD && !victim->pcdata->clan )
      ch_printf_color( ch, "&PZ&w) Clan        : &ONone\n\r" );
   if( get_trust( ch ) >= LEVEL_SUB_IMPLEM )
      ch_printf_color( ch, "&P=&w) Council     : &O%s\n\r",
                       victim->pcdata->council ? victim->pcdata->council->name : "None" );
   ch_printf_color( ch, "&PQ&w) Quit\n\r" );
   ch_printf_color( ch, "Enter choice : " );
   OLC_MODE( d ) = MEDIT_PC_MAIN_MENU;
}
/*
 * Bogus command for resetting stuff
 */
void do_medit_reset( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim = ch->dest_buf;
   switch ( ch->substate )
   {
      default:
         return;
      case SUB_MOB_DESC:
         if( !ch->dest_buf )
         {
            send_to_char( "Fatal error, report to Tagith.\n\r", ch );
            bug( "do_medit_reset: sub_mob_desc: NULL ch->dest_buf", 0 );
            cleanup_olc( ch->desc );
            ch->substate = SUB_NONE;
            return;
         }
         STRFREE( victim->description );
         victim->description = copy_buffer( ch );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->description );
            victim->pIndexData->description = QUICKLINK( victim->description );
         }
         stop_editing( ch );
         ch->dest_buf = victim;
         ch->substate = SUB_NONE;
         ch->desc->connected = CON_MEDIT;
         medit_disp_menu( ch->desc );
         return;
   }
}
/**************************************************************************
  The GARGANTAUN event handler
 **************************************************************************/
void medit_parse( DESCRIPTOR_DATA * d, char *arg )
{
   CHAR_DATA *victim = d->character->dest_buf;
   int number = 0, minattr, maxattr;
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CLAN_DATA *clan;
   DEITY_DATA *deity;
   COUNCIL_DATA *council;
   AREA_DATA *tarea;
   bool found = FALSE;
   char *pwdnew, *p;
   if( IS_NPC( victim ) )
   {
      minattr = 1;
      maxattr = 25;
   }
   else
   {
      minattr = 3;
      maxattr = 25;
   }
   switch ( OLC_MODE( d ) )
   {
      case MEDIT_CONFIRM_SAVESTRING:
         switch ( UPPER( *arg ) )
         {
            case 'Y':
               /*
                * If its a mob, fold_area otherwise save_char_obj 
                */
               send_to_char( "Saving...\n\r", d->character );
               if( IS_NPC( victim ) )
               {
                  sprintf( log_buf, "OLC: %s edits mob %d(%s)", d->character->name, victim->pIndexData->vnum, victim->name );
                  for( tarea = first_asort; tarea; tarea = tarea->next )
                  {
                     if( OLC_VNUM( d ) >= tarea->low_m_vnum && OLC_VNUM( d ) <= tarea->hi_m_vnum )
                     {
                        if( get_trust( d->character ) >= LEVEL_SUB_IMPLEM )
                           fold_area( tarea, tarea->filename, FALSE );
                        found = TRUE;
                     }
                  }
                  /*
                   * I'm assuming that if it isn't an installed area, its the char's 
                   */
                  if( !found
                      && ( tarea = d->character->pcdata->area ) != NULL
                      && get_trust( d->character ) >= LEVEL_CREATOR && IS_SET( tarea->status, AREA_LOADED ) )
                  {
                     tarea = d->character->pcdata->area;
                     sprintf( buf, "%s%s", BUILD_DIR, tarea->filename );
                     fold_area( tarea, buf, FALSE );
                  }
               }
               else
               {
                  sprintf( log_buf, "OLC: %s edits %s", d->character->name, victim->name );
                  save_char_obj( victim );
               }
               log_string_plus( log_buf, LOG_BUILD, d->character->level );
               cleanup_olc( d );
               return;
            case 'N':
               cleanup_olc( d );
               return;
            default:
               send_to_char( "Invalid choice!\n\r", d->character );
               send_to_char( "Do you wish to save to disk? : ", d->character );
               return;
         }
         break;
      case MEDIT_NPC_MAIN_MENU:
         switch ( UPPER( *arg ) )
         {
            case 'Q':
               cleanup_olc( d );
               return;
            case '1':
               OLC_MODE( d ) = MEDIT_SEX;
               medit_disp_sex( d );
               return;
            case '2':
               OLC_MODE( d ) = MEDIT_NAME;
               send_to_char( "\n\rEnter name: ", d->character );
               return;
            case '3':
               OLC_MODE( d ) = MEDIT_S_DESC;
               send_to_char( "\n\rEnter short description: ", d->character );
               return;
            case '4':
               OLC_MODE( d ) = MEDIT_L_DESC;
               send_to_char( "\n\rEnter long description: ", d->character );
               return;
            case '5':
               OLC_MODE( d ) = MEDIT_D_DESC;
               d->character->substate = SUB_MOB_DESC;
               d->character->last_cmd = do_medit_reset;
               send_to_char( "Enter new mob description:\r\n", d->character );
               if( !victim->description )
                  victim->description = STRALLOC( "" );
               start_editing( d->character, victim->description );
               return;
            case '6':
               OLC_MODE( d ) = MEDIT_CLASS;
               medit_disp_Classes( d );
               return;
            case '7':
               OLC_MODE( d ) = MEDIT_RACE;
               medit_disp_races( d );
               return;
            case '8':
               OLC_MODE( d ) = MEDIT_LEVEL;
               send_to_char( "\n\rEnter level: ", d->character );
               return;
            case '9':
               OLC_MODE( d ) = MEDIT_ALIGNMENT;
               send_to_char( "\n\rEnter alignment: ", d->character );
               return;
            case 'A':
               OLC_MODE( d ) = MEDIT_STRENGTH;
               send_to_char( "\n\rEnter strength: ", d->character );
               return;
            case 'B':
               OLC_MODE( d ) = MEDIT_INTELLIGENCE;
               send_to_char( "\n\rEnter intelligence: ", d->character );
               return;
            case 'C':
               OLC_MODE( d ) = MEDIT_WISDOM;
               send_to_char( "\n\rEnter wisdom: ", d->character );
               return;
            case 'D':
               OLC_MODE( d ) = MEDIT_DEXTERITY;
               send_to_char( "\n\rEnter dexterity: ", d->character );
               return;
            case 'E':
               OLC_MODE( d ) = MEDIT_CONSTITUTION;
               send_to_char( "\n\rEnter constitution: ", d->character );
               return;
            case 'F':
               OLC_MODE( d ) = MEDIT_CHARISMA;
               send_to_char( "\n\rEnter charisma: ", d->character );
               return;
            case 'G':
               OLC_MODE( d ) = MEDIT_LUCK;
               send_to_char( "\n\rEnter luck: ", d->character );
               return;
            case 'H':
               OLC_MODE( d ) = MEDIT_DAMNUMDIE;
               send_to_char( "\n\rEnter number of damage dice: ", d->character );
               return;
            case 'I':
               OLC_MODE( d ) = MEDIT_DAMSIZEDIE;
               send_to_char( "\n\rEnter size of damage dice: ", d->character );
               return;
            case 'J':
               OLC_MODE( d ) = MEDIT_DAMPLUS;
               send_to_char( "\n\rEnter amount to add to damage: ", d->character );
               return;
            case 'K':
               OLC_MODE( d ) = MEDIT_HITNUMDIE;
               send_to_char( "\n\rEnter number of hitpoint dice: ", d->character );
               return;
            case 'L':
               OLC_MODE( d ) = MEDIT_HITSIZEDIE;
               send_to_char( "\n\rEnter size of hitpoint dice: ", d->character );
               return;
            case 'M':
               OLC_MODE( d ) = MEDIT_HITPLUS;
               send_to_char( "\n\rEnter amount to add to hitpoints: ", d->character );
               return;
            case 'N':
               OLC_MODE( d ) = MEDIT_GOLD;
               send_to_char( "\n\rEnter amount of gold mobile carries: ", d->character );
               return;
            case 'O':
               OLC_MODE( d ) = MEDIT_SPEC;
               medit_disp_spec( d );
               return;
            case 'P':
               OLC_MODE( d ) = MEDIT_SAVE_MENU;
               medit_disp_saving_menu( d );
               return;
            case 'R':
               OLC_MODE( d ) = MEDIT_RESISTANT;
               medit_disp_ris( d );
               return;
            case 'S':
               OLC_MODE( d ) = MEDIT_IMMUNE;
               medit_disp_ris( d );
               return;
            case 'T':
               OLC_MODE( d ) = MEDIT_SUSCEPTIBLE;
               medit_disp_ris( d );
               return;
            case 'U':
               OLC_MODE( d ) = MEDIT_POS;
               medit_disp_positions( d );
               return;
            case 'X':
               OLC_MODE( d ) = MEDIT_PARTS;
               medit_disp_parts( d );
               return;
            case 'Y':
               OLC_MODE( d ) = MEDIT_NPC_FLAGS;
               medit_disp_mob_flags( d );
               return;
            case 'Z':
               OLC_MODE( d ) = MEDIT_AFF_FLAGS;
               medit_disp_aff_flags( d );
               return;
            default:
               medit_disp_npc_menu( d );
               return;
         }
         break;
      case MEDIT_PC_MAIN_MENU:
         switch ( UPPER( *arg ) )
         {
            case 'Q':
               if( OLC_CHANGE( d ) )
               {
                  send_to_char( "Do you wish to save changes to disk? (y/n): ", d->character );
                  OLC_MODE( d ) = MEDIT_CONFIRM_SAVESTRING;
               }
               else
                  cleanup_olc( d );
               return;
            case '1':
               OLC_MODE( d ) = MEDIT_SEX;
               medit_disp_sex( d );
               return;
            case '2':
               OLC_MODE( d ) = MEDIT_NAME;
               return;
            case '3':
               OLC_MODE( d ) = MEDIT_D_DESC;
               d->character->substate = SUB_MOB_DESC;
               d->character->last_cmd = do_medit_reset;
               send_to_char( "Enter new player description:\r\n", d->character );
               if( !victim->description )
                  victim->description = STRALLOC( "" );
               start_editing( d->character, victim->description );
               return;
            case '4':
               OLC_MODE( d ) = MEDIT_CLASS;
               medit_disp_Classes( d );
               return;
            case '5':
               OLC_MODE( d ) = MEDIT_RACE;
               medit_disp_races( d );
               return;
            case '6':
               send_to_char( "\n\rNPC Only!!", d->character );
               break;
            case '7':
               OLC_MODE( d ) = MEDIT_ALIGNMENT;
               send_to_char( "\n\rEnter alignment: ", d->character );
               return;
            case '8':
               OLC_MODE( d ) = MEDIT_STRENGTH;
               send_to_char( "\n\rEnter strength: ", d->character );
               return;
            case '9':
               OLC_MODE( d ) = MEDIT_INTELLIGENCE;
               send_to_char( "\n\rEnter intelligence: ", d->character );
               return;
            case 'A':
               OLC_MODE( d ) = MEDIT_WISDOM;
               send_to_char( "\n\rEnter wisdom: ", d->character );
               return;
            case 'B':
               OLC_MODE( d ) = MEDIT_DEXTERITY;
               send_to_char( "\n\rEnter dexterity: ", d->character );
               return;
            case 'C':
               OLC_MODE( d ) = MEDIT_CONSTITUTION;
               send_to_char( "\n\rEnter constitution: ", d->character );
               return;
            case 'D':
               OLC_MODE( d ) = MEDIT_CHARISMA;
               send_to_char( "\n\rEnter charisma: ", d->character );
               return;
            case 'E':
               OLC_MODE( d ) = MEDIT_LUCK;
               send_to_char( "\n\rEnter luck: ", d->character );
               return;
            case 'F':
               OLC_MODE( d ) = MEDIT_HITPOINT;
               send_to_char( "\n\rEnter hitpoints: ", d->character );
               return;
            case 'G':
               OLC_MODE( d ) = MEDIT_MANA;
               send_to_char( "\n\rEnter mana: ", d->character );
               return;
            case 'H':
               OLC_MODE( d ) = MEDIT_MOVE;
               send_to_char( "\n\rEnter moves: ", d->character );
               return;
            case 'I':
               OLC_MODE( d ) = MEDIT_GOLD;
               send_to_char( "\n\rEnter amount of gold player carries: ", d->character );
               return;
            case 'J':
               OLC_MODE( d ) = MEDIT_MENTALSTATE;
               send_to_char( "\n\rEnter players mentalstate: ", d->character );
               return;
            case 'K':
               OLC_MODE( d ) = MEDIT_EMOTIONAL;
               send_to_char( "\n\rEnter players emotional state: ", d->character );
               return;
            case 'L':
               OLC_MODE( d ) = MEDIT_THIRST;
               send_to_char( "\n\rEnter player's thirst (0 = dehydrated): ", d->character );
               return;
            case 'M':
               OLC_MODE( d ) = MEDIT_FULL;
               send_to_char( "\n\rEnter player's fullness (0 = starving): ", d->character );
               return;
            case 'N':
               OLC_MODE( d ) = MEDIT_DRUNK;
               send_to_char( "\n\rEnter player's drunkeness (0 = sober): ", d->character );
               return;
            case 'O':
               OLC_MODE( d ) = MEDIT_FAVOR;
               send_to_char( "\n\rEnter player's favor (-2500 to 2500): ", d->character );
               return;
            case 'P':
               OLC_MODE( d ) = MEDIT_SAVE_MENU;
               medit_disp_saving_menu( d );
               return;
            case 'R':
               OLC_MODE( d ) = MEDIT_RESISTANT;
               medit_disp_ris( d );
               return;
            case 'S':
               OLC_MODE( d ) = MEDIT_IMMUNE;
               medit_disp_ris( d );
               return;
            case 'T':
               OLC_MODE( d ) = MEDIT_SUSCEPTIBLE;
               medit_disp_ris( d );
               return;
            case 'U':
               send_to_char( "NPCs Only!!\n\r", d->character );
               break;
            case 'V':
               OLC_MODE( d ) = MEDIT_PC_FLAGS;
               medit_disp_plr_flags( d );
               return;
            case 'W':
               OLC_MODE( d ) = MEDIT_PCDATA_FLAGS;
               medit_disp_pcdata_flags( d );
               return;
            case 'X':
               OLC_MODE( d ) = MEDIT_AFF_FLAGS;
               medit_disp_aff_flags( d );
               return;
            case 'Y':
               OLC_MODE( d ) = MEDIT_DEITY;
               medit_disp_deities( d );
               return;
            case 'Z':
               if( get_trust( d->character ) < LEVEL_GOD )
                  break;
               OLC_MODE( d ) = MEDIT_CLAN;
               medit_disp_clans( d );
               return;
            case '=':
               if( get_trust( d->character ) < LEVEL_SUB_IMPLEM )
                  break;
               OLC_MODE( d ) = MEDIT_COUNCIL;
               medit_disp_councils( d );
               return;
            default:
               medit_disp_npc_menu( d );
               return;
         }
         break;
      case MEDIT_NAME:
         if( !IS_NPC( victim ) && get_trust( d->character ) > LEVEL_SUB_IMPLEM - 1 )
         {
            sprintf( buf, "%s %s", victim->name, arg );
            do_pcrename( d->character, buf );
            olc_log( d, "Changes name to %s", arg );
            return;
         }
         STRFREE( victim->name );
         victim->name = STRALLOC( arg );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->player_name );
            victim->pIndexData->player_name = QUICKLINK( victim->name );
         }
         olc_log( d, "Changed name to %s", arg );
         break;
      case MEDIT_S_DESC:
         STRFREE( victim->short_descr );
         victim->short_descr = STRALLOC( arg );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->short_descr );
            victim->pIndexData->short_descr = QUICKLINK( victim->short_descr );
         }
         olc_log( d, "Changed short desc to %s", arg );
         break;
      case MEDIT_L_DESC:
         STRFREE( victim->long_descr );
         strcpy( buf, arg );
         strcat( buf, "\n\r" );
         victim->long_descr = STRALLOC( buf );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
         {
            STRFREE( victim->pIndexData->long_descr );
            victim->pIndexData->long_descr = QUICKLINK( victim->long_descr );
         }
         olc_log( d, "Changed long desc to %s", arg );
         break;
      case MEDIT_D_DESC:
         /*
          * . We should never get here .
          */
         cleanup_olc( d );
         bug( "OLC: medit_parse(): Reached D_DESC case!", 0 );
         break;
      case MEDIT_NPC_FLAGS:
         /*
          * REDONE, again, then again 
          */
         if( is_number( arg ) )
            if( atoi( arg ) == 0 )
               break;
         while( arg[0] != STRING_NULL )
         {
            arg = one_argument( arg, arg1 );
            if( is_number( arg1 ) )
            {
               number = atoi( arg1 );
               number -= 1;
               if( number < 0 || number > MAX_ACT )
               {
                  send_to_char( "Invalid flag, try again: \n\r", d->character );
                  return;
               }
            }
            else
            {
               number = get_actflag( arg1 );
               if( number < 0 )
               {
                  send_to_char( "Invalid flag, try again: \n\r", d->character );
                  return;
               }
            }
            if( IS_NPC( victim )
                && number == ACT_PROTOTYPE
                && get_trust( d->character ) < 35 && !is_name( "protoflag", d->character->pcdata->bestowments ) )
               send_to_char( "You don't have permission to change the prototype flag.\n\r", d->character );
            else if( IS_NPC( victim ) && number == ACT_IS_NPC )
               send_to_char( "It isn't possible to change that flag.\n\r", d->character );
            else
            {
               xTOGGLE_BIT( victim->act, number );
            }
            if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
               victim->pIndexData->act = victim->act;
         }
         medit_disp_mob_flags( d );
         return;
      case MEDIT_PC_FLAGS:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            if( ( number > 0 ) || ( number < 31 ) )
            {
               number -= 1;   /* offset :P */
               xTOGGLE_BIT( victim->act, number );
               olc_log( d, "%s the flag %s", xIS_SET( victim->act, number ) ? "Added" : "Removed", plr_flags[number] );
            }
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_actflag( arg1 );
               if( number > 0 )
               {
                  xTOGGLE_BIT( victim->act, number );
                  olc_log( d, "%s the flag %s", xIS_SET( victim->act, number ) ? "Added" : "Removed", plr_flags[number] );
               }
            }
         }
         medit_disp_plr_flags( d );
         return;
      case MEDIT_PCDATA_FLAGS:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            if( ( number > 0 ) || ( number < 31 ) )
            {
               number -= 1;
               TOGGLE_BIT( victim->pcdata->flags, 1 << number );
               olc_log( d, "%s the pcflag %s", IS_SET( victim->pcdata->flags, 1 << number ) ? "Added" : "Removed",
                        pc_flags[number] );
            }
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_actflag( arg1 );
               if( number > 0 )
               {
                  TOGGLE_BIT( victim->pcdata->flags, 1 << number );
                  olc_log( d, "%s the pcflag %s", IS_SET( victim->pcdata->flags, 1 << number ) ? "Added" : "Removed",
                           pc_flags[number] );
               }
            }
         }
         medit_disp_pcdata_flags( d );
         return;
      case MEDIT_AFF_FLAGS:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            if( ( number > 0 ) || ( number < 31 ) )
            {
               number -= 1;
               xTOGGLE_BIT( victim->affected_by, number );
               olc_log( d, "%s the affect %s", xIS_SET( victim->affected_by, number ) ? "Added" : "Removed",
                        a_flags[number] );
            }
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_actflag( arg1 );
               if( number > 0 )
               {
                  xTOGGLE_BIT( victim->affected_by, number );
                  olc_log( d, "%s the affect %s", xIS_SET( victim->affected_by, number ) ? "Added" : "Removed",
                           a_flags[number] );
               }
            }
         }
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->affected_by = victim->affected_by;
         medit_disp_aff_flags( d );
         return;
/*-------------------------------------------------------------------*/
/*. Numerical responses .*/
      case MEDIT_HITPOINT:
         victim->max_hit = URANGE( 1, atoi( arg ), 32700 );
         olc_log( d, "Changed hitpoints to %d", victim->max_hit );
         break;
      case MEDIT_MANA:
         victim->max_mana = URANGE( 1, atoi( arg ), 30000 );
         olc_log( d, "Changed mana to %d", victim->max_mana );
         break;
      case MEDIT_MOVE:
         victim->max_move = URANGE( 1, atoi( arg ), 30000 );
         olc_log( d, "Changed moves to %d", victim->max_move );
         break;
      case MEDIT_PRACTICE:
         victim->practice = URANGE( 1, atoi( arg ), 300 );
         olc_log( d, "Changed practives to %d", victim->practice );
         break;
      case MEDIT_PASSWORD:
         if( get_trust( d->character ) < LEVEL_SUB_IMPLEM )
            break;
         if( strlen( arg ) < 5 )
         {
            send_to_char( "Password too short, try again: ", d->character );
            return;
         }
         pwdnew = crypt( arg, victim->name );
         for( p = pwdnew; *p != STRING_NULL; p++ )
         {
            if( *p == '~' )
            {
               send_to_char( "Unacceptable choice, try again: ", d->character );
               return;
            }
         }
         DISPOSE( victim->pcdata->pwd );
         victim->pcdata->pwd = str_dup( pwdnew );
         if( IS_SET( sysdata.save_flags, SV_PASSCHG ) )
            save_char_obj( victim );
         olc_log( d, "Modified password" );
         break;
      case MEDIT_SAV1:
         victim->saving_poison_death = URANGE( -30, atoi( arg ), 30 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->saving_poison_death = victim->saving_poison_death;
         medit_disp_saving_menu( d );
         olc_log( d, "Changed save_poison_death to %d", victim->saving_poison_death );
         return;
      case MEDIT_SAV2:
         victim->saving_mental = URANGE( -30, atoi( arg ), 30 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->saving_mental = victim->saving_mental;
         medit_disp_saving_menu( d );
         olc_log( d, "Changed save_wand to %d", victim->saving_mental );
         return;
      case MEDIT_SAV3:
         victim->saving_physical = URANGE( -30, atoi( arg ), 30 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->saving_physical = victim->saving_physical;
         medit_disp_saving_menu( d );
         olc_log( d, "Changed save_paralysis_petrification to %d", victim->saving_physical );
         return;
      case MEDIT_SAV4:
         victim->saving_weapons = URANGE( -30, atoi( arg ), 30 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->saving_weapons = victim->saving_weapons;
         medit_disp_saving_menu( d );
         olc_log( d, "Changed save_breath to %d", victim->saving_weapons );
         return;
      case MEDIT_STRENGTH:
         victim->perm_str = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_str = victim->perm_str;
         olc_log( d, "Changed strength to %d", victim->perm_str );
         break;
      case MEDIT_INTELLIGENCE:
         victim->perm_int = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_int = victim->perm_int;
         olc_log( d, "Changed intelligence to %d", victim->perm_int );
         break;
      case MEDIT_WISDOM:
         victim->perm_wis = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_wis = victim->perm_wis;
         olc_log( d, "Changed victim wisdom to %d", victim->perm_wis );
         break;
      case MEDIT_DEXTERITY:
         victim->perm_dex = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_dex = victim->perm_dex;
         olc_log( d, "Changed dexterity to %d", victim->perm_dex );
         break;
      case MEDIT_CONSTITUTION:
         victim->perm_con = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_con = victim->perm_con;
         olc_log( d, "Changed constitution to %d", victim->perm_con );
         break;
      case MEDIT_CHARISMA:
         victim->perm_cha = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_cha = victim->perm_cha;
         olc_log( d, "Changed charisma to %d", victim->perm_cha );
         break;
      case MEDIT_LUCK:
         victim->perm_lck = URANGE( minattr, atoi( arg ), maxattr );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->perm_lck = victim->perm_lck;
         olc_log( d, "Changed luck to %d", victim->perm_lck );
         break;
      case MEDIT_SEX:
         victim->sex = URANGE( 0, atoi( arg ), 2 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->sex = victim->sex;
         olc_log( d, "Changed sex to %s", victim->sex == 1 ? "Male" : victim->sex == 2 ? "Female" : "Neutral" );
         break;
      case MEDIT_HITROLL:
         victim->hitroll = URANGE( 0, atoi( arg ), 85 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->hitroll = victim->hitroll;
         olc_log( d, "Changed hitroll to %d", victim->hitroll );
         break;
      case MEDIT_DAMROLL:
         victim->damroll = URANGE( 0, atoi( arg ), 65 );
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->damroll = victim->damroll;
         olc_log( d, "Changed damroll to %d", victim->damroll );
         break;
      case MEDIT_DAMNUMDIE:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->damnodice = URANGE( 0, atoi( arg ), 100 );
         olc_log( d, "Changed damnumdie to %d", victim->pIndexData->damnodice );
         break;
      case MEDIT_DAMSIZEDIE:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->damsizedice = URANGE( 0, atoi( arg ), 100 );
         olc_log( d, "Changed damsizedie to %d", victim->pIndexData->damsizedice );
         break;
      case MEDIT_DAMPLUS:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->damplus = URANGE( 0, atoi( arg ), 1000 );
         olc_log( d, "Changed damplus to %d", victim->pIndexData->damplus );
         break;
      case MEDIT_HITNUMDIE:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->hitnodice = URANGE( 0, atoi( arg ), 32767 );
         olc_log( d, "Changed hitnumdie to %d", victim->pIndexData->hitnodice );
         break;
      case MEDIT_HITSIZEDIE:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->hitsizedice = URANGE( 0, atoi( arg ), 30000 );
         olc_log( d, "Changed hitsizedie to %d", victim->pIndexData->hitsizedice );
         break;
      case MEDIT_HITPLUS:
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->hitplus = URANGE( 0, atoi( arg ), 30000 );
         olc_log( d, "Changed hitplus to %d", victim->pIndexData->hitplus );
         break;
      case MEDIT_AC:
         victim->armor = URANGE( -300, atoi( arg ), 300 );
         olc_log( d, "Changed armor to %d", victim->armor );
         break;
      case MEDIT_GOLD:
         victim->gold = UMAX( 0, atoi( arg ) );
         olc_log( d, "Changed gold to %d", victim->gold );
         break;
      case MEDIT_POS:
         victim->position = URANGE( 0, atoi( arg ), POS_STANDING );
         olc_log( d, "Changed position to %d", victim->position );
         break;
      case MEDIT_DEFAULT_POS:
         victim->defposition = URANGE( 0, atoi( arg ), POS_STANDING );
         olc_log( d, "Changed default position to %d", victim->defposition );
         break;
      case MEDIT_FAVOR:
         victim->pcdata->favor = URANGE( -2500, atoi( arg ), 2500 );
         olc_log( d, "Changed favor to %d", victim->pcdata->favor );
         break;
      case MEDIT_SAVE_MENU:
         number = atoi( arg );
         switch ( number )
         {
            default:
               send_to_char( "Invalid saving throw, try again: ", d->character );
               return;
            case 0:
               break;
            case 1:
               OLC_MODE( d ) = MEDIT_SAV1;
               send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
               return;
            case 2:
               OLC_MODE( d ) = MEDIT_SAV2;
               send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
               return;
            case 3:
               OLC_MODE( d ) = MEDIT_SAV3;
               send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
               return;
            case 4:
               OLC_MODE( d ) = MEDIT_SAV4;
               send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
               return;
            case 5:
               OLC_MODE( d ) = MEDIT_SAV5;
               send_to_char( "\n\rEnter throw (-30 to 30): ", d->character );
               return;
         }
         /*
          * If we reach here, we are going back to the main menu 
          */
         break;
      case MEDIT_CLASS:
         number = atoi( arg );
         if( IS_NPC( victim ) )
         {
            victim->Class = URANGE( 0, number, MAX_NPC_CLASS - 1 );
            if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
               victim->pIndexData->Class = victim->Class;
            break;
         }
         victim->Class = URANGE( 0, number, MAX_CLASS );
         olc_log( d, "Changed Class to %s", npc_Class[victim->Class] );
         break;
      case MEDIT_RACE:
         number = atoi( arg );
         if( IS_NPC( victim ) )
         {
            victim->race = URANGE( 0, number, MAX_NPC_RACE - 1 );
            if( xIS_SET( victim->act, ACT_PROTOTYPE ) )
               victim->pIndexData->race = victim->race;
            break;
         }
         victim->race = URANGE( 0, number, MAX_RACE - 1 );
         olc_log( d, "Changed race to %s", npc_race[victim->race] );
         break;
      case MEDIT_PARTS:
         number = atoi( arg );
         if( number < 0 || number > 29 )
         {
            send_to_char( "Invalid part, try again: ", d->character );
            return;
         }
         else
         {
            if( number == 0 )
               break;
            else
            {
               number -= 1;
               TOGGLE_BIT( victim->xflags, 1 << number );
            }
            if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
               victim->pIndexData->xflags = victim->xflags;
         }
         olc_log( d, "%s the body part %s", IS_SET( victim->xflags, 1 << ( number - 1 ) ) ? "Added" : "Removed",
                  part_flags[number] );
         medit_disp_parts( d );
         return;
      case MEDIT_LEVEL:
         victim->level = URANGE( 1, atoi( arg ), MAX_LEVEL - 1 );
         olc_log( d, "Changed level to %d", victim->level );
         break;
      case MEDIT_ALIGNMENT:
         victim->alignment = URANGE( -1000, atoi( arg ), 1000 );
         olc_log( d, "Changed alignment to %d", victim->alignment );
         break;
      case MEDIT_RESISTANT:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            number -= 1;   /* offset */
            if( number < 0 || number > 23 )
            {
               send_to_char( "Invalid flag, try again: \n\r", d->character );
               return;
            }
            TOGGLE_BIT( victim->resistant, 1 << number );
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_risflag( arg1 );
               if( number < 0 || number > 23 )
               {
                  send_to_char( "Invalid flag, try again: \n\r", d->character );
                  return;
               }
               TOGGLE_BIT( victim->resistant, 1 << number );
            }
         }
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->resistant = victim->resistant;
         medit_disp_ris( d );
         olc_log( d, "%s the resistant %s", IS_SET( victim->resistant, 1 << number ) ? "Added" : "Removed",
                  ris_flags[number] );
         return;
      case MEDIT_IMMUNE:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            number -= 1;
            if( number < 0 || number > 23 )
            {
               send_to_char( "Invalid flag, try again: \n\r", d->character );
               return;
            }
            TOGGLE_BIT( victim->immune, 1 << number );
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_risflag( arg1 );
               if( number < 0 || number > 23 )
               {
                  send_to_char( "Invalid flag, try again: \n\r", d->character );
                  return;
               }
               TOGGLE_BIT( victim->immune, 1 << number );
            }
         }
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->immune = victim->immune;
         medit_disp_ris( d );
         olc_log( d, "%s the immune %s", IS_SET( victim->immune, 1 << number ) ? "Added" : "Removed", ris_flags[number] );
         return;
      case MEDIT_SUSCEPTIBLE:
         if( is_number( arg ) )
         {
            number = atoi( arg );
            if( number == 0 )
               break;
            number -= 1;
            if( number < 0 || number > 23 )
            {
               send_to_char( "Invalid flag, try again: \n\r", d->character );
               return;
            }
            TOGGLE_BIT( victim->susceptible, 1 << number );
         }
         else
         {
            while( arg[0] != STRING_NULL )
            {
               arg = one_argument( arg, arg1 );
               number = get_risflag( arg1 );
               if( number < 0 || number > 23 )
               {
                  send_to_char( "Invalid flag, try again: \n\r", d->character );
                  return;
               }
               TOGGLE_BIT( victim->susceptible, 1 << number );
            }
         }
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->susceptible = victim->susceptible;
         medit_disp_ris( d );
         olc_log( d, "%s the suscept %s", IS_SET( victim->susceptible, 1 << number ) ? "Added" : "Removed",
                  ris_flags[number] );
         return;
      case MEDIT_SPEC:
         number = atoi( arg );
         switch ( number )
         {
            case 0:
               victim->spec_fun = NULL;
               break;
            case 1:
               victim->spec_fun = spec_lookup( "spec_cast_adept" );
               break;
            case 2:
               victim->spec_fun = spec_lookup( "spec_cast_cleric" );
               break;
            case 3:
               victim->spec_fun = spec_lookup( "spec_cast_mage" );
               break;
            case 4:
               victim->spec_fun = spec_lookup( "spec_cast_undead" );
               break;
            case 5:
               victim->spec_fun = spec_lookup( "spec_executioner" );
               break;
            case 6:
               victim->spec_fun = spec_lookup( "spec_fido" );
               break;
            case 7:
               victim->spec_fun = spec_lookup( "spec_guard" );
               break;
            case 8:
               victim->spec_fun = spec_lookup( "spec_janitor" );
               break;
            case 9:
               victim->spec_fun = spec_lookup( "spec_poison" );
               break;
            case 10:
               victim->spec_fun = spec_lookup( "spec_thief" );
               break;
         }
         if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) )
            victim->pIndexData->spec_fun = victim->spec_fun;
         olc_log( d, "Changes spec_func to %s", lookup_spec( victim->spec_fun ) );
         break;
      case MEDIT_DEITY:
         number = atoi( arg );
         if( number < 0 || number > olc_top_deity + 1 )
         {
            send_to_char( "Invalid deity, try again: ", d->character );
            return;
         }
         if( number == 0 )
         {
            STRFREE( victim->pcdata->deity_name );
            victim->pcdata->deity_name = STRALLOC( "" );
            victim->pcdata->deity = NULL;
            break;
         }
         number -= 1;
         deity = get_deity( olc_deity_list[number] );
         if( !deity )
         {
            bug( "Unknown deity linked into olc_deity_list.", 0 );
            break;
         }
         STRFREE( victim->pcdata->deity_name );
         victim->pcdata->deity_name = QUICKLINK( deity->name );
         victim->pcdata->deity = deity;
         olc_log( d, "Deity changed to %s", deity->name );
         break;
      case MEDIT_CLAN:
         if( get_trust( d->character ) < LEVEL_GOD )
            break;
         number = atoi( arg );
         if( number < 0 || number > olc_top_order + 1 )
         {
            send_to_char( "Invalid choice, try again: ", d->character );
            return;
         }
         if( number == 0 )
         {
            if( !IS_IMMORTAL( victim ) )
            {
               --victim->pcdata->clan->members;
               save_clan( victim->pcdata->clan );
            }
            STRFREE( victim->pcdata->clan_name );
            victim->pcdata->clan_name = STRALLOC( "" );
            victim->pcdata->clan = NULL;
            break;
         }
         clan = get_clan( olc_clan_list[number - 1] );
         if( !clan )
         {
            bug( "Non-existant clan linked into olc_clan_list.", 0 );
            break;
         }
         if( victim->pcdata->clan != NULL && !IS_IMMORTAL( victim ) )
         {
            --victim->pcdata->clan->members;
            save_clan( victim->pcdata->clan );
         }
         STRFREE( victim->pcdata->clan_name );
         victim->pcdata->clan_name = QUICKLINK( clan->name );
         victim->pcdata->clan = clan;
         if( !IS_IMMORTAL( victim ) )
         {
            ++victim->pcdata->clan->members;
            save_clan( victim->pcdata->clan );
         }
         olc_log( d, "Clan changed to %s", clan->name );
         break;
      case MEDIT_COUNCIL:
         if( get_trust( d->character ) < LEVEL_SUB_IMPLEM )
            break;
         number = atoi( arg );
         if( number < 0 || number > olc_top_council )
         {
            send_to_char( "Invalid council, try again: ", d->character );
            return;
         }
         if( number == 0 )
         {
            STRFREE( victim->pcdata->council_name );
            victim->pcdata->council_name = STRALLOC( "" );
            victim->pcdata->council = NULL;
            break;
         }
         number -= 1;   /* Offset cause 0 is first element but 0 is None, soo */
         council = get_council( olc_council_list[number] );
         if( !council )
         {
            bug( "Unknown council linked into olc_council_list.", 0 );
            break;
         }
         STRFREE( victim->pcdata->council_name );
         victim->pcdata->council_name = QUICKLINK( council->name );
         victim->pcdata->council = council;
         olc_log( d, "Council changed to %s", council->name );
         break;
/*-------------------------------------------------------------------*/
      default:
         /*
          * . We should never get here .
          */
         bug( "OLC: medit_parse(): Reached default case!", 0 );
         cleanup_olc( d );
         return;;
   }
/*-------------------------------------------------------------------*/
/*. END OF CASE 
    If we get here, we have probably changed something, and now want to
    return to main menu.  Use OLC_CHANGE as a 'has changed' flag .*/
   OLC_CHANGE( d ) = TRUE;
   medit_disp_menu( d );
}
/*. End of medit_parse() .*/
