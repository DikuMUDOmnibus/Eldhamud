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
 *                      Low-level communication module                      *
 ****************************************************************************/
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include "./Headers/mud.h"
#include "./Headers/sha256.h"
/*
 * Socket and TCP/IP stuff.
 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <arpa/telnet.h>
#include <netdb.h>
#define closesocket close
#ifdef sun
int gethostname ( char *name, int namelen );
#endif
const char echo_off_str[] = { IAC, WILL, TELOPT_ECHO, STRING_NULL };
const char echo_on_str[] = { IAC, WONT, TELOPT_ECHO, STRING_NULL };
const char go_ahead_str[] = { IAC, GA, STRING_NULL };
void auth_maxdesc args ( ( int *md, fd_set * ins, fd_set * outs, fd_set * excs ) );
void auth_check args ( ( fd_set * ins, fd_set * outs, fd_set * excs ) );
void set_auth args ( ( DESCRIPTOR_DATA * d ) );
void kill_auth args ( ( DESCRIPTOR_DATA * d ) );
void save_sysdata args ( ( SYSTEM_DATA sys ) );
void    medit_parse     args ( ( DESCRIPTOR_DATA *d, char *arg ) );
void    redit_parse     args ( ( DESCRIPTOR_DATA *d, char *arg ) );
void    oedit_parse     args ( ( DESCRIPTOR_DATA *d, char *arg ) );
bool    is_inolc        args ( ( DESCRIPTOR_DATA *d ) );

/*
 * Global variables.
 */
DESCRIPTOR_DATA *first_descriptor;  /* First descriptor     */
DESCRIPTOR_DATA *last_descriptor;   /* Last descriptor      */
DESCRIPTOR_DATA *d_next;   /* Next descriptor in loop */
int num_descriptors;
FILE *fpReserve;  /* Reserved file handle    */
bool mud_down; /* Shutdown       */
bool service_shut_down; /* Shutdown by operator closing down service */
bool wizlock;  /* Game is wizlocked    */
time_t boot_time;
char str_boot_time[MAX_INPUT_LENGTH];
char lastplayercmd[MAX_INPUT_LENGTH * 2];
time_t current_time; /* Time of this pulse      */
int control;   /* Controlling descriptor  */
int newdesc;   /* New descriptor    */
fd_set in_set; /* Set of desc's for reading  */
fd_set out_set;   /* Set of desc's for writing  */
fd_set exc_set;   /* Set of desc's with errors  */
int maxdesc;
char *alarm_section = "(unknown)";

/*
 * OS-dependent local functions.
 */
void game_loop args ( ( void ) );
int init_socket args ( ( int mudport ) );
void new_descriptor args ( ( int new_desc ) );
bool read_from_descriptor args ( ( DESCRIPTOR_DATA * d ) );
bool write_to_descriptor args ( ( int desc, char *txt, int length ) );
/*
 * Other local functions (OS-independent).
 */
bool check_reconnect args ( ( DESCRIPTOR_DATA * d, char *name, bool fConn ) );
bool check_playing args ( ( DESCRIPTOR_DATA * d, char *name, bool kick ) );
int main args ( ( int argc, char **argv ) );
void nanny args ( ( DESCRIPTOR_DATA * d, char *argument ) );
bool flush_buffer args ( ( DESCRIPTOR_DATA * d, bool fPrompt ) );
void read_from_buffer args ( ( DESCRIPTOR_DATA * d ) );
void stop_idling args ( ( CHAR_DATA * ch ) );
void free_desc args ( ( DESCRIPTOR_DATA * d ) );
void display_prompt args ( ( DESCRIPTOR_DATA * d ) );
void set_pager_input args ( ( DESCRIPTOR_DATA * d, char *argument ) );
bool pager_output args ( ( DESCRIPTOR_DATA * d ) );
void mail_count args ( ( CHAR_DATA * ch ) );
int port;
int main ( int argc, char **argv )
{

	struct timeval now_time;
	char hostn[128];
	bool fCopyOver = FALSE;

#ifdef IMC
	int imcsocket = -1;
#endif
	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug ( 2 );
#endif
	DONT_UPPER = FALSE;
	num_descriptors = 0;
	first_descriptor = NULL;
	last_descriptor = NULL;
	sysdata.NO_NAME_RESOLVING = TRUE;
	sysdata.WAIT_FOR_AUTH = TRUE;
	/*
	 * Init time.
	 */
	gettimeofday ( &now_time, NULL );
	current_time = ( time_t ) now_time.tv_sec;
	boot_time = time ( 0 ); /*  <-- I think this is what you wanted */
	strcpy ( str_boot_time, ctime ( &current_time ) );
	init_pfile_scan_time( );   /* Pfile autocleanup initializer - Samson 5-8-99 */
	/*
	 * Reserve two channels for our use.
	 */

	if ( ( fpReserve = fopen ( NULL_FILE, "r" ) ) == NULL )
	{
		perror ( NULL_FILE );
		exit ( 1 );
	}

	if ( ( fpLOG = fopen ( NULL_FILE, "r" ) ) == NULL )
	{
		perror ( NULL_FILE );
		exit ( 1 );
	}

	/*
	 * Get the port number.
	 */
	port = 8000;

	if ( argc > 1 )
	{
		if ( !is_number ( argv[1] ) )
		{
			fprintf ( stderr, "Usage: %s [port #]\n", argv[0] );
			exit ( 1 );
		}
		else if ( ( port = atoi ( argv[1] ) ) <= 1024 )
		{
			fprintf ( stderr, "Port number must be above 1024.\n" );
			exit ( 1 );
		}

		if ( argv[2] && argv[2][0] )
		{
			fCopyOver = TRUE;
			control = atoi ( argv[3] );
#ifdef IMC
			imcsocket = atoi ( argv[4] );
#endif
		}
		else
			fCopyOver = FALSE;
	}

	/*
	 * Run the game.
	 */
	log_string ( "Booting Database" );

	boot_db ( fCopyOver );

	log_string ( "Initializing socket" );

	if ( !fCopyOver ) /* We have already the port if copyover'ed */
		control = init_socket ( port );

	/*
	 * I don't know how well this will work on an unnamed machine as I don't
	 * have one handy, and the man pages are ever-so-helpful.. -- Alty
	 */
	if ( gethostname ( hostn, sizeof ( hostn ) ) < 0 )
	{
		perror ( "main: gethostname" );
		strcpy ( hostn, "unresolved" );
	}

	sprintf ( log_buf, "%s ready at address %s on port %d.", sysdata.mud_name, hostn, port );

	log_string ( log_buf );

#ifdef IMC
	/* Initialize and connect to IMC2 */
	imc_startup ( FALSE, imcsocket, fCopyOver );
#endif

	if ( fCopyOver )
	{
		log_string ( "Initiating hotboot recovery." );
		hotboot_recover( );
	}

	game_loop( );

	closesocket ( control );
#ifdef IMC
	imc_shutdown ( FALSE );
#endif

	/*
	 * That's all, folks.
	 */
	log_string ( "Normal termination of game." );
	exit ( 0 );
	return 0;
}

int init_socket ( int mudport )
{
	char hostname[64];

	struct sockaddr_in sa;
	int x = 1;
	int fd;
	gethostname ( hostname, sizeof ( hostname ) );

	if ( ( fd = socket ( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror ( "Init_socket: socket" );
		exit ( 1 );
	}

	if ( setsockopt ( fd, SOL_SOCKET, SO_REUSEADDR, ( void * ) &x, sizeof ( x ) ) < 0 )
	{
		perror ( "Init_socket: SO_REUSEADDR" );
		closesocket ( fd );
		exit ( 1 );
	}

#if defined(SO_DONTLINGER) && !defined(SYSV)
	{

		struct linger ld;
		ld.l_onoff = 1;
		ld.l_linger = 1000;

		if ( setsockopt ( fd, SOL_SOCKET, SO_DONTLINGER, ( void * ) &ld, sizeof ( ld ) ) < 0 )
		{
			perror ( "Init_socket: SO_DONTLINGER" );
			closesocket ( fd );
			exit ( 1 );
		}
	}

#endif
	memset ( &sa, STRING_NULL, sizeof ( sa ) );

	sa.sin_family = AF_INET;

	sa.sin_port = htons ( mudport );

	if ( bind ( fd, ( struct sockaddr * ) &sa, sizeof ( sa ) ) == -1 )
	{
		perror ( "Init_socket: bind" );
		closesocket ( fd );
		exit ( 1 );
	}

	if ( listen ( fd, 50 ) < 0 )
	{
		perror ( "Init_socket: listen" );
		closesocket ( fd );
		exit ( 1 );
	}

	return fd;
}

static void SegVio( )
{
	abort( );
}

void clear_screen(DESCRIPTOR_DATA * d)
{
send_to_desc_color ( "\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n\r\n", d );
return;
}


/*
 * LAG alarm!       -Thoric
 */
void caught_alarm ( int signum )
{
	char buf[MAX_STRING_LENGTH];
	sprintf ( buf, "ALARM CLOCK!  In section %s", alarm_section );
	bug ( buf );
	strcpy ( buf, "Alas, the hideous malevalent entity known only as 'Lag' rises once more!\r\n" );
	echo_to_all ( AT_IMMORT, buf, ECHOTAR_ALL );

	if ( newdesc )
	{
		FD_CLR ( newdesc, &in_set );
		FD_CLR ( newdesc, &out_set );
		FD_CLR ( newdesc, &exc_set );
		log_string ( "clearing newdesc" );
	}
}

bool check_bad_desc ( int desc )
{
	if ( FD_ISSET ( desc, &exc_set ) )
	{
		FD_CLR ( desc, &in_set );
		FD_CLR ( desc, &out_set );
		log_string ( "Bad FD caught and disposed." );
		return TRUE;
	}

	return FALSE;
}
void accept_new ( int ctrl )
{

	static struct timeval null_time;
	DESCRIPTOR_DATA *d;
	/*
	 * int maxdesc; Moved up for use with id.c as extern
	 */
#if defined(MALLOC_DEBUG)

	if ( malloc_verify( ) != 1 )
		abort( );

#endif
	/*
	 * Poll all active descriptors.
	 */
	FD_ZERO ( &in_set );

	FD_ZERO ( &out_set );

	FD_ZERO ( &exc_set );

	FD_SET ( ctrl, &in_set );

	maxdesc = ctrl;

	newdesc = 0;

	for ( d = first_descriptor; d; d = d->next )
	{
		maxdesc = UMAX ( maxdesc, d->descriptor );
		FD_SET ( d->descriptor, &in_set );
		FD_SET ( d->descriptor, &out_set );
		FD_SET ( d->descriptor, &exc_set );

		if ( d == last_descriptor )
			break;
	}

	if ( select ( maxdesc + 1, &in_set, &out_set, &exc_set, &null_time ) < 0 )
	{
		perror ( "accept_new: select: poll" );
		exit ( 1 );
	}

	if ( FD_ISSET ( ctrl, &exc_set ) )
	{
		bug ( "Exception raise on controlling descriptor %d", ctrl );
		FD_CLR ( ctrl, &in_set );
		FD_CLR ( ctrl, &out_set );
	}
	else if ( FD_ISSET ( ctrl, &in_set ) )
	{
		newdesc = ctrl;
		new_descriptor ( newdesc );
	}
}

void game_loop ( void )
{

	struct timeval last_time;
	char cmdline[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA *d;
	/*
	 * time_t last_check = 0;
	 */
	signal ( SIGSEGV, SegVio );
	gettimeofday ( &last_time, NULL );
	current_time = ( time_t ) last_time.tv_sec;
	/*
	 * Main loop
	 */

	while ( !mud_down )
	{
		accept_new ( control );
		/*
		 * Kick out descriptors with raised exceptions
		 * or have been idle, then check for input.
		 */

		for ( d = first_descriptor; d; d = d_next )
		{
			if ( d == d->next )
			{
				bug ( "descriptor_loop: loop found & fixed" );
				d->next = NULL;
			}

			d_next = d->next;

			d->idle++;  /* make it so a descriptor can idle out */

			if ( FD_ISSET ( d->descriptor, &exc_set ) )
			{
				FD_CLR ( d->descriptor, &in_set );
				FD_CLR ( d->descriptor, &out_set );

				if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
					save_char_obj ( d->character );

				d->outtop = 0;
				close_socket ( d, TRUE );

				continue;
			}
			else if ( ( !d->character && d->idle > 240 )           /* 1 mins DC on creation chars */
			|| ( d->connected != CON_PLAYING && d->idle > 480 ))   /* 2 min  DC on all dead linked */
			{
				write_to_descriptor ( d->descriptor, "Idle timeout... disconnecting.\r\n", 0 );
				d->outtop = 0;
				close_socket ( d, TRUE );
				continue;
			}
			else
			{
				d->fcommand = FALSE;

				if ( FD_ISSET ( d->descriptor, &in_set ) )
				{
					d->idle = 0;

					if ( d->character )
						d->character->timer = 0;

					if ( !read_from_descriptor ( d ) )
					{
						FD_CLR ( d->descriptor, &out_set );

						if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
							save_char_obj ( d->character );

						d->outtop = 0;

						close_socket ( d, FALSE );

						continue;
					}
				}

				if ( d->character && d->character->wait > 0 )
				{
					--d->character->wait;
					continue;
				}

				read_from_buffer ( d );

				if ( d->incomm[0] != STRING_NULL )
				{
					d->fcommand = TRUE;
					stop_idling ( d->character );
					strcpy ( cmdline, d->incomm );
					d->incomm[0] = STRING_NULL;

					if ( d->character )
						set_cur_char ( d->character );

					if ( d->pagepoint )
						set_pager_input ( d, cmdline );
					else
						switch ( d->connected )
						{

							default:
								nanny ( d, cmdline );
								break;

							case CON_PLAYING:
								d->character->cmd_recurse = 0;
								interpret ( d->character, cmdline );
								break;

							case CON_EDITING:
								edit_buffer ( d->character, cmdline );
								break;
						}
				}
			}

			if ( d == last_descriptor )
				break;
		}

#ifdef IMC
		imc_loop();
#endif
		/*
		 * Autonomous game motion.
		 */
		update_handler( );

		/*
		 * Output.
		 */
		for ( d = first_descriptor; d; d = d_next )
		{
			d_next = d->next;

			if ( ( d->fcommand || d->outtop > 0 ) && FD_ISSET ( d->descriptor, &out_set ) )
			{
				if ( d->pagepoint )
				{
					if ( !pager_output ( d ) )
					{
						if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
							save_char_obj ( d->character );

						d->outtop = 0;

						close_socket ( d, FALSE );
					}
				}
				else if ( !flush_buffer ( d, TRUE ) )
				{
					if ( d->character && ( d->connected == CON_PLAYING || d->connected == CON_EDITING ) )
						save_char_obj ( d->character );

					d->outtop = 0;

					close_socket ( d, FALSE );
				}
			}

			if ( d == last_descriptor )
				break;
		}

		/*
		 * Synchronize to a clock.
		 * Sleep( last_time + 1/PULSE_PER_SECOND - now ).
		 * Careful here of signed versus unsigned arithmetic.
		 */
		{

			struct timeval now_time;
			long secDelta;
			long usecDelta;
			gettimeofday ( &now_time, NULL );
			usecDelta = ( ( int ) last_time.tv_usec ) - ( ( int ) now_time.tv_usec ) + 1000000 / PULSE_PER_SECOND;
			secDelta = ( ( int ) last_time.tv_sec ) - ( ( int ) now_time.tv_sec );

			while ( usecDelta < 0 )
			{
				usecDelta += 1000000;
				secDelta -= 1;
			}

			while ( usecDelta >= 1000000 )
			{
				usecDelta -= 1000000;
				secDelta += 1;
			}

			if ( secDelta > 0 || ( secDelta == 0 && usecDelta > 0 ) )
			{

				struct timeval stall_time;
				stall_time.tv_usec = usecDelta;
				stall_time.tv_sec = secDelta;

				if ( select ( 0, NULL, NULL, NULL, &stall_time ) < 0 && errno != EINTR )
				{
					perror ( "game_loop: select: stall" );
					exit ( 1 );
				}
			}
		}

		gettimeofday ( &last_time, NULL );

		current_time = ( time_t ) last_time.tv_sec;
	}

	fflush ( stderr ); /* make sure strerr is flushed */

	return;
}

void new_descriptor ( int new_desc )
{
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA *dnew;

	struct sockaddr_in sock;

	struct hostent *from;
	size_t desc, size;
	char bugbuf[MAX_STRING_LENGTH];
	size = sizeof ( sock );

	if ( check_bad_desc ( new_desc ) )
	{
		set_alarm ( 0 );
		return;
	}

	set_alarm ( 20 );

	alarm_section = "new_descriptor::accept";

	if ( ( desc = accept ( new_desc, ( struct sockaddr * ) &sock, &size ) ) < 0 )
	{
		perror ( "New_descriptor: accept" );
		sprintf ( bugbuf, "[*****] BUG: New_descriptor: accept" );
		log_string_plus ( bugbuf, LOG_COMM, sysdata.log_level );
		set_alarm ( 0 );
		return;
	}

	if ( check_bad_desc ( new_desc ) )
	{
		set_alarm ( 0 );
		return;
	}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif
	set_alarm ( 20 );

	alarm_section = "new_descriptor: after accept";

	if ( fcntl ( desc, F_SETFL, FNDELAY ) == -1 )
	{
		perror ( "New_descriptor: fcntl: FNDELAY" );
		set_alarm ( 0 );
		return;
	}

	if ( check_bad_desc ( new_desc ) )
		return;

	CREATE ( dnew, DESCRIPTOR_DATA, 1 );

	dnew->next = NULL;

	dnew->descriptor = desc;

	dnew->connected = CON_GET_NAME;

	dnew->outsize = 2000;

	dnew->idle = 0;

	dnew->lines = 0;

	dnew->scrlen = 24;

	dnew->port = ntohs ( sock.sin_port );

	dnew->newstate = 0;

	dnew->prevcolor = 0x07;

	CREATE ( dnew->outbuf, char, dnew->outsize );

	strcpy ( buf, inet_ntoa ( sock.sin_addr ) );

	sprintf ( log_buf, "Sock.sinaddr:  %s, port %hd.", buf, dnew->port );

	log_string_plus ( log_buf, LOG_COMM, sysdata.log_level );

	if ( sysdata.NO_NAME_RESOLVING )
		dnew->host = STRALLOC ( buf );
	else
	{
		from = gethostbyaddr ( ( char * ) &sock.sin_addr, sizeof ( sock.sin_addr ), AF_INET );
		dnew->host = STRALLOC ( ( char * ) ( from ? from->h_name : buf ) );
	}

	if ( check_total_bans ( dnew ) )
	{
		write_to_descriptor ( desc, "Your site has been banned from this Mud.\r\n", 0 );
		free_desc ( dnew );
		set_alarm ( 0 );
		return;
	}

	/*
	 * Init descriptor data.
	 */
	if ( !last_descriptor && first_descriptor )
	{
		DESCRIPTOR_DATA *d;
		bug ( "New_descriptor: last_desc is NULL, but first_desc is not! ...fixing" );

		for ( d = first_descriptor; d; d = d->next )
			if ( !d->next )
				last_descriptor = d;
	}

	LINK ( dnew, first_descriptor, last_descriptor, next, prev );

	/*
	 * Send the greeting.
	 */
	{
		extern char *help_greeting;

		if ( help_greeting[0] == '.' )
			write_to_buffer ( dnew, help_greeting + 1, 0 );
		else
			write_to_buffer ( dnew, help_greeting, 0 );
	}

	if ( ++num_descriptors > sysdata.maxplayers )
		sysdata.maxplayers = num_descriptors;

	if ( sysdata.maxplayers > sysdata.alltimemax )
	{
		if ( sysdata.time_of_max )
			DISPOSE ( sysdata.time_of_max );

		sprintf ( buf, "%24.24s", ctime ( &current_time ) );

		sysdata.time_of_max = str_dup ( buf );

		sysdata.alltimemax = sysdata.maxplayers;

		sprintf ( log_buf, "Broke all-time maximum player record: %d", sysdata.alltimemax );

		log_string_plus ( log_buf, LOG_COMM, sysdata.log_level );

		to_channel ( log_buf, CHANNEL_MONITOR, "Monitor", LEVEL_IMMORTAL );

		save_sysdata ( sysdata );
	}

	set_alarm ( 0 );

	return;
}

void free_desc ( DESCRIPTOR_DATA * d )
{
	closesocket ( d->descriptor );
	STRFREE ( d->host );
	DISPOSE ( d->outbuf );

	if ( d->pagebuf )
		DISPOSE ( d->pagebuf );

	DISPOSE ( d );

	return;
}

void close_socket ( DESCRIPTOR_DATA * dclose, bool force )
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *d;
	bool DoNotUnlink = FALSE;
	/*
	 * flush outbuf
	 */

	if ( !force && dclose->outtop > 0 )
		flush_buffer ( dclose, FALSE );

	/*
	 * say bye to whoever's snooping this descriptor
	 */
	if ( dclose->snoop_by )
		write_to_buffer ( dclose->snoop_by, "Your victim has left the game.\r\n", 0 );

	/*
	 * stop snooping everyone else
	 */
	for ( d = first_descriptor; d; d = d->next )
		if ( d->snoop_by == dclose )
			d->snoop_by = NULL;

	/*
	 * Check for switched people who go link-dead. -- Altrag
	 */
	if ( dclose->original )
	{
		if ( ( ch = dclose->character ) != NULL )
			do_return ( ch, "" );
		else
		{
			bug ( "Close_socket: dclose->original without character %s", ( dclose->original->name ? dclose->original->name : "unknown" ) );
			dclose->character = dclose->original;
			dclose->original = NULL;
		}
	}

	ch = dclose->character;

	/*
	 * sanity check :(
	 */

	if ( !dclose->prev && dclose != first_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug ( "Close_socket: %s desc:%p != first_desc:%p and desc->prev = NULL!", ch ? ch->name : d->host, dclose, first_descriptor );
		dp = NULL;

		for ( d = first_descriptor; d; d = dn )
		{
			dn = d->next;

			if ( d == dclose )
			{
				bug ( "Close_socket: %s desc:%p found, prev should be:%p, fixing.", ch ? ch->name : d->host, dclose, dp );
				dclose->prev = dp;
				break;
			}

			dp = d;
		}

		if ( !dclose->prev )
		{
			bug ( "Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}

	if ( !dclose->next && dclose != last_descriptor )
	{
		DESCRIPTOR_DATA *dp, *dn;
		bug ( "Close_socket: %s desc:%p != last_desc:%p and desc->next = NULL!", ch ? ch->name : d->host, dclose, last_descriptor );
		dn = NULL;

		for ( d = last_descriptor; d; d = dp )
		{
			dp = d->prev;

			if ( d == dclose )
			{
				bug ( "Close_socket: %s desc:%p found, next should be:%p, fixing.", ch ? ch->name : d->host, dclose, dn );
				dclose->next = dn;
				break;
			}

			dn = d;
		}

		if ( !dclose->next )
		{
			bug ( "Close_socket: %s desc:%p could not be found!.", ch ? ch->name : dclose->host, dclose );
			DoNotUnlink = TRUE;
		}
	}

	if ( dclose->character )
	{
		sprintf ( log_buf, "Closing link to %s.", ch->pcdata->filename );
		log_string_plus ( log_buf, LOG_COMM, UMAX ( sysdata.log_level, ch->level ) );
		/*
		 * if ( ch->level < LEVEL_DEMI )
		 * to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
		 */

		if ( ( dclose->connected == CON_PLAYING || dclose->connected == CON_EDITING ) )
		{
			act ( AT_ACTION, "$n has lost $s link.", ch, NULL, NULL, TO_CANSEE );
			ch->desc = NULL;
			ch->pcdata->mip_ver = STRALLOC ( "" );
			ch->pcdata->sec_code = STRALLOC ( "" );
			xREMOVE_BIT ( ch->act, PLR_MIP );
		}
		else
		{
			/*
			 * clear descriptor pointer to get rid of bug message in log
			 */
			dclose->character->desc = NULL;
			free_char ( dclose->character );
		}
	}

	if ( !DoNotUnlink )
	{
		/*
		 * make sure loop doesn't get messed up
		 */
		if ( d_next == dclose )
			d_next = d_next->next;

		UNLINK ( dclose, first_descriptor, last_descriptor, next, prev );
	}

	if ( dclose->descriptor == maxdesc )
		--maxdesc;

	free_desc ( dclose );

	--num_descriptors;

	return;
}

bool read_from_descriptor ( DESCRIPTOR_DATA * d )
{
	int iStart, iErr;
	/*
	 * Hold horses if pending command already.
	 */

	if ( d->incomm[0] != STRING_NULL )
		return TRUE;

	/*
	 * Check for overflow.
	 */
	iStart = strlen ( d->inbuf );

	if ( iStart >= sizeof ( d->inbuf ) - 10 )
	{
		sprintf ( log_buf, "%s input overflow!", d->host );
		log_string ( log_buf );
		write_to_descriptor ( d->descriptor, "\r\n*** PUT A LID ON IT!!! ***\r\n" "You cannot enter the same command more than 20 consecutive times!\r\n", 0 );
		return FALSE;
	}

	for ( ;; )
	{
		int nRead;
		nRead = recv ( d->descriptor, d->inbuf + iStart, sizeof ( d->inbuf ) - 10 - iStart, 0 );
		iErr = errno;

		if ( nRead > 0 )
		{
			iStart += nRead;

			if ( d->inbuf[iStart - 1] == '\n' || d->inbuf[iStart - 1] == '\r' )
				break;
		}
		else if ( nRead == 0 )
		{
			log_string_plus ( "EOF encountered on read.", LOG_COMM, sysdata.log_level );
			return FALSE;
		}
		else if ( iErr == EWOULDBLOCK )
			break;
		else
		{
			perror ( "Read_from_descriptor" );
			return FALSE;
		}
	}

	d->inbuf[iStart] = STRING_NULL;

	return TRUE;
}

/*
 * Transfer one line from input buffer to input line.
 */
void read_from_buffer ( DESCRIPTOR_DATA * d )
{
	int i, j, k;
	/*
	 * Hold horses if pending command already.
	 */

	if ( d->incomm[0] != STRING_NULL )
		return;

	/*
	 * Look for at least one new line.
	 */
	for ( i = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r' && i < MAX_INBUF_SIZE; i++ )
	{
		if ( d->inbuf[i] == STRING_NULL )
			return;
	}

	/*
	 * Canonical input processing.
	 */
	for ( i = 0, k = 0; d->inbuf[i] != '\n' && d->inbuf[i] != '\r'; i++ )
	{
		if ( k >= 508 )
		{
			write_to_descriptor ( d->descriptor, "Line too long.\r\n", 0 );
			/*
			 * skip the rest of the line
			 */
			/*
			 * for ( ; d->inbuf[i] != STRING_NULL || i>= MAX_INBUF_SIZE ; i++ )
			 * {
			 * if ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
			 * break;
			 * }
			 */
			d->inbuf[i] = '\n';
			d->inbuf[i + 1] = STRING_NULL;
			break;
		}

		if ( d->inbuf[i] == '\b' && k > 0 )
			--k;
		else if ( isascii ( d->inbuf[i] ) && isprint ( d->inbuf[i] ) )
			d->incomm[k++] = d->inbuf[i];
	}

	/*
	 * Finish off the line.
	 */
	if ( k == 0 )
		d->incomm[k++] = ' ';

	d->incomm[k] = STRING_NULL;

	/*
	 * Deal with bozos with #repeat 1000 ...
	 */
	if ( k > 1 || d->incomm[0] == '!' )
	{
		if ( d->incomm[0] != '!' && strcmp ( d->incomm, d->inlast ) )
		{
			d->repeat = 0;
		}
	}

	/*
	 * Do '!' substitution.
	 */
	if ( d->incomm[0] == '!' )
		strcpy ( d->incomm, d->inlast );
	else
		strcpy ( d->inlast, d->incomm );

	/*
	 * Shift the input buffer.
	 */
	while ( d->inbuf[i] == '\n' || d->inbuf[i] == '\r' )
		i++;

	for ( j = 0; ( d->inbuf[j] = d->inbuf[i + j] ) != STRING_NULL; j++ )
		;

	return;
}

/*
 * Low level output function.
 */
bool flush_buffer ( DESCRIPTOR_DATA * d, bool fPrompt )
{
char buf[MAX_INPUT_LENGTH];
char snoopbuf[MAX_INPUT_LENGTH];
	/*
	 * If buffer has more than 7K inside, spit out 1K at a time   -Thoric
	 */

	if ( !mud_down && d->outtop > 7168 )
	{
		memcpy ( buf, d->outbuf, 1024 );
		d->outtop -= 1024;
		memmove ( d->outbuf, d->outbuf + 1024, d->outtop );

		if ( d->snoop_by )
		{
			buf[1024] = STRING_NULL;

			if ( d->character && d->character->name )
			{
				if ( d->original && d->original->name )
					sprintf ( snoopbuf, "%s (%s)", d->character->name, d->original->name );
				else
					sprintf ( snoopbuf, "%s", d->character->name );

				write_to_buffer ( d->snoop_by, snoopbuf, 0 );
			}

			write_to_buffer ( d->snoop_by, "% ", 2 );

			write_to_buffer ( d->snoop_by, buf, 0 );
		}

		if ( !write_to_descriptor ( d->descriptor, buf, 1024 ) )
		{
			d->outtop = 0;
			return FALSE;
		}

		return TRUE;
	}

	/*
	 * Bust a prompt.
	 */
	if ( fPrompt && !mud_down && d->connected == CON_PLAYING )
	{
		CHAR_DATA *ch;
		ch = d->original ? d->original : d->character;
		send_mip_points ( ch );
		if ( xIS_SET ( ch->act, PLR_BLANK ) )
			write_to_buffer ( d, "\r\n", 2 );

		if ( xIS_SET ( ch->act, PLR_PROMPT ) )
			display_prompt ( d );
		
		if ( ch->fighting )
			send_mip_attacker ( ch );

		if ( xIS_SET ( ch->act, PLR_TELNET_GA ) )
			write_to_buffer ( d, go_ahead_str, 0 );
	}

	/*
	 * Short-circuit if nothing to write.
	 */
	if ( d->outtop == 0 )
		return TRUE;

	/*
	 * Snoop-o-rama.
	 */
	if ( d->snoop_by )
	{
		/*
		 * without check, 'force mortal quit' while snooped caused crash, -h
		 */
		if ( d->character && d->character->name )
		{
			/*
			 * Show original snooped names. -- Altrag
			 */
			if ( d->original && d->original->name )
				sprintf ( buf, "%s (%s)", d->character->name, d->original->name );
			else
				sprintf ( buf, "%s", d->character->name );

			write_to_buffer ( d->snoop_by, buf, 0 );
		}

		write_to_buffer ( d->snoop_by, "% ", 2 );

		write_to_buffer ( d->snoop_by, d->outbuf, d->outtop );
	}

	/*
	 * OS-dependent output.
	 */
	if ( !write_to_descriptor ( d->descriptor, d->outbuf, d->outtop ) )
	{
		d->outtop = 0;
		return FALSE;
	}
	else
	{
		d->outtop = 0;
		return TRUE;
	}
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer ( DESCRIPTOR_DATA * d, const char *txt, int length )
{
	if ( !d )
	{
		bug ( "Write_to_buffer: NULL descriptor" );
		return;
	}

	/*
	 * Normally a bug... but can happen if loadup is used.
	 */
	if ( !d->outbuf )
		return;

	/*
	 * Find length in case caller didn't.
	 */
	if ( length <= 0 )
		length = strlen ( txt );

	/*
	 * Uncomment if debugging or something
	 * if ( length != strlen(txt) )
	 * {
	 * bug( "Write_to_buffer: length(%d) != strlen(txt)!", length );
	 * length = strlen(txt);
	 * }
	 */
	/*
	 * Initial \r\n if needed.
	 */
	if ( d->outtop == 0 && !d->fcommand )
	{
		d->outbuf[0] = '\n';
		d->outbuf[1] = '\r';
		d->outtop = 2;
	}

	/*
	 * Expand the buffer as needed.
	 */
	while ( d->outtop + length >= d->outsize )
	{
		if ( d->outsize > 32000 )
		{
			/*
			 * empty buffer
			 */
			d->outtop = 0;
			/*
			 * Bugfix by Samson - moved bug() call up
			 */
			bug ( "Buffer overflow. Closing (%s).", d->character ? d->character->name : "???" );
			close_socket ( d, TRUE );
			return;
		}

		d->outsize *= 2;

		RECREATE ( d->outbuf, char, d->outsize );
	}

	/*
	 * Copy.
	 */
	strncpy ( d->outbuf + d->outtop, txt, length );

	d->outtop += length;

	d->outbuf[d->outtop] = STRING_NULL;

	return;
}

/*
 * Lowest level output function. Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'), try lowering
 * the max block size.
 *
 * Added block checking to prevent random booting of the descriptor. Thanks go
 * out to Rustry for his suggestions. -Orion
 */
bool write_to_descriptor ( int desc, char *txt, int length )
{
	int iStart = 0;
	int nWrite = 0;
	int nBlock = 0;
	int iErr = 0;

	if ( length <= 0 )
		length = strlen ( txt );

	for ( iStart = 0; iStart < length; iStart += nWrite )
	{
		nBlock = UMIN ( length - iStart, 4096 );
		nWrite = send ( desc, txt + iStart, nBlock, 0 );

		if ( nWrite == -1 )
		{
			iErr = errno;

			if ( iErr == EWOULDBLOCK )
			{
				/*
				 * This is a SPAMMY little bug error. I would suggest
				 * not using it, but I've included it in case. -Orion
				 *
				 perror( "Write_to_descriptor: Send is blocking" );
				 */
				nWrite = 0;
				continue;
			}
			else
			{
				perror ( "Write_to_descriptor" );
				return FALSE;
			}
		}
	}

	return TRUE;
}

/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny ( DESCRIPTOR_DATA * d, char *argument )
{
	/*
	 * extern int lang_array[];
	 * extern char *lang_names[];
	 */
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	char *pwdnew;
	char *p;
	bool fOld, chk;

	if ( d->connected != CON_NOTE_TEXT )
	{
		while ( isspace ( *argument ) )
			argument++;
	}

	ch = d->character;

	switch ( d->connected )
	{

		default:
			bug ( "Nanny: bad d->connected %d.", d->connected );
			close_socket ( d, TRUE );
			return;

/*		case CON_OEDIT:
			oedit_parse ( d, argument );
			break;

		case CON_REDIT:
			redit_parse ( d, argument );
			break;

		case CON_MEDIT:
			medit_parse ( d, argument );
			break;  */


		case CON_GET_NAME:

			if ( argument[0] == STRING_NULL )
			{
				close_socket ( d, FALSE );
				return;
			}

			argument[0] = UPPER ( argument[0] );

			if ( !check_parse_name ( argument, ( d->newstate != 0 ) ) )
			{
				sprintf ( buf, "Sorry, %s does not pass the profanity filter, Please enter another name: ", argument );
				write_to_buffer ( d, buf, 0 );
				return;
			}

			if ( !str_cmp ( argument, "Register" ) )
			{
				if ( d->newstate == 0 )
				{
					if ( sysdata.DENY_NEW_PLAYERS == TRUE )
					{
						send_to_desc_color ( "\033[1;35mPlease be patient, the game server is currently undergoing routine system \r\n", d );
						send_to_desc_color ( "\033[1;35mmaintainence. New players are not accepted at this time. Please try again \r\n", d );
						send_to_desc_color ( "\033[1;35min 5 minuites.\r\n", d );
						close_socket ( d, FALSE );
					}

					clear_screen( d );

					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color ( "\033[0;36m                               New Character Name                               \r\n", d );
					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color ( "\033[0;37m Naming of your character is a very important aspect of playing The Oriental    \r\n", d );
					send_to_desc_color ( "\033[0;37m Dojo make sure that the name fits in a medieval martial arts style of game. The\r\n", d );
					send_to_desc_color ( "\033[0;37m name should not contain profanity, be a common name or the name of an object,  \r\n", d );
					send_to_desc_color ( "\033[0;37m it should also not be a name of a movie, book or other fictional character.    \r\n", d );
					send_to_desc_color ( "\033[0;37m If your name does not meet these criteria, you will be asked to change it to   \r\n", d );
					send_to_desc_color ( "\033[0;37m something more appropriate by The Oriental Dojo the game staff [GM] or [GS]     \r\n", d );
					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color ( "\033[1;37m Please enter the name of your new character: \r\n", d );
					d->newstate++;
					d->connected = CON_GET_NAME;
					return;
				}
				else
				{
					send_to_desc_color ( "\033[1;35mSorry, %s does not pass the profanity filter, Please enter another name: \r\n", d );
					return;
				}
			}

			if ( check_playing ( d, argument, FALSE ) == BERR )
			{
				send_to_desc_color ( "\033[1;37mName: ", d );
				return;
			}

			fOld = load_char_obj ( d, argument, TRUE, FALSE );

			if ( !d->character )
			{
				sprintf ( log_buf, "Bad player file %s@%s.", argument, d->host );
				log_string ( log_buf );
				send_to_desc_color ( "\033[0;31mYour playerfile is corrupt...\033[1;31mPlease notify Tommi, \033[1;37meldhamud@gmail.com.\r\n", d );
				close_socket ( d, FALSE );
				return;
			}

			ch = d->character;

			if ( check_bans ( ch, BAN_SITE ) )
			{
				send_to_desc_color ( "\033[0;32mCONGRATULATIONS...\033[1;31mYour site has been banned from this Mud.\r\n", d );
				close_socket ( d, FALSE );
				return;
			}

			if ( xIS_SET ( ch->act, PLR_DENY ) )
			{
				sprintf ( log_buf, "Denying access to %s@%s.", argument, d->host );
				log_string_plus ( log_buf, LOG_COMM, sysdata.log_level );

				if ( d->newstate != 0 )
				{
					sprintf ( buf, "Sorry, the name %s has been taken, Please enter another name: \r\n", argument );
					write_to_buffer ( d, buf, 0 );
					d->connected = CON_GET_NAME;
					d->character->desc = NULL;
					free_char ( d->character ); /* Big Memory Leak before --Shaddai */
					d->character = NULL;
					return;
				}

				send_to_desc_color ( "\033[0;31mYou are denied access. \033[1;31mPlease contact eldhamud@gmail.com\r\n", d );

				close_socket ( d, FALSE );
				return;
			}

			chk = check_reconnect ( d, argument, FALSE );

			if ( chk == BERR )
				return;

			if ( chk )
			{
				fOld = TRUE;
			}
			else
			{
				if ( wizlock && !IS_IMMORTAL ( ch ) )
				{
					send_to_desc_color ( "\033[1;35mThe game is wizlocked.  Only immortals can connect now.\r\n", d );
					send_to_desc_color ( "\033[1;37mPlease try back later.\r\n", d );
					close_socket ( d, FALSE );
					return;
				}
			}

			if ( fOld )
			{
				if ( d->newstate != 0 )
				{
					sprintf ( buf, "\033[1;35mSorry, the name %s has been taken, Please enter another name: \r\n", argument );
					write_to_buffer ( d, buf, 0 );
					d->connected = CON_GET_NAME;
					d->character->desc = NULL;
					free_char ( d->character ); /* Big Memory Leak before --Shaddai */
					d->character = NULL;
					return;
				}

				send_to_desc_color ( "\033[1;37mEnter Your Password: ", d );

				d->connected = CON_GET_OLD_PASSWORD;
				return;
			}
			else
			{
				if ( d->newstate == 0 )
				{
					send_to_desc_color ( "\r\n\033[1;35mNo such player exists.\r\n ", d );
					send_to_desc_color ( "\033[1;35mPlease check your spelling or type \033[1;37mREGISTER \033[1;35mto start a new player.\r\n", d );
					send_to_desc_color ( "\033[1;37mName: ", d );
					d->connected = CON_GET_NAME;
					d->character->desc = NULL;
					free_char ( d->character ); /* Big Memory Leak before --Shaddai */
					d->character = NULL;
					return;
				}

				send_to_desc_color_args ( d, "\033[1;37mDo you wish for your name to be, %s \033[1;35m(Y/N)\033[1;37m?\r\n", argument );

				d->connected = CON_CONFIRM_NEW_NAME;
				return;
			}

			break;

		case CON_GET_OLD_PASSWORD:
			write_to_buffer ( d, "\r\n", 2 );
			/* This if check is what you will want to keep once it is no longer necessary to convert pfiles */
			if ( str_cmp ( sha256_crypt ( argument ), ch->pcdata->pwd ) )
			{
				write_to_buffer ( d, "Wrong password, disconnecting.\r\n", 0 );
				/* clear descriptor pointer to get rid of bug message in log */
				d->character->desc = NULL;
				close_socket ( d, FALSE );
				return;
			}


			write_to_buffer ( d, echo_on_str, 0 );

			if ( check_playing ( d, ch->pcdata->filename, TRUE ) )
				return;

			chk = check_reconnect ( d, ch->pcdata->filename, TRUE );
			if ( chk == BERR )
			{
				if ( d->character && d->character->desc )
					d->character->desc = NULL;
				close_socket ( d, FALSE );
				return;
			}
			if ( chk == TRUE )
				return;

			strncpy ( buf, ch->pcdata->filename, MAX_STRING_LENGTH );
			d->character->desc = NULL;
			free_char ( d->character );
			d->character = NULL;
			fOld = load_char_obj ( d, buf, FALSE, FALSE );
			ch = d->character;
			if ( ch->position > POS_SITTING && ch->position < POS_STANDING )
				ch->position = POS_STANDING;

			sprintf ( log_buf, "%s@%s has connected.", ch->pcdata->filename, d->host );
			log_string_plus ( log_buf, LOG_COMM, sysdata.log_level );
			if ( ch->pcdata->version < 4 )
			{
				DISPOSE ( ch->pcdata->pwd );
				ch->pcdata->pwd = str_dup ( sha256_crypt ( argument ) );
			}

			{

				struct tm *tme;
				time_t now;
				char day[50];
				now = time ( 0 );
				tme = localtime ( &now );
				strftime ( day, 50, "%a %b %d %H:%M:%S %Y", tme );
				sprintf ( log_buf, "%-20s     %-24s    %s", ch->pcdata->filename, day, d->host );
				write_last_file ( log_buf );
			}
			send_to_desc_color ( "\033[1;37mPress ENTER", d );
			d->connected = CON_PRESS_ENTER;
			break;

		case CON_CONFIRM_NEW_NAME:

			switch ( *argument )
			{

				case 'y':

				case 'Y':
					clear_screen( d );
					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color ( "\033[0;36m                                     Password                                   \r\n", d );
					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color ( "\033[0;37m Your password should have a minimum of 8 characters and be a mixture of        \r\n", d );
					send_to_desc_color ( "\033[0;37m numbers and letters. The less obvious the password is the more secure your     \r\n", d );
					send_to_desc_color ( "\033[0;37m account will be and won't be easily guessed by anyone. Account security is     \r\n", d );
					send_to_desc_color ( "\033[0;37m your responcibility.                                                           \r\n", d );
					send_to_desc_color ( "\033[0;32m--------------------------------------------------------------------------------\r\n", d );
					send_to_desc_color_args ( d, "\033[1;37m Please enter a password for %s: %s", ch->name, echo_off_str );
					d->connected = CON_GET_NEW_PASSWORD;
					break;

				case 'n':

				case 'N':
					send_to_desc_color ( "\033[1;35mOk, what would you like your name to be: \r\n ", d );
					d->character->desc = NULL;
					free_char ( d->character );
					d->character = NULL;
					d->connected = CON_GET_NAME;
					break;

				default:
					send_to_desc_color ( "\033[1;35mPlease type Yes or No: \r\n ", d );
					break;
			}

			break;

		case CON_GET_NEW_PASSWORD:
			send_to_desc_color ( "\r\n", d );

			if ( strlen ( argument ) < 8 )
			{
				send_to_desc_color ( "\033[1;35mThe minumum password lendth is 8 characters. Your password should\r\n", d );
				send_to_desc_color ( "\033[1;35mcontain a mixture of Upper and Lower case letters and numbers.\r\n" "Password: ", d );
				return;
			}

			if ( argument[0] == '!' )
			{
				send_to_desc_color ( "\033[1;35mNew password cannot begin with the '!' character.\r\n", d );
				return;
			}

			pwdnew = sha256_crypt ( argument );  /* SHA-256 Encryption */

			for ( p = pwdnew; *p != STRING_NULL; p++ )
			{
				if ( *p == '~' )
				{
					send_to_desc_color ( "\033[0;32mNew password not acceptable, cannot use the \033[1;37m~ \033[0;32mcharacter.\r\nPassword:\r\n", d );
					return;
				}
			}

			DISPOSE ( ch->pcdata->pwd );

			ch->pcdata->pwd = str_dup ( pwdnew );
			send_to_desc_color ( "\r\n\033[1;35mPlease retype the password to confirm:\r\n", d );
			d->connected = CON_CONFIRM_NEW_PASSWORD;
			break;

		case CON_CONFIRM_NEW_PASSWORD:
			write_to_buffer ( d, "\r\n", 2 );

			if ( str_cmp ( sha256_crypt ( argument ), ch->pcdata->pwd ) )
			{
				send_to_desc_color ( "\033[1;35mPasswords don't match. \033[1;37mRetype password:\r\n", d );
				d->connected = CON_GET_NEW_PASSWORD;
				return;
			}
			send_to_desc_color ( "\033[1;37mPRESS ENTER TO CONTINUE.\r\n\033[0;37m", d );
			d->connected = CON_PRESS_ENTER;
			break;

		case CON_PRESS_ENTER:

			if ( IS_IMMORTAL ( ch ) )
			{
				send_to_desc_color ( "\r\n", d );
				do_last ( ch, "10" );
				do_help ( ch, "imotd" );
			}
			else
			{
				do_help ( ch, "motd" );
			}

			d->connected = CON_READ_MOTD;

			break;

		case CON_READ_MOTD:
		{
			if ( !IS_IMMORTAL ( ch ) )
			{
				sprintf ( buf, "\033[0;32m[&CANNOUNCEMENT\033[0;32m]&c %s has entered the game.", ch->name );
				talk_info ( AT_GREEN, buf, FALSE );

			}
		}

		add_char ( ch );
		d->connected = CON_PLAYING;
		
		if ( ch->level == 0 )
		{
			OBJ_DATA *obj;
			int iLang;
			ch->pcdata->clan = NULL;
			// need to sort out attribute prime here.......
			ch->perm_str += race_table[ch->race]->str_plus;
			ch->perm_int += race_table[ch->race]->int_plus;
			ch->perm_wis += race_table[ch->race]->wis_plus;
			ch->perm_dex += race_table[ch->race]->dex_plus;
			ch->perm_con += race_table[ch->race]->con_plus;
			ch->perm_cha += race_table[ch->race]->cha_plus;
			ch->affected_by = race_table[ch->race]->affected;
			ch->perm_lck += race_table[ch->race]->lck_plus;
			ch->armor += race_table[ch->race]->ac_plus;
			ch->alignment += race_table[ch->race]->alignment;
			ch->attacks = race_table[ch->race]->attacks;
			ch->defenses = race_table[ch->race]->defenses;
			ch->saving_poison_death = race_table[ch->race]->saving_poison_death;
			ch->saving_mental 	= race_table[ch->race]->saving_mental;
			ch->saving_physical 	= race_table[ch->race]->saving_physical;
			ch->saving_weapons 	= race_table[ch->race]->saving_weapons;
			ch->height = number_range ( race_table[ch->race]->height * .9, race_table[ch->race]->height * 1.1 );
			ch->weight = number_range ( race_table[ch->race]->weight * .9, race_table[ch->race]->weight * 1.1 );
			ch->Class = 0;
			ch->race = 0;

			if ( ( iLang = skill_lookup ( "common" ) ) < 0 )
				bug ( "Nanny: cannot find common language." );
			else
				ch->pcdata->learned[iLang] = 100;

		/*	for ( iLang = 0; lang_array[iLang] != LANG_UNKNOWN; iLang++ )
				if ( lang_array[iLang] == race_table[ch->race]->language )
					break;

			if ( lang_array[iLang] == LANG_UNKNOWN )
				bug ( "Nanny: invalid racial language." );
			else
			{
				if ( ( iLang = skill_lookup ( lang_names[iLang] ) ) < 0 )
					bug ( "Nanny: cannot find racial language." );
				else
					ch->pcdata->learned[iLang] = 100;
			}*/

			reset_colors ( ch );

			ch->level = 1;
			ch->exp = 0;
			ch->max_hit += race_table[ch->race]->hit;
			ch->max_mana += race_table[ch->race]->mana;
			ch->hit = UMAX ( 1, ch->max_hit );
			ch->mana = UMAX ( 1, ch->max_mana );
			ch->move = ch->max_move;
			ch->practice = 10;   /*no more mr meany, better give them something to start with Tommi Aug 2005 */
			ch->gold = 5000;
			sprintf ( buf, "Student of the Dojo" );
			set_title ( ch, buf );
			/*
			 * Added by Narn.  Start new characters with autoexit and autgold
			 * already turned on.  Very few people don't use those.
			 */
			xSET_BIT ( ch->act, PLR_AUTOLOOT );
			xSET_BIT ( ch->act, PLR_AUTOGOLD );
			xSET_BIT ( ch->act, PLR_AUTOEXIT );
			xSET_BIT ( ch->act, PLR_AUTOMAP );
			xSET_BIT ( ch->act, PLR_AUTOSAC );
			SET_BIT ( ch->pcdata->flags, PCFLAG_DEADLY );
			xSET_BIT ( ch->act, PLR_ANSI );
			/*
			 * Outfit all players in basic equipments Tommi.
			 */

			if ( ch->Class == CLASS_BUSHI || ch->Class == CLASS_SHUGENJA )
			{
				{
					OBJ_INDEX_DATA *obj_ind = get_obj_index ( 4014 );

					if ( obj_ind != NULL )
					{
						obj = create_object ( obj_ind, 0 );
						obj_to_char ( obj, ch );
						equip_char ( ch, obj, WEAR_WIELD );
					}
				}
			}
			

			if ( !sysdata.WAIT_FOR_AUTH )
				char_to_room ( ch, get_room_index ( ROOM_VNUM_SCHOOL ) );
		}
		else if ( !IS_IMMORTAL ( ch ) && ch->pcdata->release_date > 0 && ch->pcdata->release_date > current_time )
		{
			if ( ch->in_room->vnum == 6 || ch->in_room->vnum == 8 || ch->in_room->vnum == 1206 )
				char_to_room ( ch, ch->in_room );
			else
				char_to_room ( ch, get_room_index ( 8 ) );
		}
		else if ( ch->in_room && ( IS_IMMORTAL ( ch ) || !xIS_SET ( ch->in_room->room_flags, ROOM_PROTOTYPE ) ) )
		{
			char_to_room ( ch, ch->in_room );
		}
		else if ( IS_IMMORTAL ( ch ) )
		{
			char_to_room ( ch, get_room_index ( ROOM_VNUM_CHAT ) );
		}
		else
		{
			char_to_room ( ch, get_room_index ( ROOM_VNUM_TEMPLE ) );
		}

		if ( get_timer ( ch, TIMER_SHOVEDRAG ) > 0 )
			remove_timer ( ch, TIMER_SHOVEDRAG );

		if ( get_timer ( ch, TIMER_PKILLED ) > 0 )
			remove_timer ( ch, TIMER_PKILLED );

		act ( AT_ACTION, "$n has entered the game.", ch, NULL, NULL, TO_CANSEE );

		if ( ch->pcdata->pet )
		{
			act ( AT_ACTION, "$n returns to $s master from the Void.", ch->pcdata->pet, NULL, ch, TO_NOTVICT );
			act ( AT_ACTION, "$N returns with you to the realms.", ch, NULL, ch->pcdata->pet, TO_CHAR );
		}
		send_to_char ( "Welcome to The Oriental Dojo\r\n", ch );
		do_look ( ch, "auto" );
		if ( !ch->was_in_room && ch->in_room == get_room_index ( ROOM_VNUM_TEMPLE ) )
			ch->was_in_room = get_room_index ( ROOM_VNUM_TEMPLE );
		else if ( ch->was_in_room == get_room_index ( ROOM_VNUM_TEMPLE ) )
			ch->was_in_room = get_room_index ( ROOM_VNUM_TEMPLE );
		else if ( !ch->was_in_room )
			ch->was_in_room = ch->in_room;

		break;
	}

	return;
}

bool is_reserved_name ( char *name )
{
	RESERVE_DATA *res;

	for ( res = first_reserved; res; res = res->next )
		if ( ( *res->name == '*' && !str_infix ( res->name + 1, name ) ) || !str_cmp ( res->name, name ) )
			return TRUE;

	return FALSE;
}

/*
 * Parse a name for acceptability.
 */
bool check_parse_name ( char *name, bool newchar )
{
	/*
	 * Names checking should really only be done on new characters, otherwise
	 * we could end up with people who can't access their characters.  Would
	 * have also provided for that new area havoc mentioned below, while still
	 * disallowing current area mobnames.  I personally think that if we can
	 * have more than one mob with the same keyword, then may as well have
	 * players too though, so I don't mind that removal.  -- Alty
	 */
	if ( is_reserved_name ( name ) && newchar )
		return FALSE;

	/*
	 * Length restrictions.
	 */
	if ( strlen ( name ) < 3 )
		return FALSE;

	if ( strlen ( name ) > 12 )
		return FALSE;

	/*
	 * Alphanumerics only.
	 * Lock out IllIll twits.
	 */
	{
		char *pc;
		bool fIll;
		fIll = TRUE;

		for ( pc = name; *pc != STRING_NULL; pc++ )
		{
			if ( !isalpha ( *pc ) )
				return FALSE;

			if ( LOWER ( *pc ) != 'i' && LOWER ( *pc ) != 'l' )
				fIll = FALSE;
		}

		if ( fIll )
			return FALSE;
	}

	/*
	 * Code that followed here used to prevent players from naming
	 * themselves after mobs... this caused much havoc when new areas
	 * would go in...
	 */
	return TRUE;
}

/*
 * Look for link-dead player to reconnect.
 */
bool check_reconnect ( DESCRIPTOR_DATA * d, char *name, bool fConn )
{
	CHAR_DATA *ch;

	for ( ch = first_char; ch; ch = ch->next )
	{
		if ( !IS_NPC ( ch ) && ( !fConn || !ch->desc ) && ch->pcdata->filename && !str_cmp ( name, ch->pcdata->filename ) )
		{
			if ( fConn && ch->switched )
			{
				write_to_buffer ( d, "Already playing.\r\nName: ", 0 );
				d->connected = CON_GET_NAME;

				if ( d->character )
				{
					/*
					 * clear descriptor pointer to get rid of bug message in log
					 */
					d->character->desc = NULL;
					free_char ( d->character );
					d->character = NULL;
				}

				return BERR;
			}

			if ( fConn == FALSE )
			{
				DISPOSE ( d->character->pcdata->pwd );
				d->character->pcdata->pwd = str_dup ( ch->pcdata->pwd );
			}
			else
			{
				/*
				 * clear descriptor pointer to get rid of bug message in log
				 */
				d->character->desc = NULL;
				free_char ( d->character );
				d->character = ch;
				ch->desc = d;
				ch->timer = 0;
				send_to_char ( "Reconnecting... Welcome back.\r\n", ch );
				do_look ( ch, "auto" );
				act ( AT_ACTION, "$n has reconnected.", ch, NULL, NULL, TO_CANSEE );
				sprintf ( log_buf, "%s (%s) reconnected.", ch->name, d->host );
				log_string_plus ( log_buf, LOG_COMM, UMAX ( sysdata.log_level, ch->level ) );
				d->connected = CON_PLAYING;
			}

			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing ( DESCRIPTOR_DATA * d, char *name, bool kick )
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA *dold;
	int cstate;

	for ( dold = first_descriptor; dold; dold = dold->next )
	{
		if ( dold != d && ( dold->character || dold->original ) && !str_cmp ( name, dold->original ? dold->original->pcdata->filename : dold->character->pcdata->filename ) )
		{
			cstate = dold->connected;
			ch = dold->original ? dold->original : dold->character;

			if ( !ch->name || ( cstate != CON_PLAYING && cstate != CON_EDITING ) )
			{
				write_to_buffer ( d, "Already connected - try again.\r\n", 0 );
				sprintf ( log_buf, "%s already connected.", ch->pcdata->filename );
				log_string_plus ( log_buf, LOG_COMM, sysdata.log_level );
				return BERR;
			}

			if ( !kick )
				return TRUE;

			write_to_buffer ( d, "Already playing... Kicking off old connection.\r\n", 0 );
			write_to_buffer ( dold, "Kicking off old connection... bye!\r\n", 0 );
			close_socket ( dold, FALSE );
			/*
			 * clear descriptor pointer to get rid of bug message in log
			 */
			d->character->desc = NULL;
			free_char ( d->character );
			d->character = ch;
			ch->desc = d;
			ch->timer = 0;
			if ( ch->switched )
				do_return ( ch->switched, "" );
			ch->switched = NULL;
			send_to_char ( "Reconnecting... Welcome back.\r\n", ch );
			do_look ( ch, "auto" );
			act ( AT_ACTION, "$n has reconnected, kicking off old link.", ch, NULL, NULL, TO_CANSEE );
			sprintf ( log_buf, "%s@%s reconnected, kicking off old link.", ch->pcdata->filename, d->host );
			log_string_plus ( log_buf, LOG_COMM, UMAX ( sysdata.log_level, ch->level ) );

			/*
			 * if ( ch->level < LEVEL_SAVIOR )
			 * to_channel( log_buf, CHANNEL_MONITOR, "Monitor", ch->level );
			 */
			d->connected = cstate;
			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling ( CHAR_DATA * ch )
{
	ROOM_INDEX_DATA *was_in_room;

	if ( !ch || !ch->desc || ch->desc->connected != CON_PLAYING || !IS_IDLE ( ch ) )
		return;

	ch->timer = 0;
	was_in_room = ch->was_in_room;
	char_from_room ( ch );
	char_to_room ( ch, was_in_room );
	ch->was_in_room = ch->in_room;
	REMOVE_BIT ( ch->pcdata->flags, PCFLAG_IDLE );
	act ( AT_ACTION, "$n has returned from the void.", ch, NULL, NULL, TO_ROOM );
	return;
}

/*
 * Function to strip off the "a" or "an" or "the" or "some" from an object's
 * short description for the purpose of using it in a sentence sent to
 * the owner of the object.  (Ie: an object with the short description
 * "a long dark blade" would return "long dark blade" for use in a sentence
 * like "Your long dark blade".  The object name isn't always appropriate
 * since it contains keywords that may not look proper.  -Thoric
 */
char *myobj ( OBJ_DATA * obj )
{
	if ( !str_prefix ( "a ", obj->short_descr ) )
		return obj->short_descr + 2;

	if ( !str_prefix ( "an ", obj->short_descr ) )
		return obj->short_descr + 3;

	if ( !str_prefix ( "the ", obj->short_descr ) )
		return obj->short_descr + 4;

	if ( !str_prefix ( "some ", obj->short_descr ) )
		return obj->short_descr + 5;

	return obj->short_descr;
}

char *obj_short ( OBJ_DATA * obj )
{
	static char buf[MAX_STRING_LENGTH];

	if ( obj->count > 1 )
	{
		sprintf ( buf, "%s (%d)", obj->short_descr, obj->count );
		return buf;
	}

	return obj->short_descr;
}


#define NAME(ch)        (IS_NPC(ch) ? ch->short_descr : ch->name)
char *act_string ( const char *format, CHAR_DATA * to, CHAR_DATA * ch, void *arg1, void *arg2, int flags )
{
	static char *const he_she[] = { "it", "he", "she" };
	static char *const him_her[] = { "it", "him", "her" };
	static char *const his_her[] = { "its", "his", "her" };
	static char buf[MAX_STRING_LENGTH];
	char fname[MAX_INPUT_LENGTH];
	char *point = buf;
	const char *str = format;
	const char *i;
	CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
	OBJ_DATA *obj1 = ( OBJ_DATA * ) arg1;
	OBJ_DATA *obj2 = ( OBJ_DATA * ) arg2;

	if ( str[0] == '$' )
		DONT_UPPER = FALSE;

	while ( *str != '\0' )
	{
		if ( *str != '$' )
		{
			*point++ = *str++;
			continue;
		}

		++str;

		if ( !arg2 && *str >= 'A' && *str <= 'Z' )
		{
			bug ( "Act: missing arg2 for code %c:", *str );
			bug ( format );
			i = " <@@@> ";
		}
		else
		{
			switch ( *str )
			{

				default:
					bug ( "Act: bad code %c.", *str );
					i = " <@@@> ";
					break;

				case 't':
					i = ( char * ) arg1;
					break;

				case 'T':
					i = ( char * ) arg2;
					break;

				case 'n':
					i = ( to ? PERS ( ch, to, TRUE ) : NAME ( ch ) );
					break;

				case 'N':
					i = ( to ? PERS ( vch, to, FALSE ) : NAME ( vch ) );
					break;

				case 'e':

					if ( ch->sex > 2 || ch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
						i = "it";
					}
					else
						i = he_she[URANGE ( 0, ch->sex, 2 ) ];

					break;

				case 'E':
					if ( vch->sex > 2 || vch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
						i = "it";
					}
					else
						i = he_she[URANGE ( 0, vch->sex, 2 ) ];

					break;

				case 'm':
					if ( ch->sex > 2 || ch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
						i = "it";
					}
					else
						i = him_her[URANGE ( 0, ch->sex, 2 ) ];

					break;

				case 'M':
					if ( vch->sex > 2 || vch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
						i = "it";
					}
					else
						i = him_her[URANGE ( 0, vch->sex, 2 ) ];

					break;

				case 's':
					if ( ch->sex > 2 || ch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", ch->name, ch->sex );
						i = "its";
					}
					else
						i = his_her[URANGE ( 0, ch->sex, 2 ) ];

					break;

				case 'S':
					if ( vch->sex > 2 || vch->sex < 0 )
					{
						bug ( "act_string: player %s has sex set at %d!", vch->name, vch->sex );
						i = "its";
					}
					else
						i = his_her[URANGE ( 0, vch->sex, 2 ) ];

					break;

				case 'q':
					i = ( to == ch ) ? "" : "s";

					break;

				case 'Q':
					i = ( to == ch ) ? "your" : his_her[URANGE ( 0, ch->sex, 2 ) ];

					break;

				case 'p':
					i = ( !to || can_see_obj ( to, obj1 ) ? obj_short ( obj1 ) : "something" );

					break;

				case 'P':
					i = ( !to || can_see_obj ( to, obj2 ) ? obj_short ( obj2 ) : "something" );

					break;

				case 'd':
					if ( !arg2 || ( ( char * ) arg2 ) [0] == '\0' )
						i = "door";
					else
					{
						one_argument ( ( char * ) arg2, fname );
						i = fname;
					}

					break;
			}
		}

		++str;

		while ( ( *point = *i ) != '\0' )
			++point, ++i;
	}

	strcpy ( point, "\r\n" );

	if ( !DONT_UPPER )
		buf[0] = UPPER ( buf[0] );

	return buf;
}

#undef NAME
void act ( short AType, const char *format, CHAR_DATA * ch, void *arg1, void *arg2, int type )
{
	char *txt;
	CHAR_DATA *to;
	CHAR_DATA *vch = ( CHAR_DATA * ) arg2;
	/*
	 * Discard null and zero-length messages.
	 */

	if ( !format || format[0] == STRING_NULL )
		return;

	if ( !ch )
	{
		bug ( "Act: null ch. (%s)", format );
		return;
	}

	if ( !ch->in_room )
		to = NULL;
	else if ( type == TO_CHAR )
		to = ch;
	else
		to = ch->in_room->first_person;

	/*
	 * ACT_SECRETIVE handling
	 */
	if ( IS_NPC ( ch ) && xIS_SET ( ch->act, ACT_SECRETIVE ) && type != TO_CHAR )
		return;

	if ( type == TO_VICT )
	{
		if ( !vch )
		{
			bug ( "Act: null vch with TO_VICT." );
			bug ( "%s (%s)", ch->name, format );
			return;
		}

		if ( !vch->in_room )
		{
			bug ( "Act: vch in NULL room!" );
			bug ( "%s -> %s (%s)", ch->name, vch->name, format );
			return;
		}

		to = vch;
	}

	if ( MOBtrigger && type != TO_CHAR && type != TO_VICT && to )
	{
		OBJ_DATA *to_obj;
		txt = act_string ( format, NULL, ch, arg1, arg2, STRING_IMM );

		if ( HAS_PROG ( to->in_room, ACT_PROG ) )
			rprog_act_trigger ( txt, to->in_room, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );

		for ( to_obj = to->in_room->first_content; to_obj; to_obj = to_obj->next_content )
			if ( HAS_PROG ( to_obj->pIndexData, ACT_PROG ) )
				oprog_act_trigger ( txt, to_obj, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );
	}

	/*
	 * Anyone feel like telling me the point of looping through the whole
	 * room when we're only sending to one char anyways..? -- Alty
	 */
	for ( ; to; to = ( type == TO_CHAR || type == TO_VICT ) ? NULL : to->next_in_room )
	{
		if ( ( !to->desc && ( IS_NPC ( to ) && !HAS_PROG ( to->pIndexData, ACT_PROG ) ) ) || !IS_AWAKE ( to ) )
			continue;

		if ( type == TO_CHAR )
		{
			if ( to != ch )
				continue;

			if ( !is_same_map ( ch, to ) )
				continue;
		}

		if ( type == TO_VICT && ( to != vch || to == ch ) )
			continue;

		if ( type == TO_ROOM )
		{
			if ( to == ch )
				continue;

			if ( !is_same_map ( ch, to ) )
				continue;
		}

		if ( type == TO_NOTVICT )
		{
			if ( to == ch || to == vch )
				continue;

			if ( !is_same_map ( ch, to ) )
				continue;
		}

		if ( type == TO_CANSEE )
		{
			if ( to == ch )
				continue;

			if ( IS_IMMORTAL ( ch ) && IS_PLR_FLAG ( ch, PLR_WIZINVIS ) )
			{
				if ( to->level < ch->pcdata->wizinvis )
					continue;
//				if ( to->desc && is_inolc ( to->desc ) )
//					continue;
			}

			if ( !is_same_map ( ch, to ) )
				continue;
		}

		if ( IS_IMMORTAL ( to ) )
			txt = act_string ( format, to, ch, arg1, arg2, STRING_IMM );
		else
			txt = act_string ( format, to, ch, arg1, arg2, STRING_NONE );

		if ( to->desc )
		{
			set_char_color ( AType, to );
			send_to_char ( txt, to );
		}

		if ( MOBtrigger )
		{
			/*
			 * Note: use original string, not string with ANSI. -- Alty
			 */
			mprog_act_trigger ( txt, to, ch, ( OBJ_DATA * ) arg1, ( void * ) arg2 );
		}
	}

	MOBtrigger = TRUE;

	return;
}

char *default_fprompt ( CHAR_DATA * ch )
{
	static char buf[60];
	strcpy ( buf, "\033[1;31m<\033[1;33m%hhp &C%mm &G%vmv\033[1;31m %E> " );

	if ( IS_NPC ( ch ) || IS_IMMORTAL ( ch ) )
		strcat ( buf, "%i%R" );

	return buf;
}

char *default_prompt ( CHAR_DATA * ch )
{
	static char buf[60];
	strcpy ( buf, "\033[1;31m<\033[1;33m%hhp &C%mm &G%vmv\033[1;31m> " );

	if ( IS_NPC ( ch ) || IS_IMMORTAL ( ch ) )
		strcat ( buf, "%i%R" );

	return buf;
}

int getcolor ( char clr )
{
	static const char colors[16] = "xrgObpcwzRGYBPCW";
	int r;

	for ( r = 0; r < 16; r++ )
		if ( clr == colors[r] )
			return r;

	return -1;
}

void display_prompt ( DESCRIPTOR_DATA * d )
{
	CHAR_DATA *ch = d->character;
	CHAR_DATA *och = ( d->original ? d->original : d->character );
	CHAR_DATA *victim;
	bool ansi = ( !IS_NPC ( och ) && xIS_SET ( och->act, PLR_ANSI ) );
	const char *prompt;
	const char *helpstart = "<Type HELP START>";
	char buf[MAX_STRING_LENGTH];
	char *pbuf = buf;
	int pstat, percent;

	if ( !ch )
	{
		bug ( "display_prompt: NULL ch" );
		return;
	}

	if ( !IS_NPC ( ch ) && !IS_SET ( ch->pcdata->flags, PCFLAG_HELPSTART ) )
		prompt = helpstart;
	else if ( !IS_NPC ( ch ) && ch->substate != SUB_NONE && ch->pcdata->subprompt && ch->pcdata->subprompt[0] != STRING_NULL )
		prompt = ch->pcdata->subprompt;
	else if ( IS_NPC ( ch ) || ( !ch->fighting && ( !ch->pcdata->prompt || !*ch->pcdata->prompt ) ) )
		prompt = default_prompt ( ch );
	else if ( ch->fighting )
	{
		if ( !ch->pcdata->fprompt || !*ch->pcdata->fprompt )
			prompt = default_fprompt ( ch );
		else
			prompt = ch->pcdata->fprompt;
	}
	else
		prompt = ch->pcdata->prompt;

	if ( ansi )
	{
		strcpy ( pbuf, ANSI_RESET );
		d->prevcolor = 0x08;
		pbuf += 4;
	}

	/*
	 * Clear out old color stuff
	 */
	for ( ; *prompt; prompt++ )
	{
		/*
		 * '%' = prompt commands
		 * Note: foreground changes will revert background to 0 (black)
		 */
		if ( *prompt != '%' )
		{
			* ( pbuf++ ) = *prompt;
			continue;
		}

		++prompt;

		if ( !*prompt )
			break;

		if ( *prompt == * ( prompt - 1 ) )
		{
			* ( pbuf++ ) = *prompt;
			continue;
		}

		switch ( * ( prompt - 1 ) )
		{

			default:
				bug ( "Display_prompt: bad command char '%c'.", * ( prompt - 1 ) );
				break;

			case '%':
				*pbuf = STRING_NULL;
				pstat = 0x80000000;

				switch ( *prompt )
				{

					case '%':
						*pbuf++ = '%';
						*pbuf = STRING_NULL;
						break;

					case 'a':

						if ( ch->level >= 10 )
							pstat = ch->alignment;
						else if ( IS_GOOD ( ch ) )
							strcpy ( pbuf, "good" );
						else if ( IS_EVIL ( ch ) )
							strcpy ( pbuf, "evil" );
						else
							strcpy ( pbuf, "neutral" );

						break;

					case 'A':
						sprintf ( pbuf, "%s%s%s", IS_AFFECTED ( ch, AFF_INVISIBLE ) ? "I" : "", IS_AFFECTED ( ch, AFF_HIDE ) ? "H" : "", IS_AFFECTED ( ch, AFF_SNEAK ) ? "S" : "" );

						break;

					case 'C':  /* Tank */
						if ( !IS_IMMORTAL ( ch ) )
							break;

						if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else if ( !victim->fighting || ( victim = victim->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else
						{
							if ( victim->max_hit > 0 )
								percent = ( 100 * victim->hit ) / victim->max_hit;
							else
								percent = -1;

							if ( percent >= 100 )
								strcpy ( pbuf, "perfect health" );
							else if ( percent >= 90 )
								strcpy ( pbuf, "slightly scratched" );
							else if ( percent >= 80 )
								strcpy ( pbuf, "few bruises" );
							else if ( percent >= 70 )
								strcpy ( pbuf, "some cuts" );
							else if ( percent >= 60 )
								strcpy ( pbuf, "several wounds" );
							else if ( percent >= 50 )
								strcpy ( pbuf, "nasty wounds" );
							else if ( percent >= 40 )
								strcpy ( pbuf, "bleeding freely" );
							else if ( percent >= 30 )
								strcpy ( pbuf, "covered in blood" );
							else if ( percent >= 20 )
								strcpy ( pbuf, "leaking guts" );
							else if ( percent >= 10 )
								strcpy ( pbuf, "almost dead" );
							else
								strcpy ( pbuf, "DYING" );
						}

						break;

					case 'c':

						if ( !IS_IMMORTAL ( ch ) )
							break;

						if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else
						{
							if ( victim->max_hit > 0 )
								percent = ( 100 * victim->hit ) / victim->max_hit;
							else
								percent = -1;

							if ( percent >= 100 )
								strcpy ( pbuf, "perfect health" );
							else if ( percent >= 90 )
								strcpy ( pbuf, "slightly scratched" );
							else if ( percent >= 80 )
								strcpy ( pbuf, "few bruises" );
							else if ( percent >= 70 )
								strcpy ( pbuf, "some cuts" );
							else if ( percent >= 60 )
								strcpy ( pbuf, "several wounds" );
							else if ( percent >= 50 )
								strcpy ( pbuf, "nasty wounds" );
							else if ( percent >= 40 )
								strcpy ( pbuf, "bleeding freely" );
							else if ( percent >= 30 )
								strcpy ( pbuf, "covered in blood" );
							else if ( percent >= 20 )
								strcpy ( pbuf, "leaking guts" );
							else if ( percent >= 10 )
								strcpy ( pbuf, "almost dead" );
							else
								strcpy ( pbuf, "DYING" );
						}

						break;

					case 'e':

						if ( ( victim = who_fighting ( ch ) ) != NULL )
						{
							if ( victim->max_hit > 0 )
								percent = ( 100 * victim->hit ) / victim->max_hit;
							else
								percent = -1;

							if ( percent >= 65 )
								sprintf ( pbuf, " \033[1;31mEnemy: &g%d%%", percent );
							else if ( percent >= 25 && percent < 65 )
								sprintf ( pbuf, " \033[1;31mEnemy: \033[1;33m%d%%", percent );
							else
								sprintf ( pbuf, " \033[1;31mEnemy: \033[0;31m%d%%", percent );
						}

						break;

					case 'E':

						if ( ( victim = who_fighting ( ch ) ) != NULL )
						{
							if ( victim->max_hit > 0 )
								percent = ( 100 * victim->hit ) / victim->max_hit;
							else
								percent = -1;

							if ( percent >= 89 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y+++&g+++&R]&D" );
							else if ( percent >= 79 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y+++&g++ &R]&D" );
							else if ( percent >= 69 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y+++&g+  &R]&D" );
							else if ( percent >= 59 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y+++   &R]&D" );
							else if ( percent >= 49 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y++    &R]&D" );
							else if ( percent >= 39 )
								strcpy ( pbuf, "&REnemy:[&r+++&Y+     &R]&D" );
							else if ( percent >= 29 )
								strcpy ( pbuf, "&REnemy:[&r+++      &R]&D" );
							else if ( percent >= 19 )
								strcpy ( pbuf, "&REnemy:[&r++       &R]&D" );
							else if ( percent >= 9 )
								strcpy ( pbuf, "&REnemy:[&r+        &R]&D" );
							else
								strcpy ( pbuf, "&REnemy:[         &R]&D" );
						}

						break;

					case 'h':
						pstat = ch->hit;
						break;

					case 'H':
						pstat = ch->max_hit;
						break;

					case 'm':
						pstat = ch->mana;
						break;

					case 'M':
						pstat = ch->max_mana;
						break;

					case 'N':  /* Tank */

						if ( !IS_IMMORTAL ( ch ) )
							break;

						if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else if ( !victim->fighting || ( victim = victim->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else
						{
							if ( ch == victim )
								strcpy ( pbuf, "You" );
							else if ( IS_NPC ( victim ) )
								strcpy ( pbuf, victim->short_descr );
							else
								strcpy ( pbuf, victim->name );

							pbuf[0] = UPPER ( pbuf[0] );
						}

						break;

					case 'n':

						if ( !IS_IMMORTAL ( ch ) )
							break;

						if ( !ch->fighting || ( victim = ch->fighting->who ) == NULL )
							strcpy ( pbuf, "N/A" );
						else
						{
							if ( ch == victim )
								strcpy ( pbuf, "You" );
							else if ( IS_NPC ( victim ) )
								strcpy ( pbuf, victim->short_descr );
							else
								strcpy ( pbuf, victim->name );

							pbuf[0] = UPPER ( pbuf[0] );
						}

						break;

					case 'T':

						if ( time_info.hour < 5 )
							strcpy ( pbuf, "night" );
						else if ( time_info.hour < 6 )
							strcpy ( pbuf, "dawn" );
						else if ( time_info.hour < 19 )
							strcpy ( pbuf, "day" );
						else if ( time_info.hour < 21 )
							strcpy ( pbuf, "dusk" );
						else
							strcpy ( pbuf, "night" );

						break;

					case 'b':
						pstat = 0;

						break;

					case 'B':
						pstat = 0;

						break;

					case 'u':
						pstat = num_descriptors;

						break;

					case 'U':
						pstat = sysdata.maxplayers;

						break;

					case 'v':
						pstat = ch->move;

						break;

					case 'V':
						pstat = ch->max_move;

						break;

					case 'g':
						pstat = ch->gold;

						break;

					case 'r':
						if ( IS_IMMORTAL ( och ) )
							pstat = ch->in_room->vnum;

						break;

					case 'F':
						if ( IS_IMMORTAL ( och ) )
							sprintf ( pbuf, "%s", ext_flag_string ( &ch->in_room->room_flags, r_flags ) );

						break;

					case 'R':
						if ( xIS_SET ( och->act, PLR_ROOMVNUM ) )
							sprintf ( pbuf, "\033[1;31m<#%d>&D ", ch->in_room->vnum );

						break;

					case 'x':
						pstat = ch->exp;

						break;

					case 'X':
						pstat = exp_level ( ch, ch->level + 1 ) - ch->exp;

						break;

					case 'o':  /* display name of object on auction */
						if ( auction->item )
							strcpy ( pbuf, auction->item->name );

						break;

					case 'S':
						if ( ch->style == STYLE_DRAGON )
							strcpy ( pbuf, "Dragon" );
						else if ( ch->style == STYLE_NAGA )
							strcpy ( pbuf, "Naga" );
						else if ( ch->style == STYLE_SCORPION )
							strcpy ( pbuf, "Scorpion" );
						else if ( ch->style == STYLE_TIGER )
							strcpy ( pbuf, "Tiger" );
						else if ( ch->style == STYLE_UNICORN )
							strcpy ( pbuf, "Unicorn" );
						else if ( ch->style == STYLE_PHOENIX )
							strcpy ( pbuf, "Phoenix" );
						else if ( ch->style == STYLE_CRANE )
							strcpy ( pbuf, "Crane" );
						else if ( ch->style == STYLE_BADGER )
							strcpy ( pbuf, "Badger" );					
						else
							strcpy ( pbuf, "Standard" );

						break;

					case 'i':
						if ( ( !IS_NPC ( ch ) && xIS_SET ( ch->act, PLR_WIZINVIS ) ) || ( IS_NPC ( ch ) && xIS_SET ( ch->act, ACT_MOBINVIS ) ) )
							sprintf ( pbuf, "(Invis %d) ", ( IS_NPC ( ch ) ? ch->mobinvis : ch->pcdata->wizinvis ) );
						else if ( IS_AFFECTED ( ch, AFF_INVISIBLE ) )
							sprintf ( pbuf, "(Invis) " );

						break;

					case 'I':
						pstat = ( IS_NPC ( ch ) ? ( xIS_SET ( ch->act, ACT_MOBINVIS ) ? ch->mobinvis : 0 ) : ( xIS_SET ( ch->act, PLR_WIZINVIS ) ? ch->pcdata->wizinvis : 0 ) );

						break;
				}

				if ( pstat != 0x80000000 )
					sprintf ( pbuf, "%d", pstat );

				pbuf += strlen ( pbuf );

				break;
		}
	}

	*pbuf = STRING_NULL;

	send_to_char ( buf, ch );
	send_to_char ( "\r\n", ch );
	return;
}

void set_pager_input ( DESCRIPTOR_DATA * d, char *argument )
{
	while ( isspace ( *argument ) )
		argument++;

	d->pagecmd = *argument;

	return;
}

bool pager_output ( DESCRIPTOR_DATA * d )
{
	register char *last;
	CHAR_DATA *ch;
	int pclines;
	register int lines;
	bool ret;

	if ( !d || !d->pagepoint || d->pagecmd == -1 )
		return TRUE;

	ch = d->original ? d->original : d->character;

	pclines = UMAX ( ch->pcdata->pagerlen, 5 ) - 1;

	switch ( LOWER ( d->pagecmd ) )
	{

		default:
			lines = 0;
			break;

		case 'b':
			lines = -1 - ( pclines * 2 );
			break;

		case 'r':
			lines = -1 - pclines;
			break;

		case 'n':
			lines = 0;
			pclines = 0x7FFFFFFF;   /* As many lines as possible */
			break;

		case 'q':
			d->pagetop = 0;
			d->pagepoint = NULL;
			flush_buffer ( d, TRUE );
			DISPOSE ( d->pagebuf );
			d->pagesize = MAX_STRING_LENGTH;
			return TRUE;
	}

	while ( lines < 0 && d->pagepoint >= d->pagebuf )
		if ( * ( --d->pagepoint ) == '\n' )
			++lines;

	if ( *d->pagepoint == '\n' && * ( ++d->pagepoint ) == '\r' )
		++d->pagepoint;

	if ( d->pagepoint < d->pagebuf )
		d->pagepoint = d->pagebuf;

	for ( lines = 0, last = d->pagepoint; lines < pclines; ++last )
		if ( !*last )
			break;
		else if ( *last == '\n' )
			++lines;

	if ( *last == '\r' )
		++last;

	if ( last != d->pagepoint )
	{
		if ( !write_to_descriptor ( d->descriptor, d->pagepoint, ( last - d->pagepoint ) ) )
			return FALSE;

		d->pagepoint = last;
	}

	while ( isspace ( *last ) )
		++last;

	if ( !*last )
	{
		d->pagetop = 0;
		d->pagepoint = NULL;
		flush_buffer ( d, TRUE );
		DISPOSE ( d->pagebuf );
		d->pagesize = MAX_STRING_LENGTH;
		return TRUE;
	}

	d->pagecmd = -1;

	if ( xIS_SET ( ch->act, PLR_ANSI ) )
		if ( write_to_descriptor ( d->descriptor, ANSI_LBLUE, 0 ) == FALSE )
			return FALSE;

	if ( ( ret = write_to_descriptor ( d->descriptor, "(C)ontinue, (N)on-stop, (R)efresh, (B)ack, (Q)uit: [C] ", 0 ) ) == FALSE )
		return FALSE;

	if ( xIS_SET ( ch->act, PLR_ANSI ) )
	{
		char buf[32];
		sprintf ( buf, "%s", color_str ( d->pagecolor, ch ) );
		ret = write_to_descriptor ( d->descriptor, buf, 0 );
	}

	return ret;
}

