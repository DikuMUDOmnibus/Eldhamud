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
 *      Wizard/god command module       *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "./Headers/mud.h"
int get_langnum( char *flag );
int get_langflag( char *flag );


char *const save_flag[] = { 	"death",   "kill", "passwd",  "drop", "put",    "give",       "auto", "zap",
   				"auction", "get",  "receive", "idle", "backup", "quitbackup", "fill", "empty",
   				"r16",     "r17",  "r18",     "r19",  "r20",    "r21",        "r22",  "r23", 
                                "r24",     "r25",  "r26",     "r27",  "r28",    "r29",        "r30",  "r31"
};

/* from reset.c */
int generate_itemlevel args( ( AREA_DATA * pArea, OBJ_INDEX_DATA * pObjIndex ) );
/* from comm.c */
bool write_to_descriptor args( ( int desc, char *txt, int length ) );
/* from boards.c */
void note_attach( CHAR_DATA * ch );
/* from build.c */
int get_risflag( char *flag );
int get_defenseflag( char *flag );
int get_attackflag( char *flag );
/* from tables.c */
void write_race_file( int ra );
/*
 * Local functions.
 */
void save_watchlist args( ( void ) );
void close_area args( ( AREA_DATA * pArea ) );
int get_color( char *argument ); /* function proto */
void sort_reserved args( ( RESERVE_DATA * pRes ) );
bool double_exp = FALSE;
bool double_qp = FALSE;
bool quad_damage = FALSE;
void save_mapresets( void );
/*
 * Global variables.
 */
char reboot_time[50];
time_t new_boot_time_t;
extern OBJ_INDEX_DATA *obj_index_hash[MAX_KEY_HASH];
extern MOB_INDEX_DATA *mob_index_hash[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash[MAX_KEY_HASH];
int get_saveflag( char *name )
{
   int x;

   for( x = 0; x < sizeof( save_flag ) / sizeof( save_flag[0] ); x++ )
      if( !str_cmp( name, save_flag[x] ) )
         return x;

   return -1;
}

/*
 * Toggle "Do Not Disturb" flag. Used to prevent lower level imms from
 * using commands like "trans" and "goto" on higher level imms.
 */
void do_dnd( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) && ch->pcdata )
      if( IS_SET( ch->pcdata->flags, PCFLAG_DND ) )
      {
         REMOVE_BIT( ch->pcdata->flags, PCFLAG_DND );
         send_to_char( "Your 'do not disturb' flag is now off.\r\n", ch );
      }
      else
      {
         SET_BIT( ch->pcdata->flags, PCFLAG_DND );
         send_to_char( "Your 'do not disturb' flag is now on.\r\n", ch );
      }
   else
      send_to_char( "huh?\r\n", ch );
}

void do_wizhelp( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *cmd;
   int col, hash;
   int curr_lvl;
   col = 0;
   set_pager_color( AT_WHITE, ch );

   for( curr_lvl = LEVEL_AVATAR; curr_lvl <= get_trust( ch ); curr_lvl++ )
   {
      send_to_pager( "\r\n\r\n", ch );
      col = 1;
      send_to_pager( "&g--------------------------------------------------------------------------------&D\r\n", ch );
      pager_printf( ch, "&wLEVEL %-2d\r\n", curr_lvl );
      send_to_pager( "&g--------------------------------------------------------------------------------&D\r\n", ch );
      for( hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
            if( ( cmd->level == curr_lvl ) && cmd->level <= get_trust( ch ) )
            {
               pager_printf( ch, "&c%-13s", cmd->name );

               if( ++col % 6 == 0 )
                  send_to_pager( "\r\n", ch );
            }
   }

   if( col % 6 != 0 )
      send_to_pager( "\r\n", ch );

   send_to_pager( "&g--------------------------------------------------------------------------------&D\r\n", ch );
   return;
}

void do_restrict( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   short level, hash;
   CMDTYPE *cmd;
   bool found;
   found = FALSE;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Restrict which command?\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg2 );

   if( arg2[0] == STRING_NULL )
      level = get_trust( ch );
   else
      level = atoi( arg2 );

   level = UMAX( UMIN( get_trust( ch ), level ), 0 );

   hash = arg[0] % 126;

   for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
   {
      if( !str_prefix( arg, cmd->name ) && cmd->level <= get_trust( ch ) )
      {
         found = TRUE;
         break;
      }
   }

   if( found )
   {
      if( !str_prefix( arg2, "show" ) )
      {
         sprintf( buf, "%s show", cmd->name );
         do_cedit( ch, buf );
         /*
          * ch_printf( ch, "%s is at level %d.\r\n", cmd->name, cmd->level );
          */
         return;
      }

      cmd->level = level;

      ch_printf( ch, "You restrict %s to level %d\r\n", cmd->name, level );
      sprintf( buf, "%s restricting %s to level %d", ch->name, cmd->name, level );
      log_string( buf );
   }
   else
      send_to_char( "You may not restrict that command.\r\n", ch );

   return;
}

/*
 * Check if the name prefix uniquely identifies a char descriptor
 */
CHAR_DATA *get_waiting_desc( CHAR_DATA * ch, char *name )
{
   DESCRIPTOR_DATA *d;
   CHAR_DATA *ret_char = NULL;
   static unsigned int number_of_hits;
   number_of_hits = 0;

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->character && ( !str_prefix( name, d->character->name ) ) )
      {
         if( ++number_of_hits > 1 )
         {
            ch_printf( ch, "%s does not uniquely identify a char.\r\n", name );
            return NULL;
         }

         ret_char = d->character;   /* return current char on exit */
      }
   }

   if( number_of_hits == 1 )
      return ret_char;
   else
   {
      send_to_char( "No one like that waiting for authorization.\r\n", ch );
      return NULL;
   }
}


void do_bamfin( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfin );
      ch->pcdata->bamfin = str_dup( argument );
      send_to_char_color( "&YBamfin set.\r\n", ch );
   }

   return;
}

void do_bamfout( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) )
   {
      smash_tilde( argument );
      DISPOSE( ch->pcdata->bamfout );
      ch->pcdata->bamfout = str_dup( argument );
      send_to_char_color( "&YBamfout set.\r\n", ch );
   }

   return;
}

void do_rank( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;

   if( !argument || argument[0] == STRING_NULL )
   {
      send_to_char( "Usage:  rank <string>.\r\n", ch );
      send_to_char( "   or:  rank none.\r\n", ch );
      return;
   }

   smash_tilde( argument );

   DISPOSE( ch->pcdata->rank );

   if( !str_cmp( argument, "none" ) )
      ch->pcdata->rank = str_dup( "" );
   else
      ch->pcdata->rank = str_dup( argument );

   send_to_char( "Ok.\r\n", ch );

   return;
}

void do_retire( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Retire whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   if( victim->level < LEVEL_SAVIOR )
   {
      send_to_char( "The minimum level for retirement is savior.\r\n", ch );
      return;
   }

   if( IS_RETIRED( victim ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s returns from retirement.\r\n", victim->name );
      ch_printf( victim, "%s brings you back from retirement.\r\n", ch->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_RETIRED );
      ch_printf( ch, "%s is now a retired immortal.\r\n", victim->name );
      ch_printf( victim, "Courtesy of %s, you are now a retired immortal.\r\n", ch->name );
   }

   return;
}

void do_delay( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   int delay;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "Syntax:  delay <victim> <# of rounds>\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "No such character online.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Mobiles are unaffected by lag.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "For how long do you wish to delay them?\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "none" ) )
   {
      send_to_char( "All character delay removed.\r\n", ch );
      victim->wait = 0;
      return;
   }

   delay = atoi( arg );

   if( delay < 1 )
   {
      send_to_char( "Pointless.  Try a positive number.\r\n", ch );
      return;
   }

   if( delay > 999 )
   {
      send_to_char( "You cruel bastard.  Just kill them.\r\n", ch );
      return;
   }

   WAIT_STATE( victim, delay * PULSE_VIOLENCE );

   ch_printf( ch, "You've delayed %s for %d rounds.\r\n", victim->name, delay );
   return;
}

void do_deny( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Deny whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   xSET_BIT( victim->act, PLR_DENY );

   set_char_color( AT_IMMORT, victim );
   send_to_char( "You are denied access!\r\n", victim );
   ch_printf( ch, "You have denied access to %s.\r\n", victim->name );

   if( victim->fighting )
      stop_fighting( victim, TRUE );   /* Blodkai, 97 */

   do_quit( victim, "" );

   return;
}

void do_disconnect( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Disconnect whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim->desc == NULL )
   {
      act( AT_PLAIN, "$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) )
   {
      send_to_char( "They might not like that...\r\n", ch );
      return;
   }

   for( d = first_descriptor; d; d = d->next )
   {
      if( d == victim->desc )
      {
         close_socket( d, FALSE );
         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }

   bug( "Do_disconnect: *** desc not found ***.", 0 );

   send_to_char( "Descriptor not found!\r\n", ch );
   return;
}

/*
 * Force a level one player to quit.             Gorog
 */
void do_fquit( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Force whom to quit?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim->level != 1 )
   {
      send_to_char( "They are not level one!\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   send_to_char( "The MUD administrators force you to quit...\r\n", victim );

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   do_quit( victim, "" );

   ch_printf( ch, "You have forced %s to quit.\r\n", victim->name );

   return;
}

void do_forceclose( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   int desc;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Usage: forceclose <descriptor#>\r\n", ch );
      return;
   }

   desc = atoi( arg );

   for( d = first_descriptor; d; d = d->next )
   {
      if( d->descriptor == desc )
      {
         if( d->character && get_trust( d->character ) >= get_trust( ch ) )
         {
            send_to_char( "They might not like that...\r\n", ch );
            return;
         }

         close_socket( d, FALSE );

         send_to_char( "Ok.\r\n", ch );
         return;
      }
   }

   send_to_char( "Not found!\r\n", ch );

   return;
}

void do_pardon( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Syntax: pardon <character> <killer|thief|attacker>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "attacker" ) )
   {
      if( xIS_SET( victim->act, PLR_ATTACKER ) )
      {
         xREMOVE_BIT( victim->act, PLR_ATTACKER );
         ch_printf( ch, "Attacker flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer an ATTACKER.\r\n", victim );
      }

      return;
   }

   if( !str_cmp( arg2, "killer" ) )
   {
      if( xIS_SET( victim->act, PLR_KILLER ) )
      {
         xREMOVE_BIT( victim->act, PLR_KILLER );
         ch_printf( ch, "Killer flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer a KILLER.\r\n", victim );
      }

      return;
   }

   if( !str_cmp( arg2, "thief" ) )
   {
      if( xIS_SET( victim->act, PLR_THIEF ) )
      {
         xREMOVE_BIT( victim->act, PLR_THIEF );
         ch_printf( ch, "Thief flag removed from %s.\r\n", victim->name );
         set_char_color( AT_IMMORT, victim );
         send_to_char( "You are no longer a THIEF.\r\n", victim );
      }

      return;
   }

   send_to_char( "Syntax: pardon <character> <killer|thief>.\r\n", ch );

   return;
}

void echo_to_all( short AT_COLOR, char *argument, short tar )
{
   DESCRIPTOR_DATA *d;

   if( !argument || argument[0] == STRING_NULL )
      return;

   for( d = first_descriptor; d; d = d->next )
   {
      /*
       * Added showing echoes to players who are editing, so they won't
       * miss out on important info like upcoming reboots. --Narn
       */
      if( d->connected == CON_PLAYING || d->connected == CON_EDITING )
      {
         /*
          * This one is kinda useless except for switched..
          */
         if( tar == ECHOTAR_PC && IS_NPC( d->character ) )
            continue;
         else if( tar == ECHOTAR_IMM && !IS_IMMORTAL( d->character ) )
            continue;

         set_char_color( AT_COLOR, d->character );

         send_to_char( argument, d->character );

         send_to_char( "\r\n", d->character );
      }
   }

   return;
}

void do_echo( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;
   int target;
   char *parg;
   set_char_color( AT_IMMORT, ch );

   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\r\n", ch );
      return;
   }

   if( argument[0] == STRING_NULL )
   {
      send_to_char( "Echo what?\r\n", ch );
      return;
   }

   if( ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );

   parg = argument;

   argument = one_argument( argument, arg );

   if( !str_cmp( arg, "PC" ) || !str_cmp( arg, "player" ) )
      target = ECHOTAR_PC;
   else if( !str_cmp( arg, "imm" ) )
      target = ECHOTAR_IMM;
   else
   {
      target = ECHOTAR_ALL;
      argument = parg;
   }

   if( !color && ( color = get_color( argument ) ) )
      argument = one_argument( argument, arg );

   if( !color )
      color = AT_IMMORT;

   one_argument( argument, arg );

   echo_to_all( color, argument, target );
}

void echo_to_room( short AT_COLOR, ROOM_INDEX_DATA * room, char *argument )
{
   CHAR_DATA *vic;

   for( vic = room->first_person; vic; vic = vic->next_in_room )
   {
      set_char_color( AT_COLOR, vic );
      send_to_char( argument, vic );
      send_to_char( "\r\n", vic );
   }
}

void do_recho( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short color;
   set_char_color( AT_IMMORT, ch );

   if( xIS_SET( ch->act, PLR_NO_EMOTE ) )
   {
      send_to_char( "You can't do that right now.\r\n", ch );
      return;
   }

   if( argument[0] == STRING_NULL )
   {
      send_to_char( "Recho what?\r\n", ch );
      return;
   }

   one_argument( argument, arg );

   if( ( color = get_color( argument ) ) )
   {
      argument = one_argument( argument, arg );
      echo_to_room( color, ch->in_room, argument );
   }
   else
      echo_to_room( AT_IMMORT, ch->in_room, argument );
}

ROOM_INDEX_DATA *find_location( CHAR_DATA * ch, char *arg )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj;

   if( is_number( arg ) )
      return get_room_index( atoi( arg ) );

   if( !str_cmp( arg, "pk" ) )   /* "Goto pk", "at pk", etc */
      return get_room_index( last_pkroom );

   if( ( victim = get_char_world( ch, arg ) ) != NULL )
      return victim->in_room;

   if( ( obj = get_obj_world( ch, arg ) ) != NULL )
      return obj->in_room;

   return NULL;
}

/* This function shared by do_transfer and do_mptransfer
 *
 * Immortals bypass most restrictions on where to transfer victims.
 * NPCs cannot transfer victims who are:
 * 1. Not authorized yet.
 * 2. Outside of the level range for the target room's area.
 * 3. Being sent to private rooms.
 */
void transfer_char( CHAR_DATA * ch, CHAR_DATA * victim, ROOM_INDEX_DATA * location )
{
   if( !victim->in_room )
   {
      bug( "%s: victim in NULL room: %s", __FUNCTION__, victim->name );
      return;
   }

   if( IS_NPC( ch ) && room_is_private( location ) )
   {
      progbug( "Mptransfer - Private room", ch );
      return;
   }

   if( !can_see( ch, victim, FALSE ) )
      return;

   if( IS_NPC( ch ) && location->area != victim->in_room->area )
   {
      char buf[MAX_STRING_LENGTH];
      snprintf( buf, MAX_STRING_LENGTH, "Mptransfer - unauthed char (%s)", victim->name );
      progbug( buf, ch );
      return;
   }

   /*
    * If victim not in area's level range, do not transfer
    */
   if( IS_NPC( ch ) && !in_hard_range( victim, location->area ) && !xIS_SET( location->room_flags, ROOM_PROTOTYPE ) )
      return;

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n disappears in a cloud of swirling colors.", victim, NULL, NULL, TO_ROOM );
      victim->retran = victim->in_room->vnum;
   }

   char_from_room( victim );

   char_to_room( victim, location );

   if( !IS_NPC( ch ) )
   {
      act( AT_MAGIC, "$n arrives from a puff of smoke.", victim, NULL, NULL, TO_ROOM );

      if( ch != victim )
         act( AT_IMMORT, "$n has transferred you.", ch, NULL, victim, TO_VICT );

      do_look( victim, "auto" );

      if( !IS_IMMORTAL( victim ) && !IS_NPC( victim ) && !in_hard_range( victim, location->area ) )
         act( AT_DANGER, "Warning:  this player's level is not within the area's level range.", ch, NULL, NULL, TO_CHAR );
   }
}

void do_transfer( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Transfer whom (and where)?\r\n", ch );
      return;
   }

   if( arg2[0] != STRING_NULL )
   {
      if( !( location = find_location( ch, arg2 ) ) )
      {
         send_to_char( "That location does not exist.\r\n", ch );
         return;
      }
   }
   else
      location = ch->in_room;

   if( !str_cmp( arg1, "all" ) && get_trust( ch ) >= LEVEL_GREATER )
   {
      for( d = first_descriptor; d; d = d->next )
      {
         if( d->connected == CON_PLAYING && d->character && d->character != ch && d->character->in_room )
            transfer_char( ch, d->character, location );
      }

      return;
   }

   if( !( victim = get_char_world( ch, arg1 ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   /*
    * modification to prevent a low level imm from transferring a
    * higher level imm with the DND flag on.  - Gorog
    */
   if( !IS_NPC( victim ) && get_trust( ch ) < get_trust( victim )
       && victim->desc && ( victim->desc->connected == CON_PLAYING || victim->desc->connected == CON_EDITING ) && IS_SET( victim->pcdata->flags, PCFLAG_DND ) )
   {
      pager_printf( ch, "Sorry. %s does not wish to be disturbed.\r\n", victim->name );
      pager_printf( victim, "Your DND flag just foiled %s's transfer command.\r\n", ch->name );
      return;
   }

   /*
    * end of modification
    */
   transfer_char( ch, victim, location );
}

void do_retran( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Retransfer whom?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   sprintf( buf, "'%s' %d", victim->name, victim->retran );

   do_transfer( ch, buf );
   return;
}

void do_regoto( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   sprintf( buf, "%d", ch->regoto );
   do_goto( ch, buf );
   return;
}

/*  Added do_at and do_atobj to reduce lag associated with at
 *  --Shaddai
 */
void do_at( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   CHAR_DATA *wch;
   OBJ_DATA *obj;
   short origmap, origx, origy;
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
   {
      send_to_char( "At where what?\r\n", ch );
      return;
   }

   if( !is_number( arg ) )
   {
      if( ( wch = get_char_world( ch, arg ) ) != NULL && wch->in_room != NULL )
      {
         atmob( ch, wch, argument );
         return;
      }

      if( ( obj = get_obj_world( ch, arg ) ) != NULL && obj->in_room != NULL )
      {
         atobj( ch, obj, argument );
         return;
      }

      send_to_char( "No such mob or object.\r\n", ch );

      return;
   }

   if( ( location = find_location( ch, arg ) ) == NULL )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
      {
         send_to_char( "Overriding private flag!\r\n", ch );
      }
   }

   origmap = ch->map;

   origx = ch->x;
   origy = ch->y;
   /*
    * Since we're this far down, it's a given that the location isn't on a map since
    * a vnum had to be specified to get here. Therefore you want to be off map, and
    * at coords of -1, -1 to avoid problems - Samson
    */
   REMOVE_PLR_FLAG( ch, PLR_ONMAP );
   ch->map = -1;
   ch->x = -1;
   ch->y = -1;
   original = ch->in_room;
   char_from_room( ch );
   char_to_room( ch, location );
   interpret( ch, argument );
   /*
    * And even if you weren't on a map to begin with, this will still work fine
    */

   if( IS_PLR_FLAG( ch, PLR_ONMAP ) && xIS_ROOM_FLAG( original, ROOM_MAP ) )
      REMOVE_PLR_FLAG( ch, PLR_ONMAP );
   else if( !IS_PLR_FLAG( ch, PLR_ONMAP ) && xIS_ROOM_FLAG( original, ROOM_MAP ) )
      SET_PLR_FLAG( ch, PLR_ONMAP );

   ch->map = origmap;

   ch->x = origx;

   ch->y = origy;

   /*
    * See if 'ch' still exists before continuing!
    * Handles 'at XXXX quit' case.
    */
   for( wch = first_char; wch; wch = wch->next )
   {
      if( wch == ch )
      {
         char_from_room( ch );
         char_to_room( ch, original );
         break;
      }
   }

   return;
}

void do_rat( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   ROOM_INDEX_DATA *original;
   int Start, End, vnum;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL || argument[0] == STRING_NULL )
   {
      send_to_char( "Syntax: rat <start> <end> <command>\r\n", ch );
      return;
   }

   Start = atoi( arg1 );

   End = atoi( arg2 );

   if( Start < 1 || End < Start || Start > End || Start == End || End > MAX_VNUM )
   {
      send_to_char( "Invalid range.\r\n", ch );
      return;
   }

   if( !str_cmp( argument, "quit" ) )
   {
      send_to_char( "I don't think so!\r\n", ch );
      return;
   }

   original = ch->in_room;

   for( vnum = Start; vnum <= End; vnum++ )
   {
      if( ( location = get_room_index( vnum ) ) == NULL )
         continue;

      char_from_room( ch );

      char_to_room( ch, location );

      interpret( ch, argument );
   }

   char_from_room( ch );

   char_to_room( ch, original );
   send_to_char( "Done.\r\n", ch );
   return;
}

void do_rstat( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *sect;
   ROOM_INDEX_DATA *location;
   OBJ_DATA *obj;
   CHAR_DATA *rch;
   EXIT_DATA *pexit;
   AFFECT_DATA *paf;
   int cnt;
   static char *dir_text[] = { "n", "e", "s", "w", "u", "d", "ne", "nw", "se", "sw", "?" };
   one_argument( argument, arg );

   if( !str_cmp( arg, "ex" ) || !str_cmp( arg, "exits" ) )
   {
      location = ch->in_room;
      ch_printf_color( ch, "&cExits for room '&W%s&c'  Vnum &W%d\r\n", location->name, location->vnum );

      for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
         ch_printf_color( ch,
                          "&W%2d) &w%2s to %-5d  &cKey: &w%d  &cFlags: &w%d  &cKeywords: '&w%s&c'\r\n     Exdesc: &w%s     &cBack link: &w%d  &cVnum: &w%d  &cDistance: &w%d  &cPulltype: &w%s  &cPull: &w%d\r\n",
                          ++cnt,
                          dir_text[pexit->vdir],
                          pexit->to_room ? pexit->to_room->vnum : 0,
                          pexit->key,
                          pexit->exit_info,
                          pexit->keyword,
                          pexit->description[0] != STRING_NULL
                          ? pexit->description : "(none).\r\n",
                          pexit->rexit ? pexit->rexit->vnum : 0, pexit->rvnum, pexit->distance, pull_type_name( pexit->pulltype ), pexit->pull );

      return;
   }

   location = ( arg[0] == STRING_NULL ) ? ch->in_room : find_location( ch, arg );

   if( !location )
   {
      send_to_char( "No such location.\r\n", ch );
      return;
   }

   if( ch->in_room != location && room_is_private( location ) )
   {
      if( get_trust( ch ) < LEVEL_GREATER )
      {
         send_to_char( "That room is private right now.\r\n", ch );
         return;
      }
      else
         send_to_char( "Overriding private flag!\r\n", ch );
   }

   ch_printf_color( ch, "&cName: &w%s\r\n&cArea: &w%s  &cFilename: &w%s\r\n",
                    location->name, location->area ? location->area->name : "None????", location->area ? location->area->filename : "None????" );

   switch ( ch->in_room->sector_type )
   {

      default:
         sect = "?!";
         break;

      case SECT_INSIDE:
         sect = "Inside";
         break;

      case SECT_CITY:
         sect = "City";
         break;

      case SECT_FIELD:
         sect = "Field";
         break;

      case SECT_FOREST:
         sect = "Forest";
         break;

      case SECT_HILLS:
         sect = "Hills";
         break;

      case SECT_MOUNTAIN:
         sect = "Mountains";
         break;

      case SECT_WATER_SWIM:
         sect = "Swim";
         break;

      case SECT_WATER_NOSWIM:
         sect = "Noswim";
         break;

      case SECT_UNDERWATER:
         sect = "Underwater";
         break;

      case SECT_AIR:
         sect = "Air";
         break;

      case SECT_DESERT:
         sect = "Desert";
         break;

      case SECT_OCEANFLOOR:
         sect = "Oceanfloor";
         break;

      case SECT_UNDERGROUND:
         sect = "Underground";
         break;

      case SECT_LAVA:
         sect = "Lava";
         break;

      case SECT_SWAMP:
         sect = "Swamp";
         break;
   }

   ch_printf_color( ch, "&cVnum: &w%d   &cSector: &w%d (%s)   &cLight: &w%d", location->vnum, location->sector_type, sect, location->light );

   if( location->tunnel > 0 )
      ch_printf_color( ch, "   &cTunnel: &W%d", location->tunnel );

   send_to_char( "\r\n", ch );

   if( location->tele_delay > 0 || location->tele_vnum > 0 )
      ch_printf_color( ch, "&cTeleDelay: &R%d   &cTeleVnum: &R%d\r\n", location->tele_delay, location->tele_vnum );

   ch_printf_color( ch, "&cRoom flags: &w%s\r\n", ext_flag_string( &location->room_flags, r_flags ) );

   ch_printf_color( ch, "&cDescription:\r\n&w%s", location->description );

   if( location->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char_color( "&cExtra description keywords: &w'", ch );

      for( ed = location->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );

         if( ed->next )
            send_to_char( " ", ch );
      }

      send_to_char( "'\r\n", ch );
   }

   for( paf = location->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffect: &w%s &cby &w%d.\r\n", affect_loc_name( paf->location ), paf->modifier );

   send_to_char_color( "&cCharacters: &w", ch );

   for( rch = location->first_person; rch; rch = rch->next_in_room )
   {
      if( can_see( ch, rch, FALSE ) )
      {
         send_to_char( " ", ch );
         one_argument( rch->name, buf );
         send_to_char( buf, ch );
      }
   }

   send_to_char_color( "\r\n&cObjects:    &w", ch );

   for( obj = location->first_content; obj; obj = obj->next_content )
   {
      send_to_char( " ", ch );
      one_argument( obj->name, buf );
      send_to_char( buf, ch );
   }

   send_to_char( "\r\n", ch );

   if( location->first_exit )
      send_to_char_color( "&c------------------- &wEXITS &c-------------------\r\n", ch );

   for( cnt = 0, pexit = location->first_exit; pexit; pexit = pexit->next )
      ch_printf( ch,
                 "%2d) %-2s to %-5d.  Key: %d  Flags: %d  Keywords: %s.\r\n",
                 ++cnt,
                 dir_text[pexit->vdir],
                 pexit->to_room ? pexit->to_room->vnum : 0, pexit->key, pexit->exit_info, pexit->keyword[0] != STRING_NULL ? pexit->keyword : "(none)" );

   return;
}

/* Face-lift by Demora */
void do_ostat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   AFFECT_DATA *paf;
   OBJ_DATA *obj;
   set_char_color( AT_CYAN, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Ostat what?\r\n", ch );
      return;
   }

   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( obj = get_obj_world( ch, arg ) ) == NULL )
   {
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );
      return;
   }

   ch_printf_color( ch, "&cName: &C%s\r\n", obj->name );

   ch_printf_color( ch, "&cVnum: &w%d  ", obj->pIndexData->vnum );
   ch_printf_color( ch, "&cType: &w%s  ", item_type_name( obj ) );
   ch_printf_color( ch, "&cCount:  &w%d  ", obj->pIndexData->count );
   ch_printf_color( ch, "&cGcount: &w%d\r\n", obj->count );
   ch_printf_color( ch, "&cSerial#: &w%d  ", obj->serial );
   ch_printf_color( ch, "&cTopIdxSerial#: &w%d  ", obj->pIndexData->serial );
   ch_printf_color( ch, "&cTopSerial#: &w%d\r\n", cur_obj_serial );
   ch_printf_color( ch, "&cShort description: &C%s\r\n", obj->short_descr );
   ch_printf_color( ch, "&cLong description : &C%s\r\n", obj->description );

   if( obj->action_desc[0] != STRING_NULL )
      ch_printf_color( ch, "&cAction description: &w%s\r\n", obj->action_desc );

   ch_printf_color( ch, "&cWear flags : &w%s\r\n", flag_string( obj->wear_flags, w_flags ) );

   ch_printf_color( ch, "&cExtra flags: &w%s\r\n", ext_flag_string( &obj->extra_flags, o_flags ) );

   ch_printf_color( ch, "&cMagic flags: &w%s\r\n", magic_bit_name( obj->magic_flags ) );

   ch_printf_color( ch, "&cNumber: &w%d/%d   ", 1, get_obj_number( obj ) );

   ch_printf_color( ch, "&cWeight: &w%d/%d   ", obj->weight, get_obj_weight( obj ) );

   ch_printf_color( ch, "&cLayers: &w%d   ", obj->pIndexData->layers );

   ch_printf_color( ch, "&cWear_loc: &w%d\r\n", obj->wear_loc );

   ch_printf_color( ch, "&cCost: &Y%d  ", obj->cost );

   ch_printf_color( ch, "&cRent: &w%d  ", obj->pIndexData->rent );

   ch_printf_color( ch, "\r\n&cOn map: &w%s ", IS_OBJ_STAT( obj, ITEM_ONMAP ) ? map_names[obj->map] : "(none)" );

   ch_printf_color( ch, "&cCoords: &w%d %d\r\n", obj->x, obj->y );

   send_to_char_color( "&cTimer: ", ch );

   if( obj->timer > 0 )
      ch_printf_color( ch, "&R%d  ", obj->timer );
   else
      ch_printf_color( ch, "&w%d  ", obj->timer );

   ch_printf_color( ch, "&cLevel: &P%d\r\n", obj->level );

   ch_printf_color( ch, "&cIn room: &w%d  ", obj->in_room == NULL ? 0 : obj->in_room->vnum );

   ch_printf_color( ch, "&cIn object: &w%s  ", obj->in_obj == NULL ? "(none)" : obj->in_obj->short_descr );

   ch_printf_color( ch, "&cCarried by: &C%s\r\n", obj->carried_by == NULL ? "(none)" : obj->carried_by->name );

   ch_printf_color( ch, "&cIndex Values : &w%d %d %d %d %d %d.\r\n",
                    obj->pIndexData->value[0], obj->pIndexData->value[1],
                    obj->pIndexData->value[2], obj->pIndexData->value[3], obj->pIndexData->value[4], obj->pIndexData->value[5] );

   ch_printf_color( ch, "&cObject Values: &w%d %d %d %d %d %d.\r\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );

   if( obj->pIndexData->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Primary description keywords:   '", ch );

      for( ed = obj->pIndexData->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );

         if( ed->next )
            send_to_char( " ", ch );
      }

      send_to_char( "'.\r\n", ch );
   }

   if( obj->first_extradesc )
   {
      EXTRA_DESCR_DATA *ed;
      send_to_char( "Secondary description keywords: '", ch );

      for( ed = obj->first_extradesc; ed; ed = ed->next )
      {
         send_to_char( ed->keyword, ch );

         if( ed->next )
            send_to_char( " ", ch );
      }

      send_to_char( "'.\r\n", ch );
   }

   for( paf = obj->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d. (extra)\r\n", affect_loc_name( paf->location ), paf->modifier );

   for( paf = obj->pIndexData->first_affect; paf; paf = paf->next )
      ch_printf_color( ch, "&cAffects &w%s &cby &w%d.\r\n", affect_loc_name( paf->location ), paf->modifier );

   return;
}

void do_mstat( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char hpbuf[MAX_STRING_LENGTH];
   char mnbuf[MAX_STRING_LENGTH];
   char mvbuf[MAX_STRING_LENGTH];
   AFFECT_DATA *paf;
   CHAR_DATA *victim;
   SKILLTYPE *skill;
   int x;
   set_pager_color( AT_CYAN, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_pager( "Mstat whom?\r\n", ch );
      return;
   }

   if( arg[0] != '\'' && arg[0] != '"' && strlen( argument ) > strlen( arg ) )
      strcpy( arg, argument );

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_pager( "They aren't here.\r\n", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) && !IS_NPC( victim ) )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) && get_trust( ch ) < LEVEL_GREATER && xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }

   pager_printf_color( ch, "\r\n&c%s: &C%-20s", IS_NPC( victim ) ? "Mobile name" : "Name", victim->name );

   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cStatus : &w%-10s", CAN_PKILL( victim ) ? "Deadly" : IS_PKILL( victim ) ? "Pre-Deadly" : "Non-Deadly" );

   if( !IS_NPC( victim ) && victim->pcdata->clan )
      pager_printf_color( ch, "   &cClan: &w%s", victim->pcdata->clan->name );

   send_to_pager( "\r\n", ch );

   if( get_trust( ch ) >= LEVEL_GOD && !IS_NPC( victim ) && victim->desc )
      pager_printf_color( ch, "&cHost: &w%s   Descriptor: %d  &cTrust: &w%d  &cAuthBy: &w%s\r\n",
                          victim->desc->host, victim->desc->descriptor, victim->trust, victim->pcdata->authed_by[0] != STRING_NULL ? victim->pcdata->authed_by : "(unknown)" );

   if( !IS_NPC( victim ) && victim->pcdata->release_date != 0 )
      pager_printf_color( ch, "&cHelled until %24.24s by %s.\r\n", ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );

   pager_printf_color( ch, "&cVnum: &w%-5d    &cSex: &w%-6s    &cRoom: &w%-5d    &cCount: &w%d   &cKilled: &w%d\r\n",
                       IS_NPC( victim ) ? victim->pIndexData->vnum : 0,
                       victim->sex == SEX_MALE ? "male" :
                       victim->sex == SEX_FEMALE ? "female" : "neutral",
                       victim->in_room == NULL ? 0 : victim->in_room->vnum,
                       IS_NPC( victim ) ? victim->pIndexData->count : 1, IS_NPC( victim ) ? victim->pIndexData->killed : victim->pcdata->mdeaths + victim->pcdata->pdeaths );

   pager_printf_color( ch,
                       "&cStr: &C%2d&c )( Int: &C%2d&c )( Wis: &C%2d&c )( Dex: &C%2d&c )( Con: &C%2d&c )( Cha: &C%2d&c )( Lck: &C%2d&c\r\n",
                       get_curr_str( victim ), get_curr_int( victim ), get_curr_wis( victim ), get_curr_dex( victim ),
                       get_curr_con( victim ), get_curr_cha( victim ), get_curr_lck( victim ) );

   pager_printf_color( ch, "&cLevel   : &P%-2d              ", victim->level );

   pager_printf_color( ch, "&cClass  : &w%-2.2d/%-10s   &cRace      : &w%-2.2d/%-10s\r\n",
                       victim->Class,
                       IS_NPC( victim ) ? victim->Class < MAX_NPC_CLASS && victim->Class >= 0 ?
                       npc_Class[victim->Class] : "unknown" : victim->Class < MAX_PC_CLASS &&
                       Class_table[victim->Class]->who_name &&
                       Class_table[victim->Class]->who_name[0] != STRING_NULL ?
                       Class_table[victim->Class]->who_name : "unknown",
                       victim->race,
                       IS_NPC( victim ) ? victim->race < MAX_NPC_RACE && victim->race >= 0 ?
                       npc_race[victim->race] : "unknown" : victim->race < MAX_PC_RACE &&
                       race_table[victim->race]->race_name && race_table[victim->race]->race_name[0] != STRING_NULL ? race_table[victim->race]->race_name : "unknown" );

   sprintf( hpbuf, "%d/%d", victim->hit, victim->max_hit );

   sprintf( mnbuf, "%d/%d", victim->mana, victim->max_mana );

   sprintf( mvbuf, "%d/%d", victim->move, victim->max_move );

   if( !IS_NPC( victim ) )
      pager_printf_color( ch, "&cHps     : &w%-12s    &cMana   : &w%-12s    &cMove      : &w%-12s\r\n", hpbuf, mnbuf, mvbuf );

   pager_printf_color( ch, "&cHitroll : &C%-5d           &cAlign  : &w%-5d           &cArmorClass: &w%d\r\n", GET_HITROLL( victim ), victim->alignment, GET_AC( victim ) );

   pager_printf_color( ch, "&cDamroll : &C%-5d           &cWimpy  : &w%-5d           &cPosition  : &w%d\r\n", GET_DAMROLL( victim ), victim->wimpy, victim->position );

   pager_printf_color( ch, "&cFighting: &w%-13s   &cMaster : &w%-13s   &cLeader    : &w%s\r\n",
                       victim->fighting ? victim->fighting->who->name : "(none)",
                       victim->master ? victim->master->name : "(none)", victim->leader ? victim->leader->name : "(none)" );

   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cHating  : &w%-13s   &cHunting: &w%-13s   &cFearing   : &w%s\r\n",
                          victim->hating ? victim->hating->name : "(none)",
                          victim->hunting ? victim->hunting->name : "(none)", victim->fearing ? victim->fearing->name : "(none)" );
   else
      pager_printf_color( ch, "&cDeity   : &w%-13s&w   &cFavor  : &w%-5d           &cGlory     : &w%-d (%d)\r\n",
                          victim->pcdata->deity ? victim->pcdata->deity->name : "(none)", victim->pcdata->favor, victim->pcdata->quest_curr, victim->pcdata->quest_accum );

   if( IS_NPC( victim ) )
      pager_printf_color( ch,
                          "&cMob hitdie : &C%dd%d+%d    &cMob damdie : &C%dd%d+%3d    &cIndex damdie : &C%dd%d+%3d\r\n&cNumAttacks : &C%d\r\n",
                          victim->pIndexData->hitnodice, victim->pIndexData->hitsizedice, victim->pIndexData->hitplus,
                          victim->barenumdie, victim->baresizedie, victim->damplus, victim->pIndexData->damnodice,
                          victim->pIndexData->damsizedice, victim->pIndexData->damplus, victim->numattacks );

   send_to_pager( "\r\n", ch );

   pager_printf_color( ch, "&cSave versus:- &wPoison-%d Mental-%d Physical-%d Weapons-%d\r\n",
                       	victim->saving_poison_death, victim->saving_mental, victim->saving_physical, victim->saving_weapons); 

   pager_printf_color( ch, "&cItems: &w(%d/%d)  &cWeight &w(%d/%d)\r\n", victim->carry_number, can_carry_n( victim ), victim->carry_weight, can_carry_w( victim ) );

   pager_printf_color( ch, "&cYear: &w%-5d  &cSecs: &w%d  &cTimer: &w%d  &cGold: &Y%d\r\n", get_age( victim ), ( int )victim->played, victim->timer, victim->gold );

   if( get_timer( victim, TIMER_PKILLED ) )
      pager_printf_color( ch, "&cTimerPkilled:  &R%d\r\n", get_timer( victim, TIMER_PKILLED ) );

   if( get_timer( victim, TIMER_RECENTFIGHT ) )
      pager_printf_color( ch, "&cTimerRecentfight:  &R%d\r\n", get_timer( victim, TIMER_RECENTFIGHT ) );

   if( get_timer( victim, TIMER_ASUPRESSED ) )
      pager_printf_color( ch, "&cTimerAsupressed:  &R%d\r\n", get_timer( victim, TIMER_ASUPRESSED ) );

   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cAct Flags  : &w%s\r\n", ext_flag_string( &victim->act, act_flags ) );
   else
   {
      pager_printf_color( ch, "&cPlayerFlags: &w%s\r\n", ext_flag_string( &victim->act, plr_flags ) );
      pager_printf_color( ch, "&cPcflags    : &w%s\r\n", flag_string( victim->pcdata->flags, pc_flags ) );
      pager_printf_color( ch, "&RMap   : &c%s &w &cCoords: &w%d %d\r\n", IS_PLR_FLAG( victim, PLR_ONMAP ) ? map_names[victim->map] : "none", victim->x, victim->y );
   }

   pager_printf_color( ch, "&cAffected by: &C%s\r\n", affect_bit_name( &victim->affected_by ) );

   pager_printf_color( ch, "&cSpeaks: &w%d   &cSpeaking: &w%d   &cExperience: &w%d", victim->speaks, victim->speaking, victim->exp );

   if( !IS_NPC( victim ) && victim->wait )
      pager_printf_color( ch, "   &cWaitState: &R%d\r\n", victim->wait / 12 );
   else
      send_to_pager( "\r\n", ch );

   send_to_pager_color( "&cLanguages  : &w", ch );

   for( x = 0; lang_array[x] != LANG_UNKNOWN; x++ )
      if( knows_language( victim, lang_array[x], victim ) || ( IS_NPC( victim ) && victim->speaks == 0 ) )
      {
         if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
            set_pager_color( AT_RED, ch );

         send_to_pager( lang_names[x], ch );

         send_to_pager( " ", ch );

         set_pager_color( AT_PLAIN, ch );
      }
      else if( IS_SET( lang_array[x], victim->speaking ) || ( IS_NPC( victim ) && !victim->speaking ) )
      {
         set_pager_color( AT_PINK, ch );
         send_to_pager( lang_names[x], ch );
         send_to_pager( " ", ch );
         set_pager_color( AT_PLAIN, ch );
      }

   send_to_pager( "\r\n", ch );

   if( victim->pcdata && victim->pcdata->bestowments && victim->pcdata->bestowments[0] != STRING_NULL )
      pager_printf_color( ch, "&cBestowments: &w%s\r\n", victim->pcdata->bestowments );

   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cShortdesc  : &w%s\r\n&cLongdesc   : &w%s",
                          victim->short_descr[0] != STRING_NULL ? victim->short_descr : "(none set)",
                          victim->long_descr[0] != STRING_NULL ? victim->long_descr : "(none set)\r\n" );
   else
   {
      if( victim->short_descr[0] != STRING_NULL )
         pager_printf_color( ch, "&cShortdesc  : &w%s\r\n", victim->short_descr );

      if( victim->long_descr[0] != STRING_NULL )
         pager_printf_color( ch, "&cLongdesc   : &w%s\r\n", victim->long_descr );
   }

   if( IS_NPC( victim ) && victim->spec_fun )
      pager_printf_color( ch, "&cMobile has spec fun: &w%s\r\n", lookup_spec( victim->spec_fun ) );

   if( IS_NPC( victim ) )
      pager_printf_color( ch, "&cBody Parts : &w%s\r\n", flag_string( victim->xflags, part_flags ) );

   if( victim->resistant > 0 )
      pager_printf_color( ch, "&cResistant  : &w%s\r\n", flag_string( victim->resistant, ris_flags ) );

   if( victim->immune > 0 )
      pager_printf_color( ch, "&cImmune     : &w%s\r\n", flag_string( victim->immune, ris_flags ) );

   if( victim->susceptible > 0 )
      pager_printf_color( ch, "&cSusceptible: &w%s\r\n", flag_string( victim->susceptible, ris_flags ) );

   if( IS_NPC( victim ) )
   {
      pager_printf_color( ch, "&cAttacks    : &w%s\r\n", ext_flag_string( &victim->attacks, attack_flags ) );
      pager_printf_color( ch, "&cDefenses   : &w%s\r\n", ext_flag_string( &victim->defenses, defense_flags ) );
   }

   for( paf = victim->first_affect; paf; paf = paf->next )
   {
      if( ( skill = get_skilltype( paf->type ) ) != NULL )
         pager_printf_color( ch,
                             "&c%s: &w'%s' mods %s by %d for %d rnds with bits %s.",
                             skill_tname[skill->type], skill->name, affect_loc_name( paf->location ), paf->modifier, paf->duration, affect_bit_name( &paf->bitvector ) );

      send_to_char( "\r\n", ch );
   }

   return;
}

void do_mfind( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   int hash;
   int nMatch;
   bool fAll;
   set_pager_color( AT_PLAIN, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Mfind whom?\r\n", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );

   nMatch = 0;
   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_mob_index()... which loops itself, an average of 1-2 times...
    * So theoretically, the above routine may loop well over 40,000 times,
    * and my routine bellow will loop for as many index_mobiles are on
    * your mud... likely under 3000 times.
    * -Thoric
    */

   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
         if( fAll || nifty_is_name( arg, pMobIndex->player_name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\r\n", pMobIndex->vnum, capitalize( pMobIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

   return;
}

void do_ofind( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   int hash;
   int nMatch;
   bool fAll;
   set_pager_color( AT_PLAIN, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Ofind what?\r\n", ch );
      return;
   }

   fAll = !str_cmp( arg, "all" );

   nMatch = 0;
   /*
    * This goes through all the hash entry points (1024), and is therefore
    * much faster, though you won't get your vnums in order... oh well. :)
    *
    * Tests show that Furey's method will usually loop 32,000 times, calling
    * get_obj_index()... which loops itself, an average of 2-3 times...
    * So theoretically, the above routine may loop well over 50,000 times,
    * and my routine bellow will loop for as many index_objects are on
    * your mud... likely under 3000 times.
    * -Thoric
    */

   for( hash = 0; hash < MAX_KEY_HASH; hash++ )
      for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
         if( fAll || nifty_is_name( arg, pObjIndex->name ) )
         {
            nMatch++;
            pager_printf( ch, "[%5d] %s\r\n", pObjIndex->vnum, capitalize( pObjIndex->short_descr ) );
         }

   if( nMatch )
      pager_printf( ch, "Number of matches: %d\n", nMatch );
   else
      send_to_char( "Nothing like that in hell, earth, or heaven.\r\n", ch );

   return;
}

void do_gfighting( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   DESCRIPTOR_DATA *d;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   bool found = FALSE, pmobs = FALSE, phating = FALSE, phunting = FALSE;
   int low = 1, high = MAX_LEVEL, count = 0;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] != STRING_NULL )
   {
      if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
      {
         send_to_pager_color( "\r\n&wSyntax:  gfighting | gfighting <low> <high> | gfighting <low> <high> mobs\r\n", ch );
         return;
      }

      low = atoi( arg1 );

      high = atoi( arg2 );
   }

   if( low < 1 || high < low || low > high || high > MAX_LEVEL )
   {
      send_to_pager_color( "&wInvalid level range.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg3 );

   if( !str_cmp( arg3, "mobs" ) )
      pmobs = TRUE;
   else if( !str_cmp( arg3, "hating" ) )
      phating = TRUE;
   else if( !str_cmp( arg3, "hunting" ) )
      phunting = TRUE;

   pager_printf_color( ch, "\r\n&cGlobal %s conflict:\r\n", pmobs ? "mob" : "character" );

   if( !pmobs && !phating && !phunting )
   {
      for( d = first_descriptor; d; d = d->next )
         if( ( d->connected == CON_PLAYING || d->connected == CON_EDITING )
             && ( victim = d->character ) != NULL && !IS_NPC( victim ) && victim->in_room
             && can_see( ch, victim, TRUE ) && victim->fighting && victim->level >= low && victim->level <= high )
         {
            found = TRUE;
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->fighting->who->level,
                                IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->
                                who->name, IS_NPC( victim->fighting->who ) ? victim->fighting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( !phating && !phunting )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim ) && victim->in_room && can_see( ch, victim, TRUE ) && victim->fighting && victim->level >= low && victim->level <= high )
         {
            found = TRUE;
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->fighting->who->level,
                                IS_NPC( victim->fighting->who ) ? victim->fighting->who->short_descr : victim->fighting->
                                who->name, IS_NPC( victim->fighting->who ) ? victim->fighting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( !phunting && phating )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim ) && victim->in_room && can_see( ch, victim, TRUE ) && victim->hating && victim->level >= low && victim->level <= high )
         {
            found = TRUE;
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->hating->who->level, IS_NPC( victim->hating->who ) ?
                                victim->hating->who->short_descr : victim->hating->who->name, IS_NPC( victim->hating->who ) ?
                                victim->hating->who->pIndexData->vnum : 0, victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }
   else if( phunting )
   {
      for( victim = first_char; victim; victim = victim->next )
         if( IS_NPC( victim ) && victim->in_room && can_see( ch, victim, TRUE ) && victim->hunting && victim->level >= low && victim->level <= high )
         {
            found = TRUE;
            pager_printf_color( ch, "&w%-12.12s &C|%2d &wvs &C%2d| &w%-16.16s [%5d]  &c%-20.20s [%5d]\r\n",
                                victim->name, victim->level, victim->hunting->who->level, IS_NPC( victim->hunting->who ) ?
                                victim->hunting->who->short_descr : victim->hunting->who->name,
                                IS_NPC( victim->hunting->who ) ? victim->hunting->who->pIndexData->vnum : 0,
                                victim->in_room->area->name, victim->in_room == NULL ? 0 : victim->in_room->vnum );
            count++;
         }
   }

   pager_printf_color( ch, "&c%d %s conflicts located.\r\n", count, pmobs ? "mob" : "character" );

   return;
}

void do_reboot( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;
   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) && str_cmp( argument, "and sort skill table" ) )
   {
      send_to_char( "Syntax:  'reboot mud now' or 'reboot nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   sprintf( buf, "Reboot by %s.", ch->name );

   do_echo( ch, buf );

   if( !str_cmp( argument, "and sort skill table" ) )
   {
      sort_skill_table(  );
      save_skill_table(  );
   }

   /*
    * Save all characters before booting.
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );

   mud_down = TRUE;

   return;
}

void do_shutdow( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YIf you want to SHUTDOWN, spell it out.\r\n", ch );
   return;
}

void do_shutdown( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *vch;
   set_char_color( AT_IMMORT, ch );

   if( str_cmp( argument, "mud now" ) && str_cmp( argument, "nosave" ) )
   {
      send_to_char( "Syntax:  'shutdown mud now' or 'shutdown nosave'\r\n", ch );
      return;
   }

   if( auction->item )
      do_auction( ch, "stop" );

   sprintf( buf, "Shutdown by %s.", ch->name );

   append_file( ch, SHUTDOWN_FILE, buf );

   strcat( buf, "\r\n" );

   do_echo( ch, buf );

   /*
    * Save all characters before booting.
    */
   if( str_cmp( argument, "nosave" ) )
      for( vch = first_char; vch; vch = vch->next )
         if( !IS_NPC( vch ) )
            save_char_obj( vch );

   mud_down = TRUE;

   return;
}

void do_snoop( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   DESCRIPTOR_DATA *d;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Snoop whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( !victim->desc )
   {
      send_to_char( "No descriptor to snoop.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Cancelling all snoops.\r\n", ch );

      for( d = first_descriptor; d; d = d->next )
         if( d->snoop_by == ch->desc )
            d->snoop_by = NULL;

      return;
   }

   if( victim->desc->snoop_by )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   /*
    * Minimum snoop level... a secret mset value
    * makes the snooper think that the victim is already being snooped
    */
   if( get_trust( victim ) >= get_trust( ch ) || ( victim->pcdata && victim->pcdata->min_snoop > get_trust( ch ) ) )
   {
      send_to_char( "Busy already.\r\n", ch );
      return;
   }

   if( ch->desc )
   {
      for( d = ch->desc->snoop_by; d; d = d->snoop_by )
         if( d->character == victim || d->original == victim )
         {
            send_to_char( "No snoop loops.\r\n", ch );
            return;
         }
   }

   /*
    * Snoop notification for higher imms, if desired, uncomment this
    */
#ifdef TOOSNOOPY
   if( get_trust( victim ) > LEVEL_GOD && get_trust( ch ) < LEVEL_SUPREME )
      write_to_descriptor( victim->desc->descriptor, "\r\nYou feel like someone is watching your every move...\r\n", 0 );

#endif
   victim->desc->snoop_by = ch->desc;

   send_to_char( "Ok.\r\n", ch );

   return;
}

void do_statshield( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( IS_NPC( ch ) || get_trust( ch ) < LEVEL_GREATER )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Statshield which mobile?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "No such mobile.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "You can only statshield mobiles.\r\n", ch );
      return;
   }

   if( xIS_SET( victim->act, ACT_STATSHIELD ) )
   {
      xREMOVE_BIT( victim->act, ACT_STATSHIELD );
      ch_printf( ch, "You have lifted the statshield on %s.\r\n", victim->short_descr );
   }
   else
   {
      xSET_BIT( victim->act, ACT_STATSHIELD );
      ch_printf( ch, "You have applied a statshield to %s.\r\n", victim->short_descr );
   }

   return;
}

void do_switch( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Switch into whom?\r\n", ch );
      return;
   }

   if( !ch->desc )
      return;

   if( ch->desc->original )
   {
      send_to_char( "You are already switched.\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) && xIS_SET( victim->act, ACT_STATSHIELD ) && get_trust( ch ) < LEVEL_GREATER )
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Their godly glow prevents you from getting close enough.\r\n", ch );
      return;
   }

   if( victim->desc )
   {
      send_to_char( "Character in use.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && ch->level < LEVEL_GREATER )
   {
      send_to_char( "You cannot switch into a player!\r\n", ch );
      return;
   }

   if( victim->switched )
   {
      send_to_char( "You can't switch into a player that is switched!\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && xIS_SET( victim->act, PLR_FREEZE ) )
   {
      send_to_char( "You shouldn't switch into a player that is frozen!\r\n", ch );
      return;
   }

   ch->desc->character = victim;

   ch->desc->original = ch;
   victim->desc = ch->desc;
   ch->desc = NULL;
   ch->switched = victim;
   send_to_char( "Ok.\r\n", victim );
   return;
}

void do_return( CHAR_DATA * ch, char *argument )
{
   if( !IS_NPC( ch ) && get_trust( ch ) < LEVEL_IMMORTAL )
   {
      send_to_char( "Huh?\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, ch );

   if( !ch->desc )
      return;

   if( !ch->desc->original )
   {
      send_to_char( "You aren't switched.\r\n", ch );
      return;
   }

   send_to_char( "You return to your original body.\r\n", ch );

   ch->desc->character = ch->desc->original;
   ch->desc->original = NULL;
   ch->desc->character->desc = ch->desc;
   ch->desc->character->switched = NULL;
   ch->desc = NULL;
   return;
}

void do_minvoke( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   MOB_INDEX_DATA *pMobIndex;
   CHAR_DATA *victim;
   int vnum;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Syntax:  minvoke <vnum>\r\n", ch );
      return;
   }

   if( IS_PLR_FLAG( ch, PLR_ONMAP ) )
   {
      send_to_char( "You cannot minvoke a mob to the overland\r\n", ch );
      return;
   }

   if( !is_number( arg ) )
   {
      char arg2[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg, arg2 );
      vnum = -1;

      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pMobIndex = mob_index_hash[hash]; pMobIndex; pMobIndex = pMobIndex->next )
            if( nifty_is_name( arg2, pMobIndex->player_name ) && ++cnt == count )
            {
               vnum = pMobIndex->vnum;
               break;
            }

      if( vnum == -1 )
      {
         send_to_char( "No such mobile exists.\r\n", ch );
         return;
      }
   }
   else
      vnum = atoi( arg );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\r\n", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this mobile.\r\n", ch );
         return;
      }

      if( vnum < pArea->low_m_vnum && vnum > pArea->hi_m_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }

   if( ( pMobIndex = get_mob_index( vnum ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\r\n", ch );
      return;
   }

   /*
    * If you load one on the map, make sure it gets placed properly - Samson 8-21-99
    */
   if( IS_PLR_FLAG( ch, PLR_ONMAP ) )  /*make sure we are on the map when we do this tommi 2005 */
   {
      fix_maps( ch, victim );
      victim->sector = get_terrain( ch->map, ch->x, ch->y );
   }

   victim = create_mobile( pMobIndex );

   char_to_room( victim, ch->in_room );
   act( AT_IMMORT, "$n invokes $N!", ch, NULL, victim, TO_ROOM );
   /*
    * How about seeing what we're invoking for a change. -Blodkai
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\r\n", pMobIndex->short_descr, pMobIndex->vnum, pMobIndex->player_name, victim->level );
   return;
}

void do_oinvoke( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   OBJ_INDEX_DATA *pObjIndex;
   OBJ_DATA *obj;
   int vnum;
   int level;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: oinvoke <vnum> <level>.\r\n", ch );
      return;
   }

   if( arg2[0] == STRING_NULL )
   {
      level = get_trust( ch );
   }
   else
   {
      if( !is_number( arg2 ) )
      {
         send_to_char( "Syntax:  oinvoke <vnum> <level>\r\n", ch );
         return;
      }

      level = atoi( arg2 );

      if( level < 0 || level > get_trust( ch ) )
      {
         send_to_char( "Limited to your trust level.\r\n", ch );
         return;
      }
   }

   if( !is_number( arg1 ) )
   {
      char arg[MAX_INPUT_LENGTH];
      int hash, cnt;
      int count = number_argument( arg1, arg );
      vnum = -1;

      for( hash = cnt = 0; hash < MAX_KEY_HASH; hash++ )
         for( pObjIndex = obj_index_hash[hash]; pObjIndex; pObjIndex = pObjIndex->next )
            if( nifty_is_name( arg, pObjIndex->name ) && ++cnt == count )
            {
               vnum = pObjIndex->vnum;
               break;
            }

      if( vnum == -1 )
      {
         send_to_char( "No such object exists.\r\n", ch );
         return;
      }
   }
   else
      vnum = atoi( arg1 );

   if( get_trust( ch ) < LEVEL_DEMI )
   {
      AREA_DATA *pArea;

      if( IS_NPC( ch ) )
      {
         send_to_char( "Huh?\r\n", ch );
         return;
      }

      if( !ch->pcdata || !( pArea = ch->pcdata->area ) )
      {
         send_to_char( "You must have an assigned area to invoke this object.\r\n", ch );
         return;
      }

      if( vnum < pArea->low_o_vnum && vnum > pArea->hi_o_vnum )
      {
         send_to_char( "That number is not in your allocated range.\r\n", ch );
         return;
      }
   }

   if( ( pObjIndex = get_obj_index( vnum ) ) == NULL )
   {
      send_to_char( "No object has that vnum.\r\n", ch );
      return;
   }

   if( level == 0 )
   {
      AREA_DATA *temp_area;

      if( ( temp_area = get_area_obj( pObjIndex ) ) == NULL )
         level = ch->level;
      else
      {
         level = generate_itemlevel( temp_area, pObjIndex );
         level = URANGE( 0, level, LEVEL_AVATAR );
      }
   }

   obj = create_object( pObjIndex, level );

   if( CAN_WEAR( obj, ITEM_TAKE ) )
   {
      obj = obj_to_char( obj, ch );
   }
   else
   {
      obj = obj_to_room( obj, ch->in_room, ch );
      act( AT_IMMORT, "$n fashions $p from ether!", ch, obj, NULL, TO_ROOM );
   }

   /*
    * I invoked what? --Blodkai
    */
   ch_printf_color( ch, "&YYou invoke %s (&W#%d &Y- &W%s &Y- &Wlvl %d&Y)\r\n", pObjIndex->short_descr, pObjIndex->vnum, pObjIndex->name, obj->level );

   return;
}

void do_purge( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim, *tch;
   OBJ_DATA *obj;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      /*
       * 'purge'
       */
      CHAR_DATA *vnext;
      OBJ_DATA *obj_next;

      for( victim = ch->in_room->first_person; victim; victim = vnext )
      {
         vnext = victim->next_in_room;
         /*
          * GACK! Why did this get removed??
          */

         if( !IS_NPC( victim ) )
            continue;

         for( tch = ch->in_room->first_person; tch; tch = tch->next_in_room )
            if( !IS_NPC( tch ) && tch->dest_buf == victim )
               break;

         if( tch && !IS_NPC( tch ) && tch->dest_buf == victim )
            continue;

         /*
          * This will work in normal rooms too since they should always be -1,-1,-1 outside of the maps.
          */
         if( is_same_map( ch, victim ) )
            extract_char( victim, TRUE );
      }

      for( obj = ch->in_room->first_content; obj; obj = obj_next )
      {
         obj_next = obj->next_content;

         for( tch = ch->in_room->first_person; tch; tch = tch->next_in_room )
            if( !IS_NPC( tch ) && tch->dest_buf == obj )
               break;

         if( tch && !IS_NPC( tch ) && tch->dest_buf == obj )
            continue;

         /*
          * This will work in normal rooms too since they should always be -1,-1,-1 outside of the maps.
          */
         if( ch->map == obj->map && ch->x == obj->x && ch->y == obj->y )
            extract_obj( obj );
      }

      act( AT_IMMORT, "$n purges the room!", ch, NULL, NULL, TO_ROOM );

      act( AT_IMMORT, "You have purged the room!", ch, NULL, NULL, TO_CHAR );
      save_house_by_vnum( ch->in_room->vnum );  /* Prevent House Object Duplication */
      return;
   }

   victim = NULL;

   obj = NULL;
   /*
    * fixed to get things in room first -- i.e., purge portal (obj),
    * * no more purging mobs with that keyword in another room first
    * * -- Tri
    */

   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      if( ( victim = get_char_world( ch, arg ) ) == NULL && ( obj = get_obj_world( ch, arg ) ) == NULL ) /* no get_obj_room */
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }
   }

   /*
    * Single object purge in room for high level purge - Scryn 8/12
    */
   if( obj )
   {
      int vnum = ( obj->in_room ? obj->in_room->vnum : ROOM_VNUM_LIMBO );
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p.", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      save_house_by_vnum( vnum );   /* Prevent House Object Duplication */
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );

   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_low_purge( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Purge what?\r\n", ch );
      return;
   }

   victim = NULL;

   obj = NULL;

   if( ( victim = get_char_room( ch, arg ) ) == NULL && ( obj = get_obj_here( ch, arg ) ) == NULL )
   {
      send_to_char( "You can't find that here.\r\n", ch );
      return;
   }

   if( obj )
   {
      separate_obj( obj );
      act( AT_IMMORT, "$n purges $p!", ch, obj, NULL, TO_ROOM );
      act( AT_IMMORT, "You make $p disappear in a puff of smoke!", ch, obj, NULL, TO_CHAR );
      extract_obj( obj );
      return;
   }

   if( !IS_NPC( victim ) )
   {
      send_to_char( "Not on PC's.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "You cannot purge yourself!\r\n", ch );
      return;
   }

   act( AT_IMMORT, "$n purges $N.", ch, NULL, victim, TO_NOTVICT );

   act( AT_IMMORT, "You make $N disappear in a puff of smoke!", ch, NULL, victim, TO_CHAR );
   extract_char( victim, TRUE );
   return;
}

void do_balzhur( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char *name;
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;
   set_char_color( AT_BLOOD, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Who is deserving of such a fate?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't currently playing.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "This will do little good on mobiles.\r\n", ch );
      return;
   }

   if( victim->level >= get_trust( ch ) )
   {
      send_to_char( "I wouldn't even think of that if I were you...\r\n", ch );
      return;
   }

   set_char_color( AT_WHITE, ch );

   send_to_char( "You summon the demon Balzhur to wreak your wrath!\r\n", ch );
   send_to_char( "Balzhur sneers at you evilly, then vanishes in a puff of smoke.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   send_to_char( "You hear an ungodly sound in the distance that makes your blood run cold!\r\n", victim );
   sprintf( buf, "Balzhur screams, 'You are MINE %s!!!'", victim->name );
   echo_to_all( AT_IMMORT, buf, ECHOTAR_ALL );
   victim->level = 2;
   victim->trust = 0;
   victim->exp = 2000;
   victim->max_hit = 10;
   victim->max_mana = 100;
   victim->max_move = 100;

   for( sn = 0; sn < top_sn; sn++ )
      victim->pcdata->learned[sn] = 0;

   victim->practice = 0;

   victim->hit = victim->max_hit;

   victim->mana = victim->max_mana;

   victim->move = victim->max_move;

   name = capitalize( victim->name );

   sprintf( buf, "%s%s", GOD_DIR, name );

   set_char_color( AT_RED, ch );

   if( !remove( buf ) )
      send_to_char( "Player's immortal data destroyed.\r\n", ch );
   else if( errno != ENOENT )
   {
      ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\r\n", errno, strerror( errno ) );
      sprintf( buf2, "%s balzhuring %s", ch->name, buf );
      perror( buf2 );
   }

   sprintf( buf2, "%s.are", name );

   for( pArea = first_build; pArea; pArea = pArea->next )
      if( !str_cmp( pArea->filename, buf2 ) )
      {
         sprintf( buf, "%s%s", BUILD_DIR, buf2 );

         if( IS_SET( pArea->status, AREA_LOADED ) )
            fold_area( pArea, buf, FALSE );

         close_area( pArea );

         sprintf( buf2, "%s.bak", buf );

         set_char_color( AT_RED, ch ); /* Log message changes colors */

         if( !rename( buf, buf2 ) )
            send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
         else if( errno != ENOENT )
         {
            ch_printf( ch, "Unknown error #%d - %s (area data).  Report to  Thoric.\r\n", errno, strerror( errno ) );
            sprintf( buf2, "%s destroying %s", ch->name, buf );
            perror( buf2 );
         }

         break;
      }

   make_wizlist(  );

   advance_level( victim );
   do_help( victim, "M_BALZHUR_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake after a long period of time...\r\n", victim );

   while( victim->first_carrying )
      extract_obj( victim->first_carrying );

   return;
}

void do_advance( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;
   int iLevel;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL || !is_number( arg2 ) )
   {
      send_to_char( "Syntax:  advance <character> <level>\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That character is not in the room.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You cannot advance a mobile.\r\n", ch );
      return;
   }

   if( get_trust( ch ) <= get_trust( victim ) || ch == victim )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 1 || level > MAX_LEVEL )
   {
      ch_printf( ch, "Level range is 1 to %d.\r\n", MAX_LEVEL );
      return;
   }

   if( level > get_trust( ch ) )
   {
      send_to_char( "Level limited to your trust level.\r\n", ch );
      return;
   }

   if( level <= victim->level )
   {
      int sn;
      set_char_color( AT_IMMORT, victim );

      if( victim->level >= LEVEL_AVATAR && IS_IMMORTAL( victim ) )
      {
         if( victim->pcdata->bestowments )
            DISPOSE( victim->pcdata->bestowments );

         victim->pcdata->bestowments = str_dup( "" );
         xREMOVE_BIT( victim->act, PLR_HOLYLIGHT );

         if( !IS_RETIRED( victim ) )
         {
            /*
             * Fixed bug here, was removing the immortal data of the person
             * who used advance - Orion Elder
             */
            sprintf( buf, "%s%s", GOD_DIR, capitalize( victim->name ) );
            /*
             * Added to notify of removal of Immortal data. - Orion Elder
             */

            if( !remove( buf ) )
               send_to_char( "Player's immortal data destroyed.\r\n", ch );
         }
      }

      if( level < victim->level )
      {
         ch_printf( ch, "Demoting %s from level %d to level %d!\r\n", victim->name, victim->level, level );
         send_to_char( "Cursed and forsaken!  The gods have lowered your level...\r\n", victim );
      }
      else
      {
         ch_printf( ch, "%s is already level %d.  Re-advancing...\r\n", victim->name, level );
         send_to_char( "Deja vu!  Your mind reels as you re-live your past levels!\r\n", victim );
      }

      victim->level = 1;
      victim->exp = 0;
      victim->max_hit = 50;
      victim->max_mana = 200;
      victim->max_move = 200;

      for( sn = 0; sn < top_sn; sn++ )
         victim->pcdata->learned[sn] = 0;

      victim->practice = 0;
      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      advance_level( victim );
      /*
       * Rank fix added by Narn.
       */
      DISPOSE( victim->pcdata->rank );
      victim->pcdata->rank = str_dup( "" );
      /*
       * Stuff added to make sure character's wizinvis level doesn't stay
       * higher than actual level, take wizinvis away from advance < 50
       */
      victim->pcdata->wizinvis = victim->trust;

      if( victim->level <= LEVEL_AVATAR )
      {
         xREMOVE_BIT( victim->act, PLR_WIZINVIS );
         victim->pcdata->wizinvis = 0;
      }
   }
   else
   {
      ch_printf( ch, "Raising %s from level %d to level %d!\r\n", victim->name, victim->level, level );

      if( victim->level >= LEVEL_AVATAR )
      {
         set_char_color( AT_IMMORT, victim );
         act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at you!", ch, NULL, victim, TO_VICT );
         act( AT_IMMORT, "$n makes some arcane gestures with $s hands, then points $s finger at $N!", ch, NULL, victim, TO_NOTVICT );
         set_char_color( AT_WHITE, victim );
         send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
         set_char_color( AT_LBLUE, victim );
      }

      switch ( level )
      {

         default:
            send_to_char( "The gods feel fit to raise your level!\r\n", victim );
            break;

         case LEVEL_IMMORTAL:
         case LEVEL_ACOLYTE:
         case LEVEL_CREATOR:
         case LEVEL_SAVIOR:
         case LEVEL_DEMI:
         case LEVEL_TRUEIMM:
         case LEVEL_LESSER:
         case LEVEL_GOD:
         case LEVEL_GREATER:
         case LEVEL_ASCENDANT:
         case LEVEL_SUB_IMPLEM:
         case LEVEL_IMPLEMENTOR:
         case LEVEL_ETERNAL:
         case LEVEL_INFINITE:
         case LEVEL_SUPREME:
            do_help( victim, "IMMORTAL_ADVANCE" );
            set_char_color( AT_WHITE, victim );
            send_to_char( "You awake... all your possessions are gone.\r\n", victim );

            while( victim->first_carrying )
               extract_obj( victim->first_carrying );
            break;
      }
   }

   for( iLevel = victim->level; iLevel < level; iLevel++ )
   {
      if( level < LEVEL_IMMORTAL )
         send_to_char( "You raise a level!!\r\n", victim );

      victim->level += 1;
      advance_level( victim );
   }

   victim->exp = 0;
   victim->trust = 0;
   return;
}

void do_elevate( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Syntax: elevate <char>\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( victim->level == LEVEL_IMMORTAL )
   {
      send_to_char( "Elevating a player...\r\n", ch );
      set_char_color( AT_IMMORT, victim );
      act( AT_IMMORT, "$n begins to chant softly... then makes some arcane gestures...", ch, NULL, NULL, TO_ROOM );
      set_char_color( AT_WHITE, victim );
      send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
      set_char_color( AT_LBLUE, victim );
      do_help( victim, "M_GODLVL2_" );
      victim->level = LEVEL_ACOLYTE;
      set_char_color( AT_WHITE, victim );
      advance_level( victim );
      victim->exp = 0;
      victim->trust = 0;
      return;
   }

   if( victim->level == LEVEL_ACOLYTE )
   {
      send_to_char( "Elevating a player...\r\n", ch );
      set_char_color( AT_IMMORT, victim );
      act( AT_IMMORT, "$n begins to chant softly... then makes some arcane gestures...", ch, NULL, NULL, TO_ROOM );
      set_char_color( AT_WHITE, victim );
      send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
      set_char_color( AT_LBLUE, victim );
      do_help( victim, "M_GODLVL3_" );
      victim->level = LEVEL_CREATOR;
      set_char_color( AT_WHITE, victim );
      advance_level( victim );
      victim->exp = 0;
      victim->trust = 0;
      return;
   }
   else
      send_to_char( "You cannot elevate this character.\r\n", ch );

   return;
}

void do_immortalize( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Syntax:  immortalize <char>\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   /*
    * Added this check, not sure why the code didn't already have it. Samson 1-18-98
    */
   if( victim->level >= LEVEL_IMMORTAL )
   {
      ch_printf( ch, "Don't be silly, %s is already immortal.\r\n", victim->name );
      return;
   }

   if( victim->level != LEVEL_AVATAR )
   {
      send_to_char( "This player is not yet worthy of immortality.\r\n", ch );
      return;
   }

   send_to_char( "Immortalizing a player...\r\n", ch );

   set_char_color( AT_IMMORT, victim );
   act( AT_IMMORT, "$n begins to chant softly... then raises $s arms to the sky...", ch, NULL, NULL, TO_ROOM );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You suddenly feel very strange...\r\n\r\n", victim );
   set_char_color( AT_LBLUE, victim );
   do_help( victim, "M_GODLVL1_" );
   set_char_color( AT_WHITE, victim );
   send_to_char( "You awake... all your possessions are gone.\r\n", victim );

   while( victim->first_carrying )
      extract_obj( victim->first_carrying );

   victim->level = LEVEL_IMMORTAL;

   advance_level( victim );

   /*
    * Remove clan/guild/order and update accordingly
    */
   if( victim->pcdata->clan )
   {
      if( victim->speaking & LANG_CLAN )
         victim->speaking = LANG_COMMON;

      REMOVE_BIT( victim->speaks, LANG_CLAN );

      --victim->pcdata->clan->members;

      if( !str_cmp( victim->name, victim->pcdata->clan->leader ) )
      {
         STRFREE( victim->pcdata->clan->leader );
         victim->pcdata->clan->leader = STRALLOC( "" );
      }

      if( !str_cmp( victim->name, victim->pcdata->clan->number1 ) )
      {
         STRFREE( victim->pcdata->clan->number1 );
         victim->pcdata->clan->number1 = STRALLOC( "" );
      }

      if( !str_cmp( victim->name, victim->pcdata->clan->number2 ) )
      {
         STRFREE( victim->pcdata->clan->number2 );
         victim->pcdata->clan->number2 = STRALLOC( "" );
      }

      victim->pcdata->clan = NULL;

      STRFREE( victim->pcdata->clan_name );
   }

   victim->exp = 0;
   victim->trust = 0;
   save_char_obj( victim );
   return;
}

void do_trust( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   int level;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL || !is_number( arg2 ) )
   {
      send_to_char( "Syntax:  trust <char> <level>.\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That player is not here.\r\n", ch );
      return;
   }

   if( ( level = atoi( arg2 ) ) < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Level must be 0 (reset) or 1 to 65.\r\n", ch );
      return;
   }

   if( level > get_trust( ch ) )
   {
      send_to_char( "Limited to your own trust.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You can't do that.\r\n", ch );
      return;
   }

   victim->trust = level;

   send_to_char( "Ok.\r\n", ch );
   return;
}

/*
 * Summer 1997 --Blod
 */
void do_scatter( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *pRoomIndex;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Scatter whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "It's called teleport.  Try it.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }

   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );

      if( pRoomIndex )
         if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }

   if( victim->fighting )
      stop_fighting( victim, TRUE );

   act( AT_MAGIC, "With the sweep of an arm, $n flings $N to the winds.", ch, NULL, victim, TO_NOTVICT );

   act( AT_MAGIC, "With the sweep of an arm, $n flings you to the astral winds.", ch, NULL, victim, TO_VICT );

   act( AT_MAGIC, "With the sweep of an arm, you fling $N to the astral winds.", ch, NULL, victim, TO_CHAR );

   char_from_room( victim );

   char_to_room( victim, pRoomIndex );

   victim->position = POS_RESTING;

   act( AT_MAGIC, "$n staggers forth from a sudden gust of wind, and collapses.", victim, NULL, NULL, TO_ROOM );

   do_look( victim, "auto" );

   return;
}

void do_strew( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   ROOM_INDEX_DATA *pRoomIndex;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Strew who, what?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "It would work better if they were here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Try taking it out on someone else first.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "coins" ) )
   {
      if( victim->gold < 1 )
      {
         send_to_char( "Drat, this one's got no gold to start with.\r\n", ch );
         return;
      }

      victim->gold = 0;

      act( AT_MAGIC, "$n gestures and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You gesture and an unearthly gale sends $N's coins flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "As $n gestures, an unearthly gale sends your currency flying!", ch, NULL, victim, TO_VICT );
      return;
   }

   for( ;; )
   {
      pRoomIndex = get_room_index( number_range( 0, MAX_VNUM ) );

      if( pRoomIndex )
         if( !xIS_SET( pRoomIndex->room_flags, ROOM_PRIVATE )
             && !xIS_SET( pRoomIndex->room_flags, ROOM_SOLITARY ) && !xIS_SET( pRoomIndex->room_flags, ROOM_NO_ASTRAL ) && !xIS_SET( pRoomIndex->room_flags, ROOM_PROTOTYPE ) )
            break;
   }

   if( !str_cmp( arg2, "inventory" ) )
   {
      act( AT_MAGIC, "$n speaks a single word, sending $N's possessions flying!", ch, NULL, victim, TO_NOTVICT );
      act( AT_MAGIC, "You speak a single word, sending $N's possessions flying!", ch, NULL, victim, TO_CHAR );
      act( AT_MAGIC, "$n speaks a single word, sending your possessions flying!", ch, NULL, victim, TO_VICT );

      for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
      {
         obj_next = obj_lose->next_content;
         obj_from_char( obj_lose );
         obj_to_room( obj_lose, pRoomIndex, ch );
         pager_printf_color( ch, "\t&w%s sent to %d\r\n", capitalize( obj_lose->short_descr ), pRoomIndex->vnum );
      }

      return;
   }

   send_to_char( "Strew their coins or inventory?\r\n", ch );

   return;
}

void do_strip( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   OBJ_DATA *obj_next;
   OBJ_DATA *obj_lose;
   int count = 0;
   set_char_color( AT_OBJECT, ch );

   if( !argument )
   {
      send_to_char( "Strip who?\r\n", ch );
      return;
   }

   if( ( victim = get_char_room( ch, argument ) ) == NULL )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }

   if( victim == ch )
   {
      send_to_char( "Kinky.\r\n", ch );
      return;
   }

   if( !IS_NPC( victim ) && get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You haven't the power to succeed against them.\r\n", ch );
      return;
   }

   act( AT_OBJECT, "Searching $N ...", ch, NULL, victim, TO_CHAR );

   for( obj_lose = victim->first_carrying; obj_lose; obj_lose = obj_next )
   {
      obj_next = obj_lose->next_content;
      obj_from_char( obj_lose );
      obj_to_char( obj_lose, ch );
      pager_printf_color( ch, "  &G... %s (&g%s) &Gtaken.\r\n", capitalize( obj_lose->short_descr ), obj_lose->name );
      count++;
   }

   if( !count )
      send_to_pager( "&GNothing found to take.\r\n", ch );

   return;
}

void do_restore( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Syntax: Restore <player name>\r\n", ch );
      send_to_char( "Syntax: Restore all\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( !ch->pcdata )
         return;

      if( get_trust( ch ) < LEVEL_SUB_IMPLEM )
      {
         if( IS_NPC( ch ) )
         {
            send_to_char( "You can't do that.\r\n", ch );
            return;
         }
      }

      last_restore_all_time = current_time;

      ch->pcdata->restore_time = current_time;
      save_char_obj( ch );
      send_to_char( "Beginning 'restore all' ...\r\n", ch );

      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && !IS_IMMORTAL( vch ) && !CAN_PKILL( vch ) && !in_arena( vch ) )
         {
            vch->hit = vch->max_hit;
            vch->mana = vch->max_mana;
            vch->move = vch->max_move;
            update_pos( vch );

            act( AT_IMMORT, "$n has restored you.", ch, NULL, vch, TO_VICT );
         }
      }

      send_to_char( "Restored.\r\n", ch );
   }
   else
   {
      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_NEOPHYTE && victim != ch && !( IS_NPC( victim ) && xIS_SET( victim->act, ACT_PROTOTYPE ) ) )
      {
         send_to_char( "You can't do that.\r\n", ch );
         return;
      }

      victim->hit = victim->max_hit;
      victim->mana = victim->max_mana;
      victim->move = victim->max_move;
      update_pos( victim );

      if( ch != victim )
         act( AT_IMMORT, "$n has restored you.", ch, NULL, victim, TO_VICT );

      send_to_char( "Restored.\r\n", ch );

      return;
   }
}
void do_freeze( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_LBLUE, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Freeze whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   set_char_color( AT_LBLUE, victim );

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed, and they saw...\r\n", ch );
      ch_printf( victim, "%s is attempting to freeze you.\r\n", ch->name );
      return;
   }

   if( xIS_SET( victim->act, PLR_FREEZE ) )
   {
      xREMOVE_BIT( victim->act, PLR_FREEZE );
      send_to_char( "Your frozen form suddenly thaws.\r\n", victim );
      ch_printf( ch, "%s is now unfrozen.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_FREEZE );
      send_to_char( "A godly force turns your body to ice!\r\n", victim );
      ch_printf( ch, "You have frozen %s.\r\n", victim->name );
   }

   save_char_obj( victim );

   return;
}

void do_log( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Log whom?\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( fLogAll )
      {
         fLogAll = FALSE;
         send_to_char( "Log ALL off.\r\n", ch );
      }
      else
      {
         fLogAll = TRUE;
         send_to_char( "Log ALL on.\r\n", ch );
      }

      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   /*
    * No level check, gods can log anyone.
    */
   if( xIS_SET( victim->act, PLR_LOG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LOG );
      ch_printf( ch, "LOG applied to %s.\r\n", victim->name );
   }

   return;
}

void do_litterbug( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Set litterbug flag on whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( xIS_SET( victim->act, PLR_LITTERBUG ) )
   {
      xREMOVE_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "You can drop items again.\r\n", victim );
      ch_printf( ch, "LITTERBUG removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_LITTERBUG );
      send_to_char( "A strange force prevents you from dropping any more items!\r\n", victim );
      ch_printf( ch, "LITTERBUG set on %s.\r\n", victim->name );
   }

   return;
}

void do_noemote( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Noemote whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( xIS_SET( victim->act, PLR_NO_EMOTE ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can emote again.\r\n", victim );
      ch_printf( ch, "NOEMOTE removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_EMOTE );
      send_to_char( "You can't emote!\r\n", victim );
      ch_printf( ch, "NOEMOTE applied to %s.\r\n", victim->name );
   }

   return;
}

void do_notell( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Notell whom?", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( xIS_SET( victim->act, PLR_NO_TELL ) )
   {
      xREMOVE_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can use tells again.\r\n", victim );
      ch_printf( ch, "NOTELL removed from %s.\r\n", victim->name );
   }
   else
   {
      xSET_BIT( victim->act, PLR_NO_TELL );
      send_to_char( "You can't use tells!\r\n", victim );
      ch_printf( ch, "NOTELL applied to %s.\r\n", victim->name );
   }

   return;
}

void do_notitle( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Notitle whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( IS_SET( victim->pcdata->flags, PCFLAG_NOTITLE ) )
   {
      REMOVE_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      send_to_char( "You can set your own title again.\r\n", victim );
      ch_printf( ch, "NOTITLE removed from %s.\r\n", victim->name );
   }
   else
   {
      SET_BIT( victim->pcdata->flags, PCFLAG_NOTITLE );
      sprintf( buf, "the %s", title_table[victim->Class][victim->level][victim->sex == SEX_FEMALE ? 1 : 0] );
      set_title( victim, buf );
      send_to_char( "You can't set your own title!\r\n", victim );
      ch_printf( ch, "NOTITLE set on %s.\r\n", victim->name );
   }

   return;
}

void do_silence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Silence whom?", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      send_to_char( "Player already silenced, use unsilence to remove.\r\n", ch );
   }
   else
   {
      xSET_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can't use channels!\r\n", victim );
      ch_printf( ch, "You SILENCE %s.\r\n", victim->name );
   }

   return;
}

/*
 * Much better than toggling this with do_silence, yech --Blodkai
 */
void do_unsilence( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Unsilence whom?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "Not on NPC's.\r\n", ch );
      return;
   }

   if( get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You failed.\r\n", ch );
      return;
   }

   set_char_color( AT_IMMORT, victim );

   if( xIS_SET( victim->act, PLR_SILENCE ) )
   {
      xREMOVE_BIT( victim->act, PLR_SILENCE );
      send_to_char( "You can use channels again.\r\n", victim );
      ch_printf( ch, "SILENCE removed from %s.\r\n", victim->name );
   }
   else
   {
      send_to_char( "That player is not silenced.\r\n", ch );
   }

   return;
}

void do_peace( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *rch;
   act( AT_IMMORT, "$n booms, 'PEACE!'", ch, NULL, NULL, TO_ROOM );
   act( AT_IMMORT, "You boom, 'PEACE!'", ch, NULL, NULL, TO_CHAR );

   for( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
   {
      if( rch->fighting )
      {
         stop_fighting( rch, TRUE );
         do_sit( rch, "" );
      }

      /*
       * Added by Narn, Nov 28/95
       */
      stop_hating( rch );
      stop_hunting( rch );
      stop_fearing( rch );
   }

   send_to_char_color( "&YOk.\r\n", ch );
   return;
}

void do_wizlock( CHAR_DATA * ch, char *argument )
{
   extern bool wizlock;
   wizlock = !wizlock;
   set_char_color( AT_DANGER, ch );

   if( wizlock )
      send_to_char( "Game wizlocked.\r\n", ch );
   else
      send_to_char( "Game un-wizlocked.\r\n", ch );

   return;
}

void do_noresolve( CHAR_DATA * ch, char *argument )
{
   sysdata.NO_NAME_RESOLVING = !sysdata.NO_NAME_RESOLVING;

   if( sysdata.NO_NAME_RESOLVING )
      send_to_char_color( "&YName resolving disabled.\r\n", ch );
   else
      send_to_char_color( "&YName resolving enabled.\r\n", ch );

   return;
}

/* Output of command reformmated by Samson 2-8-98, and again on 4-7-98 */
void do_users( CHAR_DATA * ch, char *argument )
{
   DESCRIPTOR_DATA *d;
   int count;
   char *st;
   set_pager_color( AT_PLAIN, ch );
   count = 0;
   send_to_pager( "Desc|     Constate      |Idle|    Player    | HostIP                   \r\n", ch );
   send_to_pager( "----+-------------------+----+--------------+--------------------------\r\n", ch );

   for( d = first_descriptor; d; d = d->next )
   {
      switch ( d->connected )
      {

         case CON_PLAYING:
            st = "Playing";
            break;

         case CON_GET_NAME:
            st = "Get name";
            break;

         case CON_GET_OLD_PASSWORD:
            st = "Get password";
            break;

         case CON_CONFIRM_NEW_NAME:
            st = "Confirm name";
            break;

         case CON_GET_NEW_PASSWORD:
            st = "New password";
            break;

         case CON_CONFIRM_NEW_PASSWORD:
            st = "Confirm password";
            break;

         case CON_GET_NEW_SEX:
            st = "Get sex";
            break;

         case CON_READ_MOTD:
            st = "Reading MOTD";
            break;

         case CON_EDITING:
            st = "In line editor";
            break;

         case CON_PRESS_ENTER:
            st = "Press enter";
            break;

         default:
            st = "Invalid!!!!";
            break;
      }

      if( !argument || argument[0] == STRING_NULL )
      {
         if( get_trust( ch ) >= LEVEL_ASCENDANT || ( d->character && can_see( ch, d->character, FALSE ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %-17s |%4d| %-12s | %s \r\n", d->descriptor, st, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
      else
      {
         if( ( get_trust( ch ) >= LEVEL_SUPREME || ( d->character && can_see( ch, d->character, TRUE ) ) )
             && ( !str_prefix( argument, d->host ) || ( d->character && !str_prefix( argument, d->character->name ) ) ) )
         {
            count++;
            pager_printf( ch, " %3d| %2d|%4d| %-12s | %s \r\n", d->descriptor, d->connected, d->idle / 4,
                          d->original ? d->original->name : d->character ? d->character->name : "(None!)", d->host );
         }
      }
   }

   pager_printf( ch, "%d user%s.\r\n", count, count == 1 ? "" : "s" );

   return;
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   bool mobsonly;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL || argument[0] == STRING_NULL )
   {
      send_to_char( "Force whom to do what?\r\n", ch );
      return;
   }

   mobsonly = get_trust( ch ) < sysdata.level_forcepc;

   if( !str_cmp( arg, "all" ) )
   {
      CHAR_DATA *vch;
      CHAR_DATA *vch_next;

      if( mobsonly )
      {
         send_to_char( "Force whom to do what?\r\n", ch );
         return;
      }

      for( vch = first_char; vch; vch = vch_next )
      {
         vch_next = vch->next;

         if( !IS_NPC( vch ) && get_trust( vch ) < get_trust( ch ) )
         {
            act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, vch, TO_VICT );
            interpret( vch, argument );
         }
      }
   }
   else
   {
      CHAR_DATA *victim;

      if( ( victim = get_char_world( ch, arg ) ) == NULL )
      {
         send_to_char( "They aren't here.\r\n", ch );
         return;
      }

      if( victim == ch )
      {
         send_to_char( "Aye aye, right away!\r\n", ch );
         return;
      }

      if( ( get_trust( victim ) >= get_trust( ch ) ) || ( mobsonly && !IS_NPC( victim ) ) )
      {
         send_to_char( "Do it yourself!\r\n", ch );
         return;
      }

      if( get_trust( ch ) < LEVEL_GOD && IS_NPC( victim ) && !str_prefix( "mp", argument ) )
      {
         send_to_char( "You can't force a mob to do that!\r\n", ch );
         return;
      }

      act( AT_IMMORT, "$n forces you to '$t'.", ch, argument, victim, TO_VICT );

      interpret( victim, argument );
   }

   send_to_char( "Ok.\r\n", ch );

   return;
}

void do_invis( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   short level;
   set_char_color( AT_IMMORT, ch );
   /*
    * if ( IS_NPC(ch)) return;
    */
   argument = one_argument( argument, arg );

   if( arg[0] != STRING_NULL )
   {
      if( !is_number( arg ) )
      {
         send_to_char( "Usage: invis | invis <level>\r\n", ch );
         return;
      }

      level = atoi( arg );

      if( level < 2 || level > get_trust( ch ) )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }

      if( !IS_NPC( ch ) )
      {
         ch->pcdata->wizinvis = level;
         ch_printf( ch, "Wizinvis level set to %d.\r\n", level );
      }

      if( IS_NPC( ch ) )
      {
         ch->mobinvis = level;
         ch_printf( ch, "Mobinvis level set to %d.\r\n", level );
      }

      return;
   }

   if( !IS_NPC( ch ) )
   {
      if( ch->pcdata->wizinvis < 2 )
         ch->pcdata->wizinvis = ch->level;
   }

   if( IS_NPC( ch ) )
   {
      if( ch->mobinvis < 2 )
         ch->mobinvis = ch->level;
   }

   if( xIS_SET( ch->act, PLR_WIZINVIS ) )
   {
      xREMOVE_BIT( ch->act, PLR_WIZINVIS );
      act( AT_IMMORT, "$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly fade back into existence.\r\n", ch );
   }
   else
   {
      act( AT_IMMORT, "$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM );
      send_to_char( "You slowly vanish into thin air.\r\n", ch );
      xSET_BIT( ch->act, PLR_WIZINVIS );
   }

   return;
}

void do_holylight( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_IMMORT, ch );

   if( IS_NPC( ch ) )
      return;

   if( xIS_SET( ch->act, PLR_HOLYLIGHT ) )
   {
      xREMOVE_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode off.\r\n", ch );
   }
   else
   {
      xSET_BIT( ch->act, PLR_HOLYLIGHT );
      send_to_char( "Holy light mode on.\r\n", ch );
   }

   return;
}

void do_rassign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   short r_lo, r_hi;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   r_lo = atoi( arg2 );
   r_hi = atoi( arg3 );

   if( arg1[0] == STRING_NULL || r_lo < 0 || r_hi < 0 )
   {
      send_to_char( "Syntax: rassign <who> <low> <high>\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_CREATOR )
   {
      send_to_char( "They wouldn't know what to do with a room range.\r\n", ch );
      return;
   }

   if( r_lo > r_hi )
   {
      send_to_char( "Unacceptable room range.\r\n", ch );
      return;
   }

   if( r_lo == 0 )
      r_hi = 0;

   victim->pcdata->r_range_lo = r_lo;

   victim->pcdata->r_range_hi = r_hi;

   assign_area( victim );

   send_to_char( "Done.\r\n", ch );

   set_char_color( AT_IMMORT, victim );

   ch_printf( victim, "%s has assigned you the room vnum range %d - %d.\r\n", ch->name, r_lo, r_hi );

   assign_area( victim );  /* Put back by Thoric on 02/07/96 */

   if( !victim->pcdata->area )
   {
      bug( "rassign: assign_area failed", 0 );
      return;
   }

   if( r_lo == 0 )   /* Scryn 8/12/95 */
   {
      REMOVE_BIT( victim->pcdata->area->status, AREA_LOADED );
      SET_BIT( victim->pcdata->area->status, AREA_DELETED );
   }
   else
   {
      SET_BIT( victim->pcdata->area->status, AREA_LOADED );
      REMOVE_BIT( victim->pcdata->area->status, AREA_DELETED );
   }

   return;
}

void do_oassign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   short o_lo, o_hi;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   o_lo = atoi( arg2 );
   o_hi = atoi( arg3 );

   if( arg1[0] == STRING_NULL || o_lo < 0 || o_hi < 0 )
   {
      send_to_char( "Syntax: oassign <who> <low> <high>\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
   {
      send_to_char( "They wouldn't know what to do with an object range.\r\n", ch );
      return;
   }

   if( o_lo > o_hi )
   {
      send_to_char( "Unacceptable object range.\r\n", ch );
      return;
   }

   victim->pcdata->o_range_lo = o_lo;

   victim->pcdata->o_range_hi = o_hi;
   assign_area( victim );
   send_to_char( "Done.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has assigned you the object vnum range %d - %d.\r\n", ch->name, o_lo, o_hi );
   return;
}

void do_massign( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   short m_lo, m_hi;
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   m_lo = atoi( arg2 );
   m_hi = atoi( arg3 );

   if( arg1[0] == STRING_NULL || m_lo < 0 || m_hi < 0 )
   {
      send_to_char( "Syntax: massign <who> <low> <high>\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg1 ) ) == NULL )
   {
      send_to_char( "They don't seem to be around.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) || get_trust( victim ) < LEVEL_SAVIOR )
   {
      send_to_char( "They wouldn't know what to do with a monster range.\r\n", ch );
      return;
   }

   if( m_lo > m_hi )
   {
      send_to_char( "Unacceptable monster range.\r\n", ch );
      return;
   }

   victim->pcdata->m_range_lo = m_lo;

   victim->pcdata->m_range_hi = m_hi;
   assign_area( victim );
   send_to_char( "Done.\r\n", ch );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has assigned you the monster vnum range %d - %d.\r\n", ch->name, m_lo, m_hi );
   return;
}

void do_cmdtable( CHAR_DATA * ch, char *argument )
{
   int hash, cnt;
   CMDTYPE *cmd;
   char arg[MAX_INPUT_LENGTH];
   one_argument( argument, arg );

   if( strcmp( arg, "lag" ) ) /* display normal command table */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands and Number of Uses This Run\r\n", ch );
      set_pager_color( AT_PLAIN, ch );

      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->userec.num_uses );
            else
               pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->userec.num_uses );
         }

      send_to_char( "\r\n", ch );
   }
   else  /* display commands causing lag */
   {
      set_pager_color( AT_IMMORT, ch );
      send_to_pager( "Commands that have caused lag this run\r\n", ch );
      set_pager_color( AT_PLAIN, ch );

      for( cnt = hash = 0; hash < 126; hash++ )
         for( cmd = command_hash[hash]; cmd; cmd = cmd->next )
         {
            if( !cmd->lag_count )
               continue;
            else if( ( ++cnt ) % 4 )
               pager_printf( ch, "%-6.6s %4d\t", cmd->name, cmd->lag_count );
            else
               pager_printf( ch, "%-6.6s %4d\r\n", cmd->name, cmd->lag_count );
         }

      send_to_char( "\r\n", ch );
   }

   return;
}

void do_mortalize( CHAR_DATA * ch, char *argument )
{
   char fname[1024];
   char name[256];

   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   AREA_DATA *pArea;
   int sn;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, name );

   if( name[0] == STRING_NULL )
   {
      send_to_char( "Usage: mortalize <playername>\r\n", ch );
      return;
   }

   name[0] = UPPER( name[0] );

   sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );

   if( stat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );
      loaded = load_char_obj( d, name, FALSE, FALSE );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );

      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\r\n", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }

      d->character->desc = NULL;

      victim = d->character;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      victim->level = LEVEL_AVATAR;
      victim->exp = exp_level( victim, 50 );
      victim->max_hit = 800;
      victim->max_mana = 800;
      victim->max_move = 800;

      for( sn = 0; sn < top_sn; sn++ )
         victim->pcdata->learned[sn] = 0;

      victim->practice = 0;

      victim->hit = victim->max_hit;

      victim->mana = victim->max_mana;

      victim->move = victim->max_move;

      advance_level( victim );

      DISPOSE( victim->pcdata->rank );

      victim->pcdata->rank = str_dup( "" );

      if( xIS_SET( victim->act, PLR_WIZINVIS ) )
         victim->pcdata->wizinvis = victim->trust;

      if( xIS_SET( victim->act, PLR_WIZINVIS ) && ( victim->level <= LEVEL_AVATAR ) )
      {
         xREMOVE_BIT( victim->act, PLR_WIZINVIS );
         victim->pcdata->wizinvis = victim->trust;
      }

      sprintf( buf, "%s%s", GOD_DIR, capitalize( victim->name ) );

      if( !remove( buf ) )
         send_to_char( "Player's immortal data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric\r\n", errno, strerror( errno ) );
         sprintf( buf2, "%s mortalizing %s", ch->name, buf );
         perror( buf2 );
      }

      sprintf( buf2, "%s.are", capitalize( argument ) );

      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !strcmp( pArea->filename, buf2 ) )
         {
            sprintf( buf, "%s%s", BUILD_DIR, buf2 );

            if( IS_SET( pArea->status, AREA_LOADED ) )
               fold_area( pArea, buf, FALSE );

            close_area( pArea );

            sprintf( buf2, "%s.bak", buf );

            set_char_color( AT_RED, ch );

            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\r\n", errno, strerror( errno ) );
               sprintf( buf2, "%s mortalizing %s", ch->name, buf );
               perror( buf2 );
            }
         }

      make_wizlist(  );

      while( victim->first_carrying )
         extract_obj( victim->first_carrying );

      do_quit( victim, "" );

      return;
   }

   send_to_char( "No such player.\r\n", ch );

   return;
}

/*
 * Load up a player file
 */
void do_loadup( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *temp;
   char fname[1024];
   char name[256];

   struct stat fst;
   bool loaded;
   DESCRIPTOR_DATA *d;
   int old_room_vnum;
   char buf[MAX_STRING_LENGTH];
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, name );

   if( name[0] == STRING_NULL )
   {
      send_to_char( "Usage: loadup <playername>\r\n", ch );
      return;
   }

   for( temp = first_char; temp; temp = temp->next )
   {
      if( IS_NPC( temp ) )
         continue;

      if( can_see( ch, temp, TRUE ) && !str_cmp( name, temp->name ) )
         break;
   }

   if( temp != NULL )
   {
      send_to_char( "They are already playing.\r\n", ch );
      return;
   }

   name[0] = UPPER( name[0] );

   sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   /*
    * Bug fix here provided by Senir to stop /dev/null crash
    */

   if( stat( fname, &fst ) == -1 || !check_parse_name( capitalize( name ), FALSE ) )
   {
      send_to_char( "&YNo such player exists.\r\n", ch );
      return;
   }

   if( check_parse_name( name, TRUE ) && lstat( fname, &fst ) != -1 )
   {
      CREATE( d, DESCRIPTOR_DATA, 1 );
      d->next = NULL;
      d->prev = NULL;
      d->connected = CON_GET_NAME;
      d->outsize = 2000;
      CREATE( d->outbuf, char, d->outsize );
      loaded = load_char_obj( d, name, FALSE, FALSE );
      add_char( d->character );
      old_room_vnum = d->character->in_room->vnum;
      char_to_room( d->character, ch->in_room );

      if( get_trust( d->character ) >= get_trust( ch ) )
      {
         do_say( d->character, "Do *NOT* disturb me again!" );
         send_to_char( "I think you'd better leave that player alone!\r\n", ch );
         d->character->desc = NULL;
         do_quit( d->character, "" );
         return;
      }

      d->character->desc = NULL;

      d->character->retran = old_room_vnum;
      d->character = NULL;
      DISPOSE( d->outbuf );
      DISPOSE( d );
      ch_printf( ch, "Player %s loaded from room %d.\r\n", capitalize( name ), old_room_vnum );
      sprintf( buf, "%s appears from nowhere, eyes glazed over.\r\n", capitalize( name ) );
      act( AT_IMMORT, buf, ch, NULL, NULL, TO_ROOM );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * else no player file
    */
   send_to_char( "No such player.\r\n", ch );

   return;
}

void do_fixchar( CHAR_DATA * ch, char *argument )
{
   char name[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   set_char_color( AT_IMMORT, ch );
   one_argument( argument, name );

   if( name[0] == STRING_NULL )
   {
      send_to_char( "Usage: fixchar <playername>\r\n", ch );
      return;
   }

   victim = get_char_room( ch, name );

   if( !victim )
   {
      send_to_char( "They're not here.\r\n", ch );
      return;
   }

   fix_char( victim );
   send_to_char( "Done.\r\n", ch );
}


/*
 * Extract area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "joe.are susan.are"
 * - Gorog
 */
void extract_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;
   *out = STRING_NULL;

   while( inp && *inp )
   {
      inp = one_argument( inp, buf );

      if( ( len = strlen( buf ) ) >= 5 && !strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );

         strcat( out, buf );
      }
   }
}

/*
 * Remove area names from "input" string and place result in "output" string
 * e.g. "aset joe.are sedit susan.are cset" --> "aset sedit cset"
 * - Gorog
 */
void remove_area_names( char *inp, char *out )
{
   char buf[MAX_INPUT_LENGTH], *pbuf = buf;
   int len;
   *out = STRING_NULL;

   while( inp && *inp )
   {
      inp = one_argument( inp, buf );

      if( ( len = strlen( buf ) ) < 5 || strcmp( ".are", pbuf + len - 4 ) )
      {
         if( *out )
            strcat( out, " " );

         strcat( out, buf );
      }
   }
}

/*
 * Allows members of the Area Council to add Area names to the bestow field.
 * Area names mus end with ".are" so that no commands can be bestowed.
 */
void do_bestowarea( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   int arg_len;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "Syntax:\r\n"
                    "bestowarea <victim> <filename>.are\r\n"
                    "bestowarea <victim> none             removes bestowed areas\r\n"
                    "bestowarea <victim> list             lists bestowed areas\r\n" "bestowarea <victim>                  lists bestowed areas\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }

   if( get_trust( victim ) < LEVEL_IMMORTAL )
   {
      send_to_char( "They aren't an immortal.\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( !*argument || !str_cmp( argument, "list" ) )
   {
      extract_area_names( victim->pcdata->bestowments, buf );
      ch_printf( ch, "Bestowed areas: %s\r\n", buf );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      remove_area_names( victim->pcdata->bestowments, buf );
      smash_tilde( buf );
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( buf );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   arg_len = strlen( argument );

   if( arg_len < 5 || argument[arg_len - 4] != '.' || argument[arg_len - 3] != 'a' || argument[arg_len - 2] != 'r' || argument[arg_len - 1] != 'e' )
   {
      send_to_char( "You can only bestow an area name\r\n", ch );
      send_to_char( "E.G. bestow joe sam.are\r\n", ch );
      return;
   }

   sprintf( buf, "%s %s", victim->pcdata->bestowments, argument );

   DISPOSE( victim->pcdata->bestowments );
   victim->pcdata->bestowments = str_dup( buf );
   set_char_color( AT_IMMORT, victim );
   ch_printf( victim, "%s has bestowed on you the area: %s\r\n", ch->name, argument );
   send_to_char( "Done.\r\n", ch );
}

void do_bestow( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH], buf[MAX_STRING_LENGTH], arg_buf[MAX_STRING_LENGTH];
   CHAR_DATA *victim;
   CMDTYPE *cmd;
   bool fComm = FALSE;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Bestow whom with what?\r\n", ch );
      return;
   }

   if( ( victim = get_char_world( ch, arg ) ) == NULL )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't give special abilities to a mob!\r\n", ch );
      return;
   }

   if( victim == ch || get_trust( victim ) >= get_trust( ch ) )
   {
      send_to_char( "You aren't powerful enough...\r\n", ch );
      return;
   }

   if( !victim->pcdata->bestowments )
      victim->pcdata->bestowments = str_dup( "" );

   if( argument[0] == STRING_NULL || !str_cmp( argument, "show list" ) )
   {
      ch_printf( ch, "Current bestowed commands on %s: %s.\r\n", victim->name, victim->pcdata->bestowments );
      return;
   }

   if( !str_cmp( argument, "none" ) )
   {
      DISPOSE( victim->pcdata->bestowments );
      victim->pcdata->bestowments = str_dup( "" );
      ch_printf( ch, "Bestowments removed from %s.\r\n", victim->name );
      ch_printf( victim, "%s has removed your bestowed commands.\r\n", ch->name );
      return;
   }

   arg_buf[0] = STRING_NULL;

   argument = one_argument( argument, arg );

   while( arg[0] != STRING_NULL )
   {
      char *cmd_buf, cmd_tmp[MAX_INPUT_LENGTH];
      bool cFound = FALSE;

      if( !( cmd = find_command( arg ) ) )
      {
         ch_printf( ch, "No such command as %s!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }
      else if( cmd->level > get_trust( ch ) )
      {
         ch_printf( ch, "You can't bestow the %s command!\r\n", arg );
         argument = one_argument( argument, arg );
         continue;
      }

      cmd_buf = victim->pcdata->bestowments;

      cmd_buf = one_argument( cmd_buf, cmd_tmp );

      while( cmd_tmp[0] != STRING_NULL )
      {
         if( !str_cmp( cmd_tmp, arg ) )
         {
            cFound = TRUE;
            break;
         }

         cmd_buf = one_argument( cmd_buf, cmd_tmp );
      }

      if( cFound == TRUE )
      {
         argument = one_argument( argument, arg );
         continue;
      }

      sprintf( arg, "%s ", arg );

      strcat( arg_buf, arg );
      argument = one_argument( argument, arg );
      fComm = TRUE;
   }

   if( !fComm )
   {
      send_to_char( "Good job, knucklehead... you just bestowed them with that master command called 'NOTHING!'\r\n", ch );
      return;
   }

   if( arg_buf[strlen( arg_buf ) - 1] == ' ' )
      arg_buf[strlen( arg_buf ) - 1] = STRING_NULL;

   sprintf( buf, "%s %s", victim->pcdata->bestowments, arg_buf );

   DISPOSE( victim->pcdata->bestowments );

   smash_tilde( buf );

   victim->pcdata->bestowments = str_dup( buf );

   set_char_color( AT_IMMORT, victim );

   ch_printf( victim, "%s has bestowed on you the command(s): %s\r\n", ch->name, arg_buf );

   send_to_char( "Done.\r\n", ch );
}

struct tm *update_time( struct tm *old_time )
{
   time_t sttime;
   sttime = mktime( old_time );
   return localtime( &sttime );
}

/* 
 * Updated to SHA256 - 
 */
void do_form_password( CHAR_DATA *ch, char *argument )
{
   char *pwcheck;

   set_char_color( AT_IMMORT, ch );

   if( !argument || argument[0] == '\0' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      return;
   }

   /*
    * This is arbitrary to discourage weak passwords 
    */
   if( strlen( argument ) < 5 )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password must be at least 5 characters in length.\r\n", ch );
      return;
   }

   if( argument[0] == '!' )
   {
      send_to_char( "Usage: formpass <password>\r\n", ch );
      send_to_char( "New password cannot begin with the '!' character.\r\n", ch );
      return;
   }

   pwcheck = sha256_crypt( argument );
   ch_printf( ch, "%s results in the encrypted string: %s\r\n", argument, pwcheck );
   return;
}

/*
 * Purge a player file.  No more player.  -- Altrag
 */
void do_destro( CHAR_DATA * ch, char *argument )
{
   set_char_color( AT_RED, ch );
   send_to_char( "If you want to destroy a character, spell it out!\r\n", ch );
   return;
}

/*
 * This could have other applications too.. move if needed. -- Altrag
 */
void close_area( AREA_DATA * pArea )
{
   CHAR_DATA *ech, *ech_next;
   OBJ_DATA *eobj, *eobj_next;
   ROOM_INDEX_DATA *rid, *rid_next;
   OBJ_INDEX_DATA *oid, *oid_next;
   MOB_INDEX_DATA *mid, *mid_next;
   NEIGHBOR_DATA *neighbor, *neighbor_next;
   int icnt;

   for( ech = first_char; ech; ech = ech_next )
   {
      ech_next = ech->next;

      if( ech->fighting )
         stop_fighting( ech, TRUE );

      if( IS_NPC( ech ) )
      {
         /*
          * if mob is in area, or part of area.
          */
         if( URANGE( pArea->low_m_vnum, ech->pIndexData->vnum, pArea->hi_m_vnum ) == ech->pIndexData->vnum || ( ech->in_room && ech->in_room->area == pArea ) )
            extract_char( ech, TRUE );

         continue;
      }

      if( ech->in_room && ech->in_room->area == pArea )
         do_recall( ech, "" );
   }

   for( eobj = first_object; eobj; eobj = eobj_next )
   {
      eobj_next = eobj->next;
      /*
       * if obj is in area, or part of area.
       */

      if( URANGE( pArea->low_o_vnum, eobj->pIndexData->vnum, pArea->hi_o_vnum ) == eobj->pIndexData->vnum || ( eobj->in_room && eobj->in_room->area == pArea ) )
         extract_obj( eobj );
   }

   for( icnt = 0; icnt < MAX_KEY_HASH; icnt++ )
   {
      for( rid = room_index_hash[icnt]; rid; rid = rid_next )
      {
         rid_next = rid->next;

         if( rid->area != pArea )
            continue;

         delete_room( rid );
      }

      for( mid = mob_index_hash[icnt]; mid; mid = mid_next )
      {
         mid_next = mid->next;

         if( mid->vnum < pArea->low_m_vnum || mid->vnum > pArea->hi_m_vnum )
            continue;

         delete_mob( mid );
      }

      for( oid = obj_index_hash[icnt]; oid; oid = oid_next )
      {
         oid_next = oid->next;

         if( oid->vnum < pArea->low_o_vnum || oid->vnum > pArea->hi_o_vnum )
            continue;

         delete_obj( oid );
      }
   }

   if( pArea->weather )
   {
      for( neighbor = pArea->weather->first_neighbor; neighbor; neighbor = neighbor_next )
      {
         neighbor_next = neighbor->next;
         UNLINK( neighbor, pArea->weather->first_neighbor, pArea->weather->last_neighbor, next, prev );
         STRFREE( neighbor->name );
         DISPOSE( neighbor );
      }

      DISPOSE( pArea->weather );
   }

   DISPOSE( pArea->name );

   DISPOSE( pArea->filename );
   DISPOSE( pArea->resetmsg );
   STRFREE( pArea->author );
   UNLINK( pArea, first_area, last_area, next, prev );
   UNLINK( pArea, first_build, last_build, next, prev );
   UNLINK( pArea, first_asort, last_asort, next_sort, prev_sort );
   UNLINK( pArea, first_bsort, last_bsort, next_sort, prev_sort );
   DISPOSE( pArea );
}

void do_destroy( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char buf[MAX_STRING_LENGTH];
   char buf2[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char *name;

   struct stat fst;
   set_char_color( AT_RED, ch );
   one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Destroy what player file?\r\n", ch );
      return;
   }

   /*
    * Set the file points.
    */
   name = capitalize( arg );

   sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );

   sprintf( buf2, "%s%c/%s", BACKUP_DIR, tolower( arg[0] ), name );

   /*
    * This check makes sure the name is valid and that the file is there, else there
    * is no need to go on. -Orion
    */
   if( !check_parse_name( name, TRUE ) || lstat( buf, &fst ) == -1 )
   {
      ch_printf( ch, "No player exists by the name %s.\r\n", name );
      return;
   }

   for( victim = first_char; victim; victim = victim->next )
      if( !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
         break;

   if( !victim )
   {
      DESCRIPTOR_DATA *d;
      /*
       * Make sure they aren't halfway logged in.
       */

      for( d = first_descriptor; d; d = d->next )
         if( ( victim = d->character ) && !IS_NPC( victim ) && !str_cmp( victim->name, arg ) )
            break;

      if( d )
         close_socket( d, TRUE );
   }
   else
   {
      int x, y;
      quitting_char = victim;
      save_char_obj( victim );
      saving_char = NULL;
      extract_char( victim, TRUE );

      for( x = 0; x < MAX_WEAR; x++ )
         for( y = 0; y < MAX_LAYERS; y++ )
            save_equipment[x][y] = NULL;
   }

   if( !rename( buf, buf2 ) )
   {
      AREA_DATA *pArea;
      set_char_color( AT_RED, ch );
      ch_printf( ch, "Player %s destroyed.  Pfile saved in backup directory.\r\n", name );
      sprintf( buf, "%s%s", HOUSE_DIR, name );

      if( !remove( buf ) )
         send_to_char( "Player's housing data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (housing data)." "  Report to Coder.\r\n", errno, strerror( errno ) );
         sprintf( buf2, "%s destroying %s", ch->name, buf );
         perror( buf2 );
      }

      sprintf( buf, "%s%s", GOD_DIR, name );

      if( !remove( buf ) )
         send_to_char( "Player's immortal data destroyed.\r\n", ch );
      else if( errno != ENOENT )
      {
         ch_printf( ch, "Unknown error #%d - %s (immortal data).  Report to Thoric.\r\n", errno, strerror( errno ) );
         sprintf( buf2, "%s destroying %s", ch->name, buf );
         perror( buf2 );
      }

      sprintf( buf2, "%s.are", name );

      for( pArea = first_build; pArea; pArea = pArea->next )
         if( !str_cmp( pArea->filename, buf2 ) )
         {
            sprintf( buf, "%s%s", BUILD_DIR, buf2 );

            if( IS_SET( pArea->status, AREA_LOADED ) )
            {
               fold_area( pArea, buf, FALSE );
               close_area( pArea );
            }

            sprintf( buf2, "%s.bak", buf );

            set_char_color( AT_RED, ch ); /* Log message changes colors */

            if( !rename( buf, buf2 ) )
               send_to_char( "Player's area data destroyed.  Area saved as backup.\r\n", ch );
            else if( errno != ENOENT )
            {
               ch_printf( ch, "Unknown error #%d - %s (area data).  Report to Thoric.\r\n", errno, strerror( errno ) );
               sprintf( buf2, "%s destroying %s", ch->name, buf );
               perror( buf2 );
            }

            break;
         }
   }
   else if( errno == ENOENT )
   {
      set_char_color( AT_PLAIN, ch );
      send_to_char( "Player does not exist.\r\n", ch );
   }
   else
   {
      set_char_color( AT_WHITE, ch );
      ch_printf( ch, "Unknown error #%d - %s.  Report to Thoric.\r\n", errno, strerror( errno ) );
      sprintf( buf, "%s destroying %s", ch->name, arg );
      perror( buf );
   }

   return;
}

void save_sysdata args( ( SYSTEM_DATA sys ) );
void do_cset( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_STRING_LENGTH];
   short level;
   set_pager_color( AT_PLAIN, ch );

   if( argument[0] == STRING_NULL )
   {
      pager_printf_color( ch, "\r\n&WMud_name: %s", sysdata.mud_name );
      pager_printf_color( ch,
                          "\r\n&WMail:\r\n  &wRead all mail: &W%d  &wRead mail for free: &W%d  &wWrite mail for free: &W%d\r\n",
                          sysdata.read_all_mail, sysdata.read_mail_free, sysdata.write_mail_free );
      pager_printf_color( ch, "  &wTake all mail: &W%d\r\n", sysdata.take_others_mail );
      pager_printf_color( ch, "\r\n&wPfile autocleanup status: &W%s  &wDays before purging newbies: &W%d\r\n", sysdata.CLEANPFILES ? "On" : "Off", sysdata.newbie_purge );
      pager_printf_color( ch, "&wDays before purging regular players: &W%d\r\n", sysdata.regular_purge );
      pager_printf_color( ch, "&WChannels:\r\n  &wMuse: &W%d   &wThink: &W%d   &wLog: &W%d   &wBuild: &W%d\r\n",
                          sysdata.muse_level, sysdata.think_level, sysdata.log_level, sysdata.build_level );
      pager_printf_color( ch, "&WBuilding:\r\n  &wPrototype modification: &W%d  &wPlayer msetting: &W%d\r\n", sysdata.level_modify_proto, sysdata.level_mset_player );
      pager_printf_color( ch, "&WGuilds:\r\n  &wOverseer: &W%s   &wAdvisor: &W%s\r\n", sysdata.guild_overseer, sysdata.guild_advisor );
      pager_printf_color( ch, "&WBan Data:\r\n  &wBan Site Level: &W%d   &wBan Class Level: &W%d   ", sysdata.ban_site_level, sysdata.ban_Class_level );
      pager_printf_color( ch, "&wBan Race Level: &W%d\r\n", sysdata.ban_race_level );
      pager_printf_color( ch, "&WDefenses:\r\n  &wDodge_mod: &W%d    &wParry_mod: &W%d    &wTumble_mod: &W%d\r\n", sysdata.dodge_mod, sysdata.parry_mod, sysdata.tumble_mod );
      pager_printf_color( ch, "&WOther:\r\n  &wForce on players:             &W%-2d     ", sysdata.level_forcepc );
      pager_printf_color( ch, "&wPrivate room override:         &W%-2d\r\n", sysdata.level_override_private );
      pager_printf_color( ch, "  &wPenalty to bash plr vs. plr:  &W%-7d", sysdata.bash_plr_vs_plr );
      pager_printf_color( ch, "&wPenalty to non-tank bash:      &W%-3d\r\n", sysdata.bash_nontank );
      pager_printf_color( ch, "  &wPenalty to gouge plr vs. plr: &W%-7d", sysdata.gouge_plr_vs_plr );
      pager_printf_color( ch, "&wPenalty to non-tank gouge:     &W%-3d\r\n", sysdata.gouge_nontank );
      pager_printf_color( ch, "  &wPenalty regular stun chance:  &W%-7d", sysdata.stun_regular );
      pager_printf_color( ch, "&wPenalty to stun plr vs. plr:   &W%-3d\r\n", sysdata.stun_plr_vs_plr );
      pager_printf_color( ch, "  &wPercent damage plr vs. plr:   &W%-7d", sysdata.dam_plr_vs_plr );
      pager_printf_color( ch, "&wPercent damage plr vs. mob:    &W%-3d \r\n", sysdata.dam_plr_vs_mob );
      pager_printf_color( ch, "  &wPercent damage mob vs. plr:   &W%-7d", sysdata.dam_mob_vs_plr );
      pager_printf_color( ch, "&wPercent damage mob vs. mob:    &W%-3d\r\n", sysdata.dam_mob_vs_mob );
      pager_printf_color( ch, "  &wGet object without take flag: &W%-7d", sysdata.level_getobjnotake );
      pager_printf_color( ch, "&wAutosave frequency (minutes):  &W%d\r\n", sysdata.save_frequency );
      pager_printf_color( ch, "  &wMax level difference bestow:  &W%-7d", sysdata.bestow_dif );
      pager_printf_color( ch, "&wSaving Pets is:                &W%s\r\n", ( sysdata.save_pets ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wPkill looting is:             &W%s\r\n", ( sysdata.pk_loot ) ? "ON" : "off" );
      pager_printf_color( ch, "  &wSave flags: &W%s\r\n", flag_string( sysdata.save_flags, save_flag ) );
      pager_printf_color( ch, "  &wIdents retries: &W%d\r\n", sysdata.ident_retries );
      return;
   }

   argument = one_argument( argument, arg );

   smash_tilde( argument );

   if( !str_cmp( arg, "help" ) )
   {
      do_help( ch, "controls" );
      return;
   }

   if( !str_cmp( arg, "pfiles" ) )
   {
      sysdata.CLEANPFILES = !sysdata.CLEANPFILES;

      if( sysdata.CLEANPFILES )
         send_to_char( "Pfile autocleanup enabled.\r\n", ch );
      else
         send_to_char( "Pfile autocleanup disabled.\r\n", ch );

      return;
   }

   if( !str_cmp( arg, "save" ) )
   {
      save_sysdata( sysdata );
      send_to_char( "Cset functions saved.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "mudname" ) )
   {
      if( sysdata.mud_name )
         DISPOSE( sysdata.mud_name );

      sysdata.mud_name = str_dup( argument );

      send_to_char( "Name set.\r\n", ch );

      return;
   }

   if( !str_cmp( arg, "saveflag" ) )
   {
      int x = get_saveflag( argument );

      if( x == -1 )
         send_to_char( "Not a save flag.\r\n", ch );
      else
      {
         TOGGLE_BIT( sysdata.save_flags, 1 << x );
         send_to_char( "Ok.\r\n", ch );
      }

      return;
   }

   if( !str_prefix( arg, "guild_overseer" ) )
   {
      STRFREE( sysdata.guild_overseer );
      sysdata.guild_overseer = STRALLOC( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_prefix( arg, "guild_advisor" ) )
   {
      STRFREE( sysdata.guild_advisor );
      sysdata.guild_advisor = STRALLOC( argument );
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   level = ( short )atoi( argument );

   if( !str_prefix( arg, "savefrequency" ) )
   {
      sysdata.save_frequency = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "newbie_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\r\n", ch );
         return;
      }

      sysdata.newbie_purge = level;

      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "regular_purge" ) )
   {
      if( level < 1 )
      {
         send_to_char( "You must specify a period of at least 1 day.\r\n", ch );
         return;
      }

      sysdata.regular_purge = level;

      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "bash_pvp" ) )
   {
      sysdata.bash_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "bash_nontank" ) )
   {
      sysdata.bash_nontank = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "gouge_pvp" ) )
   {
      sysdata.gouge_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "gouge_nontank" ) )
   {
      sysdata.gouge_nontank = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dodge_mod" ) )
   {
      sysdata.dodge_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "parry_mod" ) )
   {
      sysdata.parry_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "tumble_mod" ) )
   {
      sysdata.tumble_mod = level > 0 ? level : 1;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "stun" ) )
   {
      sysdata.stun_regular = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "stun_pvp" ) )
   {
      sysdata.stun_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_pvp" ) )
   {
      sysdata.dam_plr_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "get_notake" ) )
   {
      sysdata.level_getobjnotake = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_pvm" ) )
   {
      sysdata.dam_plr_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_mvp" ) )
   {
      sysdata.dam_mob_vs_plr = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "dam_mvm" ) )
   {
      sysdata.dam_mob_vs_mob = level;
      send_to_char( "Ok.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "ident_retries" ) || !str_cmp( arg, "ident" ) )
   {
      sysdata.ident_retries = level;

      if( level > 20 )
         send_to_char( "Caution:  This setting may cause the game to lag.\r\n", ch );
      else if( level <= 0 )
         send_to_char( "Ident lookups turned off.\r\n", ch );
      else
         send_to_char( "Ok.\r\n", ch );

      return;
   }

   if( level < 0 || level > MAX_LEVEL )
   {
      send_to_char( "Invalid value for new control.\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "read_all" ) )
      sysdata.read_all_mail = level;
   else if( !str_cmp( arg, "read_free" ) )
      sysdata.read_mail_free = level;
   else if( !str_cmp( arg, "write_free" ) )
      sysdata.write_mail_free = level;
   else if( !str_cmp( arg, "take_all" ) )
      sysdata.take_others_mail = level;
   else if( !str_cmp( arg, "muse" ) )
      sysdata.muse_level = level;
   else if( !str_cmp( arg, "think" ) )
      sysdata.think_level = level;
   else if( !str_cmp( arg, "log" ) )
      sysdata.log_level = level;
   else if( !str_cmp( arg, "build" ) )
      sysdata.build_level = level;
   else if( !str_cmp( arg, "proto_modify" ) )
      sysdata.level_modify_proto = level;
   else if( !str_cmp( arg, "override_private" ) )
      sysdata.level_override_private = level;
   else if( !str_cmp( arg, "bestow_dif" ) )
      sysdata.bestow_dif = level > 0 ? level : 1;
   else if( !str_cmp( arg, "forcepc" ) )
      sysdata.level_forcepc = level;
   else if( !str_cmp( arg, "ban_site_level" ) )
      sysdata.ban_site_level = level;
   else if( !str_cmp( arg, "ban_race_level" ) )
      sysdata.ban_race_level = level;
   else if( !str_cmp( arg, "ban_Class_level" ) )
      sysdata.ban_Class_level = level;
   else if( !str_cmp( arg, "petsave" ) )
   {
      if( level )
         sysdata.save_pets = TRUE;
      else
         sysdata.save_pets = FALSE;
   }
   else if( !str_cmp( arg, "pk_loot" ) )
   {
      if( level )
      {
         send_to_char( "Pkill looting is enabled.\r\n", ch );
         sysdata.pk_loot = TRUE;
      }
      else
      {
         send_to_char( "Pkill looting is disabled.  (use cset pkloot 1 to enable)\r\n", ch );
         sysdata.pk_loot = FALSE;
      }
   }
   else if( !str_cmp( arg, "mset_player" ) )
      sysdata.level_mset_player = level;
   else
   {
      send_to_char( "Invalid argument.\r\n", ch );
      return;
   }

   send_to_char( "Ok.\r\n", ch );

   return;
}

void do_orange( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YFunction under construction.\r\n", ch );
   return;
}

void do_mrange( CHAR_DATA * ch, char *argument )
{
   send_to_char_color( "&YFunction under construction.\r\n", ch );
   return;
}

void do_hell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   short htime;
   bool h_d = FALSE;

   struct tm *tms;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "Hell who, and for how long?\r\n", ch );
      return;
   }

   if( !( victim = get_char_world( ch, arg ) ) || IS_NPC( victim ) )
   {
      send_to_char( "They aren't here.\r\n", ch );
      return;
   }

   if( IS_IMMORTAL( victim ) )
   {
      send_to_char( "There is no point in helling an immortal.\r\n", ch );
      return;
   }

   if( victim->pcdata->release_date != 0 )
   {
      ch_printf( ch, "They are already in hell until %24.24s, by %s.\r\n", ctime( &victim->pcdata->release_date ), victim->pcdata->helled_by );
      return;
   }

   argument = one_argument( argument, arg );

   if( !*arg || !is_number( arg ) )
   {
      send_to_char( "Hell them for how long?\r\n", ch );
      return;
   }

   htime = atoi( arg );

   if( htime <= 0 )
   {
      send_to_char( "You cannot hell for zero or negative time.\r\n", ch );
      return;
   }

   argument = one_argument( argument, arg );

   if( !*arg || !str_cmp( arg, "hours" ) )
      h_d = TRUE;
   else if( str_cmp( arg, "days" ) )
   {
      send_to_char( "Is that value in hours or days?\r\n", ch );
      return;
   }
   else if( htime > 30 )
   {
      send_to_char( "You may not hell a person for more than 30 days at a time.\r\n", ch );
      return;
   }

   tms = localtime( &current_time );

   if( h_d )
      tms->tm_hour += htime;
   else
      tms->tm_mday += htime;

   victim->pcdata->release_date = mktime( tms );

   victim->pcdata->helled_by = STRALLOC( ch->name );

   ch_printf( ch, "%s will be released from hell at %24.24s.\r\n", victim->name, ctime( &victim->pcdata->release_date ) );

   act( AT_MAGIC, "$n disappears in a cloud of hellish light.", victim, NULL, ch, TO_NOTVICT );

   char_from_room( victim );

   char_to_room( victim, get_room_index( ROOM_VNUM_HELL ) );

   act( AT_MAGIC, "$n appears in a could of hellish light.", victim, NULL, ch, TO_NOTVICT );

   do_look( victim, "auto" );

   ch_printf( victim, "The immortals are not pleased with your actions.\r\n"
              "You shall remain in hell for %d %s%s.\r\n", htime, ( h_d ? "hour" : "day" ), ( htime == 1 ? "" : "s" ) );

   save_char_obj( victim );   /* used to save ch, fixed by Thoric 09/17/96 */

   return;
}

void do_unhell( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg[MAX_INPUT_LENGTH];
   ROOM_INDEX_DATA *location;
   set_char_color( AT_IMMORT, ch );
   argument = one_argument( argument, arg );

   if( !*arg )
   {
      send_to_char( "Unhell whom..?\r\n", ch );
      return;
   }

   location = ch->in_room;

   /*
    * ch->in_room = get_room_index(ROOM_VNUM_HELL);
    */
   victim = get_char_world( ch, arg );
   /*
    * ch->in_room = location;          The case of unhell self, etc.
    */

   if( !victim || IS_NPC( victim ) )
   {
      send_to_char( "No such player character present.\r\n", ch );
      return;
   }

   if( victim->in_room->vnum != ROOM_VNUM_HELL )
   {
      send_to_char( "No one like that is in hell.\r\n", ch );
      return;
   }

   if( victim->pcdata->clan )
      location = get_room_index( victim->pcdata->clan->recall );
   else
      location = get_room_index( ROOM_VNUM_TEMPLE );

   if( !location )
      location = ch->in_room;

   MOBtrigger = FALSE;

   act( AT_MAGIC, "$n disappears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );

   char_from_room( victim );

   char_to_room( victim, location );

   send_to_char( "The gods have smiled on you and released you from hell early!\r\n", victim );

   do_look( victim, "auto" );

   if( victim != ch )
      send_to_char( "They have been released.\r\n", ch );

   if( victim->pcdata->helled_by )
   {
      if( str_cmp( ch->name, victim->pcdata->helled_by ) )
         ch_printf( ch, "(You should probably write a note to %s, explaining the early release.)\r\n", victim->pcdata->helled_by );

      STRFREE( victim->pcdata->helled_by );

      victim->pcdata->helled_by = NULL;
   }

   MOBtrigger = FALSE;

   act( AT_MAGIC, "$n appears in a cloud of godly light.", victim, NULL, ch, TO_NOTVICT );
   victim->pcdata->release_date = 0;
   save_char_obj( victim );
   return;
}

/*
 * Free a social structure     -Thoric
 */
void free_social( SOCIALTYPE * social )
{
   if( social->name )
      DISPOSE( social->name );

   if( social->char_no_arg )
      DISPOSE( social->char_no_arg );

   if( social->others_no_arg )
      DISPOSE( social->others_no_arg );

   if( social->char_found )
      DISPOSE( social->char_found );

   if( social->others_found )
      DISPOSE( social->others_found );

   if( social->vict_found )
      DISPOSE( social->vict_found );

   if( social->char_auto )
      DISPOSE( social->char_auto );

   if( social->others_auto )
      DISPOSE( social->others_auto );

   DISPOSE( social );
}

/*
 * Remove a social from it's hash index    -Thoric
 */
void unlink_social( SOCIALTYPE * social )
{
   SOCIALTYPE *tmp, *tmp_next;
   int hash;

   if( !social )
   {
      bug( "Unlink_social: NULL social", 0 );
      return;
   }

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( social == ( tmp = social_index[hash] ) )
   {
      social_index[hash] = tmp->next;
      return;
   }

   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;

      if( social == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a social to the social index table   -Thoric
 * Hashed and insert sorted
 */
void add_social( SOCIALTYPE * social )
{
   int hash, x;
   SOCIALTYPE *tmp, *prev;

   if( !social )
   {
      bug( "Add_social: NULL social", 0 );
      return;
   }

   if( !social->name )
   {
      bug( "Add_social: NULL social->name", 0 );
      return;
   }

   if( !social->char_no_arg )
   {
      bug( "Add_social: NULL social->char_no_arg", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase
    */
   for( x = 0; social->name[x] != STRING_NULL; x++ )
      social->name[x] = LOWER( social->name[x] );

   if( social->name[0] < 'a' || social->name[0] > 'z' )
      hash = 0;
   else
      hash = ( social->name[0] - 'a' ) + 1;

   if( ( prev = tmp = social_index[hash] ) == NULL )
   {
      social->next = social_index[hash];
      social_index[hash] = social;
      return;
   }

   for( ; tmp; tmp = tmp->next )
   {
      if( ( x = strcmp( social->name, tmp->name ) ) == 0 )
      {
         bug( "Add_social: trying to add duplicate name to bucket %d", hash );
         free_social( social );
         return;
      }
      else if( x < 0 )
      {
         if( tmp == social_index[hash] )
         {
            social->next = social_index[hash];
            social_index[hash] = social;
            return;
         }

         prev->next = social;

         social->next = tmp;
         return;
      }

      prev = tmp;
   }

   /*
    * add to end
    */
   prev->next = social;

   social->next = NULL;

   return;
}

/*
 * Social editor/displayer/save/delete    -Thoric
 */
void do_sedit( CHAR_DATA * ch, char *argument )
{
   SOCIALTYPE *social;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   set_char_color( AT_SOCIAL, ch );
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: sedit <social> [field]\r\n", ch );
      send_to_char( "Syntax: sedit <social> create\r\n", ch );

      if( get_trust( ch ) > LEVEL_GOD )
         send_to_char( "Syntax: sedit <social> delete\r\n", ch );

      if( get_trust( ch ) > LEVEL_LESSER )
         send_to_char( "Syntax: sedit <save>\r\n", ch );

      send_to_char( "\r\nField being one of:\r\n", ch );

      send_to_char( "  cnoarg onoarg cfound ofound vfound cauto oauto\r\n", ch );

      return;
   }

   if( get_trust( ch ) > LEVEL_NEOPHYTE && !str_cmp( arg1, "save" ) )
   {
      save_socials(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   social = find_social( arg1 );

   if( !str_cmp( arg2, "create" ) )
   {
      if( social )
      {
         send_to_char( "That social already exists!\r\n", ch );
         return;
      }

      CREATE( social, SOCIALTYPE, 1 );

      social->name = str_dup( arg1 );
      sprintf( arg2, "You %s.", arg1 );
      social->char_no_arg = str_dup( arg2 );
      add_social( social );
      send_to_char( "Social added.\r\n", ch );
      return;
   }

   if( !social )
   {
      send_to_char( "Social not found.\r\n", ch );
      return;
   }

   if( arg2[0] == STRING_NULL || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Social: %s\r\n\r\nCNoArg: %s\r\n", social->name, social->char_no_arg );
      ch_printf( ch, "ONoArg: %s\r\nCFound: %s\r\nOFound: %s\r\n",
                 social->others_no_arg ? social->others_no_arg : "(not set)",
                 social->char_found ? social->char_found : "(not set)", social->others_found ? social->others_found : "(not set)" );
      ch_printf( ch, "VFound: %s\r\nCAuto : %s\r\nOAuto : %s\r\n",
                 social->vict_found ? social->vict_found : "(not set)",
                 social->char_auto ? social->char_auto : "(not set)", social->others_auto ? social->others_auto : "(not set)" );
      return;
   }

   if( get_trust( ch ) > LEVEL_GOD && !str_cmp( arg2, "delete" ) )
   {
      unlink_social( social );
      free_social( social );
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "cnoarg" ) )
   {
      if( argument[0] == STRING_NULL || !str_cmp( argument, "clear" ) )
      {
         send_to_char( "You cannot clear this field.  It must have a message.\r\n", ch );
         return;
      }

      if( social->char_no_arg )
         DISPOSE( social->char_no_arg );

      social->char_no_arg = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "onoarg" ) )
   {
      if( social->others_no_arg )
         DISPOSE( social->others_no_arg );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->others_no_arg = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "cfound" ) )
   {
      if( social->char_found )
         DISPOSE( social->char_found );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->char_found = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "ofound" ) )
   {
      if( social->others_found )
         DISPOSE( social->others_found );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->others_found = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "vfound" ) )
   {
      if( social->vict_found )
         DISPOSE( social->vict_found );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->vict_found = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "cauto" ) )
   {
      if( social->char_auto )
         DISPOSE( social->char_auto );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->char_auto = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "oauto" ) )
   {
      if( social->others_auto )
         DISPOSE( social->others_auto );

      if( argument[0] != STRING_NULL && str_cmp( argument, "clear" ) )
         social->others_auto = str_dup( argument );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      one_argument( argument, arg1 );

      if( arg1[0] == STRING_NULL )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }

      if( arg1[0] != social->name[0] )
      {
         unlink_social( social );
         relocate = TRUE;
      }
      else
         relocate = FALSE;

      if( social->name )
         DISPOSE( social->name );

      social->name = str_dup( arg1 );

      if( relocate )
         add_social( social );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   /*
    * display usage message
    */
   do_sedit( ch, "" );
}

/*
 * Free a command structure     -Thoric
 */
void free_command( CMDTYPE * command )
{
   if( command->name )
      DISPOSE( command->name );

   if( command->fun_name )
      DISPOSE( command->fun_name );

   DISPOSE( command );
}

/*
 * Remove a command from it's hash index   -Thoric
 */
void unlink_command( CMDTYPE * command )
{
   CMDTYPE *tmp, *tmp_next;
   int hash;

   if( !command )
   {
      bug( "Unlink_command NULL command", 0 );
      return;
   }

   hash = command->name[0] % 126;

   if( command == ( tmp = command_hash[hash] ) )
   {
      command_hash[hash] = tmp->next;
      return;
   }

   for( ; tmp; tmp = tmp_next )
   {
      tmp_next = tmp->next;

      if( command == tmp_next )
      {
         tmp->next = tmp_next->next;
         return;
      }
   }
}

/*
 * Add a command to the command hash table   -Thoric
 */
void add_command( CMDTYPE * command )
{
   int hash, x;
   CMDTYPE *tmp, *prev;

   if( !command )
   {
      bug( "Add_command: NULL command", 0 );
      return;
   }

   if( !command->name )
   {
      bug( "Add_command: NULL command->name", 0 );
      return;
   }

   if( !command->do_fun )
   {
      bug( "Add_command: NULL command->do_fun", 0 );
      return;
   }

   /*
    * make sure the name is all lowercase
    */
   for( x = 0; command->name[x] != STRING_NULL; x++ )
      command->name[x] = LOWER( command->name[x] );

   hash = command->name[0] % 126;

   if( ( prev = tmp = command_hash[hash] ) == NULL )
   {
      command->next = command_hash[hash];
      command_hash[hash] = command;
      return;
   }

   /*
    * add to the END of the list
    */
   for( ; tmp; tmp = tmp->next )
      if( !tmp->next )
      {
         tmp->next = command;
         command->next = NULL;
      }

   return;
}

/*
 * Command editor/displayer/save/delete    -Thoric
 * Added support for interpret flags                            -Shaddai
 */
void do_cedit( CHAR_DATA * ch, char *argument )
{
   CMDTYPE *command;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   set_char_color( AT_IMMORT, ch );
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: cedit save cmdtable\r\n", ch );

      if( get_trust( ch ) > LEVEL_SUB_IMPLEM )
      {
         send_to_char( "Syntax: cedit <command> create [code]\r\n", ch );
         send_to_char( "Syntax: cedit <command> delete\r\n", ch );
         send_to_char( "Syntax: cedit <command> show\r\n", ch );
         send_to_char( "Syntax: cedit <command> raise\r\n", ch );
         send_to_char( "Syntax: cedit <command> lower\r\n", ch );
         send_to_char( "Syntax: cedit <command> list\r\n", ch );
         send_to_char( "Syntax: cedit <command> [field]\r\n", ch );
         send_to_char( "\r\nField being one of:\r\n", ch );
         send_to_char( "  level position log code flags\r\n", ch );
      }

      return;
   }

   if( get_trust( ch ) > LEVEL_GREATER && !str_cmp( arg1, "save" ) && !str_cmp( arg2, "cmdtable" ) )
   {
      save_commands(  );
      send_to_char( "Saved.\r\n", ch );
      return;
   }

   command = find_command( arg1 );

   if( get_trust( ch ) > LEVEL_SUB_IMPLEM && !str_cmp( arg2, "create" ) )
   {
      if( command )
      {
         send_to_char( "That command already exists!\r\n", ch );
         return;
      }

      CREATE( command, CMDTYPE, 1 );

      command->lag_count = 0; /* FB */
      command->name = str_dup( arg1 );
      command->level = get_trust( ch );

      if( *argument )
         one_argument( argument, arg2 );
      else
         sprintf( arg2, "do_%s", arg1 );

      command->do_fun = skill_function( arg2 );

      command->fun_name = str_dup( arg2 );

      add_command( command );

      send_to_char( "Command added.\r\n", ch );

      if( command->do_fun == skill_notfound )
         ch_printf( ch, "Code %s not found.  Set to no code.\r\n", arg2 );

      return;
   }

   if( !command )
   {
      send_to_char( "Command not found.\r\n", ch );
      return;
   }
   else if( command->level > get_trust( ch ) )
   {
      send_to_char( "You cannot touch this command.\r\n", ch );
      return;
   }

   if( arg2[0] == STRING_NULL || !str_cmp( arg2, "show" ) )
   {
      ch_printf( ch, "Command:  %s\r\nLevel:    %d\r\nPosition: %d\r\nLog:      %d\r\nCode:     %s\r\nFlags:  %s\r\n",
                 command->name, command->level, command->position, command->log, command->fun_name, flag_string( command->flags, cmd_flags ) );

      if( command->userec.num_uses )
         send_timer( &command->userec, ch );

      return;
   }

   if( get_trust( ch ) <= LEVEL_SUB_IMPLEM )
   {
      do_cedit( ch, "" );
      return;
   }

   if( !str_cmp( arg2, "raise" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( ( tmp = command_hash[hash] ) == command )
      {
         send_to_char( "That command is already at the top.\r\n", ch );
         return;
      }

      if( tmp->next == command )
      {
         command_hash[hash] = command;
         tmp_next = tmp->next;
         tmp->next = command->next;
         command->next = tmp;
         ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
         return;
      }

      for( ; tmp; tmp = tmp->next )
      {
         tmp_next = tmp->next;

         if( tmp_next->next == command )
         {
            tmp->next = command;
            tmp_next->next = command->next;
            command->next = tmp_next;
            ch_printf( ch, "Moved %s above %s.\r\n", command->name, command->next->name );
            return;
         }
      }

      send_to_char( "ERROR -- Not Found!\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "lower" ) )
   {
      CMDTYPE *tmp, *tmp_next;
      int hash = command->name[0] % 126;

      if( command->next == NULL )
      {
         send_to_char( "That command is already at the bottom.\r\n", ch );
         return;
      }

      tmp = command_hash[hash];

      if( tmp == command )
      {
         tmp_next = tmp->next;
         command_hash[hash] = command->next;
         command->next = tmp_next->next;
         tmp_next->next = command;
         ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
         return;
      }

      for( ; tmp; tmp = tmp->next )
      {
         if( tmp->next == command )
         {
            tmp_next = command->next;
            tmp->next = tmp_next;
            command->next = tmp_next->next;
            tmp_next->next = command;
            ch_printf( ch, "Moved %s below %s.\r\n", command->name, tmp_next->name );
            return;
         }
      }

      send_to_char( "ERROR -- Not Found!\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "list" ) )
   {
      CMDTYPE *tmp;
      int hash = command->name[0] % 126;
      pager_printf( ch, "Priority placement for [%s]:\r\n", command->name );

      for( tmp = command_hash[hash]; tmp; tmp = tmp->next )
      {
         if( tmp == command )
            set_pager_color( AT_GREEN, ch );
         else
            set_pager_color( AT_PLAIN, ch );

         pager_printf( ch, "  %s\r\n", tmp->name );
      }

      return;
   }

   if( !str_cmp( arg2, "delete" ) )
   {
      unlink_command( command );
      free_command( command );
      send_to_char( "Deleted.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "code" ) )
   {
      DO_FUN *fun = skill_function( argument );

      if( fun == skill_notfound )
      {
         send_to_char( "Code not found.\r\n", ch );
         return;
      }

      command->do_fun = fun;

      DISPOSE( command->fun_name );
      command->fun_name = str_dup( argument );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "level" ) )
   {
      int level = atoi( argument );

      if( ( level < 0 || level > get_trust( ch ) ) )
      {
         send_to_char( "Level out of range.\r\n", ch );
         return;
      }

      command->level = level;

      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "log" ) )
   {
      int clog = atoi( argument );

      if( clog < 0 || clog > LOG_COMM )
      {
         send_to_char( "Log out of range.\r\n", ch );
         return;
      }

      command->log = clog;

      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "position" ) )
   {
      int position = atoi( argument );

      if( position < 0 || position > POS_DRAG )
      {
         send_to_char( "Position out of range.\r\n", ch );
         return;
      }

      command->position = position;

      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "flags" ) )
   {
      int flag;

      if( is_number( argument ) )
         flag = atoi( argument );
      else
         flag = get_cmdflag( argument );

      if( flag < 0 || flag >= 32 )
      {
         if( is_number( argument ) )
            ch_printf( ch, "Invalid flag: range is from 0 to 31.\n" );
         else
            ch_printf( ch, "Unknown flag %s.\n", argument );

         return;
      }

      TOGGLE_BIT( command->flags, 1 << flag );

      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      bool relocate;
      one_argument( argument, arg1 );

      if( arg1[0] == STRING_NULL )
      {
         send_to_char( "Cannot clear name field!\r\n", ch );
         return;
      }

      if( arg1[0] != command->name[0] )
      {
         unlink_command( command );
         relocate = TRUE;
      }
      else
         relocate = FALSE;

      if( command->name )
         DISPOSE( command->name );

      command->name = str_dup( arg1 );

      if( relocate )
         add_command( command );

      send_to_char( "Done.\r\n", ch );

      return;
   }

   /*
    * display usage message
    */
   do_cedit( ch, "" );
}

/*
 * Display Class information     -Thoric
 */
void do_showClass( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];

   struct Class_type *Class;
   int cl, low, hi;
   set_pager_color( AT_PLAIN, ch );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: showClass <Class> [level range]\r\n", ch );
      return;
   }

   if( is_number( arg1 ) && ( cl = atoi( arg1 ) ) >= 0 && cl < MAX_CLASS )
      Class = Class_table[cl];
   else
   {
      Class = NULL;

      for( cl = 0; cl < MAX_CLASS && Class_table[cl]; cl++ )
         if( !str_cmp( Class_table[cl]->who_name, arg1 ) )
         {
            Class = Class_table[cl];
            break;
         }
   }

   if( !Class )
   {
      send_to_char( "No such Class.\r\n", ch );
      return;
   }

   pager_printf_color( ch, "&wCLASS: &W%s\r\n", Class->who_name );
   pager_printf_color( ch, "&wMax Skill Adept: &W%-3d &w\r\n", Class->skill_adept );
   pager_printf_color( ch, "Thac0 : &W%-5d     &wThac32: &W%d\r\n", Class->thac0_00, Class->thac0_32 );
   pager_printf_color( ch, "&w  Hp Min/  Hp Max  : &W%-2d/%-2d  &wExpBase: &W%d\r\n", Class->hp_min, Class->hp_max, Class->exp_base );
   pager_printf_color( ch, "&wMana Min/Mana Max  : &W%-2d/%-2d\r\n", Class->mana_min, Class->mana_max );
   pager_printf_color( ch, "&wMove Min/Move Max  : &W%-2d/%-2d\r\n", Class->move_min, Class->move_max );
 

   if( arg2[0] != STRING_NULL )
   {
      int x, y, cnt;
      low = UMAX( 0, atoi( arg2 ) );
      hi = URANGE( low, atoi( argument ), MAX_LEVEL );

      for( x = low; x <= hi; x++ )
      {
         set_pager_color( AT_LBLUE, ch );
         pager_printf( ch, "Male: %-30s Female: %s\r\n", title_table[cl][x][0], title_table[cl][x][1] );
         cnt = 0;
         set_pager_color( AT_BLUE, ch );

         for( y = gsn_first_spell; y < gsn_top_sn; y++ )
            if( skill_table[y]->skill_level[cl] == x )
            {
               pager_printf( ch, "  %-7s %-19s%3d     ", skill_tname[skill_table[y]->type], skill_table[y]->name, skill_table[y]->skill_adept[cl] );

               if( ++cnt % 2 == 0 )
                  send_to_pager( "\r\n", ch );
            }

         if( cnt % 2 != 0 )
            send_to_pager( "\r\n", ch );

         send_to_pager( "\r\n", ch );
      }
   }
}

/*
 * Create a new Class online.         -Shaddai
 */
bool create_new_Class( int rcindex, char *argument )
{
   int i;

   if( rcindex >= MAX_CLASS || Class_table[rcindex] == NULL )
      return FALSE;

   if( Class_table[rcindex]->who_name )
      STRFREE( Class_table[rcindex]->who_name );

   if( argument[0] != STRING_NULL )
      argument[0] = UPPER( argument[0] );

   Class_table[rcindex]->who_name = STRALLOC( argument );
   Class_table[rcindex]->skill_adept = 0;
   Class_table[rcindex]->thac0_00 = 0;
   Class_table[rcindex]->thac0_32 = 0;
   Class_table[rcindex]->hp_min = 0;
   Class_table[rcindex]->hp_max = 0;
   Class_table[rcindex]->mana_min = 0;
   Class_table[rcindex]->mana_max = 0;
   Class_table[rcindex]->move_min = 0;
   Class_table[rcindex]->move_max = 0;

   Class_table[rcindex]->exp_base = 0;

   for( i = 0; i < MAX_LEVEL; i++ )
   {
      title_table[rcindex][i][0] = str_dup( "Not set." );
      title_table[rcindex][i][1] = str_dup( "Not set." );
   }

   return TRUE;
}

/*
 * Edit Class information     -Thoric
 */
void do_setClass( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
   FILE *fpList;
   char Classlist[256];

   struct Class_type *Class;
   int cl, i;
   set_char_color( AT_PLAIN, ch );
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: setClass <Class> <field> <value>\r\n", ch );
      send_to_char( "Syntax: setClass <Class> create\r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  name \r\n", ch );
      send_to_char( "  thac0 thac32\r\n", ch );
      send_to_char( "  hpmin hpmax manamin manamax movemax movemin mtitle ftitle\r\n", ch );
      send_to_char( "  affected resist suscept skill\r\n", ch );
      return;
   }

   if( is_number( arg1 ) && ( cl = atoi( arg1 ) ) >= 0 && cl < MAX_CLASS )
      Class = Class_table[cl];
   else
   {
      Class = NULL;

      for( cl = 0; cl < MAX_CLASS && Class_table[cl]; cl++ )
      {
         if( !Class_table[cl]->who_name )
            continue;

         if( !str_cmp( Class_table[cl]->who_name, arg1 ) )
         {
            Class = Class_table[cl];
            break;
         }
      }
   }

   if( !str_cmp( arg2, "create" ) && Class )
   {
      send_to_char( "That Class already exists!\r\n", ch );
      return;
   }

   if( !Class && str_cmp( arg2, "create" ) )
   {
      send_to_char( "No such Class.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "create" ) )
   {
      if( MAX_PC_CLASS >= MAX_CLASS )
      {
         send_to_char( "You need to up MAX_CLASS in mud and make clean.\r\n", ch );
         return;
      }

      if( ( create_new_Class( MAX_PC_CLASS, arg1 ) ) == FALSE )
      {
         send_to_char( "Couldn't create a new Class.\r\n", ch );
         return;
      }

      write_Class_file( MAX_PC_CLASS );

      MAX_PC_CLASS++;
      snprintf( Classlist, 256, "%s%s", CLASS_DIR, CLASS_LIST );

      if( !( fpList = fopen( Classlist, "w" ) ) )
      {
         bug( "%s", "Can't open Class list for writing." );
         return;
      }

      for( i = 0; i < MAX_PC_CLASS; i++ )
         fprintf( fpList, "%s%s.Class\n", CLASS_DIR, Class_table[i]->who_name );

      fprintf( fpList, "%s", "$\n" );

      fclose( fpList );

      fpList = NULL;

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !argument )
   {
      send_to_char( "You must specify an argument.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "skill" ) )
   {
      SKILLTYPE *skill;
      int sn, level, adept;
      argument = one_argument( argument, arg2 );

      if( ( sn = skill_lookup( arg2 ) ) > 0 )
      {
         skill = get_skilltype( sn );
         argument = one_argument( argument, arg2 );
         level = atoi( arg2 );
         argument = one_argument( argument, arg2 );
         adept = atoi( arg2 );
         skill->skill_level[cl] = level;
         skill->skill_adept[cl] = adept;
         write_Class_file( cl );
         ch_printf( ch, "Skill \"%s\" added at level %d and %d%%.\r\n", skill->name, level, adept );
      }
      else
         ch_printf( ch, "No such skill as %s.\r\n", arg2 );

      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      char buf[256];
      snprintf( buf, 256, "%s%s.Class", CLASS_DIR, Class->who_name );
      STRFREE( Class->who_name );
      Class->who_name = STRALLOC( capitalize( argument ) );
      ch_printf( ch, "Class %s renamed to %s.\r\n", arg1, argument );
      write_Class_file( cl );
      unlink( buf );
      snprintf( Classlist, 256, "%s%s", CLASS_DIR, CLASS_LIST );

      if( !( fpList = fopen( Classlist, "w" ) ) )
      {
         bug( "%s", "Can't open Class list for writing." );
         return;
      }

      for( i = 0; i < MAX_PC_CLASS; i++ )
         fprintf( fpList, "%s%s.Class\n", CLASS_DIR, Class_table[i]->who_name );

      fprintf( fpList, "%s", "$\n" );

      fclose( fpList );

      fpList = NULL;

      return;
   }

   if( !str_cmp( arg2, "hpmin" ) )
   {
      Class->hp_min = atoi( argument );
      send_to_char( "Min HP gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "hpmax" ) )
   {
      Class->hp_max = atoi( argument );
      send_to_char( "Max HP gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "movemin" ) )
   {
      Class->move_min = atoi( argument );
      send_to_char( "Min MOVE gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "movemax" ) )
   {
      Class->move_max = atoi( argument );
      send_to_char( "Max MOVE gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "manamin" ) )
   {
      Class->mana_min = atoi( argument );
      send_to_char( "Min MANA gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "manamax" ) )
   {
      Class->mana_max = atoi( argument );
      send_to_char( "Max MANA gain set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "expbase" ) )
   {
      Class->exp_base = atoi( argument );
      send_to_char( "Expbase set.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "mtitle" ) )
   {
      char arg3[MAX_INPUT_LENGTH];
      int x;
      argument = one_argument( argument, arg3 );

      if( arg3[0] == STRING_NULL || argument[0] == STRING_NULL )
      {
         send_to_char( "Syntax: setClass <Class> mtitle <level> <title>\r\n", ch );
         return;
      }

      if( ( x = atoi( arg3 ) ) < 0 || x > MAX_LEVEL )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }

      STRFREE( title_table[cl][x][SEX_MALE] );

      title_table[cl][x][SEX_MALE] = STRALLOC( argument );
      send_to_char( "Done.\r\n", ch );
      write_Class_file( cl );
      return;
   }

   if( !str_cmp( arg2, "ftitle" ) )
   {
      char arg3[MAX_INPUT_LENGTH], arg4[MAX_INPUT_LENGTH];
      int x;
      argument = one_argument( argument, arg3 );
      argument = one_argument( argument, arg4 );

      if( arg3[0] == STRING_NULL || argument[0] == STRING_NULL )
      {
         send_to_char( "Syntax: setClass <Class> ftitle <level> <title>\r\n", ch );
         return;
      }

      if( ( x = atoi( arg4 ) ) < 0 || x > MAX_LEVEL )
      {
         send_to_char( "Invalid level.\r\n", ch );
         return;
      }

      STRFREE( title_table[cl][x][SEX_FEMALE] );

      /*
       * Bug fix below -Shaddai
       */
      title_table[cl][x][SEX_FEMALE] = STRALLOC( argument );
      send_to_char( "Done\r\n", ch );
      write_Class_file( cl );
      return;
   }

   do_setClass( ch, "" );
}

/*
 * Create an instance of a new race.   -Shaddai
 */
bool create_new_race( int rcindex, char *argument )
{
   int i = 0;

   if( rcindex >= MAX_RACE || race_table[rcindex] == NULL )
      return FALSE;

   for( i = 0; i < MAX_WHERE_NAME; i++ )
      race_table[rcindex]->where_name[i] = where_name[i];

   if( argument[0] != STRING_NULL )
      argument[0] = UPPER( argument[0] );

   sprintf( race_table[rcindex]->race_name, "%-.16s", argument );

   race_table[rcindex]->Class_restriction = 0;

   race_table[rcindex]->str_plus = 0;

   race_table[rcindex]->dex_plus = 0;

   race_table[rcindex]->wis_plus = 0;

   race_table[rcindex]->int_plus = 0;

   race_table[rcindex]->con_plus = 0;

   race_table[rcindex]->cha_plus = 0;

   race_table[rcindex]->lck_plus = 0;

   race_table[rcindex]->hit = 0;

   race_table[rcindex]->mana = 0;

   xCLEAR_BITS( race_table[rcindex]->affected );

   race_table[rcindex]->resist = 0;

   race_table[rcindex]->suscept = 0;

   race_table[rcindex]->language = 0;

   race_table[rcindex]->alignment = 0;

   race_table[rcindex]->minalign = 0;

   race_table[rcindex]->maxalign = 0;

   race_table[rcindex]->ac_plus = 0;

   race_table[rcindex]->exp_multiplier = 0;

   xCLEAR_BITS( race_table[rcindex]->attacks );

   xCLEAR_BITS( race_table[rcindex]->defenses );

   race_table[rcindex]->height = 0;

   race_table[rcindex]->weight = 0;

   race_table[rcindex]->hunger_mod = 0;

   race_table[rcindex]->thirst_mod = 0;

   race_table[rcindex]->mana_regen = 0;

   race_table[rcindex]->hp_regen = 0;

   race_table[rcindex]->race_recall = 0;

   return TRUE;
}

/* Modified by Samson to allow setting language by name - 8-6-98 */
void do_setrace( CHAR_DATA * ch, char *argument )
{
   RACE_TYPE *race;
   char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH], arg3[MAX_INPUT_LENGTH];
   FILE *fpList;
   char racelist[256];
   int value, v2, ra, i;
   set_char_color( AT_PLAIN, ch );
   smash_tilde( argument );
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Syntax: setrace <race> <field> <value>\r\n", ch );
      send_to_char( "Syntax: setrace <race> create      \r\n", ch );
      send_to_char( "\r\nField being one of:\r\n", ch );
      send_to_char( "  name Classes \r\n", ch );
      send_to_char( "  strplus dexplus wisplus intplus conplus chaplus lckplus \r\n", ch );
      send_to_char( "  hit mana affected resist suscept language\r\n", ch );
      send_to_char( "  attack defense alignment acplus \r\n", ch );
      send_to_char( "  minalign maxalign height weight      \r\n", ch );
      send_to_char( "  hungermod thirstmod expmultiplier    \r\n", ch );
      send_to_char( "  saving_poison_death saving_mental saving_physical saving_weapons\r\n", ch );
      send_to_char( "  mana_regen hp_regen                  \r\n", ch );
      return;
   }

   if( is_number( arg1 ) && ( ra = atoi( arg1 ) ) >= 0 && ra < MAX_RACE )
      race = race_table[ra];
   else
   {
      race = NULL;

      for( ra = 0; ra < MAX_RACE && race_table[ra]; ra++ )
      {
         if( !race_table[ra]->race_name )
            continue;

         if( !str_cmp( race_table[ra]->race_name, arg1 ) )
         {
            race = race_table[ra];
            break;
         }
      }
   }

   if( !str_cmp( arg2, "create" ) && race )
   {
      send_to_char( "That race already exists!\r\n", ch );
      return;
   }
   else if( !race && str_cmp( arg2, "create" ) )
   {
      send_to_char( "No such race.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "create" ) )
   {
      if( MAX_PC_RACE >= MAX_RACE )
      {
         send_to_char( "You need to up MAX_RACE in mud.h and make clean.\r\n", ch );
         return;
      }

      if( ( create_new_race( MAX_PC_RACE, arg1 ) ) == FALSE )
      {
         send_to_char( "Couldn't create a new race.\r\n", ch );
         return;
      }

      write_race_file( MAX_PC_RACE );

      MAX_PC_RACE++;
      snprintf( racelist, 256, "%s%s", RACE_DIR, RACE_LIST );

      if( !( fpList = fopen( racelist, "w" ) ) )
      {
         bug( "%s", "Error opening racelist." );
         return;
      }

      for( i = 0; i < MAX_PC_RACE; i++ )
         fprintf( fpList, "%s%s.race\n", RACE_DIR, race_table[i]->race_name );

      fprintf( fpList, "%s", "$\n" );

      fclose( fpList );

      fpList = NULL;

      send_to_char( "Done.\r\n", ch );

      return;
   }

   if( !argument )
   {
      send_to_char( "You must specify an argument.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "name" ) )
   {
      char buf[256];
      snprintf( buf, 256, "%s%s.race", RACE_DIR, race->race_name );
      snprintf( race->race_name, 16, "%s", capitalize( argument ) );
      write_race_file( ra );
      unlink( buf );
      snprintf( racelist, 256, "%s%s", RACE_DIR, RACE_LIST );

      if( !( fpList = fopen( racelist, "w" ) ) )
      {
         bug( "%s", "Error opening racelist." );
         return;
      }

      for( i = 0; i < MAX_PC_RACE; i++ )
         fprintf( fpList, "%s%s.race\n", RACE_DIR, race_table[i]->race_name );

      fprintf( fpList, "%s", "$\n" );

      fclose( fpList );

      fpList = NULL;

      send_to_char( "Race name set.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "strplus" ) )
   {
      race->str_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "dexplus" ) )
   {
      race->dex_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "wisplus" ) )
   {
      race->wis_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "intplus" ) )
   {
      race->int_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "conplus" ) )
   {
      race->con_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "chaplus" ) )
   {
      race->cha_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "lckplus" ) )
   {
      race->lck_plus = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "hit" ) )
   {
      race->hit = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "mana" ) )
   {
      race->mana = ( short )atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "affected" ) )
   {
      if( !argument || argument[0] == STRING_NULL )
      {
         send_to_char( "Usage: setrace <race> affected <flag> [flag]...\r\n", ch );
         return;
      }

      while( argument[0] != STRING_NULL )
      {
         argument = one_argument( argument, arg3 );
         value = get_aflag( arg3 );

         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->affected, value );
      }

      write_race_file( ra );

      send_to_char( "Racial affects set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "resist" ) )
   {
      if( !argument || argument[0] == STRING_NULL )
      {
         send_to_char( "Usage: setrace <race> resist <flag> [flag]...\r\n", ch );
         return;
      }

      while( argument[0] != STRING_NULL )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );

         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            TOGGLE_BIT( race->resist, 1 << value );
      }

      write_race_file( ra );

      send_to_char( "Racial resistances set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "suscept" ) )
   {
      if( !argument || argument[0] == STRING_NULL )
      {
         send_to_char( "Usage: setrace <race> suscept <flag> [flag]...\r\n", ch );
         return;
      }

      while( argument[0] != STRING_NULL )
      {
         argument = one_argument( argument, arg3 );
         value = get_risflag( arg3 );

         if( value < 0 || value > 31 )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            TOGGLE_BIT( race->suscept, 1 << value );
      }

      write_race_file( ra );

      send_to_char( "Racial susceptabilities set.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "language" ) )
   {
      argument = one_argument( argument, arg3 );
      value = get_langflag( arg3 );

      if( value == LANG_UNKNOWN )
         ch_printf( ch, "Unknown language: %s\r\n", arg3 );
      else
      {
         if( !( value &= VALID_LANGS ) )
            ch_printf( ch, "Player races may not speak %s.\r\n", arg3 );
      }

      v2 = get_langnum( arg3 );

      if( v2 == -1 )
         ch_printf( ch, "Unknown language: %s\r\n", arg3 );
      else
         TOGGLE_BIT( race->language, 1 << v2 );

      write_race_file( ra );

      send_to_char( "Racial language set.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "Classes" ) )
   {
      for( i = 0; i < MAX_CLASS; i++ )
      {
         if( !str_cmp( argument, Class_table[i]->who_name ) )
         {
            TOGGLE_BIT( race->Class_restriction, 1 << i );  /* k, that's boggling */
            write_race_file( ra );
            send_to_char( "Classes set.\r\n", ch );
            return;
         }
      }

      send_to_char( "No such Class.\r\n", ch );

      return;
   }

   if( !str_cmp( arg2, "acplus" ) )
   {
      race->ac_plus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "alignment" ) )
   {
      race->alignment = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   /*
    * not implemented
    */
   if( !str_cmp( arg2, "defense" ) )
   {
      if( !argument || argument[0] == STRING_NULL )
      {
         send_to_char( "Usage: setrace <race> defense <flag> [flag]...\r\n", ch );
         return;
      }

      while( argument[0] != STRING_NULL )
      {
         argument = one_argument( argument, arg3 );
         value = get_defenseflag( arg3 );

         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->defenses, value );
      }

      write_race_file( ra );

      return;
   }

   /*
    * not implemented
    */
   if( !str_cmp( arg2, "attack" ) )
   {
      if( !argument || argument[0] == STRING_NULL )
      {
         send_to_char( "Usage: setrace <race> attack <flag> [flag]...\r\n", ch );
         return;
      }

      while( argument[0] != STRING_NULL )
      {
         argument = one_argument( argument, arg3 );
         value = get_attackflag( arg3 );

         if( value < 0 || value > MAX_BITS )
            ch_printf( ch, "Unknown flag: %s\r\n", arg3 );
         else
            xTOGGLE_BIT( race->attacks, value );
      }

      write_race_file( ra );

      return;
   }

   if( !str_cmp( arg2, "minalign" ) )
   {
      race->minalign = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "maxalign" ) )
   {
      race->maxalign = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "height" ) )
   {
      race->height = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "weight" ) )
   {
      race->weight = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "thirstmod" ) )
   {
      race->thirst_mod = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "hungermod" ) )
   {
      race->hunger_mod = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "maxalign" ) )
   {
      race->maxalign = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "expmultiplier" ) )
   {
      race->exp_multiplier = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "saving_poison_death" ) )
   {
      race->saving_poison_death = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "saving_mental" ) )
   {
      race->saving_mental = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "saving_physical" ) )
   {
      race->saving_physical = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "saving_weapons" ) )
   {
      race->saving_weapons = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }
   /*
    * unimplemented stuff follows
    */
   if( !str_cmp( arg2, "mana_regen" ) )
   {
      race->mana_regen = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "hp_regen" ) )
   {
      race->hp_regen = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

#ifdef NEW_RACE_STUFF
   if( !str_cmp( arg2, "carry_weight" ) )
   {
      race->acplus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

   if( !str_cmp( arg2, "carry_number" ) )
   {
      race->acplus = atoi( argument );
      write_race_file( ra );
      send_to_char( "Done.\r\n", ch );
      return;
   }

#endif
   do_setrace( ch, "" );
}

void do_showrace( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char buf[MAX_STRING_LENGTH];

   struct race_type *race;
   int ra, i, ct;
   set_pager_color( AT_PLAIN, ch );
   argument = one_argument( argument, arg1 );

   if( arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: showrace  \r\n", ch );
      /*
       * Show the races code addition by Blackmane
       */
      /*
       * fixed printout by Miki
       */
      ct = 0;

      for( i = 0; i < MAX_RACE; i++ )
      {
         ++ct;
         pager_printf( ch, "%2d> %-11s", i, race_table[i]->race_name );

         if( ct % 5 == 0 )
            send_to_pager( "\r\n", ch );
      }

      send_to_pager( "\r\n", ch );

      return;
   }

   if( is_number( arg1 ) && ( ra = atoi( arg1 ) ) >= 0 && ra < MAX_RACE )
      race = race_table[ra];
   else
   {
      race = NULL;

      for( ra = 0; ra < MAX_RACE && race_table[ra]; ra++ )
         if( !str_cmp( race_table[ra]->race_name, arg1 ) )
         {
            race = race_table[ra];
            break;
         }
   }

   if( !race )
   {
      send_to_char( "No such race.\r\n", ch );
      return;
   }

   sprintf( buf, "RACE: %s\r\n", race->race_name );

   send_to_char( buf, ch );
   ct = 0;
   sprintf( buf, "Disallowed Classes: " );
   send_to_char( buf, ch );

   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( IS_SET( race->Class_restriction, 1 << i ) )
      {
         ct++;
         sprintf( buf, "%s ", Class_table[i]->who_name );
         send_to_char( buf, ch );

         if( ct % 6 == 0 )
            send_to_char( "\r\n", ch );
      }
   }

   if( ( ct % 6 != 0 ) || ( ct == 0 ) )
      send_to_char( "\r\n", ch );

   ct = 0;

   sprintf( buf, "Allowed Classes: " );

   send_to_char( buf, ch );

   for( i = 0; i < MAX_CLASS; i++ )
   {
      if( !IS_SET( race->Class_restriction, 1 << i ) )
      {
         ct++;
         sprintf( buf, "%s ", Class_table[i]->who_name );
         send_to_char( buf, ch );

         if( ct % 6 == 0 )
            send_to_char( "\r\n", ch );
      }
   }

   if( ( ct % 6 != 0 ) || ( ct == 0 ) )
      send_to_char( "\r\n", ch );

   sprintf( buf, "Str Plus: %-3d\tDex Plus: %-3d\tWis Plus: %-3d\tInt Plus: %-3d\t\r\n", race->str_plus, race->dex_plus, race->wis_plus, race->int_plus );
   send_to_char( buf, ch );
   sprintf( buf, "Con Plus: %-3d\tCha Plus: %-3d\tLck Plus: %-3d\r\n", race->con_plus, race->cha_plus, race->lck_plus );
   send_to_char( buf, ch );
   sprintf( buf, "Hit Pts:  %-3d\tMana: %-3d\tAlign: %-4d\tAC: %-d\r\n", race->hit, race->mana, race->alignment, race->ac_plus );
   send_to_char( buf, ch );
   sprintf( buf, "Min Align: %d\tMax Align: %-d\t\tXP Mult: %-d%%\r\n", race->minalign, race->maxalign, race->exp_multiplier );
   send_to_char( buf, ch );
   sprintf( buf, "Height: %3d in.\t\tWeight: %4d lbs.\tHungerMod: %d\tThirstMod: %d\r\n", race->height, race->weight, race->hunger_mod, race->thirst_mod );
   send_to_char( buf, ch );
   send_to_char( "Affected by: ", ch );
   send_to_char( affect_bit_name( &race->affected ), ch );
   send_to_char( "\r\n", ch );
   send_to_char( "Resistant to: ", ch );
   send_to_char( flag_string( race->resist, ris_flags ), ch );
   send_to_char( "\r\n", ch );
   send_to_char( "Susceptible to: ", ch );
   send_to_char( flag_string( race->suscept, ris_flags ), ch );
   send_to_char( "\r\n", ch );
   sprintf( buf, "Saves: Poison-%d Mental-%d Physical-%d Weapons-%d\r\n",
           race->saving_poison_death, race->saving_mental, race->saving_physical, race->saving_weapons );
    send_to_char( buf, ch );
   send_to_char( "Innate Attacks: ", ch );
   send_to_char( ext_flag_string( &race->attacks, attack_flags ), ch );
   send_to_char( "\r\n", ch );
   send_to_char( "Innate Defenses: ", ch );
   send_to_char( ext_flag_string( &race->defenses, defense_flags ), ch );
   send_to_char( "\r\n", ch );
}

RESERVE_DATA *first_reserved;
RESERVE_DATA *last_reserved;

void save_reserved( void )
{
   RESERVE_DATA *res;
   FILE *fp;
   fclose( fpReserve );

   if( !( fp = fopen( SYSTEM_DIR RESERVED_LIST, "w" ) ) )
   {
      bug( "Save_reserved: cannot open " RESERVED_LIST, 0 );
      perror( RESERVED_LIST );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }

   for( res = first_reserved; res; res = res->next )
      fprintf( fp, "%s~\n", res->name );

   fprintf( fp, "$~\n" );

   fclose( fp );

   fpReserve = fopen( NULL_FILE, "r" );

   return;
}

void do_reserve( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   RESERVE_DATA *res;
   set_char_color( AT_PLAIN, ch );
   argument = one_argument( argument, arg );

   if( !*arg )
   {
      int wid = 0;
      send_to_char( "-- Reserved Names --\r\n", ch );

      for( res = first_reserved; res; res = res->next )
      {
         ch_printf( ch, "%c%-17s ", ( *res->name == '*' ? '*' : ' ' ), ( *res->name == '*' ? res->name + 1 : res->name ) );

         if( ++wid % 4 == 0 )
            send_to_char( "\r\n", ch );
      }

      if( wid % 4 != 0 )
         send_to_char( "\r\n", ch );

      return;
   }

   for( res = first_reserved; res; res = res->next )
      if( !str_cmp( arg, res->name ) )
      {
         UNLINK( res, first_reserved, last_reserved, next, prev );
         DISPOSE( res->name );
         DISPOSE( res );
         save_reserved(  );
         send_to_char( "Name no longer reserved.\r\n", ch );
         return;
      }

   CREATE( res, RESERVE_DATA, 1 );

   res->name = str_dup( arg );
   sort_reserved( res );
   save_reserved(  );
   send_to_char( "Name reserved.\r\n", ch );
   return;
}

/*
 * Command to display the weather status of all the areas
 * Last Modified: July 21, 1997
 * Fireblade
 */
void do_showweather( CHAR_DATA * ch, char *argument )
{
   AREA_DATA *pArea;
   char arg[MAX_INPUT_LENGTH];

   if( !ch )
   {
      bug( "do_showweather: NULL char data" );
      return;
   }

   argument = one_argument( argument, arg );

   set_char_color( AT_BLUE, ch );
   ch_printf( ch, "%-40s%-8s %-8s %-8s\r\n", "Area Name:", "Temp:", "Precip:", "Wind:" );

   for( pArea = first_area; pArea; pArea = pArea->next )
   {
      if( arg[0] == STRING_NULL || nifty_is_name_prefix( arg, pArea->name ) )
      {
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "%-40s", pArea->name );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->temp );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->temp_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ") " );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->precip );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->precip_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ") " );
         set_char_color( AT_WHITE, ch );
         ch_printf( ch, "%3d", pArea->weather->wind );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, "(" );
         set_char_color( AT_LBLUE, ch );
         ch_printf( ch, "%3d", pArea->weather->wind_vector );
         set_char_color( AT_BLUE, ch );
         ch_printf( ch, ")\r\n" );
      }
   }

   return;
}

/*
 * Command to control global weather variables and to reset weather
 * Last Modified: July 23, 1997
 * Fireblade
 */
void do_setweather( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   set_char_color( AT_BLUE, ch );
   argument = one_argument( argument, arg );

   if( arg[0] == STRING_NULL )
   {
      ch_printf( ch, "%-15s%-6s\r\n", "Parameters:", "Value:" );
      ch_printf( ch, "%-15s%-6d\r\n", "random", rand_factor );
      ch_printf( ch, "%-15s%-6d\r\n", "climate", climate_factor );
      ch_printf( ch, "%-15s%-6d\r\n", "neighbor", neigh_factor );
      ch_printf( ch, "%-15s%-6d\r\n", "unit", weath_unit );
      ch_printf( ch, "%-15s%-6d\r\n", "maxvector", max_vector );
      ch_printf( ch, "\r\nResulting values:\r\n" );
      ch_printf( ch, "Weather variables range from " "%d to %d.\r\n", -3 * weath_unit, 3 * weath_unit );
      ch_printf( ch, "Weather vectors range from " "%d to %d.\r\n", -1 * max_vector, max_vector );
      ch_printf( ch, "The maximum a vector can " "change in one update is %d.\r\n", rand_factor + 2 * climate_factor + ( 6 * weath_unit / neigh_factor ) );
   }
   else if( !str_cmp( arg, "random" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set maximum random " "change in vectors to what?\r\n" );
      }
      else
      {
         rand_factor = atoi( argument );
         ch_printf( ch, "Maximum random " "change in vectors now " "equals %d.\r\n", rand_factor );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "climate" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set climate effect " "coefficient to what?\r\n" );
      }
      else
      {
         climate_factor = atoi( argument );
         ch_printf( ch, "Climate effect " "coefficient now equals " "%d.\r\n", climate_factor );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "neighbor" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set neighbor effect " "divisor to what?\r\n" );
      }
      else
      {
         neigh_factor = atoi( argument );

         if( neigh_factor <= 0 )
            neigh_factor = 1;

         ch_printf( ch, "Neighbor effect " "coefficient now equals " "1/%d.\r\n", neigh_factor );

         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "unit" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set weather unit " "size to what?\r\n" );
      }
      else
      {
         weath_unit = atoi( argument );
         ch_printf( ch, "Weather unit size " "now equals %d.\r\n", weath_unit );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "maxvector" ) )
   {
      if( !is_number( argument ) )
      {
         ch_printf( ch, "Set maximum vector " "size to what?\r\n" );
      }
      else
      {
         max_vector = atoi( argument );
         ch_printf( ch, "Maximum vector size " "now equals %d.\r\n", max_vector );
         save_weatherdata(  );
      }
   }
   else if( !str_cmp( arg, "reset" ) )
   {
      init_area_weather(  );
      ch_printf( ch, "Weather system reinitialized.\r\n" );
   }
   else if( !str_cmp( arg, "update" ) )
   {
      int i, number;
      number = atoi( argument );

      if( number < 1 )
         number = 1;

      for( i = 0; i < number; i++ )
         weather_update(  );

      ch_printf( ch, "Weather system updated.\r\n" );
   }
   else
   {
      ch_printf( ch, "You may only use one of the " "following fields:\r\n" );
      ch_printf( ch, "\trandom\r\n\tclimate\r\n" "\tneighbor\r\n\tunit\r\n\tmaxvector\r\n" );
      ch_printf( ch, "You may also reset or update " "the system using the fields 'reset' " "and 'update' respectively.\r\n" );
   }

   return;
}
void do_pcrename( CHAR_DATA * ch, char *argument )
{
   CHAR_DATA *victim;
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char newname[MAX_STRING_LENGTH];
   char oldname[MAX_STRING_LENGTH];
   char backname[MAX_STRING_LENGTH];
   char buf[MAX_STRING_LENGTH];
   argument = one_argument( argument, arg1 );
   one_argument( argument, arg2 );
   smash_tilde( arg2 );

   if( IS_NPC( ch ) )
      return;

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Syntax: rename <victim> <new name>\r\n", ch );
      return;
   }

   if( !check_parse_name( arg2, 1 ) )
   {
      send_to_char( "Illegal name.\r\n", ch );
      return;
   }

   /*
    * Just a security precaution so you don't rename someone you don't mean
    * * too --Shaddai
    */
   if( ( victim = get_char_room( ch, arg1 ) ) == NULL )
   {
      send_to_char( "That person is not in the room.\r\n", ch );
      return;
   }

   if( IS_NPC( victim ) )
   {
      send_to_char( "You can't rename NPC's.\r\n", ch );
      return;
   }

   if( get_trust( ch ) < get_trust( victim ) )
   {
      send_to_char( "I don't think they would like that!\r\n", ch );
      return;
   }

   sprintf( newname, "%s%c/%s", PLAYER_DIR, tolower( arg2[0] ), capitalize( arg2 ) );

   sprintf( oldname, "%s%c/%s", PLAYER_DIR, tolower( victim->pcdata->filename[0] ), capitalize( victim->pcdata->filename ) );
   sprintf( backname, "%s%c/%s", BACKUP_DIR, tolower( victim->pcdata->filename[0] ), capitalize( victim->pcdata->filename ) );

   if( access( newname, F_OK ) == 0 )
   {
      send_to_char( "That name already exists.\r\n", ch );
      return;
   }

   /*
    * Have to remove the old god entry in the directories
    */
   if( IS_IMMORTAL( victim ) )
   {
      char godname[MAX_STRING_LENGTH];
      sprintf( godname, "%s%s", GOD_DIR, capitalize( victim->pcdata->filename ) );
      remove( godname );
   }

   /*
    * Remember to change the names of the areas
    */
   if( victim->pcdata->area )
   {
      char filename[MAX_STRING_LENGTH];
      char newfilename[MAX_STRING_LENGTH];
      sprintf( filename, "%s%s.are", BUILD_DIR, victim->name );
      sprintf( newfilename, "%s%s.are", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
      sprintf( filename, "%s%s.are.bak", BUILD_DIR, victim->name );
      sprintf( newfilename, "%s%s.are.bak", BUILD_DIR, capitalize( arg2 ) );
      rename( filename, newfilename );
   }

   STRFREE( victim->name );

   victim->name = STRALLOC( capitalize( arg2 ) );
   STRFREE( victim->pcdata->filename );
   victim->pcdata->filename = STRALLOC( capitalize( arg2 ) );
   remove( backname );

   if( remove( oldname ) )
   {
      sprintf( buf, "Error: Couldn't delete file %s in do_rename.", oldname );
      send_to_char( "Couldn't delete the old file!\r\n", ch );
      log_string( oldname );
   }

   /*
    * Time to save to force the affects to take place
    */
   save_char_obj( victim );

   /*
    * Now lets update the wizlist
    */
   if( IS_IMMORTAL( victim ) )
      make_wizlist(  );

   send_to_char( "Character was renamed.\r\n", ch );

   return;
}

void do_invade( CHAR_DATA * ch, char *argument )
{
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char arg3[MAX_INPUT_LENGTH];
   CHAR_DATA *victim;
   AREA_DATA *tarea;
   int count, created;
   bool found = FALSE;
   MOB_INDEX_DATA *pMobIndex;
   ROOM_INDEX_DATA *location;
   argument = one_argument( argument, arg1 );
   argument = one_argument( argument, arg2 );
   argument = one_argument( argument, arg3 );
   count = atoi( arg2 );

   if( arg1[0] == STRING_NULL || arg2[0] == STRING_NULL )
   {
      send_to_char( "Invade <area> <# of invaders> <mob vnum>\r\n", ch );
      return;
   }

   for( tarea = first_area; tarea; tarea = tarea->next )
      if( !str_cmp( tarea->filename, arg1 ) )
      {
         found = TRUE;
         break;
      }

   if( !found )
   {
      send_to_char( "Area not found.\r\n", ch );
      return;
   }

   if( count > 300 )
   {
      send_to_char( "Whoa...Less than 300 please.\r\n", ch );
      return;
   }

   if( ( pMobIndex = get_mob_index( atoi( arg3 ) ) ) == NULL )
   {
      send_to_char( "No mobile has that vnum.\r\n", ch );
      return;
   }

   for( created = 0; created < count; created++ )
   {
      if( ( location = get_room_index( number_range( tarea->low_r_vnum, tarea->hi_r_vnum ) ) ) == NULL )
      {
         --created;
         continue;
      }

      if( xIS_SET( location->room_flags, ROOM_SAFE ) )
      {
         --created;
         continue;
      }
      victim = create_mobile( pMobIndex );
      char_to_room( victim, location );
      victim->timer = 30;
//      do_create_token( victim, "global" );
      act( AT_IMMORT, "$N appears as part of an invasion force!", ch, NULL, victim, TO_ROOM );
   }
   send_to_char( "The invasion was successful!\r\n", ch );
   return;
}


void do_double( CHAR_DATA * ch, char *argument )
{
   char arg[MAX_INPUT_LENGTH];
   char arg1[MAX_INPUT_LENGTH];
   char arg2[MAX_INPUT_LENGTH];
   char buf[MAX_INPUT_LENGTH];
   int amount;
   argument = one_argument( argument, arg );
   argument = one_argument( argument, arg1 );
   one_argument( argument, arg2 );

   if( arg[0] == STRING_NULL || arg1[0] == STRING_NULL )
   {
      send_to_char( "Syntax: all <on|off> minutes.\r\n", ch );
      send_to_char( "Syntax: <type> <on|off> minutes.\r\n", ch );
      send_to_char( "Valid types are: exp | quest | quad\r\n", ch );
      return;
   }

   if( !str_cmp( arg, "all" ) )
   {
      if( !str_cmp( arg1, "on" ) )
      {
         if( arg2[0] == STRING_NULL || !is_number( arg2 ) )
         {
            send_to_char( "You need to apply the number of minutes.\r\n", ch );
            return;
         }

         if( double_exp || double_qp || quad_damage )
         {
            send_to_char( "One of the types is already in affect! Please turn it off in order to use the all option.\r\n", ch );
            return;
         }

         amount = atoi( arg2 );

         if( amount < 0 || amount > 500 )
         {
            send_to_char( "Please choose an amount between 0 and 500.\r\n", ch );
            return;
         }

         global_exp = amount;

         double_exp = TRUE;
         global_qp = amount;
         double_qp = TRUE;
         global_quad = amount;
         quad_damage = TRUE;
         sprintf( buf,
                  "&R[&CANNOUNCEMENT&R]&c:{xIn a moment of insanity, EldhaMUD has been blessed with %d minutes of double exp, double qp, and quad damage for everyone!{x\r\n",
                  amount );
         talk_info( AT_GREEN, buf, TRUE );
         return;
      }

      if( !str_cmp( arg1, "off" ) )
      {
         if( !double_exp && !double_qp && !quad_damage )
         {
            send_to_char( "All of the double types are off. Turn them on first.\r\n", ch );
            return;
         }

         double_exp = FALSE;

         global_exp = 0;
         global_qp = 0;
         double_qp = FALSE;
         quad_damage = FALSE;
         global_quad = 0;
         sprintf( buf, "&g&R[&CANNOUNCEMENT&R]&c:&x &R%s has removed all of the double types!{x\r\n", ch->name );
         talk_info( AT_GREEN, buf, FALSE );
         return;
      }
   }

   if( !str_cmp( arg, "exp" ) )
   {
      if( !str_cmp( arg1, "on" ) )
      {
         if( arg2[0] == STRING_NULL || !is_number( arg2 ) )
         {
            send_to_char( "You need to apply the number of minutes.\r\n", ch );
            return;
         }

         if( double_exp )
         {
            send_to_char( "Double exp is already in affect!\r\n", ch );
            return;
         }

         amount = atoi( arg2 );

         if( amount < 0 || amount > 500 )
         {
            send_to_char( "Please choose an amount between 0 and 500.\r\n", ch );
            return;
         }

         global_exp = amount;

         double_exp = TRUE;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:{xIn a moment of insanity, EldhaMUD has been blessed with %d minutes of double exp for everyone!{x\r\n", amount );
         talk_info( AT_GREEN, buf, TRUE );
         return;
      }

      if( !str_cmp( arg, "off" ) )
      {
         if( !double_exp )
         {
            send_to_char( "Double exp is not on please turn it on first!\r\n", ch );
            return;
         }

         double_exp = FALSE;

         global_exp = 0;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:&x {R%s has removed double experience!{x\r\n", ch->name );
         talk_info( AT_GREEN, buf, FALSE );
         return;
      }
   }

   if( !str_cmp( arg, "quest" ) )
   {
      if( !str_cmp( arg1, "on" ) )
      {
         if( arg2[0] == STRING_NULL || !is_number( arg2 ) )
         {
            send_to_char( "You need to apply the number of minutes.\r\n", ch );
            return;
         }

         if( double_qp )
         {
            send_to_char( "Double questpoints is already in affect!\r\n", ch );
            return;
         }

         amount = atoi( arg2 );

         if( amount < 0 || amount > 500 )
         {
            send_to_char( "Please choose an amount between 0 and 500.\r\n", ch );
            return;
         }

         global_qp = amount;

         double_qp = TRUE;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:{xIn a moment of insanity, EldhaMUD has been blessed with %d minutes of double questpoints for everyone!{x\r\n", amount );
         talk_info( AT_GREEN, buf, TRUE );
         return;
      }

      if( !str_cmp( arg, "off" ) )
      {
         if( !double_qp )
         {
            send_to_char( "Double questpoints is not on please turn it on first!\r\n", ch );
            return;
         }

         double_qp = FALSE;

         global_qp = 0;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:{x {R%s has removed double questpoints!{x\r\n", ch->name );
         talk_info( AT_GREEN, buf, FALSE );
         return;
      }
   }

   if( !str_cmp( arg, "quad" ) )
   {
      if( !str_cmp( arg1, "on" ) )
      {
         if( arg2[0] == STRING_NULL || !is_number( arg2 ) )
         {
            send_to_char( "You need to apply the number of minutes.\r\n", ch );
            return;
         }

         if( quad_damage )
         {
            send_to_char( "Quad damage is already in affect!\r\n", ch );
            return;
         }

         amount = atoi( arg2 );

         if( amount < 0 || amount > 500 )
         {
            send_to_char( "Please choose an amount between 0 and 500.\r\n", ch );
            return;
         }

         global_quad = amount;

         quad_damage = TRUE;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:{xIn a moment of insanity, EldhaMUD has been blessed with %d minutes of quad damage for everyone!{x\r\n", amount );
         talk_info( AT_GREEN, buf, TRUE );
         return;
      }

      if( !str_cmp( arg, "off" ) )
      {
         if( !quad_damage )
         {
            send_to_char( "Quad damage is not on please turn it on first!\r\n", ch );
            return;
         }

         quad_damage = FALSE;

         global_quad = 0;
         sprintf( buf, "{G&R[&CANNOUNCEMENT&R]&c:{x {R%s has removed quad damage!{x\r\n", ch->name );
         talk_info( AT_GREEN, buf, FALSE );
         return;
      }
   }

   send_to_char( "Syntax: all <on|off> minutes.\r\n", ch );

   send_to_char( "Syntax: <type > <on|off> minutes.\r\n", ch );
   send_to_char( "Valid types are: exp | quest | quad\r\n", ch );
}

/* Shogar's code to hunt for exits/entrances to/from a zone, very nice */
/* Display improvements and overland support by Samson of Alsherok */
void do_aexit( CHAR_DATA * ch, char *argument )
{
   ROOM_INDEX_DATA *room;
   int i, vnum, lrange, trange;
   AREA_DATA *tarea, *otherarea;
   EXIT_DATA *pexit;
   bool found = FALSE;
#ifdef OVERLANDCODE
   ENTRANCE_DATA *enter;
#endif

   if( argument[0] == STRING_NULL )
      tarea = ch->in_room->area;
   else
   {
      for( tarea = first_area; tarea; tarea = tarea->next )
         if( !str_cmp( tarea->filename, argument ) )
         {
            found = TRUE;
            break;
         }

      if( !found )
      {
         for( tarea = first_build; tarea; tarea = tarea->next )
            if( !str_cmp( tarea->filename, argument ) )
            {
               found = TRUE;
               break;
            }
      }

      if( !found )
      {
         send_to_char( "Area not found. Check 'zones' for the filename.\r\n", ch );
         return;
      }
   }

   trange = tarea->hi_r_vnum;

   lrange = tarea->low_r_vnum;

   for( vnum = lrange; vnum <= trange; vnum++ )
   {
      if( ( room = get_room_index( vnum ) ) == NULL )
         continue;

      if( xIS_SET( room->room_flags, ROOM_TELEPORT ) && ( room->tele_vnum < lrange || room->tele_vnum > trange ) )
      {
         pager_printf( ch, "From: %-20.20s Room: %5d To: Room: %5d (Teleport)\r\n", tarea->filename, vnum, room->tele_vnum );
      }

      for( i = 0; i < MAX_DIR + 1; i++ )
      {
         if( ( pexit = get_exit( room, i ) ) == NULL )
            continue;

#ifdef OVERLANDCODE
         if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
         {
            pager_printf( ch, "To: Overland %4dX %4dY From: %20.20s Room: %5d (%s)\r\n", pexit->x, pexit->y, tarea->filename, vnum, dir_name[i] );
            continue;
         }

#endif
         if( pexit->to_room->area != tarea )
         {
            pager_printf( ch, "To: %-20.20s Room: %5d From: %-20.20s Room: %5d (%s)\r\n", pexit->to_room->area->filename, pexit->vnum, tarea->filename, vnum, dir_name[i] );
         }
      }
   }

   for( otherarea = first_area; otherarea; otherarea = otherarea->next )
   {
      if( tarea == otherarea )
         continue;

      trange = otherarea->hi_r_vnum;

      lrange = otherarea->low_r_vnum;

      for( vnum = lrange; vnum <= trange; vnum++ )
      {
         if( ( room = get_room_index( vnum ) ) == NULL )
            continue;

         if( xIS_SET( room->room_flags, ROOM_TELEPORT ) )
         {
            if( room->tele_vnum >= tarea->low_r_vnum && room->tele_vnum <= tarea->hi_r_vnum )
               pager_printf( ch, "From: %-20.20s Room: %5d To: %-20.20s Room: %5d (Teleport)\r\n", otherarea->filename, vnum, tarea->filename, room->tele_vnum );
         }

         for( i = 0; i < MAX_DIR + 1; i++ )
         {
            if( ( pexit = get_exit( room, i ) ) == NULL )
               continue;

#ifdef OVERLANDCODE
            if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
               continue;

#endif
            if( pexit->to_room->area == tarea )
            {
               pager_printf( ch, "From: %-20.20s Room: %5d To: %-20.20s Room: %5d (%s)\r\n",
                             otherarea->filename, vnum, pexit->to_room->area->filename, pexit->vnum, dir_name[i] );
            }
         }
      }
   }

#ifdef OVERLANDCODE
   for( enter = first_entrance; enter; enter = enter->next )
   {
      if( enter->vnum >= tarea->low_r_vnum && enter->vnum <= tarea->hi_r_vnum )
      {
         pager_printf( ch, "From: Overland %4dX %4dY To: Room: %5d\r\n", enter->herex, enter->herey, enter->vnum );
      }
   }

#endif
   return;
}

/* ltruncate a char string if it's length exceeds a given value. */
void ltrunc( char *s, int len )
{
   if( strlen( s ) > len )
      s[len] = STRING_NULL;
}

void do_equip_me( CHAR_DATA * ch )
{
   OBJ_DATA *obj;
   OBJ_INDEX_DATA *obj_ind;
   ROOM_INDEX_DATA *location;
   location = NULL;

//   if( !str_cmp( ch->in_room->area->filename, "school.are" ) )
//   {

      obj_ind = get_obj_index( 4000 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4001 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4002 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4003 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4004 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4005 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4006 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      obj_ind = get_obj_index( 4008 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

	obj_ind = get_obj_index( 4007 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

	obj_ind = get_obj_index( 4014 );

      if( obj_ind != NULL )
      {
         obj = create_object( obj_ind, ch->level );
         obj_to_char( obj, ch );
      }

      location = get_room_index( ROOM_VNUM_RECALL );
      char_from_room( ch );
      char_to_room( ch, location );
      do_look( ch, "auto" );
      send_to_char( "You have been successfuly equiped and have been transfered to the Academy. Goodluck.\r\n", ch );
      return;
//   }
//   else
      send_to_char( "You can only perform this command from with the New Player Tutorial \r\n", ch );
   return;
}

