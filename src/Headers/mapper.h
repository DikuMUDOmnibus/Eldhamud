/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                                                                           *
 ****************************************************************************/
/* Defines for ASCII Automapper */

#define MAPX     10
#define MAPY      8
/* You can change MAXDEPTH to 1 if the diagonal directions are confusing */
#define MAXDEPTH  2

#define BOUNDARY(x, y) (((x) < 0) || ((y) < 0) || ((x) > MAPX) || ((y) > MAPY))

typedef struct  map_type                MAP_TYPE;

/* Structure for the map itself */
struct map_type
{
  char   tegn;  /* Character to print at this map coord */
  int    vnum;  /* Room this coord represents */
  int    depth; /* Recursive depth this coord was found at */
  EXT_BV    info;
  bool   can_see;
};

/* mapper.c */
char *get_exits(CHAR_DATA *ch);


/* act_info.c */
bool check_blind( CHAR_DATA *ch );


