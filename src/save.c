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
 *		     Character saving and loading module		    *
 ****************************************************************************/
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <dirent.h>
#include "./Headers/mud.h"
/*
 * Increment with every major format change.
 */
#define SAVEVERSION	4
/*
 * Array to keep track of equipment temporarily.		-Thoric
 */
OBJ_DATA *save_equipment[MAX_WEAR][MAX_LAYERS];
CHAR_DATA *quitting_char, *loading_char, *saving_char;
int file_ver;

/*
 * Array of containers read for proper re-nesting of objects.
 */
static OBJ_DATA *rgObjNest[MAX_NEST];
/*
 * Local functions.
 */
void fwrite_char args( ( CHAR_DATA * ch, FILE * fp ) );
void fread_char args( ( CHAR_DATA * ch, FILE * fp, bool preload, bool copyover ) );
/*
 * Un-equip character before saving to ensure proper	-Thoric
 * stats are saved in case of changes to or removal of EQ
 */
void de_equip_char( CHAR_DATA * ch )
{
   char buf[MAX_STRING_LENGTH];
   OBJ_DATA *obj;
   int x, y;
   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         save_equipment[x][y] = NULL;
   for( obj = ch->first_carrying; obj; obj = obj->next_content )
      if( obj->wear_loc > -1 && obj->wear_loc < MAX_WEAR )
      {
         if( get_trust( ch ) >= obj->level )
         {
            for( x = 0; x < MAX_LAYERS; x++ )
               if( !save_equipment[obj->wear_loc][x] )
               {
                  save_equipment[obj->wear_loc][x] = obj;
                  break;
               }
            if( x == MAX_LAYERS )
            {
               sprintf( buf, "%s had on more than %d layers of clothing in one location (%d): %s", ch->name, MAX_LAYERS, obj->wear_loc, obj->name );
               bug( buf, 0 );
            }
         }
         else
         {
            sprintf( buf, "%s had on %s:  ch->level = %d  obj->level = %d", ch->name, obj->name, ch->level, obj->level );
            bug( buf, 0 );
         }
         unequip_char( ch, obj );
      }
}

/*
 * Re-equip character					-Thoric
 */
void re_equip_char( CHAR_DATA * ch )
{
   int x, y;
   for( x = 0; x < MAX_WEAR; x++ )
      for( y = 0; y < MAX_LAYERS; y++ )
         if( save_equipment[x][y] != NULL )
         {
            if( quitting_char != ch )
               equip_char( ch, save_equipment[x][y], x );
            save_equipment[x][y] = NULL;
         }
         else
            break;
}

/*
 * Save a character and inventory.
 * Would be cool to save NPC's too for quest purposes,
 *   some of the infrastructure is provided.
 */
void save_char_obj( CHAR_DATA * ch )
{
   char strsave[MAX_INPUT_LENGTH];
   char strback[MAX_INPUT_LENGTH];
   FILE *fp;
   if( !ch )
   {
      bug( "Save_char_obj: null ch!", 0 );
      return;
   }
   if( IS_NPC( ch ) || ch->level < 2 )
      return;
   saving_char = ch;
   if( ch->desc && ch->desc->original )
      ch = ch->desc->original;
   de_equip_char( ch );
   ch->save_time = current_time;
   sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( ch->pcdata->filename[0] ), capitalize( ch->pcdata->filename ) );
   /*
    * Auto-backup pfile (can cause lag with high disk access situtations).
    */
   /*
    * Backup of each pfile on save as above can cause lag in high disk
    * access situations on big muds like Realms.  Quitbackup saves most
    * of that and keeps an adequate backup -- Blodkai, 10/97 
    */
   if( IS_SET( sysdata.save_flags, SV_BACKUP ) || ( IS_SET( sysdata.save_flags, SV_QUITBACKUP ) && quitting_char == ch ) )
   {
      sprintf( strback, "%s%c/%s", BACKUP_DIR, tolower( ch->pcdata->filename[0] ), capitalize( ch->pcdata->filename ) );
      rename( strsave, strback );
   }
   /*
    * Save immortal stats, level & vnums for wizlist      -Thoric
    * and do_vnums command
    *
    * Also save the player flags so we the wizlist builder can see
    * who is a guest and who is retired.
    */
   if( ch->level >= LEVEL_IMMORTAL )
   {
      sprintf( strback, "%s%s", GOD_DIR, capitalize( ch->pcdata->filename ) );
      if( ( fp = fopen( strback, "w" ) ) == NULL )
      {
         perror( strsave );
         bug( "Save_god_level: fopen", 0 );
      }
      else
      {
         fprintf( fp, "Level        %d\n", ch->level );
         fprintf( fp, "Pcflags      %d\n", ch->pcdata->flags );
         if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
            fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
         if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
            fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
         if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
            fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
         fclose( fp );
      }
   }
   if( ( fp = fopen( TEMP_FILE, "w" ) ) == NULL )
   {
      perror( strsave );
      bug( "Save_char_obj: fopen", 0 );
   }
   else
   {
      bool ferr;
      fwrite_char( ch, fp );
      if( ch->first_carrying )
         fwrite_obj( ch, ch->last_carrying, fp, 0, OS_CARRY, ch->pcdata->hotboot );
      if( sysdata.save_pets && ch->pcdata->pet )
         fwrite_mobile( fp, ch->pcdata->pet );
      fprintf( fp, "#END\n" );
      ferr = ferror( fp );
      fclose( fp );
      if( ferr )
      {
         perror( strsave );
         bug( "Error writing temp file for %s -- not copying", strsave );
      }
      else
         rename( TEMP_FILE, strsave );
   }
   re_equip_char( ch );
   quitting_char = NULL;
   saving_char = NULL;
   return;
}

/*
 * Write the char.
 */
void fwrite_char( CHAR_DATA * ch, FILE * fp )
{
   AFFECT_DATA *paf;
   int sn;
   short pos;
   SKILLTYPE *skill = NULL;
   fprintf( fp, "#PLAYER\n" );
   fprintf( fp, "Version      %d\n", SAVEVERSION );
   fprintf( fp, "Name         %s~\n", ch->name );
   if( ch->description[0] != STRING_NULL )
      fprintf( fp, "Description  %s~\n", ch->description );
   fprintf( fp, "Sex          %d\n", ch->sex );
   fprintf( fp, "Class        %d\n", ch->Class );
   fprintf( fp, "Race         %d\n", ch->race );
   fprintf( fp, "Languages    %d %d\n", ch->speaks, ch->speaking );
   fprintf( fp, "Level        %d\n", ch->level );
   fprintf( fp, "Played       %d\n", ch->played + ( int )( current_time - ch->logon ) );
   fprintf( fp, "Room         %d\n", ( ch->in_room == get_room_index( ROOM_VNUM_LIMBO ) && ch->was_in_room ) ? ch->was_in_room->vnum : ch->in_room->vnum );
   fprintf( fp, "HpManaMove   %d %d %d %d %d %d\n", ch->hit, ch->max_hit, ch->mana, ch->max_mana, ch->move, ch->max_move );
   fprintf( fp, "Gold         %d\n", ch->gold );
   fprintf( fp, "Exp          %d\n", ch->exp );
   fprintf( fp, "Height          %d\n", ch->height );
   fprintf( fp, "Weight          %d\n", ch->weight );
   if( !xIS_EMPTY( ch->act ) )
      fprintf( fp, "Act          %s\n", print_bitvector( &ch->act ) );
   if( !xIS_EMPTY( ch->affected_by ) )
      fprintf( fp, "AffectedBy   %s\n", print_bitvector( &ch->affected_by ) );
   if( !xIS_EMPTY( ch->no_affected_by ) )
      fprintf( fp, "NoAffectedBy %s\n", print_bitvector( &ch->no_affected_by ) );
   /*
    * Strip off fighting positions & store as
    * new style (pos>=100 flags new style in character loading)
    */
   pos = ch->position;
   if( pos == POS_BERSERK || pos == POS_AGGRESSIVE || pos == POS_FIGHTING || pos == POS_DEFENSIVE || pos == POS_EVASIVE )
      pos = POS_STANDING;
   pos += 100;
   fprintf( fp, "Position     %d\n", pos );
   fprintf( fp, "Style     %d\n", ch->style );
   fprintf( fp, "Practice     %d\n", ch->practice );
   fprintf( fp, "SavingThrows %d %d %d %d\n", ch->saving_poison_death, ch->saving_mental, ch->saving_physical, ch->saving_weapons );
   fprintf( fp, "Alignment    %d\n", ch->alignment );
   fprintf( fp, "Favor	       %d\n", ch->pcdata->favor );
   fprintf( fp, "Balance      %d\n", ch->pcdata->balance );
   fprintf( fp, "Glory        %d\n", ch->pcdata->quest_curr );
   fprintf( fp, "MGlory       %d\n", ch->pcdata->quest_accum );
   fprintf( fp, "Hitroll      %d\n", ch->hitroll );
   fprintf( fp, "Damroll      %d\n", ch->damroll );
   fprintf( fp, "Tattoo        %ld\n", ch->tattoo );
   fprintf( fp, "Armor        %d\n", ch->armor );
   fprintf( fp, "Remorts         %d\n", ch->remorts );
   if( ch->wimpy )
      fprintf( fp, "Wimpy        %d\n", ch->wimpy );
   if( ch->deaf )
      fprintf( fp, "Deaf         %d\n", ch->deaf );
   if( ch->resistant )
      fprintf( fp, "Resistant    %d\n", ch->resistant );
   if( ch->no_resistant )
      fprintf( fp, "NoResistant  %d\n", ch->no_resistant );
   if( ch->immune )
      fprintf( fp, "Immune       %d\n", ch->immune );
   if( ch->no_immune )
      fprintf( fp, "NoImmune     %d\n", ch->no_immune );
   if( ch->susceptible )
      fprintf( fp, "Susceptible  %d\n", ch->susceptible );
   if( ch->no_susceptible )
      fprintf( fp, "NoSusceptible  %d\n", ch->no_susceptible );
   if( ch->pcdata && ch->pcdata->outcast_time )
      fprintf( fp, "Outcast_time %ld\n", ch->pcdata->outcast_time );
   fprintf( fp, "Password     %s~\n", ch->pcdata->pwd );
   if( ch->pcdata->rank && ch->pcdata->rank[0] != STRING_NULL )
      fprintf( fp, "Rank         %s~\n", ch->pcdata->rank );
   if( ch->pcdata->bestowments && ch->pcdata->bestowments[0] != STRING_NULL )
      fprintf( fp, "Bestowments  %s~\n", ch->pcdata->bestowments );
   fprintf( fp, "Title        %s~\n", ch->pcdata->title );
   if( ch->pcdata->homepage && ch->pcdata->homepage[0] != STRING_NULL )
      fprintf( fp, "Homepage     %s~\n", ch->pcdata->homepage );
   if( ch->pcdata->bio && ch->pcdata->bio[0] != STRING_NULL )
      fprintf( fp, "Bio          %s~\n", ch->pcdata->bio );
   if( ch->pcdata->authed_by && ch->pcdata->authed_by[0] != STRING_NULL )
      fprintf( fp, "AuthedBy     %s~\n", ch->pcdata->authed_by );
   if( ch->pcdata->min_snoop )
      fprintf( fp, "Minsnoop     %d\n", ch->pcdata->min_snoop );
   if( ch->pcdata->prompt && *ch->pcdata->prompt )
      fprintf( fp, "Prompt       %s~\n", ch->pcdata->prompt );
   if( ch->pcdata->fprompt && *ch->pcdata->fprompt )
      fprintf( fp, "FPrompt	     %s~\n", ch->pcdata->fprompt );
   if( ch->pcdata->pagerlen != 24 )
      fprintf( fp, "Pagerlen     %d\n", ch->pcdata->pagerlen );
   fprintf( fp, "\n" );

   /*
    * If ch is ignoring players then store those players 
    */
   {
      IGNORE_DATA *temp;
      for( temp = ch->pcdata->first_ignored; temp; temp = temp->next )
      {
         fprintf( fp, "Ignored      %s~\n", temp->name );
      }
   }
   if( IS_IMMORTAL( ch ) )
   {
      if( ch->pcdata->bamfin && ch->pcdata->bamfin[0] != STRING_NULL )
         fprintf( fp, "Bamfin       %s~\n", ch->pcdata->bamfin );
      if( ch->pcdata->bamfout && ch->pcdata->bamfout[0] != STRING_NULL )
         fprintf( fp, "Bamfout      %s~\n", ch->pcdata->bamfout );
      if( ch->trust )
         fprintf( fp, "Trust        %d\n", ch->trust );
      if( ch->pcdata && ch->pcdata->restore_time )
         fprintf( fp, "Restore_time %ld\n", ch->pcdata->restore_time );
      fprintf( fp, "WizInvis     %d\n", ch->pcdata->wizinvis );
      if( ch->pcdata->r_range_lo && ch->pcdata->r_range_hi )
         fprintf( fp, "RoomRange    %d %d\n", ch->pcdata->r_range_lo, ch->pcdata->r_range_hi );
      if( ch->pcdata->o_range_lo && ch->pcdata->o_range_hi )
         fprintf( fp, "ObjRange     %d %d\n", ch->pcdata->o_range_lo, ch->pcdata->o_range_hi );
      if( ch->pcdata->m_range_lo && ch->pcdata->m_range_hi )
         fprintf( fp, "MobRange     %d %d\n", ch->pcdata->m_range_lo, ch->pcdata->m_range_hi );
   }
   if( ch->pcdata->deity_name && ch->pcdata->deity_name[0] != STRING_NULL )
      fprintf( fp, "Deity	     %s~\n", ch->pcdata->deity_name );
   if( ch->pcdata->clan_name && ch->pcdata->clan_name[0] != STRING_NULL )
      fprintf( fp, "Clan         %s~\n", ch->pcdata->clan_name );
   fprintf( fp, "Flags        %d\n", ch->pcdata->flags );
   if( ch->pcdata->release_date )
      fprintf( fp, "Helled       %d %s~\n", ( int )ch->pcdata->release_date, ch->pcdata->helled_by );
   fprintf( fp, "PKills       %d\n", ch->pcdata->pkills );
   fprintf( fp, "PDeaths      %d\n", ch->pcdata->pdeaths );
   if( get_timer( ch, TIMER_PKILLED ) && ( get_timer( ch, TIMER_PKILLED ) > 0 ) )
      fprintf( fp, "PTimer       %d\n", get_timer( ch, TIMER_PKILLED ) );
   fprintf( fp, "MKills       %d\n", ch->pcdata->mkills );
   fprintf( fp, "MDeaths      %d\n", ch->pcdata->mdeaths );
   fprintf( fp, "IllegalPK    %d\n", ch->pcdata->illegal_pk );
   fprintf( fp, "AttrPerm     %d %d %d %d %d %d %d\n", ch->perm_str, ch->perm_int, ch->perm_wis, ch->perm_dex, ch->perm_con, ch->perm_cha, ch->perm_lck );
   fprintf( fp, "AttrMod      %d %d %d %d %d %d %d\n", ch->mod_str, ch->mod_int, ch->mod_wis, ch->mod_dex, ch->mod_con, ch->mod_cha, ch->mod_lck );
   if( ch->desc && ch->desc->host )
      fprintf( fp, "Site         %s\n", ch->desc->host );
   else
      fprintf( fp, "Site         (Link-Dead)\n" );
   for( sn = 1; sn < top_sn; sn++ )
   {
      if( skill_table[sn]->name && ch->pcdata->learned[sn] > 0 )
         switch ( skill_table[sn]->type )
         {
            default:
               fprintf( fp, "Skill        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_SPELL:
               fprintf( fp, "Spell        %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_WEAPON:
               fprintf( fp, "Weapon       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
            case SKILL_TONGUE:
               fprintf( fp, "Tongue       %d '%s'\n", ch->pcdata->learned[sn], skill_table[sn]->name );
               break;
         }
   }
   for( paf = ch->first_affect; paf; paf = paf->next )
   {
      if( paf->type >= 0 && ( skill = get_skilltype( paf->type ) ) == NULL )
         continue;
      if( paf->type >= 0 && paf->type < TYPE_PERSONAL )
         fprintf( fp, "AffectData   '%s' %3d %3d %3d %s\n", skill->name, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
      else
         fprintf( fp, "Affect       %3d %3d %3d %3d %s\n", paf->type, paf->duration, paf->modifier, paf->location, print_bitvector( &paf->bitvector ) );
   }
   
   /*
    * Overland Map - Samson 7-31-99 
    */
   fprintf( fp, "Coordinates	%d %d %d\n", ch->x, ch->y, ch->map );
   if( ch->pcdata->nextquest != 0 )
      fprintf( fp, "NextQuest %d\n", ch->pcdata->nextquest );
   /*
    * Save color values - Samson 9-29-98 
    */
   {
      int x;
      fprintf( fp, "MaxColors    %d\n", MAX_COLORS );
      fprintf( fp, "Colors       " );
      for( x = 0; x < MAX_COLORS; x++ )
         fprintf( fp, "%d ", ch->colors[x] );
      fprintf( fp, "\n" );
   }
   #ifdef IMC
    imc_savechar( ch, fp );
   #endif
   fprintf( fp, "End\n\n" );
   return;
}

/*
 * Write an object and its contents.
 */
void fwrite_obj( CHAR_DATA * ch, OBJ_DATA * obj, FILE * fp, int iNest, short os_type, bool hotboot )
{
   EXTRA_DESCR_DATA *ed;
   AFFECT_DATA *paf;
   short wear, wear_loc, x;
   if( iNest >= MAX_NEST )
   {
      bug( "fwrite_obj: iNest hit MAX_NEST %d", iNest );
      return;
   }
   /*
    * Slick recursion to write lists backwards,
    *   so loading them will load in forwards order.
    */
   if( obj->prev_content && ( os_type != OS_CORPSE && os_type != OS_LOCKER ) )
      if( os_type == OS_CARRY )
         fwrite_obj( ch, obj->prev_content, fp, iNest, OS_CARRY, hotboot );
   /*
    * Castrate storage characters.
    * Catch deleted objects                                    -Thoric
    * Do NOT save prototype items!            -Thoric
    */
   if( !hotboot )
   {
      if( ( ch && ch->level < obj->level )
          || ( obj->item_type == ITEM_KEY && !IS_OBJ_STAT( obj, ITEM_CLANOBJECT ) ) || obj_extracted( obj ) || IS_OBJ_STAT( obj, ITEM_PROTOTYPE ) )
         return;
   }
   /*
    * Munch magic flagged containers for now - bandaid 
    */
   if( obj->item_type == ITEM_CONTAINER && IS_OBJ_STAT( obj, ITEM_MAGIC ) )
      xTOGGLE_BIT( obj->extra_flags, ITEM_MAGIC );
   /*
    * DO NOT save corpses lying on the ground as a hotboot item, they already saved elsewhere! - Samson 
    */
   if( hotboot && obj->item_type == ITEM_CORPSE_PC )
      return;
   /*
    * Corpse saving. -- Altrag 
    */
   fprintf( fp, ( os_type == OS_CORPSE ? "#CORPSE\n" : "#OBJECT\n" ) );
   if( iNest )
      fprintf( fp, "Nest         %d\n", iNest );
   if( obj->count > 1 )
      fprintf( fp, "Count        %d\n", obj->count );
   if( obj->name && obj->pIndexData->name && str_cmp( obj->name, obj->pIndexData->name ) )
      fprintf( fp, "Name         %s~\n", obj->name );
   if( obj->short_descr && obj->pIndexData->short_descr && str_cmp( obj->short_descr, obj->pIndexData->short_descr ) )
      fprintf( fp, "ShortDescr   %s~\n", obj->short_descr );
   if( obj->description && obj->pIndexData->description && str_cmp( obj->description, obj->pIndexData->description ) )
      fprintf( fp, "Description  %s~\n", obj->description );
   if( obj->action_desc && obj->pIndexData->action_desc && str_cmp( obj->action_desc, obj->pIndexData->action_desc ) )
      fprintf( fp, "ActionDesc   %s~\n", obj->action_desc );
   fprintf( fp, "Vnum         %d\n", obj->pIndexData->vnum );
   if( ( os_type == OS_CORPSE || os_type == OS_LOCKER || hotboot ) && obj->in_room )
   {
      fprintf( fp, "Room         %d\n", obj->in_room->vnum );
      fprintf( fp, "Rvnum	   %d\n", obj->room_vnum );
   }
   if( !xSAME_BITS( obj->extra_flags, obj->pIndexData->extra_flags ) )
      fprintf( fp, "ExtraFlags   %s\n", print_bitvector( &obj->extra_flags ) );
   if( obj->wear_flags != obj->pIndexData->wear_flags )
      fprintf( fp, "WearFlags    %d\n", obj->wear_flags );
   wear_loc = -1;
   for( wear = 0; wear < MAX_WEAR; wear++ )
      for( x = 0; x < MAX_LAYERS; x++ )
         if( obj == save_equipment[wear][x] )
         {
            wear_loc = wear;
            break;
         }
         else if( !save_equipment[wear][x] )
            break;
   if( wear_loc != -1 )
      fprintf( fp, "WearLoc      %d\n", wear_loc );
   if( obj->item_type != obj->pIndexData->item_type )
      fprintf( fp, "ItemType     %d\n", obj->item_type );
   if( obj->weight != obj->pIndexData->weight )
      fprintf( fp, "Weight       %d\n", obj->weight );
   if( obj->level )
      fprintf( fp, "Level        %d\n", obj->level );
   if( obj->timer )
      fprintf( fp, "Timer        %d\n", obj->timer );
   if( obj->cost != obj->pIndexData->cost )
      fprintf( fp, "Cost         %d\n", obj->cost );
   fprintf( fp, "Coords	%d %d %d\n", obj->x, obj->y, obj->map );
   if( obj->value[0] || obj->value[1] || obj->value[2] || obj->value[3] || obj->value[4] || obj->value[5] )
      fprintf( fp, "Values       %d %d %d %d %d %d\n", obj->value[0], obj->value[1], obj->value[2], obj->value[3], obj->value[4], obj->value[5] );
   switch ( obj->item_type )
   {
      case ITEM_PILL:  /* was down there with staff and wand, wrongly - Scryn */
      case ITEM_POTION:
      case ITEM_SCROLL:
         if( IS_VALID_SN( obj->value[1] ) )
            fprintf( fp, "Spell 1      '%s'\n", skill_table[obj->value[1]]->name );
         if( IS_VALID_SN( obj->value[2] ) )
            fprintf( fp, "Spell 2      '%s'\n", skill_table[obj->value[2]]->name );
         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );
         break;
      case ITEM_STAFF:
      case ITEM_WAND:
         if( IS_VALID_SN( obj->value[3] ) )
            fprintf( fp, "Spell 3      '%s'\n", skill_table[obj->value[3]]->name );
         break;
      case ITEM_SALVE:
         if( IS_VALID_SN( obj->value[4] ) )
            fprintf( fp, "Spell 4      '%s'\n", skill_table[obj->value[4]]->name );
         if( IS_VALID_SN( obj->value[5] ) )
            fprintf( fp, "Spell 5      '%s'\n", skill_table[obj->value[5]]->name );
         break;
   }
   for( paf = obj->first_affect; paf; paf = paf->next )
   {
      /*
       * Save extra object affects           -Thoric
       */
      if( paf->type < 0 || paf->type >= top_sn )
      {
         fprintf( fp, "Affect       %d %d %d %d %s\n",
                  paf->type,
                  paf->duration,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) ), paf->location, print_bitvector( &paf->bitvector ) );
      }
      else
         fprintf( fp, "AffectData   '%s' %d %d %d %s\n",
                  skill_table[paf->type]->name,
                  paf->duration,
                  ( ( paf->location == APPLY_WEAPONSPELL
                      || paf->location == APPLY_WEARSPELL
                      || paf->location == APPLY_REMOVESPELL
                      || paf->location == APPLY_STRIPSN
                      || paf->location == APPLY_RECURRINGSPELL )
                    && IS_VALID_SN( paf->modifier ) ), paf->location, print_bitvector( &paf->bitvector ) );
   }
   for( ed = obj->first_extradesc; ed; ed = ed->next )
      fprintf( fp, "ExtraDescr   %s~ %s~\n", ed->keyword, ed->description );
   fprintf( fp, "End\n\n" );
   if( obj->first_content )
      fwrite_obj( ch, obj->last_content, fp, iNest + 1, OS_CARRY, hotboot );
   return;
}

/*
 * Load a char and inventory into a new ch structure.
 */
bool load_char_obj( DESCRIPTOR_DATA * d, char *name, bool preload, bool copyover )
{
   char strsave[MAX_INPUT_LENGTH];
   CHAR_DATA *ch;
   FILE *fp;
   bool found;
   struct stat fst;
   int i, x;
   char buf[MAX_INPUT_LENGTH];
   CREATE( ch, CHAR_DATA, 1 );
   for( x = 0; x < MAX_WEAR; x++ )
      for( i = 0; i < MAX_LAYERS; i++ )
         save_equipment[x][i] = NULL;
   clear_char( ch );
   loading_char = ch;
   CREATE( ch->pcdata, PC_DATA, 1 );
   d->character = ch;
   ch->desc = d;
   ch->pcdata->filename = STRALLOC( name );
   ch->name = NULL;
   ch->act = multimeb( PLR_BLANK, PLR_COMBINE, PLR_PROMPT, -1 );
   ch->perm_str = 13;
   ch->perm_int = 13;
   ch->perm_wis = 13;
   ch->perm_dex = 13;
   ch->perm_con = 13;
   ch->perm_cha = 13;
   ch->perm_lck = 13;
   ch->no_resistant = 0;
   ch->no_susceptible = 0;
   ch->no_immune = 0;
   ch->was_in_room = NULL;
   xCLEAR_BITS( ch->no_affected_by );
   ch->pcdata->wizinvis = 0;
   ch->pcdata->balance = 0;
   ch->pcdata->charmies = 0;
   ch->mobinvis = 0;
   for( i = 0; i < MAX_SKILL; i++ )
      ch->pcdata->learned[i] = 0;
   ch->pcdata->release_date = 0;
   ch->pcdata->helled_by = NULL;
   ch->saving_poison_death = 0;
   ch->saving_mental = 0;
   ch->saving_physical = 0;
   ch->saving_weapons = 0;
   ch->remorts = 0;
   ch->style = STYLE_FIGHTING;
   ch->pcdata->pagerlen = 24;
   ch->pcdata->first_ignored = NULL;   /* Ignore list */
   ch->pcdata->last_ignored = NULL;
   ch->pcdata->lt_index = 0;  /* last tell index */
   ch->pcdata->secedit = SECT_OCEAN;   /* Initialize Map OLC sector - Samson 8-1-99 */
   ch->pcdata->hotboot = FALSE;  /* Never changed except when PC is saved during hotboot save */
   
   #ifdef IMC
    imc_initchar( ch );
   #endif
   found = FALSE;
   sprintf( strsave, "%s%c/%s", PLAYER_DIR, tolower( name[0] ), capitalize( name ) );
   if( stat( strsave, &fst ) != -1 )
   {
      if( fst.st_size == 0 )
      {
         sprintf( strsave, "%s%c/%s", BACKUP_DIR, tolower( name[0] ), capitalize( name ) );
         send_to_char( "Restoring your backup player file...", ch );
      }
      else
      {
         sprintf( buf, "%s player data for: %s (%dK)", preload ? "Preloading" : "Loading", ch->pcdata->filename, ( int )fst.st_size / 1024 );
         log_string_plus( buf, LOG_COMM, LEVEL_GREATER );
      }
   }
   /*
    * else no player file 
    */
   if( ( fp = fopen( strsave, "r" ) ) != NULL )
   {
      int iNest;
      for( iNest = 0; iNest < MAX_NEST; iNest++ )
         rgObjNest[iNest] = NULL;
      found = TRUE;
      /*
       * Cheat so that bug will show line #'s -- Altrag 
       */
      fpArea = fp;
      strcpy( strArea, strsave );
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
            bug( "Load_char_obj: # not found.", 0 );
            bug( name, 0 );
            break;
         }
         word = fread_word( fp );
         if( !strcmp( word, "PLAYER" ) )
         {
            fread_char( ch, fp, preload, copyover );
            if( preload )
               break;
         }
         else if( !strcmp( word, "OBJECT" ) )   /* Objects  */
            fread_obj( ch, fp, OS_CARRY );

         else if( !strcmp( word, "MOBILE" ) )
         {
            CHAR_DATA *mob;
            if( !( mob = fread_mobile( fp ) ) )
            {
               bug( "%s: Deleted mob saved on %s - skipping", __FUNCTION__, ch->name );
               break;
            }
            ch->pcdata->pet = mob;
            mob->master = ch;
            xSET_BIT( mob->affected_by, AFF_CHARM );
         }
         else if( !strcmp( word, "END" ) )   /* Done     */
            break;
         else
         {
            bug( "Load_char_obj: bad section.", 0 );
            bug( name, 0 );
            break;
         }
      }
      fclose( fp );
      fpArea = NULL;
      strcpy( strArea, "$" );
   }
   if( !found )
   {
      ch->name 			= STRALLOC( name );
      ch->short_descr 		= STRALLOC( "" );
      ch->long_descr 		= STRALLOC( "" );
      ch->description 		= STRALLOC( "" );
      ch->pcdata->mip_ver       = STRALLOC( "" );
      ch->pcdata->sec_code      = STRALLOC( "" );
      ch->editor 		= NULL;
      ch->pcdata->clan_name 	= STRALLOC( "" );
      ch->pcdata->clan 		= NULL;
      ch->pcdata->deity_name 	= STRALLOC( "" );
      ch->pcdata->deity 	= NULL;
      ch->pcdata->pet 		= NULL;
      ch->pcdata->pwd 		= str_dup( "" );
      ch->pcdata->bamfin 	= str_dup( "" );
      ch->pcdata->bamfout 	= str_dup( "" );
      ch->pcdata->rank 		= str_dup( "" );
      ch->pcdata->bestowments 	= str_dup( "" );
      ch->pcdata->title 	= STRALLOC( "" );
      ch->pcdata->homepage 	= str_dup( "" );
      ch->pcdata->bio 		= STRALLOC( "" );
      ch->pcdata->authed_by 	= STRALLOC( "" );
      ch->pcdata->prompt 	= STRALLOC( "" );
      ch->pcdata->fprompt 	= STRALLOC( "" );
      ch->pcdata->r_range_lo = 0;
      ch->pcdata->r_range_hi = 0;
      ch->pcdata->m_range_lo = 0;
      ch->pcdata->m_range_hi = 0;
      ch->pcdata->o_range_lo = 0;
      ch->pcdata->o_range_hi = 0;
      ch->pcdata->wizinvis = 0;
   }
   else
   {
      if( !ch->name )
         ch->name = STRALLOC( name );
      if( !ch->pcdata->clan_name )
      {
         ch->pcdata->clan_name = STRALLOC( "" );
         ch->pcdata->clan = NULL;
      }
      if ( !ch->pcdata->mip_ver )
       ch->pcdata->mip_ver = STRALLOC( "" );
      if ( !ch->pcdata->sec_code )
       ch->pcdata->sec_code = STRALLOC( "" );
      if( !ch->pcdata->deity_name )
      {
         ch->pcdata->deity_name = STRALLOC( "" );
         ch->pcdata->deity = NULL;
      }
      if( !ch->pcdata->bio )
         ch->pcdata->bio = STRALLOC( "" );
      if( !ch->pcdata->authed_by )
         ch->pcdata->authed_by = STRALLOC( "" );
      if( xIS_SET( ch->act, PLR_FLEE ) )
         xREMOVE_BIT( ch->act, PLR_FLEE );
      if( IS_IMMORTAL( ch ) )
      {
         if( ch->pcdata->wizinvis < 2 )
            ch->pcdata->wizinvis = ch->level;
         assign_area( ch );
      }
      if( file_ver > 1 )
      {
         for( i = 0; i < MAX_WEAR; i++ )
            for( x = 0; x < MAX_LAYERS; x++ )
               if( save_equipment[i][x] )
               {
                  equip_char( ch, save_equipment[i][x], i );
                  save_equipment[i][x] = NULL;
               }
               else
                  break;
      }
   }
   /*
    * Rebuild affected_by and RIS to catch errors - FB 
    */
   update_aris( ch );
   loading_char = NULL;
   return found;
}

/*
 * Read in a char.
 */
#if defined(KEY)
#undef KEY
#endif
#define KEY( literal, field, value )					\
				if ( !strcmp( word, literal ) )	\
				{					\
				    field  = value;			\
				    fMatch = TRUE;			\
				    break;				\
				}
void fread_char( CHAR_DATA * ch, FILE * fp, bool preload, bool copyover )
{
   char buf[MAX_STRING_LENGTH];
   char *line;
   char *word;
   int x1, x2, x3, x4, x5, x6, x7;
   short killcnt;
   bool fMatch;
   int max_colors = 0;  /* Color code */
   file_ver = 0;
   killcnt = 0;
   /*
    * Setup color values in case player has none set - Samson 
    */
   memcpy( &ch->colors, &default_set, sizeof( default_set ) );
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
            KEY( "Act", ch->act, fread_bitvector( fp ) );
            KEY( "AffectedBy", ch->affected_by, fread_bitvector( fp ) );
            KEY( "Alignment", ch->alignment, fread_number( fp ) );
            KEY( "Armor", ch->armor, fread_number( fp ) );
            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;
               if( preload )
               {
                  fMatch = TRUE;
                  fread_to_eol( fp );
                  break;
               }
               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;
                  char *sname = fread_word( fp );
                  if( ( sn = skill_lookup( sname ) ) < 0 )
                  {
                     if( ( sn = herb_lookup( sname ) ) < 0 )
                        bug( "Fread_char: unknown skill.", 0 );
                     else
                        sn += TYPE_HERB;
                  }
                  paf->type = sn;
               }
               paf->duration = fread_number( fp );
               paf->modifier = fread_number( fp );
               paf->location = fread_number( fp );
               paf->bitvector = fread_bitvector( fp );
               LINK( paf, ch->first_affect, ch->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "AttrMod" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 13;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->mod_str = x1;
               ch->mod_int = x2;
               ch->mod_wis = x3;
               ch->mod_dex = x4;
               ch->mod_con = x5;
               ch->mod_cha = x6;
               ch->mod_lck = x7;
               if( !x7 )
                  ch->mod_lck = 0;
               fMatch = TRUE;
               break;
            }

            if( !strcmp( word, "AttrPerm" ) )
            {
               line = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = x7 = 0;
               sscanf( line, "%d %d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6, &x7 );
               ch->perm_str = x1;
               ch->perm_int = x2;
               ch->perm_wis = x3;
               ch->perm_dex = x4;
               ch->perm_con = x5;
               ch->perm_cha = x6;
               ch->perm_lck = x7;
               if( !x7 || x7 == 0 )
                  ch->perm_lck = 13;
               fMatch = TRUE;
               break;
            }
            KEY( "AuthedBy", ch->pcdata->authed_by, fread_string( fp ) );
            break;
         case 'B':
            KEY( "Balance", ch->pcdata->balance, fread_number( fp ) );
            KEY( "Bamfin", ch->pcdata->bamfin, fread_string_nohash( fp ) );
            KEY( "Bamfout", ch->pcdata->bamfout, fread_string_nohash( fp ) );
            KEY( "Bestowments", ch->pcdata->bestowments, fread_string_nohash( fp ) );
            KEY( "Bio", ch->pcdata->bio, fread_string( fp ) );
            break;
         case 'C':
            if( !strcmp( word, "Clan" ) )
            {
               ch->pcdata->clan_name = fread_string( fp );
               if( !preload && ch->pcdata->clan_name[0] != STRING_NULL && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
               {
                  sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\r\nbelong to that organization.\r\n", ch->pcdata->clan_name );
                  send_to_char( buf, ch );
                  STRFREE( ch->pcdata->clan_name );
                  ch->pcdata->clan_name = STRALLOC( "" );
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Class", ch->Class, fread_number( fp ) );
            /*
             * Load color values - Samson 9-29-98 
             */
            {
               int x;
               if( !str_cmp( word, "Colors" ) )
               {
                  for( x = 0; x < max_colors; x++ )
                     ch->colors[x] = fread_number( fp );
                  fMatch = TRUE;
                  break;
               }
            }
            if( !str_cmp( word, "Coordinates" ) )
            {
               ch->x = fread_number( fp );
               ch->y = fread_number( fp );
               ch->map = fread_number( fp );
               if( !IS_PLR_FLAG( ch, PLR_ONMAP ) )
               {
                  ch->x = -1;
                  ch->y = -1;
                  ch->map = -1;
               }
               fMatch = TRUE;
               break;
            }
            break;
         case 'D':
            KEY( "Damroll", ch->damroll, fread_number( fp ) );
            KEY( "Deaf", ch->deaf, fread_number( fp ) );
            if( !strcmp( word, "Deity" ) )
            {
               ch->pcdata->deity_name = fread_string( fp );
               if( !preload && ch->pcdata->deity_name[0] != STRING_NULL && ( ch->pcdata->deity = get_deity( ch->pcdata->deity_name ) ) == NULL )
               {
                  sprintf( buf, "Warning: the deity %s no longer exists.\r\n", ch->pcdata->deity_name );
                  send_to_char( buf, ch );
                  STRFREE( ch->pcdata->deity_name );
                  ch->pcdata->deity_name = STRALLOC( "" );
                  ch->pcdata->favor = 0;
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Description", ch->description, fread_string( fp ) );
            break;
            /*
             * 'E' was moved to after 'S' 
             */
         case 'F':
            KEY( "Favor", ch->pcdata->favor, fread_number( fp ) );
            if( !strcmp( word, "Filename" ) )
            {
               /*
                * File Name already set externally.
                */
               fread_to_eol( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Flags", ch->pcdata->flags, fread_number( fp ) );
            KEY( "FPrompt", ch->pcdata->fprompt, fread_string( fp ) );
            break;
         case 'G':
            KEY( "Glory", ch->pcdata->quest_curr, fread_number( fp ) );
            KEY( "Gold", ch->gold, fread_number( fp ) );
            /*
             * temporary measure 
             */
            if( !strcmp( word, "Guild" ) )
            {
               ch->pcdata->clan_name = fread_string( fp );
               if( !preload && ch->pcdata->clan_name[0] != STRING_NULL && ( ch->pcdata->clan = get_clan( ch->pcdata->clan_name ) ) == NULL )
               {
                  sprintf( buf, "Warning: the organization %s no longer exists, and therefore you no longer\r\nbelong to that organization.\r\n", ch->pcdata->clan_name );
                  send_to_char( buf, ch );
                  STRFREE( ch->pcdata->clan_name );
                  ch->pcdata->clan_name = STRALLOC( "" );
               }
               fMatch = TRUE;
               break;
            }
            break;
         case 'H':
            KEY( "Height", ch->height, fread_number( fp ) );
            if( !strcmp( word, "Helled" ) )
            {
               ch->pcdata->release_date = fread_number( fp );
               ch->pcdata->helled_by = fread_string( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Hitroll", ch->hitroll, fread_number( fp ) );
            KEY( "Homepage", ch->pcdata->homepage, fread_string_nohash( fp ) );
            if( !strcmp( word, "HpManaMove" ) )
            {
               ch->hit = fread_number( fp );
               ch->max_hit = fread_number( fp );
               ch->mana = fread_number( fp );
               ch->max_mana = fread_number( fp );
               ch->move = fread_number( fp );
               ch->max_move = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;
         case 'I':
            if( !strcmp( word, "Ignored" ) )
            {
               char *temp;
               char fname[1024];
               struct stat fst;
               int ign;
               IGNORE_DATA *inode;
               /*
                * Get the name 
                */
               temp = fread_string( fp );
               sprintf( fname, "%s%c/%s", PLAYER_DIR, tolower( temp[0] ), capitalize( temp ) );
               /*
                * If there isn't a pfile for the name 
                */
               /*
                * then don't add it to the list       
                */
               if( stat( fname, &fst ) == -1 )
               {
                  if( temp )
                     STRFREE( temp );
                  fMatch = TRUE;
                  break;
               }
               /*
                * Count the number of names already ignored 
                */
               for( ign = 0, inode = ch->pcdata->first_ignored; inode; inode = inode->next )
               {
                  ign++;
               }
               /*
                * Add the name unless the limit has been reached 
                */
               if( ign >= MAX_IGN )
               {
                  bug( "fread_char: too many ignored names" );
               }
               else
               {
                  /*
                   * Add the name to the list 
                   */
                  CREATE( inode, IGNORE_DATA, 1 );
                  inode->name = STRALLOC( temp );
                  inode->next = NULL;
                  inode->prev = NULL;
                  LINK( inode, ch->pcdata->first_ignored, ch->pcdata->last_ignored, next, prev );
               }
               if( temp )
                  STRFREE( temp );
               fMatch = TRUE;
               break;
            }
            KEY( "IllegalPK", ch->pcdata->illegal_pk, fread_number( fp ) );
            KEY( "Immune", ch->immune, fread_number( fp ) );
            #ifdef IMC
            if( ( fMatch = imc_loadchar( ch, fp, word ) ) )
                break;
	    #endif

            break;
         case 'L':
            KEY( "Level", ch->level, fread_number( fp ) );
            KEY( "LongDescr", ch->long_descr, fread_string( fp ) );
            if( !strcmp( word, "Languages" ) )
            {
               ch->speaks = fread_number( fp );
               ch->speaking = fread_number( fp );
               fMatch = TRUE;
            }
            break;
         case 'M':
            KEY( "MaxColors", max_colors, fread_number( fp ) );
            KEY( "MDeaths", ch->pcdata->mdeaths, fread_number( fp ) );
            KEY( "MGlory", ch->pcdata->quest_accum, fread_number( fp ) );
            KEY( "Minsnoop", ch->pcdata->min_snoop, fread_number( fp ) );
            KEY( "MKills", ch->pcdata->mkills, fread_number( fp ) );
            KEY( "Mobinvis", ch->mobinvis, fread_number( fp ) );
            if( !strcmp( word, "MobRange" ) )
            {
               ch->pcdata->m_range_lo = fread_number( fp );
               ch->pcdata->m_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;
         case 'N':
            KEY( "Name", ch->name, fread_string( fp ) );
            KEY( "NoAffectedBy", ch->no_affected_by, fread_bitvector( fp ) );
            KEY( "NoImmune", ch->no_immune, fread_number( fp ) );
            KEY( "NoResistant", ch->no_resistant, fread_number( fp ) );
            KEY( "NoSusceptible", ch->no_susceptible, fread_number( fp ) );
            KEY( "NextQuest", ch->pcdata->nextquest, fread_number( fp ) );
            break;
         case 'O':
            KEY( "Outcast_time", ch->pcdata->outcast_time, fread_number( fp ) );
            if( !strcmp( word, "ObjRange" ) )
            {
               ch->pcdata->o_range_lo = fread_number( fp );
               ch->pcdata->o_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;
         case 'P':
            KEY( "Pagerlen", ch->pcdata->pagerlen, fread_number( fp ) );
            KEY( "Password", ch->pcdata->pwd, fread_string_nohash( fp ) );
            KEY( "PDeaths", ch->pcdata->pdeaths, fread_number( fp ) );
            KEY( "PKills", ch->pcdata->pkills, fread_number( fp ) );
            KEY( "Played", ch->played, fread_number( fp ) );
            /*
             * KEY( "Position", ch->position,     fread_number( fp ) );
             */
            /*
             *  new positions are stored in the file from 100 up
             *  old positions are from 0 up
             *  if reading an old position, some translation is necessary
             */
            if( !strcmp( word, "Position" ) )
            {
               ch->position = fread_number( fp );
               if( ch->position < 100 )
               {
                  switch ( ch->position )
                  {
                     default:
                        ;
                     case 0:
                        ;
                     case 1:
                        ;
                     case 2:
                        ;
                     case 3:
                        ;
                     case 4:
                        break;
                     case 5:
                        ch->position = 6;
                        break;
                     case 6:
                        ch->position = 8;
                        break;
                     case 7:
                        ch->position = 9;
                        break;
                     case 8:
                        ch->position = 12;
                        break;
                     case 9:
                        ch->position = 13;
                        break;
                     case 10:
                        ch->position = 14;
                        break;
                     case 11:
                        ch->position = 15;
                        break;
                  }
                  fMatch = TRUE;
               }
               else
               {
                  ch->position -= 100;
                  fMatch = TRUE;
               }
            }
            KEY( "Practice", ch->practice, fread_number( fp ) );
            KEY( "Prompt", ch->pcdata->prompt, fread_string( fp ) );
            if( !strcmp( word, "PTimer" ) )
            {
               add_timer( ch, TIMER_PKILLED, fread_number( fp ), NULL, 0 );
               fMatch = TRUE;
               break;
            }
            break;
         case 'R':
            KEY( "Race", ch->race, fread_number( fp ) );
            KEY( "Rank", ch->pcdata->rank, fread_string_nohash( fp ) );
            KEY( "Resistant", ch->resistant, fread_number( fp ) );
            KEY( "Restore_time", ch->pcdata->restore_time, fread_number( fp ) );
            KEY( "Remorts", ch->remorts, fread_number( fp ) );
            if( !strcmp( word, "Room" ) )
            {
               ch->in_room = get_room_index( fread_number( fp ) );
               if( !ch->in_room )
                  ch->in_room = get_room_index( ROOM_VNUM_LIMBO );
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "RoomRange" ) )
            {
               ch->pcdata->r_range_lo = fread_number( fp );
               ch->pcdata->r_range_hi = fread_number( fp );
               fMatch = TRUE;
            }
            break;
         case 'S':
            KEY( "Sex", ch->sex, fread_number( fp ) );
            KEY( "ShortDescr", ch->short_descr, fread_string( fp ) );
            KEY( "Style", ch->style, fread_number( fp ) );
            KEY( "Susceptible", ch->susceptible, fread_number( fp ) );
            if( !strcmp( word, "SavingThrows" ) )
            {
               ch->saving_poison_death = fread_number( fp );
               ch->saving_mental = fread_number( fp );
               ch->saving_physical = fread_number( fp );
               ch->saving_weapons = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "Site" ) )
            {
               if( !preload && !copyover )
                  ch_printf( ch, "Last connected from: %s\r\n", fread_word( fp ) );
               else
                  fread_to_eol( fp );
               fMatch = TRUE;
               if( preload )
                  word = "End";
               else
                  break;
            }
            if( !strcmp( word, "Skill" ) )
            {
               int sn;
               int value;
               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  if( file_ver < 3 )
                     sn = skill_lookup( fread_word( fp ) );
                  else
                     sn = bsearch_skill_exact( fread_word( fp ), gsn_first_skill, gsn_first_weapon - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown skill.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     /*
                      * Take care of people who have stuff they shouldn't     *
                      * * Assumes Class and level were loaded before. -- Altrag *
                      * * Assumes practices are loaded first too now. -- Altrag 
                      */
                     /*
                      * if ( ch->level < LEVEL_IMMORTAL )
                      * {
                      * if ( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                      * {
                      * ch->pcdata->learned[sn] = 0;
                      * ch->practice++;
                      * }
                      * } 
                      */
                  }
                  fMatch = TRUE;
                  break;
               }
            }
            if( !strcmp( word, "Spell" ) )
            {
               int sn;
               int value;
               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_spell, gsn_first_skill - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown spell.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ch->practice++;
                        }
                  }
                  fMatch = TRUE;
                  break;
               }
            }
            if( strcmp( word, "End" ) )
               break;
         case 'E':
            if( !strcmp( word, "End" ) )
            {
               if( !ch->short_descr )
                  ch->short_descr = STRALLOC( "" );
               if( !ch->long_descr )
                  ch->long_descr = STRALLOC( "" );
               if( !ch->description )
                  ch->description = STRALLOC( "" );
               if( !ch->pcdata->pwd )
                  ch->pcdata->pwd = str_dup( "" );
               if( !ch->pcdata->bamfin )
                  ch->pcdata->bamfin = str_dup( "" );
               if( !ch->pcdata->bamfout )
                  ch->pcdata->bamfout = str_dup( "" );
               if( !ch->pcdata->bio )
                  ch->pcdata->bio = STRALLOC( "" );
               if( !ch->pcdata->rank )
                  ch->pcdata->rank = str_dup( "" );
               if( !ch->pcdata->bestowments )
                  ch->pcdata->bestowments = str_dup( "" );
               if( !ch->pcdata->title )
                  ch->pcdata->title = STRALLOC( "" );
               if( !ch->pcdata->homepage )
                  ch->pcdata->homepage = str_dup( "" );
               if( !ch->pcdata->authed_by )
                  ch->pcdata->authed_by = STRALLOC( "" );
               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );
               if( !ch->pcdata->fprompt )
                  ch->pcdata->fprompt = STRALLOC( "" );
               ch->editor = NULL;
               if( !IS_IMMORTAL( ch ) && !ch->speaking )
                  ch->speaking = LANG_COMMON;
               if( !ch->pcdata->prompt )
                  ch->pcdata->prompt = STRALLOC( "" );
               /*
                * this disallows chars from being 6', 180lbs, but easier than a flag 
                */
               if( ch->height == 72 )
                  ch->height = number_range( race_table[ch->race]->height * .9, race_table[ch->race]->height * 1.1 );
               if( ch->weight == 180 )
                  ch->weight = number_range( race_table[ch->race]->weight * .9, race_table[ch->race]->weight * 1.1 );
               REMOVE_PLR_FLAG( ch, PLR_MAPEDIT ); /* In case they saved while editing */
               return;
            }
            KEY( "Exp", ch->exp, fread_number( fp ) );
            break;
         case 'T':
            if( !strcmp( word, "Tongue" ) )
            {
               int sn;
               int value;
               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_tongue, gsn_top_sn - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown tongue.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ch->practice++;
                        }
                  }
                  fMatch = TRUE;
               }
               break;
            }
            KEY( "Trust", ch->trust, fread_number( fp ) );
            /*
             * Let no character be trusted higher than one below maxlevel -- Narn 
             */
            ch->trust = UMIN( ch->trust, MAX_LEVEL - 1 );
            if( !strcmp( word, "Title" ) )
            {
               ch->pcdata->title = fread_string( fp );
               if( isalpha( ch->pcdata->title[0] ) || isdigit( ch->pcdata->title[0] ) )
               {
                  sprintf( buf, " %s", ch->pcdata->title );
                  if( ch->pcdata->title )
                     STRFREE( ch->pcdata->title );
                  ch->pcdata->title = STRALLOC( buf );
               }
               fMatch = TRUE;
               break;
            }
            KEY( "Tattoo", ch->tattoo, fread_number( fp ) );
            break;
         case 'V':
            if( !strcmp( word, "Vnum" ) )
            {
               ch->pIndexData = get_mob_index( fread_number( fp ) );
               fMatch = TRUE;
               break;
            }
            if( !str_cmp( word, "Version" ) )
            {
               file_ver = fread_number( fp );
               ch->pcdata->version = file_ver;
               fMatch = TRUE;
               break;
            }
            break;
         case 'W':
            KEY( "Weight", ch->weight, fread_number( fp ) );
            if( !strcmp( word, "Weapon" ) )
            {
               int sn;
               int value;
               if( preload )
                  word = "End";
               else
               {
                  value = fread_number( fp );
                  sn = bsearch_skill_exact( fread_word( fp ), gsn_first_weapon, gsn_first_tongue - 1 );
                  if( sn < 0 )
                     bug( "Fread_char: unknown weapon.", 0 );
                  else
                  {
                     ch->pcdata->learned[sn] = value;
                     if( ch->level < LEVEL_IMMORTAL )
                        if( skill_table[sn]->skill_level[ch->Class] >= LEVEL_IMMORTAL )
                        {
                           ch->pcdata->learned[sn] = 0;
                           ch->practice++;
                        }
                  }
                  fMatch = TRUE;
               }
               break;
            }
            KEY( "Wimpy", ch->wimpy, fread_number( fp ) );
            KEY( "WizInvis", ch->pcdata->wizinvis, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         sprintf( buf, "Fread_char: no match: %s", word );
         bug( buf, 0 );
      }
   }
}
void fread_obj( CHAR_DATA * ch, FILE * fp, short os_type )
{
   OBJ_DATA *obj;
   char *word;
   char buf[MAX_STRING_LENGTH];
   int iNest;
   bool fMatch;
   bool fNest;
   bool fVnum;
   ROOM_INDEX_DATA *room = NULL;
   if( ch )
   {
      room = ch->in_room;
      if( ch->tempnum == -9999 )
         file_ver = 0;
   }
   CREATE( obj, OBJ_DATA, 1 );
   obj->count = 1;
   obj->wear_loc = -1;
   obj->weight = 1;
   obj->map = -1;
   obj->x = -1;
   obj->y = -1;
   fNest = TRUE;  /* Requiring a Nest 0 is a waste */
   fVnum = TRUE;
   iNest = 0;
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
            KEY( "ActionDesc", obj->action_desc, fread_string( fp ) );
            if( !strcmp( word, "Affect" ) || !strcmp( word, "AffectData" ) )
            {
               AFFECT_DATA *paf;
               int pafmod;
               CREATE( paf, AFFECT_DATA, 1 );
               if( !strcmp( word, "Affect" ) )
               {
                  paf->type = fread_number( fp );
               }
               else
               {
                  int sn;
                  sn = skill_lookup( fread_word( fp ) );
                  if( sn < 0 )
                     bug( "Fread_obj: unknown skill.", 0 );
                  else
                     paf->type = sn;
               }
               paf->duration = fread_number( fp );
               pafmod = fread_number( fp );
               paf->location = fread_number( fp );
               paf->bitvector = fread_bitvector( fp );
               paf->modifier = pafmod;
               LINK( paf, obj->first_affect, obj->last_affect, next, prev );
               fMatch = TRUE;
               break;
            }
            break;
         case 'C':
            if( !strcmp( word, "Coords" ) )
            {
               obj->x = fread_number( fp );
               obj->y = fread_number( fp );
               obj->map = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            KEY( "Cost", obj->cost, fread_number( fp ) );
            KEY( "Count", obj->count, fread_number( fp ) );
            break;
         case 'D':
            KEY( "Description", obj->description, fread_string( fp ) );
            break;
         case 'E':
            KEY( "ExtraFlags", obj->extra_flags, fread_bitvector( fp ) );
            if( !strcmp( word, "ExtraDescr" ) )
            {
               EXTRA_DESCR_DATA *ed;
               CREATE( ed, EXTRA_DESCR_DATA, 1 );
               ed->keyword = fread_string( fp );
               ed->description = fread_string( fp );
               LINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
               fMatch = TRUE;
            }
            if( !strcmp( word, "End" ) )
            {
               if( !fNest || !fVnum )
               {
                  if( obj->name )
                     sprintf( buf, "Fread_obj: %s incomplete object.", obj->name );
                  else
                     sprintf( buf, "Fread_obj: incomplete object." );
                  bug( buf, 0 );
                  if( obj->name )
                     STRFREE( obj->name );
                  if( obj->description )
                     STRFREE( obj->description );
                  if( obj->short_descr )
                     STRFREE( obj->short_descr );
                  DISPOSE( obj );
                  return;
               }
               else
               {
                  short wear_loc = obj->wear_loc;
                  if( !obj->name )
                     obj->name = QUICKLINK( obj->pIndexData->name );
                  if( !obj->description )
                     obj->description = QUICKLINK( obj->pIndexData->description );
                  if( !obj->short_descr )
                     obj->short_descr = QUICKLINK( obj->pIndexData->short_descr );
                  if( !obj->action_desc )
                     obj->action_desc = QUICKLINK( obj->pIndexData->action_desc );
                  LINK( obj, first_object, last_object, next, prev );
                  obj->pIndexData->count += obj->count;
                  if( !obj->serial )
                  {
                     cur_obj_serial = UMAX( ( cur_obj_serial + 1 ) & ( BV30 - 1 ), 1 );
                     obj->serial = obj->pIndexData->serial = cur_obj_serial;
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  numobjsloaded += obj->count;
                  ++physicalobjects;
                  if( file_ver > 1 || obj->wear_loc < -1 || obj->wear_loc >= MAX_WEAR )
                     obj->wear_loc = -1;
                  /*
                   * Corpse saving. -- Altrag 
                   */
                  if( os_type == OS_CORPSE )
                  {
                     if( !room )
                     {
                        bug( "Fread_obj: Corpse without room", 0 );
                        room = get_room_index( ROOM_VNUM_LIMBO );
                     }
                     /*
                      * Give the corpse a timer if there isn't one 
                      */
                     if( obj->timer < 1 )
                        obj->timer = 40;
                     if( room->vnum == ROOM_VNUM_HALLOFFALLEN && obj->first_content )
                        obj->timer = -1;
                     obj = obj_to_room( obj, room, NULL );
                  }
                  else if( os_type == OS_LOCKER )
                  {
                     obj = obj_to_room( obj, ch->pcdata->locker_room, ch );
                  }
                  else if( iNest == 0 || rgObjNest[iNest] == NULL )
                  {
                     int slot = -1;
                     bool reslot = FALSE;
                     if( file_ver > 1 && wear_loc > -1 && wear_loc < MAX_WEAR )
                     {
                        int x;
                        for( x = 0; x < MAX_LAYERS; x++ )
                           if( !save_equipment[wear_loc][x] )
                           {
                              save_equipment[wear_loc][x] = obj;
                              slot = x;
                              reslot = TRUE;
                              break;
                           }
                        if( x == MAX_LAYERS )
                           bug( "Fread_obj: too many layers %d", wear_loc );
                     }
                     obj = obj_to_char( obj, ch );
                     if( reslot && slot != -1 )
                        save_equipment[wear_loc][slot] = obj;
                  }
                  else
                  {
                     if( rgObjNest[iNest - 1] )
                     {
                        separate_obj( rgObjNest[iNest - 1] );
                        obj = obj_to_obj( obj, rgObjNest[iNest - 1] );
                     }
                     else
                        bug( "Fread_obj: nest layer missing %d", iNest - 1 );
                  }
                  if( fNest )
                     rgObjNest[iNest] = obj;
                  return;
               }
            }
            break;
         case 'I':
            KEY( "ItemType", obj->item_type, fread_number( fp ) );
            break;
         case 'L':
            KEY( "Level", obj->level, fread_number( fp ) );
            break;
         case 'N':
            KEY( "Name", obj->name, fread_string( fp ) );
            if( !strcmp( word, "Nest" ) )
            {
               iNest = fread_number( fp );
               if( iNest < 0 || iNest >= MAX_NEST )
               {
                  bug( "Fread_obj: bad nest %d.", iNest );
                  iNest = 0;
                  fNest = FALSE;
               }
               fMatch = TRUE;
            }
            break;
         case 'R':
            KEY( "Room", room, get_room_index( fread_number( fp ) ) );
            KEY( "Rvnum", obj->room_vnum, fread_number( fp ) );
            break;
         case 'S':
            KEY( "ShortDescr", obj->short_descr, fread_string( fp ) );
            if( !strcmp( word, "Spell" ) )
            {
               int iValue;
               int sn;
               iValue = fread_number( fp );
               sn = skill_lookup( fread_word( fp ) );
               if( iValue < 0 || iValue > 5 )
                  bug( "Fread_obj: bad iValue %d.", iValue );
               else if( sn < 0 )
                  bug( "Fread_obj: unknown skill.", 0 );
               else
                  obj->value[iValue] = sn;
               fMatch = TRUE;
               break;
            }
            break;
         case 'T':
            KEY( "Timer", obj->timer, fread_number( fp ) );
            break;
         case 'V':
            if( !strcmp( word, "Values" ) )
            {
               int x1, x2, x3, x4, x5, x6;
               char *ln = fread_line( fp );
               x1 = x2 = x3 = x4 = x5 = x6 = 0;
               sscanf( ln, "%d %d %d %d %d %d", &x1, &x2, &x3, &x4, &x5, &x6 );
               obj->value[0] = x1;
               obj->value[1] = x2;
               obj->value[2] = x3;
               obj->value[3] = x4;
               obj->value[4] = x5;
               obj->value[5] = x6;
               fMatch = TRUE;
               break;
            }
            if( !strcmp( word, "Vnum" ) )
            {
               int vnum;
               vnum = fread_number( fp );
               /*
                * bug( "Fread_obj: bad vnum %d.", vnum );  
                */
               if( ( obj->pIndexData = get_obj_index( vnum ) ) == NULL )
                  fVnum = FALSE;
               else
               {
                  fVnum = TRUE;
                  obj->cost = obj->pIndexData->cost;
                  obj->weight = obj->pIndexData->weight;
                  obj->item_type = obj->pIndexData->item_type;
                  obj->wear_flags = obj->pIndexData->wear_flags;
                  obj->extra_flags = obj->pIndexData->extra_flags;
               }
               fMatch = TRUE;
               break;
            }
            break;
         case 'W':
            KEY( "WearFlags", obj->wear_flags, fread_number( fp ) );
            KEY( "WearLoc", obj->wear_loc, fread_number( fp ) );
            KEY( "Weight", obj->weight, fread_number( fp ) );
            break;
      }
      if( !fMatch )
      {
         EXTRA_DESCR_DATA *ed;
         AFFECT_DATA *paf;
         bug( "Fread_obj: no match.", 0 );
         bug( word, 0 );
         fread_to_eol( fp );
         if( obj->name )
            STRFREE( obj->name );
         if( obj->description )
            STRFREE( obj->description );
         if( obj->short_descr )
            STRFREE( obj->short_descr );
         while( ( ed = obj->first_extradesc ) != NULL )
         {
            STRFREE( ed->keyword );
            STRFREE( ed->description );
            UNLINK( ed, obj->first_extradesc, obj->last_extradesc, next, prev );
            DISPOSE( ed );
         }
         while( ( paf = obj->first_affect ) != NULL )
         {
            UNLINK( paf, obj->first_affect, obj->last_affect, next, prev );
            DISPOSE( paf );
         }
         DISPOSE( obj );
         return;
      }
   }
}
void set_alarm( long seconds )
{
   alarm( seconds );
}

/*
 * Based on last time modified, show when a player was last on	-Thoric
 */
void do_last( CHAR_DATA * ch, char *argument )
{
   char buf[MAX_STRING_LENGTH];
   char arg[MAX_INPUT_LENGTH];
   char name[MAX_INPUT_LENGTH];
   struct stat fst;
   argument = one_argument( argument, arg );
   if( arg[0] == STRING_NULL )
   {
      send_to_char( "Usage: last <playername>\r\n", ch );
      send_to_char( "Usage: last <# of entries OR \'-1\' for all entries OR \'today\' for all of today's entries>\r\n", ch );
      send_to_char( "Usage: last <playername> <count>\r\n", ch );
      return;
   }
   if( get_trust( ch ) < LEVEL_ASCENDANT )
   {
      set_char_color( AT_IMMORT, ch );
      send_to_char( "Their godly glow prevents you from getting a good look.\r\n", ch );
      return;
   }
   if( isdigit( arg[0] ) || atoi( arg ) == -1 || !str_cmp( arg, "today" ) )   /*View list instead of players */
   {
      send_to_char( "--------------------------------------------------------------------------------\r\n", ch );
      send_to_char( "  Todays Logins								     \r\n", ch );
      send_to_char( "--------------------------------------------------------------------------------\r\n", ch );
      send_to_char( " &CName                     Time                        Host/Ip&w               \r\n", ch );
      send_to_char( "--------------------------------------------------------------------------------\r\n", ch );
      if( !str_cmp( arg, "today" ) )
         read_last_file( ch, -2, NULL );
      else
         read_last_file( ch, atoi( arg ), NULL );
      return;
   }
   strcpy( name, capitalize( arg ) );
   
   sprintf( buf, "%s%c/%s", PLAYER_DIR, tolower( arg[0] ), name );
   strcpy( name, capitalize( arg ) );

   if( stat( buf, &fst ) != -1 )
      sprintf( buf, "%s was last on: %s\r", name, ctime( &fst.st_mtime ) );
   else
      sprintf( buf, "%s was not found.\r\n", name );
   send_to_char( buf, ch );
}

/*
 * This will write one mobile structure pointed to be fp --Shaddai
 */
void fwrite_mobile( FILE * fp, CHAR_DATA * mob )
{
   if( !IS_NPC( mob ) || !fp )
      return;
   fprintf( fp, "#MOBILE\n" );
   fprintf( fp, "Vnum	%d\n", mob->pIndexData->vnum );
   if( mob->in_room )
      fprintf( fp, "Room	%d\n", ( mob->in_room == get_room_index( ROOM_VNUM_LIMBO ) && mob->was_in_room ) ? mob->was_in_room->vnum : mob->in_room->vnum );
   fprintf( fp, "Coordinates  %d %d %d\n", mob->x, mob->y, mob->map );
   if( mob->name && mob->pIndexData->player_name && str_cmp( mob->name, mob->pIndexData->player_name ) )
      fprintf( fp, "Name     %s~\n", mob->name );
   if( mob->short_descr && mob->pIndexData->short_descr && str_cmp( mob->short_descr, mob->pIndexData->short_descr ) )
      fprintf( fp, "Short    %s~\n", mob->short_descr );
   if( mob->long_descr && mob->pIndexData->long_descr && str_cmp( mob->long_descr, mob->pIndexData->long_descr ) )
      fprintf( fp, "Long    %s~\n", mob->long_descr );
   if( mob->description && mob->pIndexData->description && str_cmp( mob->description, mob->pIndexData->description ) )
      fprintf( fp, "Description %s~\n", mob->description );
   fprintf( fp, "Position %d\n", mob->position );
   fprintf( fp, "Flags %s\n", print_bitvector( &mob->act ) );
   /*
    * Might need these later --Shaddai
    * de_equip_char( mob );
    * re_equip_char( mob );
    */
   if( mob->first_carrying )
      fwrite_obj( mob, mob->last_carrying, fp, 0, OS_CARRY, FALSE );
   fprintf( fp, "EndMobile\n" );
   return;
}

/*
 * This will read one mobile structure pointer to by fp --Shaddai
 */
CHAR_DATA *fread_mobile( FILE * fp )
{
   CHAR_DATA *mob = NULL;
   char *word;
   bool fMatch;
   int inroom = 0;
   ROOM_INDEX_DATA *pRoomIndex = NULL;
   word = feof( fp ) ? "EndMobile" : fread_word( fp );
   if( !strcmp( word, "Vnum" ) )
   {
      int vnum;
      vnum = fread_number( fp );
      mob = create_mobile( get_mob_index( vnum ) );
      if( !mob )
      {
         for( ;; )
         {
            word = feof( fp ) ? "EndMobile" : fread_word( fp );
            /*
             * So we don't get so many bug messages when something messes up
             * * --Shaddai 
             */
            if( !strcmp( word, "EndMobile" ) )
               break;
         }
         bug( "Fread_mobile: No index data for vnum %d", vnum );
         return NULL;
      }
   }
   else
   {
      for( ;; )
      {
         word = feof( fp ) ? "EndMobile" : fread_word( fp );
         /*
          * So we don't get so many bug messages when something messes up
          * * --Shaddai 
          */
         if( !strcmp( word, "EndMobile" ) )
            break;
      }
      extract_char( mob, TRUE );
      bug( "Fread_mobile: Vnum not found", 0 );
      return NULL;
   }
   for( ;; )
   {
      word = feof( fp ) ? "EndMobile" : fread_word( fp );
      fMatch = FALSE;
      switch ( UPPER( word[0] ) )
      {
         case '*':
            fMatch = TRUE;
            fread_to_eol( fp );
            break;
         case '#':
            if( !strcmp( word, "#OBJECT" ) )
               fread_obj( mob, fp, OS_CARRY );
         case 'C':
            if( !str_cmp( word, "Coordinates" ) )
            {
               mob->x = fread_number( fp );
               mob->y = fread_number( fp );
               mob->map = fread_number( fp );
               fMatch = TRUE;
               break;
            }
            break;
         case 'D':
            KEY( "Description", mob->description, fread_string( fp ) );
            break;
         case 'E':
            if( !strcmp( word, "EndMobile" ) )
            {
               if( inroom == 0 )
                  inroom = ROOM_VNUM_TEMPLE;
               pRoomIndex = get_room_index( inroom );
               if( !pRoomIndex )
                  pRoomIndex = get_room_index( ROOM_VNUM_TEMPLE );
               char_to_room( mob, pRoomIndex );
               return mob;
            }
            break;
         case 'F':
            KEY( "Flags", mob->act, fread_bitvector( fp ) );
         case 'L':
            KEY( "Long", mob->long_descr, fread_string( fp ) );
            break;
         case 'N':
            KEY( "Name", mob->name, fread_string( fp ) );
            break;
         case 'P':
            KEY( "Position", mob->position, fread_number( fp ) );
            break;
         case 'R':
            KEY( "Room", inroom, fread_number( fp ) );
            break;
         case 'S':
            KEY( "Short", mob->short_descr, fread_string( fp ) );
            break;
      }
      if( !fMatch )
      {
         bug( "%s: no match: %s", __FUNCTION__, word );
         fread_to_eol( fp );
      }
   }
   return NULL;
}

/*
 * This will write in the saved mobile for a char --Shaddai
 */
void write_char_mobile( CHAR_DATA * ch, char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MAX_STRING_LENGTH];
   if( IS_NPC( ch ) || !ch->pcdata->pet )
      return;
   fclose( fpReserve );
   if( ( fp = fopen( argument, "w" ) ) == NULL )
   {
      sprintf( buf, "Write_char_mobile: couldn't open %s for writing!\r\n", argument );
      bug( buf, 0 );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   mob = ch->pcdata->pet;
   xSET_BIT( mob->affected_by, AFF_CHARM );
   fwrite_mobile( fp, mob );
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}

/*
 * This will read in the saved mobile for a char --Shaddai
 */
void read_char_mobile( char *argument )
{
   FILE *fp;
   CHAR_DATA *mob;
   char buf[MAX_STRING_LENGTH];
   fclose( fpReserve );
   if( ( fp = fopen( argument, "r" ) ) == NULL )
   {
      sprintf( buf, "Read_char_mobile: couldn't open %s for reading!\r\n", argument );
      bug( buf, 0 );
      fpReserve = fopen( NULL_FILE, "r" );
      return;
   }
   mob = fread_mobile( fp );
   fclose( fp );
   fpReserve = fopen( NULL_FILE, "r" );
   return;
}
