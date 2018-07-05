/*
.-------------------------------------------------------------------------------.
| Title    : MIP (Messaging Interface Protocol) v8.1 Support Module  |
| Author   : Chris Coulter (aka Gabriel Androctus)    |
| Date     : 08.26.02        |
| Dev. MUD : Perils of Quiernin ( perils.org 6000 )    |
| Platform : SMAUG v1.4a       |
| MUD Page : http://www.perils.org/      |
| Res. Page: http://www.perils.org/goodies/     |
+-------------------------------------------------------------------------------+
| DESCRIPTION:         |
| MIP (Messaging Interface Protocol) enables your mud to communicate specific   |
| text strings to a player using the Portal client software. This module        |
| provides basic support for establishing if the player is using the Portal     |
| client and assigning the appropriate information, as well as a simplified     |
| way to send the strings to trigger specific features on the player's client.  |
|          |
| For more information, visit Portal's web page ( www.gameaxle.com ) and click  |
| on the "Developers" button on the menu bar. From there you can download a PDF |
| file that explains in detail the specifics of the MIP.   |
|          |
| NOTE: If a player is not using the Portal client, the mud will function as    |
| it normally would, and the player would not receive the MIP codes. It will    |
| only function for players who has connected via the Portal mud client         |
|           |
`-------------------------------------------------------------------------------'
*/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "./Headers/mud.h"

/* Parses jumpstart command and assigns sec_code & version information.  -gabe */
void do_mip_start ( CHAR_DATA * ch, char *argument )
{
  char arg[MAX_INPUT_LENGTH];  /* version */
  char buf[MAX_STRING_LENGTH];
  int sec_code;

  if ( IS_NPC ( ch ) )
  {
    send_to_char ( "Huh?\r\n", ch );
    return;
  }

  if ( argument[0] == '\0' )
  {
    send_to_char ( "Huh?\r\n", ch );
    return;
  }

  sscanf ( argument, "%d~%s\r\n", &sec_code, arg );

  /*
   * pad sec_code with zeros
   */

  if ( sec_code >= 10000 )
    sprintf ( buf, "%d", sec_code );
  else
    if ( sec_code >= 1000 )
      sprintf ( buf, "0%d", sec_code );
    else
      if ( sec_code >= 100 )
        sprintf ( buf, "00%d", sec_code );
      else
        if ( sec_code >= 10 )
          sprintf ( buf, "000%d", sec_code );
        else
          if ( sec_code >= 0 )
            sprintf ( buf, "0000%d", sec_code );

  ch->pcdata->sec_code = str_dup ( buf );

  ch->pcdata->mip_ver = str_dup ( arg );

  xSET_BIT ( ch->act, PLR_MIP );
}



/* Initial settings are all done here. -gabe */
void init_mip ( CHAR_DATA * ch )
{
  send_mip_mask ( ch, CL_HP_MASK, "Hit Points" );
  send_mip_mask ( ch, CL_SP_MASK, "Mana" );
  send_mip_mask ( ch, CL_GP1_MASK, "Movement" );
  send_mip_mask ( ch, CL_GP2_MASK, "Unused" );
}



/* Is the character's client MIP-enhanced? -gabe */
bool mip_enabled ( CHAR_DATA * ch )
{
  if ( IS_NPC ( ch ) )
    return FALSE;

  if ( ( !ch->pcdata->sec_code || ch->pcdata->sec_code[0] == '\0' )
       && ( !ch->pcdata->mip_ver || ch->pcdata->mip_ver[0] == '\0' ) )
    return FALSE;

  if ( xIS_SET ( ch->act, PLR_MIP ) )
    return TRUE;

  return FALSE;
}



/* provides the generic prefix for all MIP communication -gabe */
void send_mip ( char *argument, CHAR_DATA * ch )
{
  char buf[MAX_STRING_LENGTH];
  char length2[MAX_STRING_LENGTH];
  int length;

  if ( !mip_enabled ( ch ) )
    return;

  length = strlen ( argument );

  if ( length >= 1000 )
    sprintf ( length2, "999" );
  else if ( length >= 100 )
      sprintf ( length2, "%d", length );
  else if ( length >= 10 )
      sprintf ( length2, "0%d", length );
  else if ( length >= 0 )
      sprintf ( length2, "00%d", length );
  else
      sprintf ( length2, "000" );

  sprintf ( buf, "#K%%%s%s%s", ch->pcdata->sec_code, length2, argument );
  send_to_char ( buf, ch );
  return;
}


void send_mip_2 ( char *argument, CHAR_DATA * victim )
{
  char buf[MAX_STRING_LENGTH];
  char length2[MAX_STRING_LENGTH];
  int length;

  if ( !mip_enabled ( victim ) )
    return;

  length = strlen ( argument );

  if ( length >= 1000 )
    sprintf ( length2, "999" );
  else if ( length >= 100 )
      sprintf ( length2, "%d", length );
  else if ( length >= 10 )
      sprintf ( length2, "0%d", length );
  else if ( length >= 0 )
      sprintf ( length2, "00%d", length );
  else
      sprintf ( length2, "000" );

  sprintf ( buf, "#K%%%s%s%s", victim->pcdata->sec_code, length2, argument );
  send_to_char ( buf, victim );
  return;
}


void send_mip_3 ( char *argument, CHAR_DATA * victim )
{
  char buf[MAX_STRING_LENGTH];
  char length2[MAX_STRING_LENGTH];
  int length;

  if ( !mip_enabled ( victim ) )
    return;

  length = strlen ( argument );

  if ( length >= 1000 )
    sprintf ( length2, "999" );
  else if ( length >= 100 )
      sprintf ( length2, "%d", length );
  else if ( length >= 10 )
      sprintf ( length2, "0%d", length );
  else if ( length >= 0 )
       sprintf ( length2, "00%d", length );
  else
       sprintf ( length2, "000" );

  sprintf ( buf, "#K%%%s%s%s", victim->pcdata->sec_code, length2, argument );
  send_to_char ( buf, victim );
  return;
}



/* Triggers sound on client's computer.  -gabe
 * Formats: .WAV
 */
void send_mip_sound ( CHAR_DATA * ch, char *filename )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_SOUND, filename );
  send_mip ( buf, ch );
  return;
}



/* Triggers music on client's computer.  -gabe
 * Formats: .MID, .RMI, .MP3
 */
void send_mip_music ( CHAR_DATA * ch, char *filename, int iterations )
{
  char buf[MAX_STRING_LENGTH];

  if ( !filename || filename[0] == '\0' )
    sprintf ( buf, "%s", CL_SEND_MUSIC );
  else
    if ( iterations > 1 )
      sprintf ( buf, "%s%s%s%d", CL_SEND_MUSIC, filename, CL_DELIM, iterations );
    else
      sprintf ( buf, "%s%s", CL_SEND_MUSIC, filename );

  send_mip ( buf, ch );

  return;
}



/* Displays an image on client's computer. -gabe
 * Formats: .BMP, .GIF send_mip_image(ch, "weapons.bmp", "Map 1");
 */
void send_mip_image ( CHAR_DATA * ch, char *filename, char *label )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s%s%s", CL_SEND_IMAGE, filename, CL_DELIM, label );
  send_mip ( buf, ch );
  return;
}



/* Display amount of time remaining (HH:MM) until next reboot.  -gabe */
void send_mip_reboot ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_REBOOT, argument );
  send_mip ( buf, ch );
  return;
}



/* Display amount of time (HH:MM) the mud has been up since last reboot. -gabe */
void send_mip_uptime ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_UPTIME, argument );
  send_mip ( buf, ch );
  return;
}



/* Trigger an .AVI on the client's computer -gabe */
void send_mip_avi ( CHAR_DATA * ch, char *filename, char *label, int height, int width, bool fRepeat )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s%s%s%s%d%s%d%s%s", CL_SEND_AVI, filename, CL_DELIM, label, CL_DELIM, height,
            CL_DELIM, width, ( fRepeat ) ? CL_DELIM : "", ( fRepeat ) ? "1" : "" );
  send_mip ( buf, ch );
  return;
}



/* Send a media file to the client's hard disk.  -gabe
 * Formats: .WAV, .MID, .RMI, .MP3, .BMP, .GIF, .AVI
 */
void mip_download ( CHAR_DATA * ch, char *filename )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s%s%s%s", CL_DOWNLOAD_MEDIA, filename, CL_DELIM, MIP_MEDIA_URL, filename );
  send_mip ( buf, ch );
  return;
}



/* Send special information to client in a text string  -gabe
 * note: haven't found a use for this, but it's part of mip. so here it is. */
void send_mip_special ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_SPECIAL, argument );
  send_mip ( buf, ch );
  return;
}



/* Send special guild information to client in a text string -gabe
 * note: haven't found a use for this, but it's a part of mip. so here it is. */
void send_mip_special2 ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_SPECIAL2, argument );
  send_mip ( buf, ch );
  return;
}



/* Send 'tell' information to player's client -gabe
 * fSender values:
 *  TRUE = ( ch == sender of tell ) + ( victim == receiver of tell )
 *  FALSE = ( ch == receiver of tell ) + ( victim == sender of tell )
 */
void send_mip_tell ( CHAR_DATA * ch, CHAR_DATA * victim, char *argument, bool fSender )
{
  char buf[MAX_STRING_LENGTH];
 
  if ( IS_NPC ( ch ) || IS_NPC ( victim ) )
    return;

  if ( fSender )
  {
    sprintf ( buf, "%sx%s%s%s%s", CL_SEND_TELL, CL_DELIM, victim->name, CL_DELIM, argument );
    send_mip_2 ( buf, ch );
    return;
  }
  else
  {
    sprintf ( buf, "%s%s%s%s%s", CL_SEND_TELL, CL_DELIM, victim->name, CL_DELIM, argument );
    send_mip ( buf, victim );
    return;
  }
}

void send_mip_imc_tell ( char *from, CHAR_DATA * victim, char *argument )
{
  char buf[MAX_STRING_LENGTH];
 
  if ( IS_NPC ( victim ) )
    return;

    sprintf ( buf, "%s%s%s%s%s", CL_SEND_TELL, CL_DELIM, from, CL_DELIM, argument );
    send_mip ( buf, victim );
    return;
  
}

/* Send short description of a room to player's client  -gabe */
void send_mip_room ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_ROOM, argument );
  send_mip ( buf, ch );
  return;
}

/* Send a metered lag measurement to client -gabe */
void send_mip_mudlag ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_MUDLAG, argument );
  send_mip ( buf, ch );
  return;
}

/* Send the current file that is being edited (??) -gabe */
void send_mip_edit ( CHAR_DATA * ch, char *filename )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_EDIT, filename );
  send_mip ( buf, ch );
  return;
}

/* Set the appropriate mask names on player's client -gabe
 * note: for type values, see mip.h */
void send_mip_mask ( CHAR_DATA * ch, char *type, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", type, argument );
  send_mip ( buf, ch );
  return;
}

/* Set the caption on the player's client -gabe */
void send_mip_caption ( CHAR_DATA * ch, char *argument )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_CAPTION, argument );
  send_mip ( buf, ch );
  return;
}

/* Begin a line-by-line transfer (???) -gabe */
void send_mip_begin_file ( CHAR_DATA * ch, int lines, char *tag )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%d%s%s", CL_SEND_BEGIN_FILE, lines, CL_DELIM, tag );
  send_mip ( buf, ch );
  return;
}

/* Continue a line-by-line transfer (???) -gabe */
void send_mip_cont_file ( CHAR_DATA * ch, char *line )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s", CL_SEND_CONT_FILE, line );
  send_mip ( buf, ch );
  return;
}

/* End a line-by-line transfer (???) -gabe */
void send_mip_end_file ( CHAR_DATA * ch )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s", CL_SEND_END_FILE );
  send_mip ( buf, ch );
  return;
}

/* Send channel information to player's client -gabe */
void send_mip_channel ( CHAR_DATA * ch, const char *cmd, char *channel, char *source, char *argument )
{
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  sprintf ( buf2, "%s-> %s-> %s", source, channel, argument );
  sprintf ( buf, "%s%s%s%s%s%s%s%s", CL_SEND_CHAT, cmd, CL_DELIM, channel, CL_DELIM, source, CL_DELIM, buf2 );
  send_mip_3 ( buf, ch );
  return;
}

/* Send exit information to player's client (for room monitor) -gabe */
void send_mip_exits ( CHAR_DATA *ch )
{
  EXIT_DATA *exit_l;
  char ex_buf[MSL];
  char buf[MSL];

  strcpy ( ex_buf, "" );

  if ( !ch->in_room )
  {
    bug ( "send_mip_exits (%s): ch->in_room == NULL", ch->name );
    return;
  }

  for ( exit_l = ch->in_room->first_exit; exit_l; exit_l = exit_l->next )
  {
    if ( exit_l->to_room
         && !IS_SET ( exit_l->exit_info, EX_CLOSED )
         && !IS_SET ( exit_l->exit_info, EX_HIDDEN )
         && !IS_SET ( exit_l->exit_info, EX_WINDOW ) )
    {
      switch ( exit_l->vdir )
      {

        default:
          break;

        case DIR_NORTH:
          strcat ( ex_buf, "n " );
          break;

        case DIR_EAST:
          strcat ( ex_buf, "e " );
          break;

        case DIR_SOUTH:
          strcat ( ex_buf, "s " );
          break;

        case DIR_WEST:
          strcat ( ex_buf, "w " );
          break;

        case DIR_UP:
          strcat ( ex_buf, "u " );
          break;

        case DIR_DOWN:
          strcat ( ex_buf, "d " );
          break;

        case DIR_NORTHEAST:
          strcat ( ex_buf, "ne " );
          break;

        case DIR_SOUTHEAST:
          strcat ( ex_buf, "se " );
          break;

        case DIR_NORTHWEST:
          strcat ( ex_buf, "nw " );
          break;

        case DIR_SOUTHWEST:
          strcat ( ex_buf, "sw " );
          break;
      }
    }
  }

  sprintf ( buf, "%s%s", CL_SEND_ROOMCODE, ex_buf );

  send_mip ( buf, ch );
  return;
}



/* Send point information (hit points, mana & move points) -gabe */
void send_mip_points ( CHAR_DATA * ch )
{
  char buf[MAX_STRING_LENGTH];

  sprintf ( buf, "%s%s%d%s%s%d%s%s%d%s%s%d%s%s%d%s%s%d", CL_SEND_COMPOSITE,
            CL_SEND_HP, ch->hit, CL_DELIM,
            CL_SEND_MAXHP, ch->max_hit, CL_DELIM,
            CL_SEND_SP, ch->mana, CL_DELIM,
            CL_SEND_MAXSP, ch->max_mana, CL_DELIM,
            CL_SEND_GP1, ch->move, CL_DELIM,
            CL_SEND_MAXGP1, ch->max_move );

  send_mip ( buf, ch );
  return;
}



/* Send combat information (mob condition/name)  -gabe */
void send_mip_attacker ( CHAR_DATA * ch )
{
  CHAR_DATA * victim;
  char buf[MAX_STRING_LENGTH];
  int condition;

  if ( !ch->fighting || ! ( victim = who_fighting ( ch ) ) )
  {
    sprintf ( buf, "%s%s%s", CL_SEND_COMPOSITE, CL_SEND_ATTACKER, CL_DELIM );
    send_mip ( buf, ch );
    sprintf ( buf, "%s%s%d", CL_SEND_COMPOSITE, CL_SEND_ATTCOND, 0 );
    send_mip ( buf, ch );
    return;
  }

  if ( victim->max_hit > 0 && victim->hit > 0 )
    condition = ( 100 * victim->hit ) / victim->max_hit;
  else
    condition = 0;

  sprintf ( buf, "%s%s%s%s%s%d", CL_SEND_COMPOSITE, CL_SEND_ATTACKER, ( IS_NPC ( victim ) ) ? capitalize
            ( victim->short_descr ) : victim->name, CL_DELIM, CL_SEND_ATTCOND, condition );

  send_mip ( buf, ch );

  return;
}
