/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 ****************************************************************************/
#include "./Headers/mud.h"
void update_all_qobjs( CHAR_DATA * ch );
void do_remort( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_INPUT_LENGTH];
   int vhit, vmana, vmove, wear_loc, value, sn;
   OBJ_DATA *obj, *obj_next;
   if( IS_NPC( ch ) )
   {
      send_to_char( "I'm sorry, mobs cannot remort!\r\n", ch );
      return;
   }
   if( ch->level == LEVEL_IMMORTAL )
   {
      send_to_char( "Heros cannot be remorted, sorry!\r\n", ch );
      return;
   }
   if( ch->level > LEVEL_HERO )
   {
      send_to_char( "Immortals cannot be remorted, sorry!\r\n", ch );
      return;
   }
   if( ch->remorts >= 3 )
   {
      send_to_char( "This char has already been remorted too many times.\r\n", ch );
      return;
   }

   send_to_char( "Having conquered that which has been set before you, you demand that\r\n"
                 "the immortals buff you up a little more, maybe with eq or a nifty set of\r\n"
                 "commands. Well it seems your ego has gotten you into some trouble, as\r\n"
                 "your mind becomes instantly filled with a loud thunder and images of death.\r\n" 
		 "Maybe today was the wrong time to annoy the immortals...\r\n", ch );
   for( wear_loc = WEAR_LIGHT; wear_loc <= MAX_WEAR; wear_loc++ )
   {
      if( ( obj = get_eq_char( ch, wear_loc ) ) == NULL )
         continue;
      unequip_char( ch, obj );
   }
   for( obj = ch->first_carrying; obj != NULL; obj = obj_next )
   {
      obj_next = obj->next_content;
      if( xIS_SET( obj->extra_flags, ITEM_QUEST ) )
         continue;
      else
      {
         obj_from_char( obj );
         extract_obj( obj );
      }
   }
   do_restore( ch, ch->name );
   vhit = ch->hit /= 3;
   vmana = ch->mana /= 3;
   vmove = ch->move /= 3;
   ch->max_hit = vhit;
   ch->max_mana = vmana;
   ch->max_move = vmove;
   ch->hit = vhit;
   ch->mana = vmana;
   ch->move = vmove;
   ch->level = 10;
   ch->exp = 0;
   ch->perm_str = 13;
   ch->perm_con = 13;
   ch->perm_cha = 13;
   ch->perm_lck = 13;
   ch->perm_wis = 13;
   ch->perm_int = 13;
   ch->perm_dex = 13;
   ch->armor = 100;
   ch->hitroll = 0;
   ch->damroll = 0;
   ch->remorts++;
   for( sn = 0; sn < top_sn; sn++ )
   {
      if( skill_table[sn]->name && ( ch->level >= skill_table[sn]->skill_level[ch->Class] || value == 0 ) )
      {
         ch->pcdata->learned[sn] = 0;
      }
   }

   send_to_char( "For all your hard work, the immortals wish to bless you with the following skills.\r\n", ch );
   if( ch->remorts > 4 )
   {
   }
   if( ch->remorts > 3 )
   {
   }
   if( ch->remorts > 2 )
   {
   }
   if( ch->remorts > 1 )
   {
   }
   if( ch->remorts > 0 )
   {
   }
   sprintf( buf, "&R[&CANNOUNCEMENT&R]&c:{xAll congratulate {R%s{x, who has just remorted. {x\r\n", ch->name );
   talk_info( AT_PURPLE, buf, TRUE );
   do_double( ch, "exp on 60" );
   update_all_qobjs( ch );
   return;
}
