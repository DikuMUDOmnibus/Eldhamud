/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 * [S]imulated [M]edieval [A]dventure multi[U]ser [G]ame      |   \\._.//   *
 * -----------------------------------------------------------|   (0...0)   *
 * SMAUG 1.4 (C) 1994, 1995, 1996, 1998  by Derek Snider      |    ).:.(    *
 * -----------------------------------------------------------|    {o o}    *
 * SMAUG code team: Thoric, Altrag, Blodkai, Narn, Haus,      |   / ' ' \   *
 * Scryn, Rennard, Swordbearer, Gorog, Grishnakh, Nivek,      |~'~.VxvxV.~'~*
 * Tricops and Fireblade                                      |             *
 * ------------------------------------------------------------------------ *
 * Merc 2.1 Diku Mud improvments copyright (C) 1992, 1993 by Michael        *
 * Chastain, Michael Quan, and Mitchell Tse.                                *
 * Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,          *
 * Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.     *
 * Win32 port by Nick Gammon                                                *
 * ------------------------------------------------------------------------ *
 *	 blazerayne@hotmail.com		    Main mud header file			    *
 ****************************************************************************/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>
#include <re_comp.h>
#include <sys/time.h>

typedef	int				ch_ret;
typedef	int				obj_ret;

#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun

#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

#if	!defined(BERR)
#define BERR	 255
#endif

typedef unsigned char			bool;

/*
 * Structure types.
 */
typedef struct	affect_data		AFFECT_DATA;
typedef struct	area_data		AREA_DATA;
typedef struct  auction_data            AUCTION_DATA;	/* auction data */
typedef struct	ban_data		BAN_DATA;
typedef struct	extracted_char_data	EXTRACT_CHAR_DATA;
typedef struct	char_data		CHAR_DATA;
typedef struct	hunt_hate_fear		HHF_DATA;
typedef struct	fighting_data		FIGHT_DATA;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data		EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data		HELP_DATA;
typedef struct	menu_data		MENU_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	obj_data		OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data			PC_DATA;
typedef struct	reset_data		RESET_DATA;
typedef struct	map_index_data		MAP_INDEX_DATA;		/* maps */
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data		SHOP_DATA;
typedef struct	race_type		RACE_TYPE;
typedef struct	repairshop_data		REPAIR_DATA;
typedef struct	reserve_data		RESERVE_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	hour_min_sec		HOUR_MIN_SEC;
typedef struct	weather_data		WEATHER_DATA;
typedef struct	neighbor_data		NEIGHBOR_DATA; /* FB */
typedef	struct	clan_data		CLAN_DATA;
typedef struct  tourney_data            TOURNEY_DATA;
typedef struct	mob_prog_data		MPROG_DATA;
typedef struct	mob_prog_act_list	MPROG_ACT_LIST;
typedef struct  mpsleep_data 		MPSLEEP_DATA;
typedef	struct	editor_data		EDITOR_DATA;
typedef struct	teleport_data		TELEPORT_DATA;
typedef struct	timer_data		TIMER;
typedef struct  godlist_data		GOD_DATA;
typedef struct	system_data		SYSTEM_DATA;
typedef	struct	smaug_affect		SMAUG_AFF;
typedef struct  who_data                WHO_DATA;
typedef	struct	skill_type		SKILLTYPE;
typedef	struct	social_type		SOCIALTYPE;
typedef	struct	cmd_type		CMDTYPE;
typedef struct  deity_data		DEITY_DATA;
typedef struct	wizent			WIZENT;
typedef struct  ignore_data		IGNORE_DATA;
typedef struct	extended_bitvector	EXT_BV;
typedef	struct	lcnv_data		LCNV_DATA;
typedef	struct	lang_data		LANG_DATA;
typedef struct  locker_data		LOCKER_DATA;
typedef struct  quest_data              QUEST_DATA;	/* questmaster data */
typedef	struct	olc_data		OLC_DATA;


/*
 * Function types.
 */
typedef	void	DO_FUN		args ( ( CHAR_DATA *ch, char *argument ) );
typedef bool	SPEC_FUN	args ( ( CHAR_DATA *ch ) );
typedef ch_ret	SPELL_FUN	args ( ( int sn, int level, CHAR_DATA *ch, void *vo ) );

#define DUR_CONV	25
#define HIDDEN_TILDE	'*'

/* 32bit bitvector defines */
#define BV00		(1 <<  0)
#define BV01		(1 <<  1)
#define BV02		(1 <<  2)
#define BV03		(1 <<  3)
#define BV04		(1 <<  4)
#define BV05		(1 <<  5)
#define BV06		(1 <<  6)
#define BV07		(1 <<  7)
#define BV08		(1 <<  8)
#define BV09		(1 <<  9)
#define BV10		(1 << 10)
#define BV11		(1 << 11)
#define BV12		(1 << 12)
#define BV13		(1 << 13)
#define BV14		(1 << 14)
#define BV15		(1 << 15)
#define BV16		(1 << 16)
#define BV17		(1 << 17)
#define BV18		(1 << 18)
#define BV19		(1 << 19)
#define BV20		(1 << 20)
#define BV21		(1 << 21)
#define BV22		(1 << 22)
#define BV23		(1 << 23)
#define BV24		(1 << 24)
#define BV25		(1 << 25)
#define BV26		(1 << 26)
#define BV27		(1 << 27)
#define BV28		(1 << 28)
#define BV29		(1 << 29)
#define BV30		(1 << 30)
#define BV31		(1 << 31)
/* 32 USED! DO NOT ADD MORE! SB */


/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		 2048
#define MAX_STRING_LENGTH	 8192  	/* buf */
#define MAX_INPUT_LENGTH	 2048  	/* arg */
#define MAX_INBUF_SIZE		 1024
#define MAX_RGRID_ROOMS		 100  	/*used in do_makerooms to set the maximum rooms you can create at one time*/
#define LAST_FILE_SIZE           500  	/*maximum entries in the last file */
#define MAX_VNUM		 65000 	/*used in lots of ifchecks to determine the maximum vnum allowed*/
#define HASHSTR			 	/* use string hashing */
#define	MAX_LAYERS		 8	/* maximum clothing layers */
#define MAX_NEST	       100	/* maximum container nesting */

/*
 * Game parameters.
 * Increase the max'es if you add more of something.
 * Adjust the pulse numbers to suit yourself.
 */
#define MAX_EXP_WORTH	           50000
#define MIN_EXP_WORTH		   100
#define MAX_REXITS		   20	/* Maximum exits allowed in 1 room */
#define MAX_SKILL		   500
#define SPELL_SILENT_MARKER        "silent"	/* No OK. or Failed. */
#define MAX_CLASS           	   9
#define MAX_NPC_CLASS		   35
#define MAX_RACE                   9
#define MAX_NPC_RACE		   85
#define MAX_QDATA 		   20  /* You can set this to however many available slots you want. */

extern int MAX_PC_RACE;
extern int MAX_PC_CLASS;
extern bool mud_down;

#define MAX_LEVEL		   45
#define MAX_CLAN		   50
#define MAX_DEITY		   50
#define	MAX_HERB		   20
#define	MAX_DISEASE		   20
#define MAX_PERSONAL		    5   /* Maximum personal skills */
#define MAX_WHERE_NAME             14
#define LEVEL_HERO		   (MAX_LEVEL - 15)
#define LEVEL_IMMORTAL		   (MAX_LEVEL - 14)
#define LEVEL_SUPREME		   MAX_LEVEL
#define LEVEL_INFINITE		   (MAX_LEVEL - 1)
#define LEVEL_ETERNAL		   (MAX_LEVEL - 2)
#define LEVEL_IMPLEMENTOR	   (MAX_LEVEL - 3)
#define LEVEL_SUB_IMPLEM	   (MAX_LEVEL - 4)
#define LEVEL_ASCENDANT		   (MAX_LEVEL - 5)
#define LEVEL_GREATER		   (MAX_LEVEL - 6)
#define LEVEL_GOD		   (MAX_LEVEL - 7)
#define LEVEL_LESSER		   (MAX_LEVEL - 8)
#define LEVEL_TRUEIMM		   (MAX_LEVEL - 9)
#define LEVEL_DEMI		   (MAX_LEVEL - 10)
#define LEVEL_SAVIOR		   (MAX_LEVEL - 11)
#define LEVEL_CREATOR		   (MAX_LEVEL - 12)
#define LEVEL_ACOLYTE		   (MAX_LEVEL - 13)
#define LEVEL_NEOPHYTE		   (MAX_LEVEL - 14)
#define LEVEL_AVATAR		   (MAX_LEVEL - 15)
#define LEVEL_LOG		    LEVEL_LESSER
#define LEVEL_HIGOD		    LEVEL_GOD

#include "overland.h"
#include "house.h" /* For housing module */
#include "bank.h" /* for banking code */
#include "pfiles.h"
#include "arena.h"
#include "color.h"
#include "hotboot.h"
#include "liquids.h" /* SMAUG Liquidtable Replacement  -Nopey */
#include "do_fun.h"
#include "gsn.h"
#include "macros.h"
#include "mip.h"


/* This is to tell if act uses uppercasestring or not --Shaddai */
bool DONT_UPPER;

#define	SECONDS_PER_TICK			 60
#define PULSE_PER_SECOND			  4
#define PULSE_VIOLENCE				 (3 * PULSE_PER_SECOND)
#define PULSE_MOBILE				 (4 * PULSE_PER_SECOND)
#define PULSE_TICK		  		 (SECONDS_PER_TICK * PULSE_PER_SECOND)
#define PULSE_AREA				 (SECONDS_PER_TICK * PULSE_PER_SECOND)
#define PULSE_AUCTION				 (SECONDS_PER_TICK * PULSE_PER_SECOND)

/*
 * SMAUG Version -- Scryn
 */
#define SMAUG_VERSION_MAJOR "Eldhamud.V2.2"
#define SMAUG_VERSION_MINOR ".45"


/*
 * Stuff for area versions --Shaddai
 */
int     area_version;
#define HAS_SPELL_INDEX   -1
#define AREA_VERSION_WRITE 2

/*
 * Command logging types.
 */
typedef enum
{
	LOG_NORMAL, LOG_ALWAYS, LOG_NEVER, LOG_BUILD, LOG_HIGH, LOG_COMM,
	LOG_WARN, LOG_ALL
} log_types;

/* short cut crash bug fix provided by gfinello@mail.karmanet.it*/
typedef enum
{
	relMSET_ON, relOSET_ON
} relation_type;

typedef struct rel_data REL_DATA;

struct rel_data
{
	void *Actor;
	void *Subject;
	REL_DATA  *next;
	REL_DATA  *prev;
	relation_type Type;
};


/*
 * Return types for move_char, damage, greet_trigger, etc, etc
 * Added by Thoric to get rid of bugs
 */
typedef enum
{
	rNONE, rCHAR_DIED, rVICT_DIED, rBOTH_DIED, rCHAR_QUIT, rVICT_QUIT,
	rBOTH_QUIT, rSPELL_FAILED, rOBJ_SCRAPPED, rOBJ_EATEN, rOBJ_EXPIRED,
	rOBJ_TIMER, rOBJ_SACCED, rOBJ_QUAFFED, rOBJ_USED, rOBJ_EXTRACTED,
	rOBJ_DRUNK, rCHAR_IMMUNE, rVICT_IMMUNE,
	rCHAR_AND_OBJ_EXTRACTED = 128,
	rERROR = 255
} ret_types;

/* Echo types for echo_to_all */
#define ECHOTAR_ALL	0
#define ECHOTAR_PC	1
#define ECHOTAR_IMM	2

/* defines for new do_who */
#define WT_MORTAL	0
#define WT_DEADLY	1
#define WT_IMM		2
#define WT_GROUPED	3
#define WT_GROUPWHO	4

/*
 * Defines for extended bitvectors
 */
#ifndef INTBITS
#define INTBITS	32
#endif
#define XBM		31	/* extended bitmask   ( INTBITS - 1 )	*/
#define RSV		5	/* right-shift value  ( sqrt(XBM+1) )	*/
#define XBI		4	/* integers in an extended bitvector	*/
#define MAX_BITS	XBI * INTBITS
/*
 * Structure for extended bitvectors -- Thoric
 */
struct extended_bitvector
{
	int		bits[XBI];
};

#ifdef IMC
#include "imc.h"
#endif

/*
 * Tongues / Languages structures
 */

struct lcnv_data
{
	LCNV_DATA *		next;
	LCNV_DATA *		prev;
	char *			old;
	int			olen;
	char *			new;
	int			nlen;
};

struct lang_data
{
	LANG_DATA *		next;
	LANG_DATA *		prev;
	char *			name;
	LCNV_DATA *		first_precnv;
	LCNV_DATA *		last_precnv;
	char *			alphabet;
	LCNV_DATA *		first_cnv;
	LCNV_DATA *		last_cnv;
};



/*
 * do_who output structure -- Narn
 */
struct who_data
{
	WHO_DATA *prev;
	WHO_DATA *next;
	char *text;
	int  type;
};

/*
 * Ban Types --- Shaddai
 */
#define BAN_SITE        1
#define BAN_WARN        -1


#define is_full_name is_name

/*
 * Site ban structure.
 */
struct	ban_data
{
	BAN_DATA *next;
	BAN_DATA *prev;
	char     *name;     /* Name of site/Class/race banned */
	char     *note;     /* Why it was banned */
	char     *ban_by;   /* Who banned this site */
	char     *ban_time; /* Time it was banned */
	int      flag;      /* Class or Race number */
	int      unban_date;/* When ban expires */
	short   duration;  /* How long it is banned for */
	short   level;     /* Level that is banned */
	bool     warn;      /* Echo on warn channel */
	bool     prefix;    /* Use of *site */
	bool     suffix;    /* Use of site* */
};



/*
 * Yeesh.. remind us of the old MERC ban structure? :)
 */
struct	reserve_data
{
	RESERVE_DATA *next;
	RESERVE_DATA *prev;
	char *name;
};

struct  quest_data
{
	int 	level_range;
	int	 	practice_amt;
	int		practice_cost;
	int		award_vnum[MAX_QDATA+1];
	int		award_value[MAX_QDATA+1];
	int		gold_amt;
	int		gold_cost;
};

/*
 * Time and weather stuff.
 */
typedef enum
{
	SUN_DARK, SUN_RISE, SUN_LIGHT, SUN_SET
} sun_positions;

typedef enum
{
	SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
} sky_conditions;

struct	time_info_data
{
	int		hour;
	int		day;
	int		month;
	int		year;
	int		sunlight;
};

struct hour_min_sec
{
	int hour;
	int min;
	int sec;
	int manual;
};

/* Define maximum number of climate settings - FB */
#define MAX_CLIMATE 5

struct	weather_data
{
	int 		temp;		/* temperature */
	int		precip;		/* precipitation */
	int		wind;		/* umm... wind */
	int		temp_vector;	/* vectors controlling */
	int		precip_vector;	/* rate of change */
	int		wind_vector;
	int		climate_temp;	/* climate of the area */
	int		climate_precip;
	int		climate_wind;
	NEIGHBOR_DATA *	first_neighbor;	/* areas which affect weather sys */
	NEIGHBOR_DATA *	last_neighbor;
	char *		echo;		/* echo string */
	int		echo_color;	/* color for the echo */
	char	      * echo_time_snd;
	char	      * echo_weather_snd;
	
};

struct neighbor_data
{
	NEIGHBOR_DATA *next;
	NEIGHBOR_DATA *prev;
	char *name;
	AREA_DATA *address;
};

/*
 * Structure used to build wizlist
 */
struct	wizent
{
	WIZENT *		next;
	WIZENT *		last;
	char *		name;
	short		level;
};


/*
 * Connected state for a channel.
 */
typedef enum
{
	CON_GET_NAME = -99,       	CON_GET_OLD_PASSWORD,      	CON_CONFIRM_NEW_NAME,
	CON_GET_NEW_PASSWORD,	    	CON_CONFIRM_NEW_PASSWORD,  	CON_GET_NEW_SEX,
	CON_GET_NEW_CLASS,	    	CON_READ_MOTD,             	CON_GET_NEW_RACE,
	CON_GET_WANT_RIPANSI,     	CON_PRESS_ENTER, 	       	CON_COPYOVER_RECOVER,
	CON_PLAYING = 0, 	    	CON_EDITING, 	       		CON_NOTE_TEXT,
	CON_REDIT,			CON_OEDIT,			CON_MEDIT,
} connection_types;

/*
 * Character substates
 */
typedef enum
{
	SUB_NONE, SUB_PAUSE, SUB_PERSONAL_DESC, SUB_BAN_DESC, SUB_OBJ_SHORT,
	SUB_OBJ_LONG, SUB_OBJ_EXTRA, SUB_MOB_LONG, SUB_MOB_DESC, SUB_ROOM_DESC,
	SUB_ROOM_EXTRA, SUB_ROOM_EXIT_DESC, SUB_WRITING_NOTE, SUB_MPROG_EDIT,
	SUB_HELP_EDIT, SUB_WRITING_MAP, SUB_PERSONAL_BIO, SUB_REPEATCMD,
	SUB_RESTRICTED, SUB_DEITYDESC, SUB_PROJ_DESC, SUB_OVERLAND_DESC,
	/* timer types ONLY below this point */
	SUB_TIMER_DO_ABORT = 128, SUB_TIMER_CANT_ABORT
} char_substates;

/*
 * Descriptor (channel) structure.
 */
struct	descriptor_data
{
	DESCRIPTOR_DATA *	next;
	DESCRIPTOR_DATA *	prev;
	DESCRIPTOR_DATA *	snoop_by;
	CHAR_DATA *		character;
	CHAR_DATA *		original;
	char *		host;
	int			port;
	int			descriptor;
	short		connected;
	int		        idle;
	short		lines;
	short		scrlen;
	bool		fcommand;
	char		inbuf		[MAX_INBUF_SIZE];
	char		incomm		[MAX_INPUT_LENGTH];
	char		inlast		[MAX_INPUT_LENGTH];
	int			repeat;
	char *		outbuf;
	unsigned long	outsize;
	int			outtop;
	char *		pagebuf;
	unsigned long	pagesize;
	int			pagetop;
	char *		pagepoint;
	char		pagecmd;
	char		pagecolor;
	int			newstate;
	unsigned char	prevcolor;
	OLC_DATA *		olc;
};

/*
 * Attribute bonus structures.
 */
struct	str_app_type
{
	short	tohit;
	short	todam;
	short	carry;
	short	wield;
};

struct	int_app_type
{
	short	learn;
};

struct	wis_app_type
{
	short	practice;
};

struct	dex_app_type
{
	short	defensive;
};

struct	con_app_type
{
	short	hitp;
	short	shock;
};

struct	cha_app_type
{
	short	charm;
};

struct  lck_app_type
{
	short	luck;
};

/* the races */
typedef enum
{
	RACE_ANGEL, RACE_VANARA, RACE_NEZUMI, RACE_SPIRIT, RACE_UNDEAD, RACE_DEMON,
} race_types;


#define CLASS_NONE	   -1 /* For skill/spells according to guild */
#define CLASS_BUSHI	    1
#define CLASS_SHUGENJA	    2

/*
 * Languages -- Altrag
 */
#define LANG_COMMON     BV00  /* Human language     */
#define LANG_CLAN   	BV01  /* Insects */
#define LANG_UNKNOWN       0  /* Anything that doesnt fit a category */

#define VALID_LANGS    ( LANG_COMMON | LANG_CLAN )
/* 19 Languages */

/* Defining the bit vectors for tattoos */
#define TATTOO_BULL			BV00
#define TATTOO_PANTHER			BV01
#define TATTOO_WOLF			BV02
#define TATTOO_BEAR			BV03
#define TATTOO_RABBIT			BV04
#define TATTOO_DRAGON			BV05
#define TATTOO_MOON			BV06

/*
 * TO types for act.
 */
typedef enum { TO_ROOM, TO_NOTVICT, TO_VICT, TO_CHAR, TO_CANSEE } to_types;

#define INIT_WEAPON_CONDITION    12
#define MAX_ITEM_IMPACT		 30

/*
 * Help table types.
 */
struct	help_data
{
	HELP_DATA *	next;
	HELP_DATA * prev;
	short	level;
	char *	keyword;
	char *	text;
};



/*
 * Shop types.
 */
#define MAX_TRADE	 5

struct	shop_data
{
	SHOP_DATA *	next;			/* Next shop in list		*/
	SHOP_DATA * prev;			/* Previous shop in list	*/
	int		keeper;			/* Vnum of shop keeper mob	*/
	short	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
	short	profit_buy;		/* Cost multiplier for buying	*/
	short	profit_sell;		/* Cost multiplier for selling	*/
	short	open_hour;		/* First opening hour		*/
	short	close_hour;		/* First closing hour		*/
};

#define MAX_FIX		3
#define SHOP_FIX	1
#define SHOP_RECHARGE	2

struct	repairshop_data
{
	REPAIR_DATA * next;			/* Next shop in list		*/
	REPAIR_DATA * prev;			/* Previous shop in list	*/
	int		  keeper;		/* Vnum of shop keeper mob	*/
	short	  fix_type [MAX_FIX];	/* Item types shop will fix	*/
	short	  profit_fix;		/* Cost multiplier for fixing	*/
	short	  shop_type;		/* Repair shop type		*/
	short	  open_hour;		/* First opening hour		*/
	short	  close_hour;		/* First closing hour		*/
};

/* Mob program structures and defines */
/* Moved these defines here from mud_prog.c as I need them -rkb */
#define MAX_IFS 20 /* should always be generous */
#define IN_IF 0
#define IN_ELSE 1
#define DO_IF 2
#define DO_ELSE 3

#define MAX_PROG_NEST 20

struct  act_prog_data
{
	struct act_prog_data *next;
	void *vo;
};

struct	mob_prog_act_list
{
	MPROG_ACT_LIST * next;
	char *	     buf;
	CHAR_DATA *      ch;
	OBJ_DATA *	     obj;
	void *	     vo;
};

struct	mob_prog_data
{
	MPROG_DATA * next;
	short	 type;
	bool	 triggered;
	int		 resetdelay;
	char *	 arglist;
	char *	 comlist;
};

/* Used to store sleeping mud progs. -rkb */
typedef enum {MP_MOB, MP_ROOM, MP_OBJ} mp_types;
struct mpsleep_data
{
	MPSLEEP_DATA * next;
	MPSLEEP_DATA * prev;

	int timer; /* Pulses to sleep */
	mp_types type; /* Mob, Room or Obj prog */
	ROOM_INDEX_DATA*room; /* Room when type is MP_ROOM */

	/* mprog_driver state variables */
	int ignorelevel;
	int iflevel;
	bool ifstate[MAX_IFS][DO_ELSE];

	/* mprog_driver arguments */
	char * com_list;
	CHAR_DATA * mob;
	CHAR_DATA * actor;
	OBJ_DATA * obj;
	void * vo;
	bool single_step;
};



bool	MOBtrigger;

/*
 * Per-Class stuff.
 */
struct	Class_type
{
	char *		who_name;		/* Name for 'who'		*/
	short		skill_adept;		/* Maximum skill level		*/
	short		thac0_00;		/* Thac0 for level  0		*/
	short		thac0_32;		/* Thac0 for level 32		*/
	short		hp_min;			/* Min hp gained on leveling	*/
	short		hp_max;			/* Max hp gained on leveling	*/
	short		mana_min;			/* Min hp gained on leveling	*/
	short		mana_max;			/* Max hp gained on leveling	*/
	short		move_min;			/* Min hp gained on leveling	*/
	short		move_max;			/* Max hp gained on leveling	*/
	short		exp_base;		/* Class base exp		*/
};

/* race dedicated stuff */
struct	race_type
{
	char 		race_name	[16];	/* Race name			*/
	EXT_BV		affected;		/* Default affect bitvectors	*/
	short		str_plus;		/* Str bonus/penalty		*/
	short		dex_plus;		/* Dex      "			*/
	short		wis_plus;		/* Wis      "			*/
	short		int_plus;		/* Int      "			*/
	short		con_plus;		/* Con      "			*/
	short		cha_plus;		/* Cha      "			*/
	short		lck_plus;		/* Lck 	    "			*/
	short      	hit;
	short      	mana;
	int      	resist;
	int      	suscept;
	int		Class_restriction;	/* Flags for illegal Classes	*/
	int         	language;               /* Default racial language      */
	short      	ac_plus;
	short      	alignment;
	EXT_BV      	attacks;
	EXT_BV      	defenses;
	short      	minalign;
	short      	maxalign;
	short      	exp_multiplier;
	short      	height;
	short      	weight;
	short      	hunger_mod;
	short      	thirst_mod;
	short		saving_poison_death;
	short		saving_mental;
	short		saving_physical;
	short		saving_weapons;
	char *		where_name[MAX_WHERE_NAME];
	short      	mana_regen;
	short      	hp_regen;
	short      	race_recall;
};

typedef enum
{
	CLAN_DEATH, CLAN_LIFE, CLAN_GROWTH, CLAN_DECAY
} clan_types;


struct	clan_data
{
	CLAN_DATA * next;		/* next clan in list			*/
	CLAN_DATA * prev;		/* previous clan in list		*/
	char *	filename;	/* Clan filename			*/
	char *	name;		/* Clan name				*/
	char *	motto;		/* Clan motto				*/
	char *	description;	/* A brief description of the clan	*/
	char *	deity;		/* Clan's deity				*/
	char *	leader;		/* Head clan leader			*/
	char *	number1;	/* First officer			*/
	char *	number2;	/* Second officer			*/
	char *	badge;		/* Clan badge on who/where/to_room      */
	char *      leadrank;	/* Leader's rank			*/
	char *      onerank;	/* Number One's rank			*/
	char *	tworank;	/* Number Two's rank			*/
	int		pkills[7];	/* Number of pkills on behalf of clan	*/
	int		pdeaths[7];	/* Number of pkills against clan	*/
	int		mkills;		/* Number of mkills on behalf of clan	*/
	int		mdeaths;	/* Number of clan deaths due to mobs	*/
	int		illegal_pk;	/* Number of illegal pk's by clan	*/
	int		score;		/* Overall score			*/
	short	clan_type;	/* See clan type defines		*/
	short	favour;		/* Deities favour upon the clan		*/
	short	strikes;	/* Number of strikes against the clan	*/
	short	members;	/* Number of clan members		*/
	short	mem_limit;	/* Number of clan members allowed	*/
	short	alignment;	/* Clan's general alignment		*/
	int		board;		/* Vnum of clan board			*/
	int		clanobj1;	/* Vnum of first clan obj		*/
	int		clanobj2;	/* Vnum of second clan obj		*/
	int		clanobj3;	/* Vnum of third clan obj		*/
	int		clanobj4;	/* Vnum of fourth clan obj		*/
	int		clanobj5;	/* Vnum of fifth clan obj		*/
	int		recall;		/* Vnum of clan's recall room		*/
	int		storeroom;	/* Vnum of clan's store room		*/
	int		guard1;		/* Vnum of clan guard type 1		*/
	int		guard2;		/* Vnum of clan guard type 2		*/
	int         Class;		/* For guilds				*/
};

struct	deity_data
{
	DEITY_DATA * next;
	DEITY_DATA * prev;
	char *	filename;
	char *	name;
	char *	description;
	short	alignment;
	short	worshippers;
	short	scorpse;
	short	sdeityobj;
	short	savatar;
	short	srecall;
	short	flee;
	short	flee_npcrace;
	short	flee_npcfoe;
	short	kill;
	short	kill_magic;
	short	kill_npcrace;
	short	kill_npcfoe;
	short	sac;
	short	bury_corpse;
	short	aid_spell;
	short	aid;
	short	backstab;
	short	steal;
	short	die;
	short	die_npcrace;
	short	die_npcfoe;
	short	spell_aid;
	short	dig_corpse;
	int		race;
	int		race2;
	int		Class;
	int		sex;
	int		npcrace;
	int		npcfoe;
	int		suscept;
	int		element;
	EXT_BV	affected;
	int		susceptnum;
	int		elementnum;
	int		affectednum;
	int	        objstat;
};


struct tourney_data
{
	int    open;
	int    low_level;
	int    hi_level;
};



/*
 * An affect.
 *
 * So limited... so few fields... should we add more?
 */
struct	affect_data
{
	AFFECT_DATA *	next;
	AFFECT_DATA *	prev;
	short		type;
	int		    duration;
	short		location;
	int			modifier;
	EXT_BV		bitvector;
};


/*
 * A SMAUG spell
 */
struct	smaug_affect
{
	SMAUG_AFF *		next;
	char *		duration;
	short		location;
	char *		modifier;
	int			bitvector;	/* this is the bit number */
};


/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (Start of section ... start here)                     *
 *                                                                         *
 ***************************************************************************/

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_CITYGUARD	   3060
#define MOB_VNUM_VAMPIRE	   80
#define MOB_VNUM_ANIMATED_CORPSE   5
#define MOB_VNUM_DEITY		   17

/*
 * ACT bits for mobs.
 * Used in #MOBILES.
 */
#define ACT_IS_NPC		  0		/* Auto set for mobs	*/
#define ACT_SENTINEL		  1		/* Stays in one room	*/
#define ACT_SCAVENGER		  2		/* Picks up objects	*/
#define ACT_ONMAP		  3
#define ACT_IS_TATTOOARTIST       4
#define ACT_AGGRESSIVE		  5		/* Attacks PC's		*/
#define ACT_STAY_AREA		  6		/* Won't leave area	*/
#define ACT_WIMPY		  7		/* Flees when hurt	*/
#define ACT_PET			  8		/* Auto set for pets	*/
#define ACT_NOTHING		  9		/* Can train PC's	*/
#define ACT_PRACTICE		 10		/* Can practice PC's	*/
#define ACT_IMMORTAL		 11		/* Cannot be killed	*/
#define ACT_DEADLY		 12		/* Has a deadly poison  */
#define ACT_POLYSELF		 13
#define ACT_META_AGGR		 14		/* Attacks other mobs	*/
#define ACT_GUARDIAN		 15		/* Protects master	*/
#define ACT_RUNNING		 16		/* Hunts quickly	*/
#define ACT_NOWANDER		 17		/* Doesn't wander	*/
#define ACT_MOUNTABLE		 18		/* Can be mounted	*/
#define ACT_MOUNTED		 19		/* Is mounted		*/
#define ACT_SCHOLAR              20		/* Can teach languages  */
#define ACT_SECRETIVE		 21		/* actions aren't seen	*/
#define ACT_HARDHAT	         22		/* Immune to falling item damage */
#define ACT_MOBINVIS		 23		/* Like wizinvis	*/
#define ACT_NOASSIST		 24		/* Doesn't assist mobs	*/
#define ACT_AUTONOMOUS		 25		/* Doesn't auto switch tanks */
#define ACT_PACIFIST             26		/* Doesn't ever fight   */
#define ACT_NOATTACK		 27		/* No physical attacks */
#define ACT_ANNOYING		 28		/* Other mobs will attack */
#define ACT_STATSHIELD		 29		/* prevent statting */
#define ACT_PROTOTYPE		 30		/* A prototype mob	*/
#define ACT_BANKER		 31		/* act flag for banking code*/
#define ACT_PKTOGGLE		 32		/* allows PC's to turn off Pk for a price*/
#define ACT_QUESTMASTER		 33		/* Questmaster */
#define ACT_NOQUEST		 34		/* Mob cannot be quested to*/
#define ACT_TRAVELMAGE		 35             /* Travelmage mob will send you places, for a price */
#define ACT_HEALER		 36             /* Healer mob to fix you up */
#define ACT_FORGE	         37
#define ACT_RESTRING		 38
#define ACT_TRAINER		 39
#define ACT_NULL		 40
#define MAX_ACT			 40
/* 32 acts */
/*
 * Bits for 'affected_by'.
 * Used in #MOBILES.
 *
 * hold and flaming are yet uncoded
 */
typedef enum
{
	AFF_BLIND, 		AFF_INVISIBLE, 		AFF_DETECT_EVIL, 	AFF_DETECT_INVIS,	AFF_DETECT_MAGIC, 
	AFF_DETECT_HIDDEN, 	AFF_HOLD, 		AFF_SANCTUARY,		AFF_FAERIE_FIRE, 	AFF_INFRARED, 
	AFF_CURSE, 		AFF_FLAMING, 		AFF_POISON,		AFF_PROTECT, 		AFF_PARALYSIS, 
	AFF_SNEAK, 		AFF_HIDE, 		AFF_SLEEP, 		AFF_CHARM,		AFF_FLYING, 
	AFF_PASS_DOOR, 		AFF_FLOATING, 		AFF_TRUESIGHT, 		AFF_DETECTTRAPS,	AFF_SCRYING, 
	AFF_FIRESHIELD, 	AFF_SHOCKSHIELD, 	AFF_HAUS1, 		AFF_ICESHIELD,		AFF_POSSESS, 
	AFF_BERSERK, 		AFF_AQUA_BREATH, 	AFF_RECURRINGSPELL,	AFF_CONTAGIOUS, 	AFF_ACIDMIST,  
	AFF_VENOMSHIELD, 	MAX_AFFECTED_BY
} affected_by_types;

/*
 * Resistant Immune Susceptible flags
 */
#define RIS_FIRE		  BV00
#define RIS_COLD		  BV01
#define RIS_ELECTRICITY		  BV02
#define RIS_ENERGY		  BV03
#define RIS_BLUNT		  BV04
#define RIS_PIERCE		  BV05
#define RIS_SLASH		  BV06
#define RIS_ACID		  BV07
#define RIS_POISON		  BV08
#define RIS_DRAIN		  BV09
#define RIS_SLEEP		  BV10
#define RIS_CHARM		  BV11
#define RIS_HOLD		  BV12
#define RIS_NONMAGIC		  BV13
#define RIS_PLUS1		  BV14
#define RIS_PLUS2		  BV15
#define RIS_PLUS3		  BV16
#define RIS_PLUS4		  BV17
#define RIS_PLUS5		  BV18
#define RIS_PLUS6		  BV19
#define RIS_MAGIC		  BV20
#define RIS_PARALYSIS		  BV21
#define RIS_LASH		  BV22
#define RIS_HACK		  BV23
#define MAX_RIS_FLAG		  BV24
/* 21 RIS's*/



/* Defense types */
typedef enum
{
	DFND_PARRY, DFND_DODGE, DFND_TUMBLE, DFND_GRIP,
	MAX_DEFENSE_TYPE
} defense_types;

/*
 * Body parts
 */
#define PART_HEAD		  BV00
#define PART_HEART		  BV01
#define PART_ARMS		  BV02
#define PART_LEGS		  BV03
#define PART_GUTS		  BV04
#define PART_BRAINS		  BV05
#define PART_HANDS		  BV06
#define PART_FEET		  BV07
#define PART_FINGERS		  BV08
#define PART_EAR		  BV09
#define PART_EYE		  BV10
#define PART_LONG_TONGUE	  BV11
#define PART_EYESTALKS		  BV12
#define PART_TENTACLES		  BV13
#define PART_FINS		  BV14
#define PART_WINGS		  BV15
#define PART_TAIL		  BV16
#define PART_SCALES		  BV17
#define PART_TUSKS		  BV18
#define PART_HORNS		  BV19
#define PART_CLAWS		  BV20
#define PART_FEATHERS		  BV21
#define PART_FORELEGS		  BV22
#define PART_PAWS		  BV23
#define PART_HOOVES		  BV24
#define PART_BEAK		  BV25
#define PART_SHARPSCALES	  BV26
#define PART_HAUNCHES		  BV27
#define PART_FANGS		  BV28


/*
 * Autosave flags
 */
#define SV_DEATH		  BV00 /* Save on death */
#define SV_KILL			  BV01 /* Save when kill made */
#define SV_PASSCHG		  BV02 /* Save on password change */
#define SV_DROP			  BV03 /* Save on drop */
#define SV_PUT			  BV04 /* Save on put */
#define SV_GIVE			  BV05 /* Save on give */
#define SV_AUTO			  BV06 /* Auto save every x minutes (define in cset) */
#define SV_ZAPDROP		  BV07 /* Save when eq zaps */
#define SV_AUCTION		  BV08 /* Save on auction */
#define SV_GET			  BV09 /* Save on get */
#define SV_RECEIVE		  BV10 /* Save when receiving */
#define SV_IDLE			  BV11 /* Save when char goes idle */
#define SV_BACKUP		  BV12 /* Make backup of pfile on save */
#define SV_QUITBACKUP		  BV13 /* Backup on quit only --Blod */
#define SV_FILL			  BV14 /* Save on do_fill */
#define SV_EMPTY		  BV15 /* Save on do_empty */

/*
 * Pipe flags
 */
#define PIPE_TAMPED		  BV01
#define PIPE_LIT		  BV02
#define PIPE_HOT		  BV03
#define PIPE_DIRTY		  BV04
#define PIPE_FILTHY		  BV05
#define PIPE_GOINGOUT		  BV06
#define PIPE_BURNT		  BV07
#define PIPE_FULLOFASH		  BV08

/*
 * Flags for act_string -- Shaddai
 */
#define STRING_NONE               0
#define STRING_IMM                BV01

/*
 * Skill/Spell flags	The minimum BV *MUST* be 11!
 */
#define SF_WATER		  BV00
#define SF_EARTH		  BV01
#define SF_AIR			  BV02
#define SF_ASTRAL		  BV03
#define SF_AREA			  BV04  /* is an area spell		*/
#define SF_DISTANT		  BV05  /* affects something far away	*/
#define SF_REVERSE		  BV06
#define SF_NOSELF		  BV07  /* Can't target yourself!	*/
#define SF_UNUSED2		  BV08  /* free for use!		*/
#define SF_ACCUMULATIVE		  BV09  /* is accumulative		*/
#define SF_RECASTABLE		  BV10  /* can be refreshed		*/
#define SF_NOSCRIBE		  BV11  /* cannot be scribed		*/
#define SF_NOBREW		  BV12  /* cannot be brewed		*/
#define SF_GROUPSPELL		  BV13  /* only affects group members	*/
#define SF_OBJECT		  BV14	/* directed at an object	*/
#define SF_CHARACTER		  BV15  /* directed at a character	*/
#define SF_SECRETSKILL		  BV16	/* hidden unless learned	*/
#define SF_PKSENSITIVE		  BV17	/* much harder for plr vs. plr	*/
#define SF_STOPONFAIL		  BV18	/* stops spell on first failure */
#define SF_NOFIGHT		  BV19  /* stops if char fighting       */
#define SF_NODISPEL               BV20  /* stops spell from being dispelled */
#define SF_RANDOMTARGET		  BV21	/* chooses a random target	*/
typedef enum { SS_NONE, SS_POISON_DEATH, SS_ROD_WANDS, SS_PARA_PETRI, SS_BREATH, SS_SPELL_STAFF
             } save_types;

#define ALL_BITS		INT_MAX
#define SDAM_MASK		ALL_BITS & ~(BV00 | BV01 | BV02)
#define SACT_MASK		ALL_BITS & ~(BV03 | BV04 | BV05)
#define SCLA_MASK		ALL_BITS & ~(BV06 | BV07 | BV08)
#define SPOW_MASK		ALL_BITS & ~(BV09 | BV10)
#define SSAV_MASK		ALL_BITS & ~(BV11 | BV12 | BV13)

typedef enum { SD_NONE, SD_FIRE, SD_COLD, SD_ELECTRICITY, SD_ENERGY, SD_ACID,
               SD_POISON, SD_DRAIN
             } spell_dam_types;

typedef enum { SA_NONE, SA_CREATE, SA_DESTROY, SA_RESIST, SA_SUSCEPT,
               SA_DIVINATE, SA_OBSCURE, SA_CHANGE
             } spell_act_types;

typedef enum { SP_NONE, SP_MINOR, SP_GREATER, SP_MAJOR } spell_power_types;

typedef enum { SC_NONE, SC_LUNAR, SC_SOLAR, SC_TRAVEL, SC_SUMMON,
               SC_LIFE, SC_DEATH, SC_ILLUSION
             } spell_Class_types;

typedef enum { SE_NONE, SE_NEGATE, SE_EIGHTHDAM, SE_QUARTERDAM, SE_HALFDAM,
               SE_3QTRDAM, SE_REFLECT, SE_ABSORB
             } spell_save_effects;

/*
 * Sex.
 * Used in #MOBILES.
 */
typedef enum { SEX_NEUTRAL, SEX_MALE, SEX_FEMALE } sex_types;

#define MAX_TRAPTYPE		   TRAP_TYPE_SEX_CHANGE

#define TRAP_ROOM      		   BV00
#define TRAP_OBJ	      	   BV01
#define TRAP_ENTER_ROOM		   BV02
#define TRAP_LEAVE_ROOM		   BV03
#define TRAP_OPEN		   BV04
#define TRAP_CLOSE		   BV05
#define TRAP_GET		   BV06
#define TRAP_PUT		   BV07
#define TRAP_PICK		   BV08
#define TRAP_UNLOCK		   BV09
#define TRAP_N			   BV10
#define TRAP_S			   BV11
#define TRAP_E	      		   BV12
#define TRAP_W	      		   BV13
#define TRAP_U	      		   BV14
#define TRAP_D	      		   BV15
#define TRAP_EXAMINE		   BV16
#define TRAP_NE			   BV17
#define TRAP_NW			   BV18
#define TRAP_SE			   BV19
#define TRAP_SW			   BV20

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_MONEY_ONE	      1
#define OBJ_VNUM_MONEY_SOME	      2
#define OBJ_VNUM_CORPSE_NPC	      3
#define OBJ_VNUM_CORPSE_PC	      4
#define OBJ_VNUM_BLOODSTAIN	      5
#define OBJ_VNUM_SCRAPS		      6
#define OBJ_VNUM_SKIN		      7
#define OBJ_VNUM_SHOPPING_BAG	      8
#define OBJ_VNUM_SCROLL_SCRIBING      9
#define OBJ_VNUM_FLASK_BREWING       10
#define OBJ_VNUM_TRAP		     11
#define OBJ_VNUM_FIRE		     12
#define OBJ_VNUM_DEITY		     13
#define OBJ_VNUM_CLOTHING 	     15

/*
 * Item types.
 * Used in #OBJECTS.
 */
typedef enum
{
	ITEM_NONE, 		ITEM_LIGHT, 		ITEM_SCROLL, 		ITEM_WAND, 		ITEM_STAFF,
	ITEM_WEAPON,		ITEM_FIREWEAPON, 	ITEM_MISSILE, 		ITEM_TREASURE, 		ITEM_ARMOR,
	ITEM_POTION,		ITEM_WORN, 		ITEM_FURNITURE, 	ITEM_TRASH, 		ITEM_OLDTRAP,
	ITEM_CONTAINER,	ITEM_NOTE, 		ITEM_DRINK_CON, 	ITEM_KEY, 		ITEM_FOOD,
	ITEM_MONEY, 		ITEM_PEN,		ITEM_BOAT, 		ITEM_CORPSE_NPC, 	ITEM_CORPSE_PC,
	ITEM_FOUNTAIN, 	ITEM_PILL,		ITEM_BLOOD, 		ITEM_BLOODSTAIN, 	ITEM_SCRAPS,
	ITEM_PIPE, 		ITEM_HERB_CON,		ITEM_HERB, 		ITEM_INCENSE, 		ITEM_FIRE,
	ITEM_BOOK, 		ITEM_SWITCH, 		ITEM_LEVER,		ITEM_PULLCHAIN, 	ITEM_BUTTON,
	ITEM_DIAL, 		ITEM_RUNE, 		ITEM_RUNEPOUCH,		ITEM_MATCH, 		ITEM_TRAP,
	ITEM_MAP, 		ITEM_PORTAL, 		ITEM_PAPER,		ITEM_TINDER, 		ITEM_LOCKPICK,
	ITEM_SPIKE, 		ITEM_DISEASE, 		ITEM_OIL, 		ITEM_FUEL,		ITEM_EMPTY1,
	ITEM_EMPTY2, 		ITEM_MISSILE_WEAPON, 	ITEM_PROJECTILE, 	ITEM_QUIVER,		ITEM_SHOVEL,
	ITEM_SALVE, 		ITEM_COOK, 		ITEM_KEYRING, 		ITEM_ODOR,  		ITEM_DYE,
	ITEM_SEWKIT, 		ITEM_HANDMADE, 		ITEM_ORE, 		ITEM_DRINK_MIX, 	ITEM_QUESTTOKEN,
	ITEM_FISHING_POLE, 	ITEM_FISHING_BAIT, 	ITEM_CHANCE
} item_types;

#define MAX_ITEM_TYPE		     ITEM_CHANCE

/*
 * Extra flags.
 * Used in #OBJECTS.
 */
typedef enum
{
	ITEM_GLOW, 		ITEM_HUM, 		ITEM_DARK, 		ITEM_LOYAL, 		ITEM_EVIL,
	ITEM_INVIS, 		ITEM_MAGIC,  		ITEM_NODROP, 		ITEM_BLESS, 		ITEM_ANTI_GOOD,
	ITEM_ANTI_EVIL, 	ITEM_ANTI_NEUTRAL,   	ITEM_NOREMOVE, 		ITEM_INVENTORY, 	ITEM_ORGANIC,
	ITEM_METAL,           	ITEM_DONATION,	        ITEM_CLANOBJECT, 	ITEM_CLANCORPSE, 	ITEM_HIDDEN,
	ITEM_POISONED, 		ITEM_COVERING, 		ITEM_DEATHROT, 		ITEM_BURIED,            ITEM_PROTOTYPE,
	ITEM_NOLOCATE, 		ITEM_GROUNDROT, 	ITEM_LOOTABLE, 		ITEM_ONMAP,             ITEM_QUEST,
	ITEM_LODGED,		ITEM_REFINED,		ITEM_UNIQUE,		ITEM_ELITE,
	MAX_ITEM_FLAG
} item_extra_flags;

/* Magic flags - extra extra_flags for objects that are used in spells */
#define ITEM_RETURNING		BV00
#define ITEM_BACKSTABBER  	BV01
#define ITEM_BANE		BV02
#define ITEM_MAGIC_LOYAL	BV03
#define ITEM_HASTE		BV04
#define ITEM_DRAIN		BV05
#define ITEM_LIGHTNING_BLADE  	BV06
#define ITEM_PKDISARMED		BV07 /* Maybe temporary, not a perma flag */

/* Lever/dial/switch/button/pullchain flags */
#define TRIG_UP			BV00
#define TRIG_UNLOCK		BV01
#define TRIG_LOCK		BV02
#define TRIG_D_NORTH		BV03
#define TRIG_D_SOUTH		BV04
#define TRIG_D_EAST		BV05
#define TRIG_D_WEST		BV06
#define TRIG_D_UP		BV07
#define TRIG_D_DOWN		BV08
#define TRIG_DOOR		BV09
#define TRIG_CONTAINER		BV10
#define TRIG_OPEN		BV11
#define TRIG_CLOSE		BV12
#define TRIG_PASSAGE		BV13
#define TRIG_OLOAD		BV14
#define TRIG_MLOAD		BV15
#define TRIG_TELEPORT		BV16
#define TRIG_TELEPORTALL	BV17
#define TRIG_TELEPORTPLUS	BV18
#define TRIG_DEATH		BV19
#define TRIG_CAST		BV20
#define TRIG_FAKEBLADE		BV21
#define TRIG_RAND4		BV22
#define TRIG_RAND6		BV23
#define TRIG_TRAPDOOR		BV24
#define TRIG_ANOTHEROOM		BV25
#define TRIG_USEDIAL		BV26
#define TRIG_ABSOLUTEVNUM	BV27
#define TRIG_SHOWROOMDESC	BV28
#define TRIG_AUTORETURN		BV29

#define TELE_SHOWDESC		BV00
#define TELE_TRANSALL		BV01
#define TELE_TRANSALLPLUS	BV02


/*
 * Wear flags.
 * Used in #OBJECTS.
 */
#define ITEM_TAKE		BV00
#define ITEM_WEAR_HEAD		BV01
#define ITEM_WEAR_BODY		BV02
#define ITEM_WEAR_BACK		BV03
#define ITEM_WEAR_ARMS		BV04
#define ITEM_WEAR_HANDS		BV05
#define ITEM_WIELD		BV06
#define ITEM_WEAR_SHIELD	BV07
#define ITEM_HOLD		BV08
#define ITEM_WEAR_LEGS		BV09
#define ITEM_WEAR_FEET		BV10
#define ITEM_LODGE_RIB		BV11
#define ITEM_LODGE_ARM		BV12
#define ITEM_LODGE_LEG		BV13
#define ITEM_MISSILE_WIELD	BV14
#define ITEM_DUAL_WIELD		BV15
#define ITEM_WEAR_MAX		14

/*
 * Apply types (for affects).
 * Used in #OBJECTS.
 */
typedef enum
{
	APPLY_NONE, 		APPLY_STR, 		APPLY_DEX, 		APPLY_INT, 		APPLY_WIS,
	APPLY_CON,		APPLY_SEX, 		APPLY_CLASS, 		APPLY_LEVEL, 		APPLY_AGE,
	APPLY_HEIGHT, 		APPLY_WEIGHT,		APPLY_MANA, 		APPLY_HIT, 		APPLY_MOVE,
	APPLY_GOLD, 		APPLY_EXP, 		APPLY_AC,		APPLY_HITROLL, 		APPLY_DAMROLL,
	APPLY_SAVING_POISON, 	APPLY_SAVING_MENTAL,	APPLY_SAVING_PHYSICAL, 	APPLY_SAVING_WEAPONS, 	APPLY_NOT_USED,
	APPLY_CHA,		APPLY_AFFECT, 		APPLY_RESISTANT, 	APPLY_IMMUNE, 		APPLY_SUSCEPTIBLE,
	APPLY_WEAPONSPELL, 	APPLY_LCK, 		APPLY_BACKSTAB, 	APPLY_PICK, 		APPLY_TRACK,
	APPLY_STEAL, 		APPLY_SNEAK, 		APPLY_HIDE, 		APPLY_PALM, 		APPLY_DETRAP,
	APPLY_DODGE,		APPLY_PEEK, 		APPLY_SCAN, 		APPLY_GOUGE, 		APPLY_SEARCH,
	APPLY_MOUNT, 		APPLY_DISARM,		APPLY_KICK, 		APPLY_PARRY, 		APPLY_BASH,
	APPLY_STUN, 		APPLY_PUNCH, 		APPLY_CLIMB,		APPLY_GRIP, 		APPLY_SCRIBE,
	APPLY_BREW, 		APPLY_WEARSPELL, 	APPLY_REMOVESPELL,	APPLY_STRIPSN,		APPLY_REMOVE,
	APPLY_DIG,		APPLY_FULL, 		APPLY_THIRST, 		APPLY_DRUNK, 		APPLY_BLOOD,
	APPLY_COOK,		APPLY_RECURRINGSPELL, 	APPLY_CONTAGIOUS,	APPLY_EXT_AFFECT, 	APPLY_ODOR,
	APPLY_ROOMFLAG, 	APPLY_SECTORTYPE, 	APPLY_ROOMLIGHT,        APPLY_TELEVNUM, 	APPLY_TELEDELAY, 	MAX_APPLY_TYPE
} apply_types;

#define REVERSE_APPLY		   1000

/*
 * Values for containers (value[1]).
 * Used in #OBJECTS.
 */
#define CONT_CLOSEABLE		   BV00
#define CONT_PICKPROOF		   BV01
#define CONT_CLOSED		   BV02
#define CONT_LOCKED		   BV03
#define CONT_EATKEY		   BV04


/*
 * Sitting/Standing/Sleeping/Sitting on/in/at Objects - Xerves
 * Used for furniture (value[2]) in the #OBJECTS Section
 */
#define SIT_ON     BV00
#define SIT_IN     BV01
#define SIT_AT     BV02

#define STAND_ON   BV03
#define STAND_IN   BV04
#define STAND_AT   BV05

#define SLEEP_ON   BV06
#define SLEEP_IN   BV07
#define SLEEP_AT   BV08

#define REST_ON     BV09
#define REST_IN     BV10
#define REST_AT     BV11


/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */
#define ROOM_VNUM_LIMBO		      2
#define ROOM_VNUM_CHAT		   1200
#define ROOM_VNUM_TEMPLE	  10040
#define ROOM_VNUM_ALTAR		  10040
#define ROOM_VNUM_RECALL	  10040
#define ROOM_VNUM_SCHOOL	   4022
#define ROOM_VNUM_MORGUE  	  10040
#define ROOM_AUTH_START		    100
#define ROOM_VNUM_HALLOFFALLEN    10040
#define ROOM_VNUM_DEADLY          10040
#define ROOM_VNUM_HELL		      6
#define ROOM_VNUM_WEAPON 	  10084
#define ROOM_VNUM_ARMOR 	  10083
#define ROOM_VNUM_OTHER 	  10090

/*
 * Sector types.
 * Used in #ROOMS.
 */
typedef enum
{
	SECT_INSIDE, SECT_CITY, SECT_FIELD, SECT_FOREST, SECT_HILLS, SECT_MOUNTAIN,
	SECT_WATER_SWIM, SECT_WATER_NOSWIM,  SECT_UNDERWATER, SECT_AIR, SECT_DESERT,
	SECT_RIVER, SECT_OCEANFLOOR, SECT_UNDERGROUND, SECT_JUNGLE, SECT_SWAMP,
	SECT_TUNDRA, SECT_ICE, SECT_OCEAN, SECT_LAVA, SECT_SHORE, SECT_TREE, SECT_STONE,
	SECT_QUICKSAND, SECT_WALL, SECT_GLACIER, SECT_EXIT, SECT_TRAIL, SECT_BLANDS,
	SECT_GRASSLAND, SECT_SCRUB, SECT_BARREN, SECT_BRIDGE, SECT_ROAD, SECT_DUNNO,
	SECT_MAX
} sector_types;


/*
 * Room flags now use ext_bit_vectors Tommi 2005
 */

typedef enum
{
	ROOM_DARK, ROOM_DEATH, ROOM_NO_MOB, ROOM_INDOORS, ROOM_LAWFUL, ROOM_NEUTRAL, ROOM_CHAOTIC,
	ROOM_NO_MAGIC, ROOM_TUNNEL, ROOM_PRIVATE, ROOM_SAFE, ROOM_SOLITARY, ROOM_PET_SHOP, ROOM_NO_RECALL,
	ROOM_DONATION, ROOM_NODROPALL, ROOM_SILENCE, ROOM_LOGSPEECH, ROOM_NODROP, ROOM_CLANSTOREROOM,
	ROOM_NO_SUMMON, ROOM_NO_ASTRAL, ROOM_TELEPORT, ROOM_TELESHOWDESC, ROOM_NOFLOOR, ROOM_NOSUPPLICATE,
	ROOM_ARENA, ROOM_NOMISSILE, ROOM_MAP, ROOM_NOBUY, ROOM_PROTOTYPE, ROOM_DND, ROOM_LOCKER, ROOM_NOMAP,
        ROOM_MAX
} room_types;

/*
 * Directions.
 * Used in #ROOMS.
 */
typedef enum
{
	DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN,
	DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, DIR_SOMEWHERE
} dir_types;

#define PT_WATER	100
#define PT_AIR		200
#define PT_EARTH	300
#define PT_FIRE		400

/*
 * Push/pull types for exits					-Thoric
 * To differentiate between the current of a river, or a strong gust of wind
 */
typedef enum
{
	PULL_UNDEFINED, PULL_VORTEX, PULL_VACUUM, PULL_SLIP, PULL_ICE, PULL_MYSTERIOUS,
	PULL_CURRENT	 = PT_WATER, PULL_WAVE,	    PULL_WHIRLPOOL, PULL_GEYSER,
	PULL_WIND	 = PT_AIR,   PULL_STORM,    PULL_COLDWIND,  PULL_BREEZE,
	PULL_LANDSLIDE = PT_EARTH, PULL_SINKHOLE, PULL_QUICKSAND, PULL_EARTHQUAKE,
	PULL_LAVA 	 = PT_FIRE,  PULL_HOTAIR
} dir_pulltypes;


#define MAX_DIR			DIR_SOUTHWEST	/* max for normal walking */
#define DIR_PORTAL		DIR_SOMEWHERE	/* portal direction	  */


/*
 * Exit flags.			EX_RES# are reserved for use by the
 * Used in #ROOMS.		SMAUG development team
 */
#define EX_ISDOOR		  BV00
#define EX_CLOSED		  BV01
#define EX_LOCKED		  BV02
#define EX_SECRET		  BV03
#define EX_SWIM			  BV04
#define EX_PICKPROOF		  BV05
#define EX_FLY			  BV06
#define EX_CLIMB		  BV07
#define EX_DIG			  BV08
#define EX_EATKEY		  BV09
#define EX_NOPASSDOOR		  BV10
#define EX_HIDDEN		  BV11
#define EX_PASSAGE		  BV12
#define EX_PORTAL 		  BV13
#define EX_RES1			  BV14
#define EX_RES2			  BV15
#define EX_xCLIMB		  BV16
#define EX_xENTER		  BV17
#define EX_xLEAVE		  BV18
#define EX_xAUTO		  BV19
#define EX_NOFLEE	  	  BV20
#define EX_xSEARCHABLE		  BV21
#define EX_BASHED                 BV22
#define EX_BASHPROOF              BV23
#define EX_NOMOB		  BV24
#define EX_WINDOW		  BV25
#define EX_xLOOK		  BV26
#define EX_ISBOLT		  BV27
#define EX_BOLTED		  BV28
#define EX_OVERLAND		  BV29
#define MAX_EXFLAG		  29


/*
 * Equpiment wear locations.
 * Used in #RESETS.
 */
typedef enum
{
	WEAR_NONE = -1, WEAR_LIGHT = 0,
	WEAR_HEAD, WEAR_BODY, WEAR_BACK, WEAR_ARMS, WEAR_HANDS,
	WEAR_WIELD, WEAR_SHIELD, WEAR_HOLD, WEAR_LEGS, WEAR_FEET,
	WEAR_LODGE_RIB, WEAR_LODGE_ARM, WEAR_LODGE_LEG, MAX_WEAR,
	WEAR_DUAL_WIELD, WEAR_MISSILE_WIELD


} wear_locations;


/* Auth Flags */
#define FLAG_WRAUTH		      1
#define FLAG_AUTH		      2

/***************************************************************************
 *                                                                         *
 *                   VALUES OF INTEREST TO AREA BUILDERS                   *
 *                   (End of this section ... stop here)                   *
 *                                                                         *
 ***************************************************************************/


/*
 * Positions.
 */
typedef enum
{
	POS_DEAD, POS_MORTAL, POS_INCAP, POS_STUNNED, POS_SLEEPING, POS_BERSERK,
	POS_RESTING, POS_AGGRESSIVE, POS_SITTING, POS_FIGHTING, POS_DEFENSIVE,
	POS_EVASIVE, POS_STANDING, POS_MOUNTED, POS_SHOVE, POS_DRAG
} positions;

/*
 * Styles.
 */
typedef enum
{
	STYLE_FIGHTING, STYLE_DRAGON,  STYLE_NAGA, STYLE_SCORPION, STYLE_TIGER, 
	STYLE_UNICORN,  STYLE_PHOENIX, STYLE_CRANE, STYLE_BADGER
} styles;

/*
 * ACT bits for players.
 */
typedef enum
{
	PLR_IS_NPC,PLR_BOUGHT_PET, PLR_SHOVEDRAG, PLR_AUTOEXIT, PLR_AUTOLOOT,
	PLR_AUTOSAC, PLR_BLANK, PLR_OUTCAST, PLR_BRIEF, PLR_COMBINE, PLR_PROMPT,
	PLR_TELNET_GA, PLR_HOLYLIGHT, PLR_WIZINVIS, PLR_ROOMVNUM, PLR_SILENCE,
	PLR_NO_EMOTE, PLR_ATTACKER, PLR_NO_TELL, PLR_LOG, PLR_DENY, PLR_FREEZE,
	PLR_THIEF, PLR_KILLER, PLR_LITTERBUG, PLR_ANSI, PLR_RIP, PLR_NICE, PLR_FLEE,
	PLR_AUTOGOLD, PLR_AUTOMAP, PLR_AFK, PLR_INVISPROMPT, PLR_ONMAP, PLR_MAPEDIT,
	PLR_QUESTOR, PLR_MIP, PLR_EXEMPT
} player_flags;

/* Bits for pc_data->flags. */
#define PCFLAG_R1                  BV00
#define PCFLAG_DEADLY              BV01
#define PCFLAG_UNAUTHED		   BV02
#define PCFLAG_NORECALL            BV03
#define PCFLAG_NOINTRO             BV04
#define PCFLAG_GAG		   BV05
#define PCFLAG_RETIRED             BV06
#define PCFLAG_GUEST               BV07
#define PCFLAG_NOSUMMON		   BV08
#define PCFLAG_PAGERON		   BV09
#define PCFLAG_NOTITLE             BV10
#define PCFLAG_GROUPWHO		   BV11
#define PCFLAG_DIAGNOSE		   BV12
#define PCFLAG_HIGHGAG		   BV13
#define PCFLAG_HELPSTART	   BV15 /* Force new players to help start */
#define PCFLAG_DND      	   BV16 /* Do Not Disturb flage */
/* DND flag prevents unwanted transfers of imms by lower level imms */
#define PCFLAG_IDLE		   BV17 /* Player is Linkdead */
#define PCFLAG_AUTOFLAGS	   BV18
#define PCFLAG_SECTORD		   BV19
#define PCFLAG_ANAME		   BV20
#define PCFLAG_HELPER		   BV21
#define PCFLAG_CODER		   BV22
#define PCFLAG_BUILDER		   BV23

typedef enum
{
	TIMER_NONE, TIMER_RECENTFIGHT, TIMER_SHOVEDRAG, TIMER_DO_FUN,
	TIMER_APPLIED, TIMER_PKILLED, TIMER_ASUPRESSED, TIMER_NUISANCE
} timer_types;

struct timer_data
{
	TIMER  *	prev;
	TIMER  *	next;
	DO_FUN *	do_fun;
	int		value;
	short	type;
	int		count;
};


/*
 * Channel bits.
 */
#define	CHANNEL_AUCTION		   BV00
#define	CHANNEL_GOSSIP		   BV01
#define	CHANNEL_OOC		   BV02
#define	CHANNEL_IMMTALK		   BV03
#define	CHANNEL_YELL		   BV04
#define CHANNEL_MONITOR		   BV05
#define CHANNEL_LOG		   BV06
#define CHANNEL_CLAN		   BV07
#define CHANNEL_BUILD		   BV08
#define CHANNEL_AVTALK		   BV09
#define CHANNEL_COMM		   BV10
#define CHANNEL_TELLS		   BV11
#define CHANNEL_NEWBIE             BV12
#define CHANNEL_RACETALK           BV13
#define CHANNEL_WARN               BV14
#define CHANNEL_WHISPER		   BV15
#define CHANNEL_ANNOUNCE	   BV16
#define CHANNEL_SAYTO		   BV17


/* Area defines - Scryn 8/11
 *
 */
#define AREA_DELETED		   BV00
#define AREA_LOADED                BV01

/* Area flags - Narn Mar/96 */
#define AFLAG_NOPKILL               BV00
#define AFLAG_FREEKILL		    BV01
#define AFLAG_NOTELEPORT	    BV02
#define AFLAG_SPELLLIMIT	    BV03

/*
 * Prototype for a mob.
 * This is the in-memory version of #MOBILES.
 */
struct	mob_index_data
{
	MOB_INDEX_DATA *next;
	MOB_INDEX_DATA *next_sort;
	SPEC_FUN *	spec_fun;
	SHOP_DATA *	pShop;
	REPAIR_DATA *	rShop;
	MPROG_DATA *	mudprogs;
	EXT_BV		progtypes;
	char *		player_name;
	char *		short_descr;
	char *		long_descr;
	char *		description;
	int		vnum;
	short		count;
	short		killed;
	short		sex;
	short		level;
	EXT_BV		act;
	EXT_BV		affected_by;
	short		alignment;
	short		mobthac0;		/* Unused */
	short		ac;
	short		hitnodice;
	short		hitsizedice;
	short		hitplus;
	short		damnodice;
	short		damsizedice;
	short		damplus;
	short		numattacks;
	int		gold;
	int		exp;
	int		xflags;
	int		immune;
	int		resistant;
	int		susceptible;
	EXT_BV		attacks;
	EXT_BV		defenses;
	int		speaks;
	int 		speaking;
	short		position;
	short		defposition;
	short		height;
	short		weight;
	short		race;
	short		Class;
	short		hitroll;
	short		damroll;
	short		perm_str;
	short		perm_int;
	short		perm_wis;
	short		perm_dex;
	short		perm_con;
	short		perm_cha;
	short		perm_lck;
	short		saving_poison_death;
	short		saving_mental;
	short		saving_physical;
	short		saving_weapons;
};


struct hunt_hate_fear
{
	char *		name;
	CHAR_DATA *		who;
};

struct fighting_data
{
	CHAR_DATA *		who;
	int			xp;
	short		align;
	short		duration;
	short		timeskilled;
};

struct	editor_data
{
	short		numlines;
	short		on_line;
	short		size;
	char		line[99][160];
};

struct	extracted_char_data
{
	EXTRACT_CHAR_DATA *	next;
	CHAR_DATA *		ch;
	ROOM_INDEX_DATA *	room;
	ch_ret		retcode;
	bool		extract;
};

/*
 * One character (PC or NPC).
 * (Shouldn't most of that build interface stuff use substate, dest_buf,
 * spare_ptr and tempnum?  Seems a little redundant)
 */
struct	char_data
{
	CHAR_DATA *		next;
	CHAR_DATA *		prev;
	CHAR_DATA *		next_in_room;
	CHAR_DATA *		prev_in_room;
	CHAR_DATA *		master;
	CHAR_DATA *		leader;
	FIGHT_DATA *		fighting;
	CHAR_DATA *		reply;
	CHAR_DATA *		retell;
	CHAR_DATA *		switched;
	CHAR_DATA *		mount;
	HHF_DATA *		hunting;
	HHF_DATA *		fearing;
	HHF_DATA *		hating;
	SPEC_FUN *		spec_fun;
	MPROG_ACT_LIST *	mpact;
	int			mpactnum;
	short			mpscriptpos;
	MOB_INDEX_DATA *	pIndexData;
	DESCRIPTOR_DATA *	desc;
	AFFECT_DATA *		first_affect;
	AFFECT_DATA *		last_affect;
	OBJ_DATA *		first_carrying;
	OBJ_DATA *		last_carrying;
	ROOM_INDEX_DATA *	in_room;
	ROOM_INDEX_DATA *	was_in_room;
	PC_DATA *		pcdata;
	DO_FUN *		last_cmd;
	DO_FUN *		prev_cmd;   /* mapping */
	void *			dest_buf;  /* This one is to assign to differen things */
	char *			alloc_ptr; /* Must str_dup and free this one */
	void *			spare_ptr;
	int			tempnum;
	EDITOR_DATA *		editor;
	TIMER	*		first_timer;
	TIMER	*		last_timer;
	char *			name;
	char *			short_descr;
	char *			long_descr;
	char *			description;
	short			num_fighting;
	short			substate;
	short			sex;
	short			Class;
	short			race;
	short			level;
	short			trust;
	int			played;
	time_t			logon;
	time_t			save_time;
	short			timer;
	short			wait;
	short			hit;
	short			max_hit;
	short			mana;
	short			max_mana;
	short			move;
	short			max_move;
	short			practice;
	short			numattacks;
	short              	remorts;
	int			gold;
	int			exp;
	EXT_BV			act;
	EXT_BV			affected_by;
	EXT_BV			no_affected_by;
	int			carry_weight;
	int			carry_number;
	int			xflags;
	int			no_immune;
	int			no_resistant;
	int			no_susceptible;
	int			immune;
	int			resistant;
	int			susceptible;
	EXT_BV			attacks;
	EXT_BV			defenses;
	int			speaks;
	int			speaking;
	short			saving_poison_death;
	short			saving_mental;
	short			saving_physical;
	short			saving_weapons;
	short			alignment;
	short			barenumdie;
	short			baresizedie;
	short			mobthac0;
	short			hitroll;
	short			damroll;
	short			hitplus;
	short			damplus;
	short			position;
	short			defposition;
	short			style;
	short			height;
	short			weight;
	short			armor;
	short			wimpy;
	int			deaf;
	short			perm_str;
	short			perm_int;
	short			perm_wis;
	short			perm_dex;
	short			perm_con;
	short			perm_cha;
	short			perm_lck;
	short			mod_str;
	short			mod_int;
	short			mod_wis;
	short			mod_dex;
	short			mod_con;
	short			mod_cha;
	short			mod_lck;
	int			retran;
	int			regoto;
	short			mobinvis;		/* Mobinvis level SB */
	short  			colors[MAX_COLORS];
	int			home_vnum;  		/* hotboot tracker */
	short 			x; 			/* Coordinates on the overland map - Samson 7-31-99 */
	short 			y;
	short 			map; 			/* Which map are they on? - Samson 8-3-99 */
	short 			sector;			/* Type of terrain to restrict a wandering mob to on overland - Samson 7-27-00 */
	int                 	hp_from_gain;         /*for stat training*/
	long			tattoo;
	OBJ_DATA *		on;
	short 			cmd_recurse;
	CHAR_DATA *		challenged;
	CHAR_DATA *    		betted_on;
	int        	  	bet_amt;
	int                	 pos;
};

/* Structure for link list of ignored players */
struct ignore_data
{
	IGNORE_DATA *next;
	IGNORE_DATA *prev;
	char *name;
};

/* Max number of people you can ignore at once */
#define MAX_IGN		6


/*
 * Data which only PC's have.
 */
struct	pc_data
{
	CHAR_DATA *		pet;
	CLAN_DATA *		clan;
	AREA_DATA *		area;
	DEITY_DATA *	deity;
	char *		homepage;
	char *		clan_name;
	char * 		council_name;
	char *		deity_name;
	char *		pwd;
	char *		bamfin;
	char *		bamfout;
	char *		filename;       /* For the safe mset name -Shaddai */
	char *              rank;
	char *		title;
	char *		bestowments;	/* Special bestowed commands	   */
	int                 flags;		/* Whether the player is deadly and whatever else we add.      */
	int			pkills;		/* Number of pkills on behalf of clan */
	int			pdeaths;	/* Number of times pkilled (legally)  */
	int			mkills;		/* Number of mobs killed		   */
	int			mdeaths;	/* Number of deaths due to mobs       */
	int			illegal_pk;	/* Number of illegal pk's committed   */
	long int            outcast_time;	/* The time at which the char was outcast */
	char *              questarea; /* Questmaster */
	char *              questroom; /* Questmaster */
	CHAR_DATA *         questgiver; /* Questmaster */
	short               nextquest; /* Questmaster */
	short               countdown; /* Questmaster */
	int                 questobj; /* Questmaster */
	int                 questmob;
	long int            restore_time;	/* The last time the char did a restore all */
	int			r_range_lo;	/* room range */
	int			r_range_hi;
	int			m_range_lo;	/* mob range  */
	int			m_range_hi;
	int			o_range_lo;	/* obj range  */
	int			o_range_hi;
	short		wizinvis;	/* wizinvis level */
	short		min_snoop;	/* minimum snoop level */
	short		learned		[MAX_SKILL];
	short		quest_number;	/* current *QUEST BEING DONE* DON'T REMOVE! */
	short		quest_curr;	/* current number of quest points */
	int			quest_accum;	/* quest points accumulated in players life */
	short		favor;		/* deity favor */
	short		charmies;	/* Number of Charmies */
	time_t		release_date;	/* Auto-helling.. Altrag */
	char *		helled_by;
	char *		bio;		/* Personal Bio */
	char *		authed_by;	/* what crazy imm authed this name ;) */
	SKILLTYPE *		special_skills[MAX_PERSONAL]; /* personalized skills/spells */
	char *		prompt;		/* User config prompts */
	char *		fprompt;	/* Fight prompts */
	char *		subprompt;	/* Substate prompt */
	short		pagerlen;	/* For pager (NOT menus) */
	bool		openedtourney;
	IGNORE_DATA	*	first_ignored; 	/* keep track of who to ignore */
	IGNORE_DATA	*	last_ignored;
	short		lt_index;	/* last_tell index */
	int 		balance;  	/* for banking code*/
	LOCKER_DATA *	locker;
	ROOM_INDEX_DATA *   locker_room;    /* Pointer to virtual room */
	short               locker_vnum;
	short		remorts;
	bool		hotboot; /* hotboot tracker */
	short 		secedit; /* Overland Map OLC - Samson 8-1-99 */
	char *      sec_code;       /* MIP Security Code    */
	char *      mip_ver;        /* MIP Version Info     */
#ifdef IMC
	IMC_CHARDATA *imcchardata;
#endif
	int version;
};

struct  locker_data
{
	short		capacity;
	short		holding;
	int 		flags;
	int			room;
};

/*
 * Liquids.
 */
#define LIQ_WATER        0
#define LIQ_MAX		18

struct	liq_type
{
	char *	liq_name;
	char *	liq_color;
	short	liq_affect[3];
};


/* Damage types from the attack_table[]
   modified for new weapon_types - Grimm
  Trimmed down to reduce duplicated types - Samson 1-9-00 */
typedef enum
{
	DAM_HIT, DAM_SLASH, DAM_STAB, DAM_HACK, DAM_CRUSH, DAM_LASH,
	DAM_PIERCE, DAM_THRUST, DAM_SLICE, DAM_MAGIC, MAX_DAM_TYPE
} damage_types;

/* New Weapon type array for profficiency checks - Samson 11-20-99 */
typedef enum
{
	WEP_BAREHAND, WEP_SWORD, WEP_DAGGER, WEP_WHIP, WEP_TALON, WEP_HAMMER,
	WEP_ARCHERY, WEP_BLOWGUN, WEP_SLING, WEP_AXE, WEP_SPEAR, WEP_STAFF, WEP_POLEARM,
	WEP_MAX
} weapon_types;

/* New projectile type array for archery weapons - Samson 1-9-00 */
typedef enum
{
	PROJ_BOLT, PROJ_ARROW, PROJ_DART, PROJ_STONE, PROJ_MAX
} projectile_types;


/*
 * Extra description data for a room or object.
 */
struct	extra_descr_data
{
	EXTRA_DESCR_DATA *next;	/* Next in list                     */
	EXTRA_DESCR_DATA *prev;	/* Previous in list                 */
	char *keyword;              /* Keyword in look/examine          */
	char *description;          /* What to see                      */
};



/*
 * Prototype for an object.
 */
struct	obj_index_data
{
	OBJ_INDEX_DATA *	next;
	OBJ_INDEX_DATA *	next_sort;
	EXTRA_DESCR_DATA *	first_extradesc;
	EXTRA_DESCR_DATA *	last_extradesc;
	AFFECT_DATA *	first_affect;
	AFFECT_DATA *	last_affect;
	MPROG_DATA *	mudprogs;               /* objprogs */
	EXT_BV		progtypes;              /* objprogs */
	char *		name;
	char *		short_descr;
	char *		description;
	char *		action_desc;
	int			vnum;
	short              level;
	short		item_type;
	EXT_BV		extra_flags;
	int			magic_flags; /*Need more bitvectors for spells - Scryn*/
	int			wear_flags;
	short		count;
	short		weight;
	int			cost;
	int			value	[6];
	int			serial;
	short		layers;
	int			rent;			/* Unused */
};


/*
 * One object.
 */
struct	obj_data
{
	OBJ_DATA *		next;
	OBJ_DATA *		prev;
	OBJ_DATA *		next_content;
	OBJ_DATA *		prev_content;
	OBJ_DATA *		first_content;
	OBJ_DATA *		last_content;
	OBJ_DATA *		in_obj;
	CHAR_DATA *		carried_by;
	EXTRA_DESCR_DATA *	first_extradesc;
	EXTRA_DESCR_DATA *	last_extradesc;
	AFFECT_DATA *	first_affect;
	AFFECT_DATA *	last_affect;
	OBJ_INDEX_DATA *	pIndexData;
	ROOM_INDEX_DATA *	in_room;
	char *		name;
	char *		short_descr;
	char *		description;
	char *		action_desc;
	char *		creator;
	short		item_type;
	short		mpscriptpos;
	EXT_BV		extra_flags;
	int			magic_flags; /*Need more bitvectors for spells - Scryn*/
	int			wear_flags;
	MPROG_ACT_LIST *	mpact;		/* mudprogs */
	int			mpactnum;	/* mudprogs */
	short		wear_loc;
	short		weight;
	int			cost;
	short		level;
	short		timer;
	int			value	[6];
	short		count;		/* support for object grouping */
	int			serial;		/* serial number	       */
	int		room_vnum; /* hotboot tracker */
	short x; /* Object coordinates on overland maps - Samson 8-21-99 */
	short y;
	short map; /* Which map is it on? - Samson 8-21-99 */
	short sewkit;
	short handmade;
	short material;
};


/*
 * Exit data.
 */
struct	exit_data
{
	EXIT_DATA *		prev;		/* previous exit in linked list	*/
	EXIT_DATA *		next;		/* next exit in linked list	*/
	EXIT_DATA *		rexit;		/* Reverse exit pointer		*/
	ROOM_INDEX_DATA *	to_room;	/* Pointer to destination room	*/
	char *		keyword;	/* Keywords for exit or door	*/
	char *		description;	/* Description of exit		*/
	int			vnum;		/* Vnum of room exit leads to	*/
	int			rvnum;		/* Vnum of room in opposite dir	*/
	int			exit_info;	/* door states & other flags	*/
	int			key;		/* Key vnum			*/
	short		vdir;		/* Physical "direction"		*/
	short		distance;	/* how far to the next room	*/
	short		pull;		/* pull of direction (current)	*/
	short		pulltype;	/* type of pull (current, wind)	*/
	short x; /* Coordinates to Overland Map - Samson 7-31-99 */
	short y;
};



/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'H': hide an object
 *   'B': set a bitvector
 *   'T': trap an object
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */

/*
 * Area-reset definition.
 */
struct	reset_data
{
	RESET_DATA *	next;
	RESET_DATA *	prev;
	RESET_DATA *	first_reset;
	RESET_DATA *	last_reset;
	RESET_DATA *	next_reset;
	RESET_DATA *	prev_reset;
	char		command;
	int			extra;
	int			arg1;
	int			arg2;
	int			arg3;
};

/* Constants for arg2 of 'B' resets. */
#define	BIT_RESET_DOOR			0
#define BIT_RESET_OBJECT		1
#define BIT_RESET_MOBILE		2
#define BIT_RESET_ROOM			3
#define BIT_RESET_TYPE_MASK		0xFF	/* 256 should be enough */
#define BIT_RESET_DOOR_THRESHOLD	8
#define BIT_RESET_DOOR_MASK		0xFF00	/* 256 should be enough */
#define BIT_RESET_SET			BV30
#define BIT_RESET_TOGGLE		BV31
#define BIT_RESET_FREEBITS	  0x3FFF0000	/* For reference */



/*
 * Area definition.
 */
struct	area_data
{
	AREA_DATA *		next;
	AREA_DATA *		prev;
	AREA_DATA *		next_sort;
	AREA_DATA *		prev_sort;
	AREA_DATA *         next_sort_name; /* Used for alphanum. sort */
	AREA_DATA *         prev_sort_name; /* Ditto, Fireblade */
	char *		name;
	char *		filename;
	int                 flags;
	short               status;  /* h, 8/11 */
	short		age;
	short		nplayer;
	short		reset_frequency;
	int			low_r_vnum;
	int			hi_r_vnum;
	int			low_o_vnum;
	int			hi_o_vnum;
	int			low_m_vnum;
	int			hi_m_vnum;
	int			low_soft_range;
	int			hi_soft_range;
	int			low_hard_range;
	int			hi_hard_range;
	int		        spelllimit;
	int			curr_spell_count;
	char *		author; /* Scryn */
	char *              resetmsg; /* Rennard */
	short		max_players;
	int			mkills;
	int			mdeaths;
	int			pkills;
	int			pdeaths;
	int			gold_looted;
	int			illegal_pk;
	int			high_economy;
	int			low_economy;
	WEATHER_DATA *	weather; /* FB */
	short 		continent; /* Added for Overland support - Samson 9-16-00 */
	ROOM_INDEX_DATA *	first_room;
	ROOM_INDEX_DATA *	last_room;
};



/*
 * Load in the gods building data. -- Altrag
 */
struct	godlist_data
{
	GOD_DATA *	next;
	GOD_DATA *	prev;
	int		level;
	int		low_r_vnum;
	int		hi_r_vnum;
	int		low_o_vnum;
	int		hi_o_vnum;
	int		low_m_vnum;
	int		hi_m_vnum;
};


/*
 * Used to keep track of system settings and statistics		-Thoric
 */
struct	system_data
{
	int		maxplayers;		/* Maximum players this boot   */
	int		alltimemax;		/* Maximum players ever	  */
	int		global_looted;		/* Gold looted this boot */
	int		upill_val;		/* Used pill value */
	int		upotion_val;		/* Used potion value */
	int		brewed_used;		/* Brewed potions used */
	int		scribed_used;		/* Scribed scrolls used */
	char *	time_of_max;		/* Time of max ever */
	char *	mud_name;		/* Name of mud */
	bool	NO_NAME_RESOLVING;	/* Hostnames are not resolved  */
	bool    	DENY_NEW_PLAYERS;	/* New players cannot connect  */
	bool	WAIT_FOR_AUTH;		/* New players must be auth'ed */
	short	read_all_mail;		/* Read all player mail(was 54)*/
	short	read_mail_free;		/* Read mail for free (was 51) */
	short	write_mail_free;	/* Write mail for free(was 51) */
	short	take_others_mail;	/* Take others mail (was 54)   */
	short	muse_level;		/* Level of muse channel */
	short	think_level;		/* Level of think channel LEVEL_HIGOD*/
	short	build_level;		/* Level of build channel LEVEL_BUILD*/
	short	log_level;		/* Level of log channel LEVEL LOG*/
	short	level_modify_proto;	/* Level to modify prototype stuff LEVEL_LESSER */
	short	level_override_private;	/* override private flag */
	short	level_mset_player;	/* Level to mset a player */
	short	bash_plr_vs_plr;	/* Bash mod player vs. player */
	short	bash_nontank;		/* Bash mod basher != primary attacker */
	short      	gouge_plr_vs_plr;       /* Gouge mod player vs. player */
	short      	gouge_nontank;	        /* Gouge mod player != primary attacker */
	short	stun_plr_vs_plr;	/* Stun mod player vs. player */
	short	stun_regular;		/* Stun difficult */
	short	dodge_mod;		/* Divide dodge chance by */
	short	parry_mod;		/* Divide parry chance by */
	short	tumble_mod;		/* Divide tumble chance by */
	short	dam_plr_vs_plr;		/* Damage mod player vs. player */
	short	dam_plr_vs_mob;		/* Damage mod player vs. mobile */
	short	dam_mob_vs_plr;		/* Damage mod mobile vs. player */
	short	dam_mob_vs_mob;		/* Damage mod mobile vs. mobile */
	short	level_getobjnotake;     /* Get objects without take flag */
	short   level_forcepc;          /* The level at which you can use force on players. */
	short	bestow_dif;		/* Max # of levels between trust and command level for a bestow to work --Blodkai */
	short	max_sn;			/* Max skills */
	char       	*guild_overseer;         /* Pointer to char containing the name of the */
	char       	*guild_advisor;		/* guild overseer and advisor. */
	int		save_flags;		/* Toggles for saving conditions */
	short	save_frequency;		/* How old to autosave someone */
	short 	save_pets;		/* Do pets save? */
	short 	ban_site_level;      /* Level to ban sites */
	short 	ban_Class_level;     /* Level to ban Classes */
	short 	ban_race_level;      /* Level to ban races */
	short 	ident_retries;	/* Number of times to retry broken pipes. */
	short 	pk_loot;		/* Pkill looting allowed? */
	short 	newbie_purge; /* Level to auto-purge newbies at - Samson 12-27-98 */
	short 	regular_purge; /* Level to purge normal players at - Samson 12-27-98 */
	bool 	CLEANPFILES; /* Should the mud clean up pfiles daily? - Samson 12-27-98 */
	void 	*dlHandle;
};

/*
 * Room type.
 */
struct	room_index_data
{
	ROOM_INDEX_DATA *	next;
	ROOM_INDEX_DATA *	next_sort;
	CHAR_DATA *		first_person;	    /* people in the room	*/
	CHAR_DATA *		last_person;	    /*		..		*/
	OBJ_DATA *		first_content;	    /* objects on floor		*/
	OBJ_DATA *		last_content;	    /*		..		*/
	EXTRA_DESCR_DATA *	first_extradesc;    /* extra descriptions	*/
	EXTRA_DESCR_DATA *	last_extradesc;	    /*		..		*/
	AREA_DATA *		area;
	EXIT_DATA *		first_exit;	    /* exits from the room	*/
	EXIT_DATA *		last_exit;	    /*		..		*/
	AFFECT_DATA *	first_affect;	    /* effects on the room	*/
	AFFECT_DATA *	last_affect;	    /*		..		*/
	MPROG_ACT_LIST *	mpact;		    /* mudprogs */
	int			mpactnum;	    /* mudprogs */
	MPROG_DATA *	mudprogs;	    /* mudprogs */
	short		NULL_VAR;
	short		mpscriptpos;
	char *		name;
	char *		description;
	int			vnum;
	EXT_BV		room_flags;
	EXT_BV		progtypes;           /* mudprogs */
	short		light;		     /* amount of light in the room */
	short		sector_type;
	int			tele_vnum;
	short		tele_delay;
	short		tunnel;		     /* max people that will fit */
	RESET_DATA *first_reset;
	RESET_DATA *last_reset;
	RESET_DATA *last_mob_reset;
	RESET_DATA *last_obj_reset;
	ROOM_INDEX_DATA *next_aroom; /* Rooms within an area */
	ROOM_INDEX_DATA *prev_aroom;
};

/*
 * Delayed teleport type.
 */
struct	teleport_data
{
	TELEPORT_DATA *	next;
	TELEPORT_DATA *	prev;
	ROOM_INDEX_DATA *	room;
	short		timer;
};


/*
 * Types of skill numbers.  Used to keep separate lists of sn's
 * Must be non-overlapping with spell/skill types,
 * but may be arbitrary beyond that.
 */
#define TYPE_UNDEFINED               1
#define TYPE_HIT                     1000  /* allows for 1000 skills/spells */
#define TYPE_HERB		     2000  /* allows for 1000 attack types  */
#define TYPE_PERSONAL		     3000  /* allows for 1000 herb types    */
#define TYPE_RACIAL		     4000  /* allows for 1000 personal types*/
#define TYPE_DISEASE		     5000  /* allows for 1000 racial types  */

/*
 *  Target types.
 */
typedef enum
{
	TAR_IGNORE, TAR_CHAR_OFFENSIVE, TAR_CHAR_DEFENSIVE, TAR_CHAR_SELF, TAR_OBJ_INV
} target_types;

typedef enum
{
	SKILL_UNKNOWN, SKILL_SPELL, SKILL_SKILL, SKILL_WEAPON, SKILL_TONGUE,
} skill_types;



struct timerset
{
	int num_uses;
	struct timeval total_time;
	struct timeval min_time;
	struct timeval max_time;
};



/*
 * Skills include spells as a particular case.
 */
struct	skill_type
{
	char *		name;			/* Name of skill		*/
	short		skill_level[MAX_CLASS];	/* Level needed by Class	*/
	short		skill_adept[MAX_CLASS];	/* Max attainable % in this skill */
	short		race_level[MAX_RACE];	/* Racial abilities: level      */
	short		race_adept[MAX_RACE];	/* Racial abilities: adept      */
	SPELL_FUN *	spell_fun;   		/* Spell pointer (for spells) */
	char *		spell_fun_name;   	/* Spell function name - Trax */
	DO_FUN *	skill_fun;      	/* Skill pointer (for skills) */
	char *		skill_fun_name;   	/* Skill function name - Trax */
	short		target;			/* Legal targets		*/
	short		minimum_position;	/* Position for caster / user	*/
//	short	slot;			/* Slot for #OBJECT loading	*/
	short	min_mana;		/* Minimum mana used		*/
	short	beats;			/* Rounds required to use skill	*/
	char *	noun_damage;		/* Damage message		*/
	char *	msg_off;		/* Wear off message		*/
	short	guild;			/* Which guild the skill belongs to */
	short	min_level;		/* Minimum level to be able to cast */
	short	type;			/* Spell/Skill/Weapon/Tongue	*/
	short	range;			/* Range of spell (rooms)	*/
	int		info;			/* Spell action/Class/etc	*/
	int		flags;			/* Flags			*/
	char *	hit_char;		/* Success message to caster	*/
	char *	hit_vict;		/* Success message to victim	*/
	char *	hit_room;		/* Success message to room	*/
	char *	hit_dest;		/* Success message to dest room	*/
	char *	miss_char;		/* Failure message to caster	*/
	char *	miss_vict;		/* Failure message to victim	*/
	char *	miss_room;		/* Failure message to room	*/
	char *	die_char;		/* Victim death msg to caster	*/
	char *	die_vict;		/* Victim death msg to victim	*/
	char *	die_room;		/* Victim death msg to room	*/
	char *	imm_char;		/* Victim immune msg to caster	*/
	char *	imm_vict;		/* Victim immune msg to victim	*/
	char *	imm_room;		/* Victim immune msg to room	*/
	char *	dice;			/* Dice roll			*/
	int		value;			/* Misc value			*/
	int		spell_sector;		/* Sector Spell work	 	*/
	char	saves;			/* What saving spell applies	*/
	char	difficulty;		/* Difficulty of casting/learning */
	SMAUG_AFF *	affects;		/* Spell affects, if any	*/
	char *	components;		/* Spell components, if any	*/
	char *	teachers;		/* Skill requires a special teacher */
	char	participants;		/* # of required participants	*/
	struct	timerset	userec;	/* Usage record			*/
};


/* how many items to track.... prevent repeat auctions */
#define AUCTION_MEM 3

struct  auction_data
{
	OBJ_DATA  * item;   /* a pointer to the item */
	CHAR_DATA * seller; /* a pointer to the seller - which may NOT quit */
	CHAR_DATA * buyer;  /* a pointer to the buyer - which may NOT quit */
	int         bet;    /* last bet - or 0 if noone has bet anything */
	short      going;  /* 1,2, sold */
	short      pulse;  /* how many pulses (.25 sec) until another call-out ? */
	int 	starting;
	OBJ_INDEX_DATA *	history[AUCTION_MEM];	/* store auction history */
	short	hist_timer;		/* clear out history buffer if auction is idle */
};

/*
 * So we can have different configs for different ports -- Shaddai
 */
extern int port;


/*
 * Cmd flag names --Shaddai
 */
extern char *const cmd_flags[];


/*
 * The functions for these prototypes can be found in misc.c
 * They are up here because they are used by the macros below
 */
bool	ext_is_empty		args ( ( EXT_BV *bits ) );
void	ext_clear_bits		args ( ( EXT_BV *bits ) );
int	ext_has_bits		args ( ( EXT_BV *var, EXT_BV *bits ) );
bool	ext_same_bits		args ( ( EXT_BV *var, EXT_BV *bits ) );
void	ext_set_bits		args ( ( EXT_BV *var, EXT_BV *bits ) );
void	ext_remove_bits		args ( ( EXT_BV *var, EXT_BV *bits ) );
void	ext_toggle_bits		args ( ( EXT_BV *var, EXT_BV *bits ) );

/*
 *  Defines for the command flags. --Shaddai
 */
#define	CMD_FLAG_POSSESS	BV00
#define CMD_WATCH		BV02	/* FB */

/*
 * Structure for a command in the command lookup table.
 */
struct	cmd_type
{
	CMDTYPE *		next;
	char *		name;
	DO_FUN *		do_fun;
	char *		fun_name;
	int                 flags;  /* Added for Checking interpret stuff -Shaddai*/
	short		position;
	short		level;
	short		log;
	struct		timerset	userec;
	int			lag_count;	/* count lag flags for this cmd - FB */
};



/*
 * Structure for a social in the socials table.
 */
struct	social_type
{
	SOCIALTYPE *	next;
	char *		name;
	char *		char_no_arg;
	char *		others_no_arg;
	char *		char_found;
	char *		others_found;
	char *		vict_found;
	char *		char_auto;
	char *		others_auto;
};



/*
 * Global constants.
 */
extern  time_t last_restore_all_time;
extern  time_t boot_time;  /* this should be moved down */
extern  HOUR_MIN_SEC * set_boot_time;
extern  struct  tm *new_boot_time;
extern  time_t new_boot_time_t;
extern FILE *fpArea;
extern char strArea[MAX_INPUT_LENGTH];
extern	const	struct	str_app_type	str_app		[26];
extern	const	struct	int_app_type	int_app		[26];
extern	const	struct	wis_app_type	wis_app		[26];
extern	const	struct	dex_app_type	dex_app		[26];
extern	const	struct	con_app_type	con_app		[26];
extern	const	struct	cha_app_type	cha_app		[26];
extern  const	struct	lck_app_type	lck_app		[26];

extern	const struct	race_type _race_table	[MAX_RACE];
extern	struct	race_type *	race_table	[MAX_RACE];
extern	const	struct	liq_type	liq_table	[LIQ_MAX];

extern char *attack_table[MAX_DAM_TYPE];
extern char *attack_table_plural[MAX_DAM_TYPE];
extern char **const s_message_table[MAX_DAM_TYPE];
extern char **const p_message_table[MAX_DAM_TYPE];
extern char *weapon_skills[WEP_MAX]; /* Used in spell_identify */
extern char *projectiles[PROJ_MAX]; /* For archery weapons */

extern	char *	const	skill_tname	[];
extern	short	const	movement_loss	[SECT_MAX];
extern	char *	const	dir_name	[];
extern	char *	const	where_name	[MAX_WHERE_NAME];
extern	const	short	rev_dir		[];
extern	char *	const	r_flags		[];
extern	char *	const	w_flags		[];
extern	char *	const   sec_flags	[];
extern	char *	const	item_w_flags	[];
extern	char *	const	o_flags		[];
extern	char *	const	a_flags		[];
extern	char *	const	o_types		[];
extern	char *	const	a_types		[];
extern	char *	const	act_flags	[];
extern	char *	const	plr_flags	[];
extern	char *	const	pc_flags	[];
extern	char *	const	ris_flags	[];
extern	char *	const	trig_flags	[];
extern	char *	const	part_flags	[];
extern	char *	const	npc_race	[];
extern	char *	const	npc_Class	[];
extern	char *	const	defense_flags	[];
extern	char *	const	attack_flags	[];
extern	char *	const	area_flags	[];
extern  char *  const   container_flags []; /* OasisOLC */
extern	char *	const	ex_pmisc	[];
extern	char *	const	ex_pwater	[];
extern	char *	const	ex_pair		[];
extern	char *	const	ex_pearth	[];
extern	char *	const	ex_pfire	[];

extern	int	const	lang_array      [];
extern	char *	const	lang_names      [];

extern	char *	const	temp_settings	[]; /* FB */
extern	char *	const	precip_settings	[];
extern	char *	const	wind_settings	[];
extern	char *	const	preciptemp_msg	[6][6];
extern	char *	const	windtemp_msg	[6][6];
extern	char *	const	precip_msg	[];
extern	char *	const	wind_msg	[];

/*
 * Global variables.
 */
extern char *   bigregex;
extern char *   preg;

extern MPSLEEP_DATA * first_mpwait; /* Storing sleeping mud progs */
extern MPSLEEP_DATA * last_mpwait; /* - */
extern MPSLEEP_DATA * current_mpwait; /* - */

extern char *	target_name;
extern char *	ranged_target_name;
extern	int	numobjsloaded;
extern	int	nummobsloaded;
extern	int	physicalobjects;
extern 	int	last_pkroom;
extern	int	num_descriptors;
extern	struct	system_data		sysdata;
extern	int	top_sn;
extern	int	top_vroom;
extern	int	top_herb;

extern		CMDTYPE		  *	command_hash	[126];

extern		struct	Class_type *	Class_table	[MAX_CLASS];
extern		char *			title_table	[MAX_CLASS]
	[MAX_LEVEL+1]
	[2];

extern		SKILLTYPE	  *	skill_table	[MAX_SKILL];
extern		SOCIALTYPE	  *	social_index	[27];
extern		CHAR_DATA	  *	cur_char;
extern		ROOM_INDEX_DATA	  *	cur_room;
extern		bool			cur_char_died;
extern		ch_ret			global_retcode;
extern		SKILLTYPE	  *	herb_table	[MAX_HERB];
extern		SKILLTYPE	  *	disease_table	[MAX_DISEASE];

extern		int			cur_obj;
extern		int			cur_obj_serial;
extern		bool			cur_obj_extracted;
extern		obj_ret			global_objcode;

extern		HELP_DATA	  *	first_help;
extern		HELP_DATA	  *	last_help;
extern		SHOP_DATA	  *	first_shop;
extern		SHOP_DATA	  *	last_shop;
extern		REPAIR_DATA	  *	first_repair;
extern		REPAIR_DATA	  *	last_repair;

extern		BAN_DATA	  *	first_ban;
extern		BAN_DATA	  *	last_ban;
extern		RESERVE_DATA	  *	first_reserved;
extern		RESERVE_DATA	  *	last_reserved;
extern		CHAR_DATA	  *	first_char;
extern		CHAR_DATA	  *	last_char;
extern		DESCRIPTOR_DATA   *	first_descriptor;
extern		DESCRIPTOR_DATA   *	last_descriptor;
extern		OBJ_DATA	  *	first_object;
extern		OBJ_DATA	  *	last_object;
extern		CLAN_DATA	  *	first_clan;
extern		CLAN_DATA	  *	last_clan;
extern		DEITY_DATA	  *	first_deity;
extern		DEITY_DATA	  *	last_deity;
extern		AREA_DATA	  *	first_area;
extern		AREA_DATA	  *	last_area;
extern		AREA_DATA	  *	first_build;
extern		AREA_DATA	  *	last_build;
extern		AREA_DATA	  *	first_asort;
extern		AREA_DATA	  *	last_asort;
extern		AREA_DATA	  *	first_bsort;
extern		AREA_DATA	  *	last_bsort;
extern          AREA_DATA         *     first_area_name; /*alphanum. sort*/
extern          AREA_DATA         *     last_area_name;  /* Fireblade */

extern		LANG_DATA	  *	first_lang;
extern		LANG_DATA	  *	last_lang;

extern          bool                    double_exp;
extern          bool                    double_qp;
extern          bool                    quad_damage;
extern          int                     global_exp;
extern          int                     global_qp;
extern          int                     global_quad;
extern          short                  display;
extern          short                  qpdisplay;
extern          short                  quaddisplay;

extern		TELEPORT_DATA	  *	first_teleport;
extern		TELEPORT_DATA	  *	last_teleport;
extern		OBJ_DATA	  *	extracted_obj_queue;
extern		EXTRACT_CHAR_DATA *	extracted_char_queue;
extern		OBJ_DATA	  *	save_equipment[MAX_WEAR][MAX_LAYERS];
extern		CHAR_DATA	  *	quitting_char;
extern		CHAR_DATA	  *	loading_char;
extern		CHAR_DATA	  *	saving_char;
extern		OBJ_DATA	  *	all_obj;

extern		char			bug_buf		[];
extern		time_t			current_time;
extern		bool			fLogAll;
extern		FILE *			fpReserve;
extern		FILE *			fpLOG;
extern		char			log_buf		[];
extern		TIME_INFO_DATA		time_info;
extern		WEATHER_DATA		weather_info;
extern		int			weath_unit;
extern		int			rand_factor;
extern		int			climate_factor;
extern		int			neigh_factor;
extern		int			max_vector;

extern          AUCTION_DATA      *     auction;
extern		struct act_prog_data *	mob_act_list;
extern          QUEST_DATA      *     questmaster;

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */
#define PLAYER_DIR	"../player/"	/* Player files			*/
#define BACKUP_DIR	"../player/backup/" /* Backup Player files	*/
#define GOD_DIR		"../gods/"	/* God Info Dir			*/
#define CLAN_DIR	"../clans/"	/* Clan data dir		*/
#define DEITY_DIR	"../deity/"	/* Deity data dir		*/
#define BUILD_DIR       "../building/"  /* Online building save dir     */
#define SYSTEM_DIR	"../system/"	/* Main system files		*/
#define PROG_DIR	"mudprogs/"	/* MUDProg files		*/
#define LOCKER_DIR      "../lockers/"   /* Player Lockers               */
#define NULL_FILE	"/dev/null"	/* To reserve one stream        */
#define	CLASS_DIR	"../classes/"	/* Classes			*/
#define RACE_DIR 	"../races/"
#define AREA_LIST	"area.lst"	/* List of areas		*/
#define BAN_LIST        SYSTEM_DIR "ban.lst"       /* List of bans                 */
#define RESERVED_LIST	"reserved.lst"	/* List of reserved names	*/
#define CLAN_LIST	"clan.lst"	/* List of clans		*/
#define COUNCIL_LIST	"council.lst"	/* List of councils		*/
#define GUILD_LIST      "guild.lst"     /* List of guilds               */
#define GOD_LIST	"gods.lst"	/* List of gods			*/
#define DEITY_LIST	"deity.lst"	/* List of deities		*/
#define	CLASS_LIST	"class.lst"	/* List of Classes		*/
#define	RACE_LIST	"race.lst"	/* List of races		*/

#define SHUTDOWN_FILE	"shutdown.txt"		/* For 'shutdown'	 */
#define LAST_LIST       SYSTEM_DIR "last.lst" /*last list*/
#define LAST_TEMP_LIST  SYSTEM_DIR "ltemp.lst" /*temp file for the last list so the data can be copyover over*/
#define BOOTLOG_FILE	SYSTEM_DIR "boot.txt"	  /* Boot up error file	 */
#define PBUG_FILE	SYSTEM_DIR "pbugs.txt"	  /* For 'bug' command   */
#define IDEA_FILE	SYSTEM_DIR "ideas.txt"	  /* For 'idea'		 */
#define TYPO_FILE	SYSTEM_DIR "typos.txt"	  /* For 'typo'		 */
#define FIXED_FILE	SYSTEM_DIR "fixed.txt"	  /* For 'fixed' command */
#define LOG_FILE	SYSTEM_DIR "log.txt"	  /* For talking in logged rooms */
#define MOBLOG_FILE	SYSTEM_DIR "moblog.txt"   /* For mplog messages  */
#define PLEVEL_FILE	SYSTEM_DIR "plevel.txt"   /* Char level info */
#define WIZLIST_FILE	SYSTEM_DIR "WIZLIST"	  /* Wizlist		 */
#define REQUEST_PIPE	SYSTEM_DIR "REQUESTS"	  /* Request FIFO	 */
#define SKILL_FILE	SYSTEM_DIR "skills.dat"   /* Skill table	 */
#define HERB_FILE	SYSTEM_DIR "herbs.dat"	  /* Herb table		 */
#define TONGUE_FILE	SYSTEM_DIR "tongues.dat"  /* Tongue tables	 */
#define SOCIAL_FILE	SYSTEM_DIR "socials.dat"  /* Socials		 */
#define COMMAND_FILE	SYSTEM_DIR "commands.dat" /* Commands		 */
#define ECONOMY_FILE	SYSTEM_DIR "economy.txt"  /* Gold looted, value of used potions/pills  */
#define TEMP_FILE	SYSTEM_DIR "charsave.tmp" /* More char save protect */
#define QUEST_FILE      SYSTEM_DIR "quest.dat" /* Storing qmaster stuff */
/*
 * Our function prototypes.
 * One big lump ... this is every function in Merc.
 */
#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define CL	CLAN_DATA
#define EDD	EXTRA_DESCR_DATA
#define RD	RESET_DATA
#define ED	EXIT_DATA
#define	ST	SOCIALTYPE
#define	CO	COUNCIL_DATA
#define DE	DEITY_DATA
#define SK	SKILLTYPE

/* act_comm.c */
bool	circle_follow	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	add_follower	args ( ( CHAR_DATA *ch, CHAR_DATA *master ) );
void	stop_follower	args ( ( CHAR_DATA *ch ) );
void	die_follower	args ( ( CHAR_DATA *ch ) );
bool	is_same_group	args ( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void	to_channel	args ( ( const char *argument, int channel, const char *verb, short level ) );
void  	talk_auction    args ( ( char *argument ) );
int	knows_language  args ( ( CHAR_DATA *ch, int language,	CHAR_DATA *cch ) );
bool    can_learn_lang  args ( ( CHAR_DATA *ch, int language ) );
int     countlangs      args ( ( int languages ) );
char *	translate	args ( ( int percent, const char *in, const char *name ) );
char *	obj_short	args ( ( OBJ_DATA *obj ) );

/* act_info.c */
int	get_door	args ( ( char *arg ) );
char *  num_punct	args ( ( int foo ) );
char *	format_obj_to_char	args ( ( OBJ_DATA *obj, CHAR_DATA *ch, bool fShort ) );
void	show_list_to_char	args ( ( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ) );
bool	is_ignoring	args ( ( CHAR_DATA *ch, CHAR_DATA *ign_ch ) );
void	show_race_line	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );

/* act_move.c */
void	clear_vrooms	args ( ( void ) );
ED *	find_door	args ( ( CHAR_DATA *ch, char *arg, bool quiet ) );
ED *	get_exit	args ( ( ROOM_INDEX_DATA *room, short dir ) );
ED *	get_exit_to	args ( ( ROOM_INDEX_DATA *room, short dir, int vnum ) );
ED *	get_exit_num	args ( ( ROOM_INDEX_DATA *room, short count ) );
ch_ret move_char ( CHAR_DATA *ch, EXIT_DATA *pexit, int fall, int direction );
void	teleport	args ( ( CHAR_DATA *ch, int room, int flags ) );
short	encumbrance	args ( ( CHAR_DATA *ch, short move ) );
bool	will_fall	args ( ( CHAR_DATA *ch, int fall ) );
ch_ret	pullcheck	args ( ( CHAR_DATA *ch, int pulse ) );
char *	rev_exit	args ( ( short vdir ) );

/* act_obj.c */

obj_ret	damage_obj	args ( ( OBJ_DATA *obj ) );
short	get_obj_resistance args ( ( OBJ_DATA *obj ) );
void    save_clan_storeroom args ( ( CHAR_DATA *ch, CLAN_DATA *clan ) );
void    obj_fall  	args ( ( OBJ_DATA *obj, bool through ) );

/* act_wiz.c */
bool create_new_race	args ( ( int rcindex, char *argument ) );
bool create_new_Class	args ( ( int rcindex, char *argument ) );
RID *	find_location	args ( ( CHAR_DATA *ch, char *arg ) );
void	echo_to_all	args ( ( short AT_COLOR, char *argument, short tar ) );
void   	get_reboot_string args ( ( void ) );
struct tm *update_time  args ( ( struct tm *old_time ) );
void	free_social	args ( ( SOCIALTYPE *social ) );
void	add_social	args ( ( SOCIALTYPE *social ) );
void	free_command	args ( ( CMDTYPE *command ) );
void	unlink_command	args ( ( CMDTYPE *command ) );
void	add_command	args ( ( CMDTYPE *command ) );



/* build.c */
int     get_cmdflag     args ( ( char *flag ) );
char *	flag_string	args ( ( int bitvector, char * const flagarray[] ) );
char *	ext_flag_string	args ( ( EXT_BV *bitvector, char * const flagarray[] ) );
int	get_mpflag	args ( ( char *flag ) );
int	get_dir		args ( ( char *txt ) );
char *	strip_cr	args ( ( char *str ) );

char *sprint_reset ( RESET_DATA *pReset, short *num );
void fix_exits ( void );


/* clans.c */
CL *	get_clan	args ( ( char *name ) );
void	load_clans	args ( ( void ) );
void	save_clan	args ( ( CLAN_DATA *clan ) );

/* deity.c */
DE *	get_deity	args ( ( char *name ) );
void	load_deity	args ( ( void ) );
void	save_deity	args ( ( DEITY_DATA *deity ) );

/* comm.c */
char *  smaug_crypt ( const char *pwd );
void	close_socket	args ( ( DESCRIPTOR_DATA *dclose, bool force ) );
void	write_to_buffer	args ( ( DESCRIPTOR_DATA *d, const char *txt, int length ) );
void    write_to_pager ( DESCRIPTOR_DATA *d, const char *txt, int length );
void	send_to_char	args ( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_char_color	args ( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager	args ( ( const char *txt, CHAR_DATA *ch ) );
void	send_to_pager_color	args ( ( const char *txt, CHAR_DATA *ch ) );
void    ch_printf ( CHAR_DATA * ch, char *fmt, ... ) __attribute__ ( ( format ( printf, 2, 3 ) ) );
void 	ch_printf_color ( CHAR_DATA *ch, char *fmt, ... );
void 	pager_printf ( CHAR_DATA *ch, char *fmt, ... );
void 	pager_printf_color ( CHAR_DATA *ch, char *fmt, ... );
void	act	args ( ( short AType, const char *format, CHAR_DATA *ch, void *arg1, void *arg2, int type ) );
char *	myobj	args ( ( OBJ_DATA *obj ) );
void send_to_desc_color ( const char *txt, DESCRIPTOR_DATA *d );
void send_to_desc_color_args ( DESCRIPTOR_DATA * d, char *fmt, ... );
char *sha256_crypt ( const char *pwd );

/* reset.c */
RD  *	make_reset	args ( ( char letter, int extra, int arg1, int arg2, int arg3 ) );
RD *add_reset ( ROOM_INDEX_DATA *room, char letter, int extra, int arg1, int arg2, int arg3 );
void	reset_area	args ( ( AREA_DATA * pArea ) );

/* db.c */
void	show_file	args ( ( CHAR_DATA *ch, char *filename ) );
char *	str_dup		args ( ( char const *str ) );
void	boot_db		args ( ( bool fCopyOver ) );
void	area_update	args ( ( void ) );
void	add_char	args ( ( CHAR_DATA *ch ) );
CD *	create_mobile	args ( ( MOB_INDEX_DATA *pMobIndex ) );
OD *	create_object	args ( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clear_char	args ( ( CHAR_DATA *ch ) );
void	free_char	args ( ( CHAR_DATA *ch ) );
char *	get_extra_descr	args ( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index	args ( ( int vnum ) );
OID *	get_obj_index	args ( ( int vnum ) );
RID *	get_room_index	args ( ( int vnum ) );
char	fread_letter	args ( ( FILE *fp ) );
int	fread_number	args ( ( FILE *fp ) );
EXT_BV  fread_bitvector	args ( ( FILE *fp ) );
void	fwrite_bitvector args ( ( EXT_BV *bits, FILE *fp ) );
char *	print_bitvector	args ( ( EXT_BV *bits ) );
char *	fread_string	args ( ( FILE *fp ) );
char *	fread_string_nohash args ( ( FILE *fp ) );
void	fread_to_eol	args ( ( FILE *fp ) );
char *	fread_word	args ( ( FILE *fp ) );
char *	fread_line	args ( ( FILE *fp ) );
int	number_fuzzy	args ( ( int number ) );
int	number_range	args ( ( int from, int to ) );
int	number_percent	args ( ( void ) );
int	number_door	args ( ( void ) );
int	number_bits	args ( ( int width ) );
int	number_mm	args ( ( void ) );
int	dice		args ( ( int number, int size ) );
int	interpolate	args ( ( int level, int value_00, int value_32 ) );
void	smash_tilde	args ( ( char *str ) );
void	hide_tilde	args ( ( char *str ) );
char *	show_tilde	args ( ( char *str ) );
bool	str_cmp		args ( ( const char *astr, const char *bstr ) );
bool	str_prefix	args ( ( const char *astr, const char *bstr ) );
bool	str_infix	args ( ( const char *astr, const char *bstr ) );
bool	str_suffix	args ( ( const char *astr, const char *bstr ) );
char *	capitalize	args ( ( const char *str ) );
char *	strlower	args ( ( const char *str ) );
char *	strupper	args ( ( const char *str ) );
char *  aoran		args ( ( const char *str ) );
void	append_file	args ( ( CHAR_DATA *ch, char *file, char *str ) );
void	append_to_file	args ( ( char *file, char *str ) );
void	bug		args ( ( const char *str, ... ) );
void	log_string_plus	args ( ( const char *str, short log_type, short level ) );
RID *make_room ( int vnum, AREA_DATA *area );
OID *	make_object	args ( ( int vnum, int cvnum, char *name ) );
MID *	make_mobile	args ( ( int vnum, int cvnum, char *name, int level ) );
ED  *	make_exit	args ( ( ROOM_INDEX_DATA *pRoomIndex, ROOM_INDEX_DATA *to_room, short door ) );
void	add_help	args ( ( HELP_DATA *pHelp ) );
void	fix_area_exits	args ( ( AREA_DATA *tarea ) );
void	load_area_file	args ( ( AREA_DATA *tarea, char *filename ) );
void	randomize_exits	args ( ( ROOM_INDEX_DATA *room, short maxdir ) );
void	make_wizlist	args ( ( void ) );
void	tail_chain	args ( ( void ) );
void    delete_room     args ( ( ROOM_INDEX_DATA *room ) );
void    delete_obj      args ( ( OBJ_INDEX_DATA *obj ) );
void    delete_mob      args ( ( MOB_INDEX_DATA *mob ) );
/* Functions to add to sorting lists. -- Altrag */
void	sort_area	args ( ( AREA_DATA *pArea, bool proto ) );
void    sort_area_by_name  args ( ( AREA_DATA *pArea ) ); /* Fireblade */


/* build.c */
void RelCreate ( relation_type, void *, void * );
void RelDestroy ( relation_type, void *, void * );
void	start_editing	args ( ( CHAR_DATA *ch, char *data ) );
void	stop_editing	args ( ( CHAR_DATA *ch ) );
void	edit_buffer	args ( ( CHAR_DATA *ch, char *argument ) );
char *copy_buffer	args ( ( CHAR_DATA *ch ) );
char *copy_buffer_nohash ( CHAR_DATA *ch );
bool	can_rmodify	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room ) );
bool	can_omodify	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_mmodify	args ( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
bool	can_medit	args ( ( CHAR_DATA *ch, MOB_INDEX_DATA *mob ) );
void	free_reset	args ( ( AREA_DATA *are, RESET_DATA *res ) );
void	free_area	args ( ( AREA_DATA *are ) );
void	assign_area	args ( ( CHAR_DATA *ch ) );
EDD *	SetRExtra	args ( ( ROOM_INDEX_DATA *room, char *keywords ) );
bool	DelRExtra	args ( ( ROOM_INDEX_DATA *room, char *keywords ) );
EDD *	SetOExtra	args ( ( OBJ_DATA *obj, char *keywords ) );
bool	DelOExtra	args ( ( OBJ_DATA *obj, char *keywords ) );
EDD *	SetOExtraProto	args ( ( OBJ_INDEX_DATA *obj, char *keywords ) );
bool	DelOExtraProto	args ( ( OBJ_INDEX_DATA *obj, char *keywords ) );
void	fold_area	args ( ( AREA_DATA *tarea, char *filename, bool install ) );
int	get_otype	args ( ( char *type ) );
int	get_atype	args ( ( char *type ) );
int	get_aflag	args ( ( char *flag ) );
int	get_oflag	args ( ( char *flag ) );
int	get_wflag	args ( ( char *flag ) );
void	init_area_weather args ( ( void ) );
void	save_weatherdata args ( ( void ) );

/* fight.c */
int	max_fight	args ( ( CHAR_DATA *ch ) );
void	violence_update	args ( ( void ) );
ch_ret	multi_hit	args ( ( CHAR_DATA *ch, CHAR_DATA *victim, int dt ) );
ch_ret	projectile_hit	args ( ( CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *wield,
                               OBJ_DATA *projectile, short dist ) );
short	ris_damage	args ( ( CHAR_DATA *ch, short dam, int ris ) );
ch_ret	damage		args ( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,
                        int dt ) );
void	update_pos	args ( ( CHAR_DATA *victim ) );
void	set_fighting	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	stop_fighting	args ( ( CHAR_DATA *ch, bool fBoth ) );
void	free_fight	args ( ( CHAR_DATA *ch ) );
CD *	who_fighting	args ( ( CHAR_DATA *ch ) );
void	check_killer	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	check_attacker	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	death_cry	args ( ( CHAR_DATA *ch ) );
void	stop_hunting	args ( ( CHAR_DATA *ch ) );
void	stop_hating	args ( ( CHAR_DATA *ch ) );
void	stop_fearing	args ( ( CHAR_DATA *ch ) );
void	start_hunting	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_hating	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	start_fearing	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hunting	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_hating	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_fearing	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	is_safe		args ( ( CHAR_DATA *ch, CHAR_DATA *victim, bool SHOW ) );
bool	legal_loot	args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
short	VAMP_AC		args ( ( CHAR_DATA *ch ) );
bool    check_illegal_pk args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void    raw_kill        args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	in_arena	args ( ( CHAR_DATA *ch ) );

/* makeobjs.c */
void	make_corpse	args ( ( CHAR_DATA *ch, CHAR_DATA *killer ) );
void	make_drops	args ( ( CHAR_DATA *ch ) );
void	make_bloodstain args ( ( CHAR_DATA *ch ) );
void	make_scraps	args ( ( OBJ_DATA *obj ) );
void	make_fire	args ( ( ROOM_INDEX_DATA *in_room, short timer ) );
OD *	create_money	args ( ( int amount ) );

/* mapper.c */
size_t mudstrlcpy ( char *dst, const char *src, size_t siz );
void draw_map ( CHAR_DATA *ch, const char *desc );
char * roomdesc ( CHAR_DATA *ch );

/* misc.c */
void actiondesc args ( ( CHAR_DATA *ch, OBJ_DATA *obj, void *vo ) );
EXT_BV	meb		args ( ( int bit ) );
EXT_BV	multimeb	args ( ( int bit, ... ) );


/* deity.c */
void adjust_favor	args ( ( CHAR_DATA *ch, int field, int mod ) );

/* mud_comm.c */
char *	mprog_type_to_name	args ( ( int type ) );

/* mud_prog.c */
#ifdef DUNNO_STRSTR
char *  strstr                  args ( ( const char *s1, const char *s2 ) );
#endif

void	mprog_wordlist_check    args ( ( char * arg, CHAR_DATA *mob,
                                      CHAR_DATA* actor, OBJ_DATA* object,
                                      void* vo, int type ) );
void	mprog_percent_check     args ( ( CHAR_DATA *mob, CHAR_DATA* actor,
                                      OBJ_DATA* object, void* vo,
                                      int type ) );
void	mprog_act_trigger       args ( ( char* buf, CHAR_DATA* mob,
                                      CHAR_DATA* ch, OBJ_DATA* obj,
                                      void* vo ) );
void	mprog_bribe_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                      int amount ) );
void	mprog_entry_trigger     args ( ( CHAR_DATA* mob ) );
void	mprog_give_trigger      args ( ( CHAR_DATA* mob, CHAR_DATA* ch,
                                      OBJ_DATA* obj ) );
void	mprog_greet_trigger     args ( ( CHAR_DATA* mob ) );
void    mprog_fight_trigger     args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_hitprcnt_trigger  args ( ( CHAR_DATA* mob, CHAR_DATA* ch ) );
void    mprog_death_trigger     args ( ( CHAR_DATA *killer, CHAR_DATA* mob ) );
void    mprog_random_trigger    args ( ( CHAR_DATA* mob ) );
void    mprog_speech_trigger    args ( ( char* txt, CHAR_DATA* mob ) );
void    mprog_script_trigger    args ( ( CHAR_DATA *mob ) );
void    mprog_hour_trigger      args ( ( CHAR_DATA *mob ) );
void    mprog_time_trigger      args ( ( CHAR_DATA *mob ) );
void    progbug                 args ( ( char *str, CHAR_DATA *mob ) );
void	rset_supermob		args ( ( ROOM_INDEX_DATA *room ) );
void	release_supermob	args ( ( void ) );
void mpsleep_update args ( ( ) );

/* player.c */
void	set_title	args ( ( CHAR_DATA *ch, char *title ) );

/* skills.c */
bool    can_use_skill           args ( ( CHAR_DATA *ch, int percent, int gsn ) );
bool	check_skill		args ( ( CHAR_DATA *ch, char *command, char *argument ) );
void	learn_from_success	args ( ( CHAR_DATA *ch, int sn ) );
void	learn_from_failure	args ( ( CHAR_DATA *ch, int sn ) );
bool	check_parry		args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	check_dodge		args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool    check_tumble            args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	check_grip		args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	disarm			args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
void	trip			args ( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	mob_fire		args ( ( CHAR_DATA *ch, char *name ) );
CD *	scan_for_victim		args ( ( CHAR_DATA *ch, EXIT_DATA *pexit, char *name ) );

/* ban.c */
int add_ban args ( ( CHAR_DATA *ch, char *arg1, char *arg2,int btime,int type ) );
void show_bans args ( ( CHAR_DATA *ch, int type ) );
void save_banlist args ( ( void ) );
void load_banlist args ( ( void ) );
bool check_total_bans args ( ( DESCRIPTOR_DATA *d ) );
bool check_bans args ( ( CHAR_DATA *ch, int type ) );

/* handler.c */
AREA_DATA *  get_area_obj args ( ( OBJ_INDEX_DATA * obj ) );
int	get_exp		args ( ( CHAR_DATA *ch ) );
int	get_exp_worth	args ( ( CHAR_DATA *ch ) );
int	exp_level	args ( ( CHAR_DATA *ch, short level ) );
short	get_trust	args ( ( CHAR_DATA *ch ) );
short	get_age		args ( ( CHAR_DATA *ch ) );
short	get_curr_str	args ( ( CHAR_DATA *ch ) );
short	get_curr_int	args ( ( CHAR_DATA *ch ) );
short	get_curr_wis	args ( ( CHAR_DATA *ch ) );
short	get_curr_dex	args ( ( CHAR_DATA *ch ) );
short	get_curr_con	args ( ( CHAR_DATA *ch ) );
short	get_curr_cha	args ( ( CHAR_DATA *ch ) );
short  get_curr_lck	args ( ( CHAR_DATA *ch ) );
bool	can_take_proto	args ( ( CHAR_DATA *ch ) );
int	can_carry_n	args ( ( CHAR_DATA *ch ) );
int	can_carry_w	args ( ( CHAR_DATA *ch ) );
bool	is_name		args ( ( const char *str, char *namelist ) );
bool	is_name_prefix	args ( ( const char *str, char *namelist ) );
bool	nifty_is_name	args ( ( char *str, char *namelist ) );
bool	nifty_is_name_prefix args ( ( char *str, char *namelist ) );
void	affect_modify	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf, bool fAdd ) );
void	affect_to_char	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_remove	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	affect_strip	args ( ( CHAR_DATA *ch, int sn ) );
bool	is_affected	args ( ( CHAR_DATA *ch, int sn ) );
void	affect_join	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	char_from_room	args ( ( CHAR_DATA *ch ) );
void	char_to_room	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
OD *	obj_to_char	args ( ( OBJ_DATA *obj, CHAR_DATA *ch ) );
void	obj_from_char	args ( ( OBJ_DATA *obj ) );
int	apply_ac	args ( ( OBJ_DATA *obj, int iWear ) );
OD *	get_eq_char	args ( ( CHAR_DATA *ch, int iWear ) );
void	equip_char	args ( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
int	count_obj_list	args ( ( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room	args ( ( OBJ_DATA *obj ) );
OD *obj_to_room ( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex, CHAR_DATA *ch );
OD *	obj_to_obj	args ( ( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj	args ( ( OBJ_DATA *obj ) );
void	extract_obj	args ( ( OBJ_DATA *obj ) );
void	extract_exit	args ( ( ROOM_INDEX_DATA *room, EXIT_DATA *pexit ) );
void	extract_room	args ( ( ROOM_INDEX_DATA *room ) );
void	clean_room	args ( ( ROOM_INDEX_DATA *room ) );
void	clean_obj	args ( ( OBJ_INDEX_DATA *obj ) );
void	clean_mob	args ( ( MOB_INDEX_DATA *mob ) );
void clean_resets ( ROOM_INDEX_DATA *room );
void	extract_char	args ( ( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room	args ( ( CHAR_DATA *ch, char *argument ) );
CD *	get_char_world	args ( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type	args ( ( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list	args ( ( CHAR_DATA *ch, char *argument,
                           OBJ_DATA *list ) );
OD *	get_obj_list_rev args ( ( CHAR_DATA *ch, char *argument,
                               OBJ_DATA *list ) );
OD *	get_obj_carry	args ( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_wear	args ( ( CHAR_DATA *ch, char *argument ) );
OD *    get_obj_vnum    args ( ( CHAR_DATA *ch, int vnum ) );
OD *	get_obj_here	args ( ( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_world	args ( ( CHAR_DATA *ch, char *argument ) );
int	get_obj_number	args ( ( OBJ_DATA *obj ) );
int	get_obj_weight	args ( ( OBJ_DATA *obj ) );
int	get_real_obj_weight args ( ( OBJ_DATA *obj ) );
bool	room_is_dark	args ( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	room_is_private	args ( ( ROOM_INDEX_DATA *pRoomIndex ) );
CD	*room_is_dnd	args ( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex ) );
bool can_see ( CHAR_DATA *ch, CHAR_DATA *victim, bool override );
bool	can_see_obj	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_drop_obj	args ( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	item_type_name	args ( ( OBJ_DATA *obj ) );
char *	affect_loc_name	args ( ( int location ) );
char *	affect_bit_name	args ( ( EXT_BV *vector ) );
char *	extra_bit_name	args ( ( EXT_BV *extra_flags ) );
char *	magic_bit_name	args ( ( int magic_flags ) );
char *  pull_type_name	args ( ( int pulltype ) );
void	name_stamp_stats args ( ( CHAR_DATA *ch ) );
void	fix_char	args ( ( CHAR_DATA *ch ) );
void	showaffect	args ( ( CHAR_DATA *ch, AFFECT_DATA *paf ) );
void	set_cur_obj	args ( ( OBJ_DATA *obj ) );
bool	obj_extracted	args ( ( OBJ_DATA *obj ) );
void	queue_extracted_obj	args ( ( OBJ_DATA *obj ) );
void	clean_obj_queue	args ( ( void ) );
void	set_cur_char	args ( ( CHAR_DATA *ch ) );
bool	char_died	args ( ( CHAR_DATA *ch ) );
void	queue_extracted_char	args ( ( CHAR_DATA *ch, bool extract ) );
void	clean_char_queue	args ( ( void ) );
void	add_timer	args ( ( CHAR_DATA *ch, short type, int count, DO_FUN *fun, int value ) );
TIMER * get_timerptr	args ( ( CHAR_DATA *ch, short type ) );
short	get_timer	args ( ( CHAR_DATA *ch, short type ) );
void	extract_timer	args ( ( CHAR_DATA *ch, TIMER *timer ) );
void	remove_timer	args ( ( CHAR_DATA *ch, short type ) );
bool	in_soft_range	args ( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	in_hard_range	args ( ( CHAR_DATA *ch, AREA_DATA *tarea ) );
bool	chance  	args ( ( CHAR_DATA *ch, short percent ) );
bool 	chance_attrib	args ( ( CHAR_DATA *ch, short percent, short attrib ) );
OD *	clone_object	args ( ( OBJ_DATA *obj ) );
void	split_obj	args ( ( OBJ_DATA *obj, int num ) );
void	separate_obj	args ( ( OBJ_DATA *obj ) );
bool	empty_obj	args ( ( OBJ_DATA *obj, OBJ_DATA *destobj,
                        ROOM_INDEX_DATA *destroom ) );
OD *	find_obj	args ( ( CHAR_DATA *ch, char *argument,
                       bool carryonly ) );
void	worsen_mental_state args ( ( CHAR_DATA *ch, int mod ) );
void	better_mental_state args ( ( CHAR_DATA *ch, int mod ) );
void	boost_economy	args ( ( AREA_DATA *tarea, int gold ) );
void	lower_economy	args ( ( AREA_DATA *tarea, int gold ) );
void	economize_mobgold args ( ( CHAR_DATA *mob ) );
bool	economy_has	args ( ( AREA_DATA *tarea, int gold ) );
void	add_kill	args ( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
int	times_killed	args ( ( CHAR_DATA *ch, CHAR_DATA *mob ) );
void	update_aris	args ( ( CHAR_DATA *ch ) );
AREA_DATA *get_area	args ( ( char *name ) ); /* FB */
OD *	get_objtype	args ( ( CHAR_DATA *ch, short type ) );
char *  tattoo_bit_name  args ( ( int tatto_flags ) );
CD *	get_mob		args ( ( int vnum ) );
OD *	obj_by_vnum	args ( ( int vnum ) );

/*infochan.c*/
void talk_info ( short AT_COLOR, char *argument, bool sound );

/* interp.c */
bool	check_pos	args ( ( CHAR_DATA *ch, short position ) );
void	interpret	args ( ( CHAR_DATA *ch, char *argument ) );
bool	is_number	args ( ( char *arg ) );
int	number_argument	args ( ( char *argument, char *arg ) );
char *	one_argument	args ( ( char *argument, char *arg_first ) );
char *	one_argument2	args ( ( char *argument, char *arg_first ) );
ST *	find_social	args ( ( char *command ) );
CMDTYPE *find_command	args ( ( char *command ) );
void	hash_commands	args ( ( void ) );
void	start_timer	args ( ( struct timeval *sttime ) );
time_t	end_timer	args ( ( struct timeval *sttime ) );
void	send_timer	args ( ( struct timerset *vtime, CHAR_DATA *ch ) );
void	update_userec	args ( ( struct timeval *time_used, struct timerset *userec ) );

/* magic.c */
bool	process_spell_components args ( ( CHAR_DATA *ch, int sn ) );
int	ch_slookup	args ( ( CHAR_DATA *ch, const char *name ) );
int	find_spell	args ( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_skill	args ( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_weapon	args ( ( CHAR_DATA *ch, const char *name, bool know ) );
int	find_tongue	args ( ( CHAR_DATA *ch, const char *name, bool know ) );
int	skill_lookup	args ( ( const char *name ) );
int	herb_lookup	args ( ( const char *name ) );
int	personal_lookup	args ( ( CHAR_DATA *ch, const char *name ) );
int	slot_lookup	args ( ( int slot ) );
int	bsearch_skill	args ( ( const char *name, int first, int top ) );
int	bsearch_skill_exact args ( ( const char *name, int first, int top ) );
int	bsearch_skill_prefix args ( ( const char *name, int first, int top ) );
bool	saves_poison_death	args ( ( CHAR_DATA *victim ) );
bool 	saves_mental 		args ( ( CHAR_DATA *victim ) );
bool	saves_physical		args ( ( CHAR_DATA *victim ) );
bool	saves_weapons		args ( ( CHAR_DATA *victim ) );
ch_ret	obj_cast_spell	args ( ( int sn, int level, CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj ) );
int	dice_parse	args ( ( CHAR_DATA *ch, int level, char *texp ) );
SK *	get_skilltype	args ( ( int sn ) );
short	get_chain_type	args ( ( ch_ret retcode ) );
ch_ret	chain_spells	args ( ( int sn, int level, CHAR_DATA *ch, void *vo, short chain ) );
void immune_casting ( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );
void failed_casting ( struct skill_type *skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );
void successful_casting ( SKILLTYPE * skill, CHAR_DATA * ch, CHAR_DATA * victim, OBJ_DATA * obj );
int ris_save ( CHAR_DATA * ch, int schance, int ris );



/* random.*  */
char *create_weather_string ( CHAR_DATA *ch, char *weather_string );
char *create_time_string ( CHAR_DATA *ch, char *time_string );

/* request.c */
void	init_request_pipe	args ( ( void ) );
void	check_requests		args ( ( void ) );

/* save.c */
/* object saving defines for fread/write_obj. -- Altrag */
#define OS_CARRY	0
#define OS_CORPSE	1
#define OS_LOCKER       2

void	save_char_obj	args ( ( CHAR_DATA *ch ) );
bool	load_char_obj	args ( ( DESCRIPTOR_DATA *d, char *name, bool preload, bool copyover ) );
void	set_alarm	args ( ( long seconds ) );
void	requip_char	args ( ( CHAR_DATA *ch ) );
void    fwrite_obj args ( ( CHAR_DATA *ch, OBJ_DATA *obj, FILE *fp, int iNest, short os_type, bool hotboot ) );
void	fread_obj	args ( ( CHAR_DATA *ch,  FILE *fp, short os_type ) );
void	de_equip_char	args ( ( CHAR_DATA *ch ) );
void	re_equip_char	args ( ( CHAR_DATA *ch ) );
void    read_char_mobile args ( ( char *argument ) );
void    write_char_mobile args ( ( CHAR_DATA *ch, char *argument ) );
CHAR_DATA * fread_mobile args ( ( FILE *fp ) );
void    fwrite_mobile	args ( ( FILE *fp, CHAR_DATA *mob ) );

/* shops.c */

/* special.c */
SF *	spec_lookup	args ( ( const char *name ) );
char *	lookup_spec	args ( ( SPEC_FUN *special ) );

/* tables.c */
int	get_skill	args ( ( char *skilltype ) );
char *	spell_name	args ( ( SPELL_FUN *spell ) );
char *	skill_name	args ( ( DO_FUN *skill ) );
void	load_skill_table args ( ( void ) );
void	save_skill_table args ( ( void ) );
void	sort_skill_table args ( ( void ) );
void	remap_slot_numbers args ( ( void ) );
void	load_socials	args ( ( void ) );
void	save_socials	args ( ( void ) );
void	load_commands	args ( ( void ) );
void	save_commands	args ( ( void ) );
SPELL_FUN *spell_function args ( ( char *name ) );
DO_FUN *skill_function  args ( ( char *name ) );
void	write_Class_file args ( ( int cl ) );
void	save_Classes	args ( ( void ) );
void	load_Classes	args ( ( void ) );
void	load_herb_table	args ( ( void ) );
void	save_herb_table	args ( ( void ) );
void	load_races	args ( ( void ) );
void	load_tongues	args ( ( void ) );
void read_last_file args ( ( CHAR_DATA *ch, int count, char *name ) );
void write_last_file args ( ( char *entry ) );

/* update.c */
void	advance_level	args ( ( CHAR_DATA *ch ) );
void	gain_exp	args ( ( CHAR_DATA *ch, int gain ) );
void    check_alignment args ( ( CHAR_DATA *ch ) );
void	update_handler	args ( ( void ) );
void    auction_update  args ( ( void ) );
void	remove_portal	args ( ( OBJ_DATA *portal ) );
void	weather_update	args ( ( void ) );

/* hashstr.c */
char *	str_alloc	args ( ( const char *str ) );
char *	quick_link	args ( ( char *str ) );
int	str_free	args ( ( char *str ) );
void	show_hash	args ( ( int count ) );
char *	hash_stats	args ( ( void ) );
char *	check_hash	args ( ( char *str ) );
void	hash_dump	args ( ( int hash ) );
void	show_high_hash	args ( ( int top ) );

/* newscore.c */
char *  get_Class 	args ( ( CHAR_DATA *ch ) );
char *  get_race 	args ( ( CHAR_DATA *ch ) );


#undef	SK
#undef	CO
#undef	ST
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef	BD
#undef	CL
#undef	EDD
#undef	RD
#undef	ED

/*
 * defines for use with this get_affect function
 */

#define RIS_000		BV00
#define RIS_R00		BV01
#define RIS_0I0		BV02
#define RIS_RI0		BV03
#define RIS_00S		BV04
#define RIS_R0S		BV05
#define RIS_0IS		BV06
#define RIS_RIS		BV07

#define GA_AFFECTED	BV09
#define GA_RESISTANT	BV10
#define GA_IMMUNE	BV11
#define GA_SUSCEPTIBLE	BV12
#define GA_RIS          BV30

/*
 * mudprograms stuff
 */
extern	CHAR_DATA *supermob;

void oprog_speech_trigger ( char *txt, CHAR_DATA *ch );
void oprog_random_trigger ( OBJ_DATA *obj );
void oprog_wear_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
bool oprog_use_trigger ( CHAR_DATA *ch, OBJ_DATA *obj, CHAR_DATA *vict, OBJ_DATA *targ, void *vo );
void oprog_remove_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_sac_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_damage_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_repair_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_drop_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_zap_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
char *oprog_type_to_name ( int type );
void oprog_greet_trigger ( CHAR_DATA *ch );
void oprog_get_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_examine_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_pull_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );
void oprog_push_trigger ( CHAR_DATA *ch, OBJ_DATA *obj );

/* mud prog defines */

#define ERROR_PROG        -1
#define IN_FILE_PROG      -2

typedef enum
{
	ACT_PROG, SPEECH_PROG, RAND_PROG, FIGHT_PROG, DEATH_PROG, HITPRCNT_PROG,
	ENTRY_PROG, GREET_PROG, ALL_GREET_PROG, GIVE_PROG, BRIBE_PROG, HOUR_PROG,
	TIME_PROG, WEAR_PROG, REMOVE_PROG, SAC_PROG, LOOK_PROG, EXA_PROG, ZAP_PROG,
	GET_PROG, DROP_PROG, DAMAGE_PROG, REPAIR_PROG, RANDIW_PROG, SPEECHIW_PROG,
	PULL_PROG, PUSH_PROG, SLEEP_PROG, REST_PROG, LEAVE_PROG, SCRIPT_PROG,
	USE_PROG
} prog_types;

/*
 * For backwards compatability
 */
#define RDEATH_PROG DEATH_PROG
#define ENTER_PROG  ENTRY_PROG
#define RFIGHT_PROG FIGHT_PROG
#define RGREET_PROG GREET_PROG
#define OGREET_PROG GREET_PROG

void rprog_leave_trigger ( CHAR_DATA *ch );
void rprog_enter_trigger ( CHAR_DATA *ch );
void rprog_sleep_trigger ( CHAR_DATA *ch );
void rprog_rest_trigger ( CHAR_DATA *ch );
void rprog_rfight_trigger ( CHAR_DATA *ch );
void rprog_death_trigger ( CHAR_DATA *killer, CHAR_DATA *ch );
void rprog_speech_trigger ( char *txt, CHAR_DATA *ch );
void rprog_random_trigger ( CHAR_DATA *ch );
void rprog_time_trigger ( CHAR_DATA *ch );
void rprog_hour_trigger ( CHAR_DATA *ch );
char *rprog_type_to_name ( int type );

#define OPROG_ACT_TRIGGER
#ifdef OPROG_ACT_TRIGGER
void oprog_act_trigger ( char *buf, OBJ_DATA *mobj, CHAR_DATA *ch,
                         OBJ_DATA *obj, void *vo );
#endif
#define RPROG_ACT_TRIGGER
#ifdef RPROG_ACT_TRIGGER
void rprog_act_trigger ( char *buf, ROOM_INDEX_DATA *room, CHAR_DATA *ch,
                         OBJ_DATA *obj, void *vo );
#endif



