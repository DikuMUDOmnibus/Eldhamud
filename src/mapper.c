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
*  In order to use any part of this Merc Diku Mud, you must comply with     *
*  both the original Diku license in 'license.doc' as well the Merc         *
*  license in 'license.txt'.  In particular, you may not remove either of   *
*  these copyright notices.                                                 *
*                                                                           *
*  Dystopia Mud improvements copyright (C) 2000, 2001 by Brian Graversen    *
*                                                                           *
*  Much time and thought has gone into this software and you are            *
*  benefitting.  We hope that you share your changes too.  What goes        *
*  around, comes around.                                                    *
*****************************************************************************
*  Converted for AFKMud 1.64 by Zarius (jeff@mindcloud.com)                 *
*  Downloaded from http://www.mindcloud.com                                 *
*  If you like the snippet let me know                                      *
****************************************************************************/
/****************************************************************************
 * 	                       Version History                              *
 ****************************************************************************
 *  (v1.0) - Converted Automapper to AFKMud 1.64 and added additional       *
 *           directions and removed room desc code into a sep func          *
 ***************************************************************************/
/*
	 TO DO
   1. Add a way of displaying up and down directions effectively
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "./Headers/mud.h"
#include "./Headers/mapper.h"
void show_char_to_char( CHAR_DATA * list, CHAR_DATA * ch );
void display_imm_toggle( CHAR_DATA * ch );
/* The map itself */
struct map_type map[MAPX + 1][MAPY + 1];
size_t mudstrlcat( char *dst, const char *src, size_t siz )
{
   register char *d = dst;
   register const char *s = src;
   register size_t n = siz;
   size_t dlen;
   /*
    * Find the end of dst and adjust bytes left but don't go past end 
    */
   while( n-- != 0 && *d != STRING_NULL )
      d++;
   dlen = d - dst;
   n = siz - dlen;
   if( n == 0 )
      return ( dlen + strlen( s ) );
   while( *s != STRING_NULL )
   {
      if( n != 1 )
      {
         *d++ = *s;
         n--;
      }
      s++;
   }
   *d = STRING_NULL;
   return ( dlen + ( s - src ) );   /* count does not include NUL */
}

char *roomdesc( CHAR_DATA * ch )
{
   static char outbuf[MAX_STRING_LENGTH];
   /*
    * Build a room desc moved here -- Zarius 
    */
   outbuf[0] = STRING_NULL;
   mudstrlcat( outbuf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH ); /* Set the color */
   mudstrlcat( outbuf, ch->in_room->description, MAX_STRING_LENGTH );
   return outbuf;
}
void do_showmap( CHAR_DATA * ch, char *argument )
{
   draw_map( ch, roomdesc( ch ) );
   return;
}

/* Take care of some repetitive code for later */
void get_exit_dir( int dir, int *x, int *y, int xorig, int yorig )
{
   /*
    * Get the next coord based on direction 
    */
   switch ( dir )
   {
      case 0: /* North */
         *x = xorig;
         *y = yorig - 1;
         break;
      case 1: /* East */
         *x = xorig + 1;
         *y = yorig;
         break;
      case 2: /* South */
         *x = xorig;
         *y = yorig + 1;
         break;
      case 3: /* West */
         *x = xorig - 1;
         *y = yorig;
         break;
      case 4: /* UP */
         break;
      case 5: /* DOWN */
         break;
      case 6: /* NE */
         *x = xorig + 1;
         *y = yorig - 1;
         break;
      case 7: /* NW */
         *x = xorig - 1;
         *y = yorig - 1;
         break;
      case 8: /* SE */
         *x = xorig + 1;
         *y = yorig + 1;
         break;
      case 9: /* SW */
         *x = xorig - 1;
         *y = yorig + 1;
         break;
      default:
         *x = -1;
         *y = -1;
         break;
   }
}
char *get_exits( CHAR_DATA * ch )
{
   static char buf[MAX_STRING_LENGTH];
   EXIT_DATA *pexit;
   bool found;
   buf[0] = STRING_NULL;
   if( !check_blind( ch ) )
      return buf;
   set_char_color( AT_EXITS, ch );
   mudstrlcpy( buf, "&R[&DExits:", MAX_STRING_LENGTH );
   found = FALSE;
   for( pexit = ch->in_room->first_exit; pexit; pexit = pexit->next )
   {
      if( IS_IMMORTAL( ch ) )
         /*
          * Immortals see all exits, even secret ones 
          */
      {
         if( pexit->to_room )
         {
            found = TRUE;
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );
            mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MAX_STRING_LENGTH );
            if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
               mudstrlcat( buf, "&w->&R(&WOverland&R)&D", MAX_STRING_LENGTH );
            /*
             * New code added to display closed, or otherwise invisible exits to immortals 
             */
            /*
             * Installed by Samson 1-25-98 
             */
            if( IS_EXIT_FLAG( pexit, EX_LOCKED ) && IS_EXIT_FLAG( pexit, EX_CLOSED ) )
               mudstrlcat( buf, "&w->&R(&WC & L&R)&D", MAX_STRING_LENGTH );
            else if( IS_EXIT_FLAG( pexit, EX_CLOSED ) )
               mudstrlcat( buf, "&w->&R(&WC&R)&D", MAX_STRING_LENGTH );
            if( IS_EXIT_FLAG( pexit, EX_WINDOW ) )
               mudstrlcat( buf, "&w->(Window)", MAX_STRING_LENGTH );
            if( IS_EXIT_FLAG( pexit, EX_HIDDEN ) )
               mudstrlcat( buf, "&w->&R(&WHidden&R)&D", MAX_STRING_LENGTH );
         }
      }
      else
      {
         if( pexit->to_room && !IS_EXIT_FLAG( pexit, EX_SECRET ) && ( !IS_EXIT_FLAG( pexit, EX_WINDOW )
                                                                      || IS_EXIT_FLAG( pexit, EX_ISDOOR ) ) && !IS_EXIT_FLAG( pexit, EX_HIDDEN ) )
         {
            found = TRUE;
            mudstrlcat( buf, " ", MAX_STRING_LENGTH );
            mudstrlcat( buf, capitalize( dir_name[pexit->vdir] ), MAX_STRING_LENGTH );
            if( IS_EXIT_FLAG( pexit, EX_OVERLAND ) )
               mudstrlcat( buf, "&w->&R(&WOverland&R)&D", MAX_STRING_LENGTH );
            if( IS_EXIT_FLAG( pexit, EX_LOCKED ) && IS_EXIT_FLAG( pexit, EX_CLOSED ) )
               mudstrlcat( buf, "&w->&R(&WC & L&R)&D", MAX_STRING_LENGTH );
            else if( IS_EXIT_FLAG( pexit, EX_CLOSED ) )
               mudstrlcat( buf, "&w->&R(&WC&R)&D", MAX_STRING_LENGTH );
         }
      }
   }
   if( !found )
   {
      mudstrlcat( buf, " none&R]&D", MAX_STRING_LENGTH );
   }
   else
   {
      mudstrlcat( buf, "&R]&D", MAX_STRING_LENGTH );
   }
   mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   return buf;
}

/* Clear one map coord */
void clear_coord( int x, int y )
{
   map[x][y].tegn = ' ';
   map[x][y].vnum = 0;
   map[x][y].depth = 0;
   xCLEAR_BITS( map[x][y].info );
   map[x][y].can_see = TRUE;
}

/* Clear all exits for one room */
void clear_room( int x, int y )
{
   int dir, exitx, exity;
   /*
    * Cycle through the four directions 
    */
   for( dir = 0; dir < 4; dir++ )
   {
      /*
       * Find next coord in this direction 
       */
      get_exit_dir( dir, &exitx, &exity, x, y );
      /*
       * If coord is valid, clear it 
       */
      if( !BOUNDARY( exitx, exity ) )
         clear_coord( exitx, exity );
   }
}

/* This function is recursive, ie it calls itself */
void map_exits( CHAR_DATA * ch, ROOM_INDEX_DATA * pRoom, int x, int y, int depth )
{
   static char map_chars[11] = "|-|-UD/\\\\/";
   int door;
   int exitx = 0, exity = 0;
   int roomx = 0, roomy = 0;
   char buf[200];
   EXIT_DATA *pExit;
   /*
    * Setup this coord as a room 
    */
   switch ( pRoom->sector_type )
   {
      case SECT_CITY:
      case SECT_INSIDE:
         map[x][y].tegn = 'O';
         break;
      case SECT_FIELD:
      case SECT_FOREST:
      case SECT_HILLS:
         map[x][y].tegn = '*';
         break;
      case SECT_MOUNTAIN:
         map[x][y].tegn = '@';
         break;
      case SECT_WATER_SWIM:
      case SECT_WATER_NOSWIM:
         map[x][y].tegn = '=';
         break;
      case SECT_AIR:
         map[x][y].tegn = '~';
         break;
      case SECT_DESERT:
         map[x][y].tegn = '+';
         break;
      case SECT_SWAMP:
         map[x][y].tegn = 's';
         break;
      case SECT_UNDERGROUND:
         map[x][y].tegn = 'U';
         break;
      default:
         map[x][y].tegn = 'O';
         sprintf( buf, "Map_exits: Bad sector type (%d) in room %d.", pRoom->sector_type, pRoom->vnum );
         bug( buf, 0 );
         break;
   }
   map[x][y].vnum = pRoom->vnum;
   map[x][y].depth = depth;
   map[x][y].info = pRoom->room_flags;
   map[x][y].can_see = room_is_dark( pRoom );
   /*
    * Limit recursion 
    */
   if( depth > MAXDEPTH )
      return;
   /*
    * This room is done, deal with it's exits 
    */
   for( door = 0; door < 10; door++ )
   {
      /*
       * Skip if there is no exit in this direction 
       */
      if( ( pExit = get_exit( pRoom, door ) ) == NULL )
         continue;
      /*
       * Skip up and down until I can figure out a good way to display it 
       */
      if( door == 4 || door == 5 )
         continue;
      /*
       * Get the coords for the next exit and room in this direction 
       */
      get_exit_dir( door, &exitx, &exity, x, y );
      get_exit_dir( door, &roomx, &roomy, exitx, exity );
      /*
       * Skip if coords fall outside map 
       */
      if( BOUNDARY( exitx, exity ) || BOUNDARY( roomx, roomy ) )
         continue;
      /*
       * Skip if there is no room beyond this exit 
       */
      if( pExit->to_room == NULL )
         continue;
      /*
       * Ensure there are no clashes with previously defined rooms 
       */
      if( ( map[roomx][roomy].vnum != 0 ) && ( map[roomx][roomy].vnum != pExit->to_room->vnum ) )
      {
         /*
          * Use the new room if the depth is higher 
          */
         if( map[roomx][roomy].depth <= depth )
            continue;
         /*
          * It is so clear the old room 
          */
         clear_room( roomx, roomy );
      }
      /*
       * No exits at MAXDEPTH 
       */
      if( depth == MAXDEPTH )
         continue;
      /*
       * No need for exits that are already mapped 
       */
      if( map[exitx][exity].depth > 0 )
         continue;
      /*
       * Fill in exit 
       */
      map[exitx][exity].depth = depth;
      map[exitx][exity].vnum = pExit->to_room->vnum;
      map[exitx][exity].tegn = map_chars[door];
      /*
       * More to do? If so we recurse 
       */
      if( ( depth < MAXDEPTH ) && ( ( map[roomx][roomy].vnum == pExit->to_room->vnum ) || ( map[roomx][roomy].vnum == 0 ) ) )
      {
         /*
          * Depth increases by one each time 
          */
         map_exits( ch, pExit->to_room, roomx, roomy, depth + 1 );
      }
   }
}

/* Reformat room descriptions to exclude undesirable characters */
void reformat_desc( char *desc )
{
   /*
    * Index variables to keep track of array/pointer elements 
    */
   unsigned int i;
   int j;
   char buf[MAX_STRING_LENGTH], *p;
   i = 0;
   j = 0;
   buf[0] = STRING_NULL;
   if( !desc )
      return;
   /*
    * Replace all "\n" and "\r" with spaces 
    */
   for( i = 0; i <= strlen( desc ); i++ )
   {
      if( ( desc[i] == '\n' ) || ( desc[i] == '\r' ) )
         desc[i] = ' ';
   }
   /*
    * Remove multiple spaces 
    */
   for( p = desc; *p != STRING_NULL; p++ )
   {
      buf[j] = *p;
      j++;
      /*
       * Two or more consecutive spaces? 
       */
      if( ( *p == ' ' ) && ( *( p + 1 ) == ' ' ) )
      {
         do
         {
            p++;
         }
         while( *( p + 1 ) == ' ' );
      }
   }
   buf[j] = STRING_NULL;
   /*
    * Copy to desc 
    */
   mudstrlcpy( desc, buf, MAX_INPUT_LENGTH );
}
int get_line( char *desc, int max_len )
{
   int i, j = 0;
   /*
    * Return if it's short enough for one line 
    */
   if( ( int )strlen( desc ) <= max_len )
      return 0;
   /*
    * Calculate end point in string without color 
    */
   for( i = 0; i <= ( int )strlen( desc ); i++ )
   {
      /*
       * Here you need to skip your color sequences 
       */
      j++;
      if( j > max_len )
         break;
   }
   /*
    * End point is now in i, find the nearest space 
    */
   for( j = i; j > 0; j-- )
   {
      if( desc[j] == ' ' )
         break;
   }
   /*
    * There could be a problem if there are no spaces on the line 
    */
   return j + 1;
}

/* Display the map to the player */
void show_map( CHAR_DATA * ch, char *text )
{
   char buf[MAX_STRING_LENGTH * 2];
   int x, y, pos;
   char *p;
   bool alldesc = FALSE;   /* Has desc been fully displayed? */
   if( !text )
      alldesc = TRUE;
   pos = 0;
   p = text;
   buf[0] = STRING_NULL;
     /*
    * Write out the main map area with text 
    */
   for( y = 0; y <= MAPY; y++ )
   {
      for( x = 0; x <= MAPX; x++ )
      {
         switch ( map[x][y].tegn )
         {
            case '-':
            case '|':
            case '\\':
            case '/':
               sprintf( buf + strlen( buf ), "&O%c&D", map[x][y].tegn );
               break;
            case 'X':
               sprintf( buf + strlen( buf ), "&R%c&d", map[x][y].tegn );
               break;
            case '*':
               sprintf( buf + strlen( buf ), "&g%c&d", map[x][y].tegn );
               break;
            case '@':
               sprintf( buf + strlen( buf ), "&r%c&d", map[x][y].tegn );
               break;
            case '=':
               sprintf( buf + strlen( buf ), "&B%c&d", map[x][y].tegn );
               break;
            case '~':
               sprintf( buf + strlen( buf ), "&C%c&d", map[x][y].tegn );
               break;
            case '+':
               sprintf( buf + strlen( buf ), "&Y%c&d", map[x][y].tegn );
               break;
            case 'O':
               sprintf( buf + strlen( buf ), "&w%c&d", map[x][y].tegn );
               break;
            case 'U':
               sprintf( buf + strlen( buf ), "&O%c&d", map[x][y].tegn );
               break;
            case 's':
               sprintf( buf + strlen( buf ), "&g%c&d", map[x][y].tegn );
               break;
            default:
               sprintf( buf + strlen( buf ), "%c", map[x][y].tegn );
         }
      }
      /*
       * Add the text, if necessary 
       */
      if( !alldesc )
      {
         pos = get_line( p, 68 );
         if( pos > 0 )
         {
            mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
            strncat( buf, p, pos );
            p += pos;
         }
         else
         {
            mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
            mudstrlcat( buf, p, MAX_STRING_LENGTH );
            alldesc = TRUE;
         }
      }
      mudstrlcat( buf, "\r\n", MAX_STRING_LENGTH );
   }
   /*
    * Finish off map area 
    */
   mudstrlcat( buf, "           &D", MAX_STRING_LENGTH );
   if( !alldesc )
   {
      pos = get_line( p, 68 );
      if( pos > 0 )
      {
	 mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
         strncat( buf, p, pos );
         p += pos;
      }
      else
      {
         mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
         mudstrlcat( buf, p, MAX_STRING_LENGTH );
         alldesc = TRUE;
      }
   }
   /*
    * Deal with any leftover text 
    */
   if( !alldesc )
   {
      do
      {
         /*
          * Note the number - no map to detract from width 
          */
         mudstrlcat( buf, "\r\n           &D", MAX_STRING_LENGTH );
         pos = get_line( p, 68 );
         if( pos > 0 )
         {
            mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
            strncat( buf, p, pos );
            p += pos;
         }
         else
         {
            mudstrlcat( buf, color_str( AT_RMDESC, ch ), MAX_STRING_LENGTH );
            mudstrlcat( buf, p, MAX_STRING_LENGTH );
            alldesc = TRUE;
         }
      }
      while( !alldesc );
   }
   mudstrlcat( buf, "&z\r\n--------------------------------------------------------------------------------&D\r\n", MAX_STRING_LENGTH );
   send_to_char( buf, ch );
   if( IS_PLR_FLAG( ch, PLR_AUTOEXIT ) )  /* Show exits */
   {
      sprintf( buf, "%s%s", color_str( AT_EXITS, ch ), get_exits( ch ) );
      send_to_char( buf, ch );
      send_mip_exits(ch);
   }
}

/* Clear, generate and display the map */
void draw_map( CHAR_DATA * ch, const char *desc )
{
   int x, y;
   static char buf[MAX_STRING_LENGTH];
   mudstrlcpy( buf, desc, MAX_INPUT_LENGTH );
   /*
    * Remove undesirable characters 
    */
   reformat_desc( buf );
   /*
    * Clear map 
    */
   for( y = 0; y <= MAPY; y++ )
   {
      for( x = 0; x <= MAPX; x++ )
      {
         clear_coord( x, y );
      }
   }
   /*
    * Start with players pos at centre of map 
    */
   x = MAPX / 2;
   y = MAPY / 2;
   map[x][y].vnum = ch->in_room->vnum;
   map[x][y].depth = 0;
   /*
    * Generate the map 
    */
   map_exits( ch, ch->in_room, x, y, 0 );
   /*
    * Current position should be a "X" 
    */
   map[x][y].tegn = 'X';
   /*
    * Send the map 
    */
   show_map( ch, buf );
   show_list_to_char( ch->in_room->first_content, ch, FALSE, FALSE );
   show_char_to_char( ch->in_room->first_person, ch );
}
