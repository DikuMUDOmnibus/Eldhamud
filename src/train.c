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
 ****************************************************************************/
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "./Headers/mud.h"
void do_train( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char buf1[20];
   CHAR_DATA *mob = NULL;
   int add_hp = 0;
   int add_mana = 0;
   char *pOutput;
   int cost = 5;

   if( IS_NPC( ch ) )
   {
      send_to_char( "Not for NPCs.  Sorry.\r\n", ch );
      return;
   }
   /*
    * Check for trainer. 
    */
   for( mob = ch->in_room->first_person; mob; mob = mob->next_in_room )
   {
      if( IS_NPC( mob ) && IS_ACT_FLAG( mob, ACT_TRAINER ) )
         break;
   }
   if( mob == NULL )
   {
      send_to_char( "You can't do that here.\r\n", ch );
      return;
   }
   if( NULLSTR( argument ) )
   {
      sprintf( buf, "You have %d practice sessions.\r\n", ch->practice );
      send_to_char( buf, ch );

      strcpy( buf, "You can train: hp (10)  mana (10)" );
      if( ch->perm_str < 25 )
      {
         sprintf( buf1, " str (%d)", cost );
         strcat( buf, buf1 );
      }
      if( ch->perm_int < 25 )
      {
         sprintf( buf1, " int (%d)", cost );
         strcat( buf, buf1 );
      }
      if( ch->perm_wis < 25 )
      {
         sprintf( buf1, " wis (%d)", cost );
         strcat( buf, buf1 );
      }
      if( ch->perm_dex < 25 )
      {
         sprintf( buf1, " dex (%d)", cost );
         strcat( buf, buf1 );
      }
      if( ch->perm_con < 25 )
      {
         sprintf( buf1, " con (%d)", cost );
         strcat( buf, buf1 );
      }
      strcat( buf, ".\r\n" );
      send_to_char( buf, ch );
      return;
   }
   if( !str_cmp( argument, "str" ) )
   {
      if( ch->perm_str >= 25 )
      {
         act( AT_ACTION, "Your strength is already at maximum.", ch, NULL, pOutput, TO_CHAR );
         return;
      }
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= cost;
      ch->perm_str += 1;
      act( AT_ACTION, "Your strength increases!", ch, NULL, pOutput, TO_CHAR );
      act( AT_ACTION, "$n's strength increases!", ch, NULL, pOutput, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "int" ) )
   {
      if( ch->perm_int >= 25 )
      {
         act( AT_ACTION, "Your intelligence is already at maximum.", ch, NULL, pOutput, TO_CHAR );
         return;
      }
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= cost;
      ch->perm_int += 1;
      act( AT_ACTION, "Your intelligence increases!", ch, NULL, pOutput, TO_CHAR );
      act( AT_ACTION, "$n's intelligence increases!", ch, NULL, pOutput, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "wis" ) )
   {
      if( ch->perm_wis >= 25 )
      {
         act( AT_ACTION, "Your wisdom is already at maximum.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= cost;
      ch->perm_wis += 1;
      act( AT_ACTION, "Your wisdom increases!", ch, NULL, NULL, TO_CHAR );
      act( AT_ACTION, "$n's wisdom increases!", ch, NULL, NULL, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "dex" ) )
   {
      if( ch->perm_dex >= 25 )
      {
         act( AT_ACTION, "Your dexterity is already at maximum.", ch, NULL, NULL, TO_CHAR );
         return;
      }
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= cost;
      ch->perm_dex += 1;
      act( AT_ACTION, "Your dexterity increases!", ch, NULL, NULL, TO_CHAR );
      act( AT_ACTION, "$n's dexterity increases!", ch, NULL, NULL, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "con" ) )
   {
      if( ch->perm_con >= 25 )
      {
         act( AT_ACTION, "Your constitution is already at maximum.", ch, NULL, pOutput, TO_CHAR );
         return;
      }
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= cost;
      ch->perm_con += 1;
      act( AT_ACTION, "Your constitution increases!", ch, NULL, NULL, TO_CHAR );
      act( AT_ACTION, "$n's constitution increases!", ch, NULL, NULL, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "hp" ) )
   {
      pOutput = "number of hit points";
      cost = 10;  /* this is pracs per "train hp" */
      add_hp = 10;   /* this is hp gained per "train hp" */
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= 10;
      /*
       *AttrPerm           += add_hp;*/
      ch->max_hit += add_hp;
      act( AT_ACTION, "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
      act( AT_ACTION, "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
      return;
   }
   else if( !str_cmp( argument, "mana" ) )
   {
      pOutput = "amount of mana";
      cost = 10;
      add_mana = 10;
      if( cost > ch->practice )
      {
         send_to_char( "You don't have enough practices.\r\n", ch );
         return;
      }
      ch->practice -= 10;
      /*
       *AttrPerm           += add_mana;*/
      ch->max_mana += add_mana;
      act( AT_ACTION, "Your $T increases!", ch, NULL, pOutput, TO_CHAR );
      act( AT_ACTION, "$n's $T increases!", ch, NULL, pOutput, TO_ROOM );
      return;
   }
   send_to_char( "Sorry that is not a trainable stat, please check your spelling.\r\n", ch );
   return;
}
