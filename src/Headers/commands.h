/****************************************************************************
 *			Eldhamud Codebase V2.2				    *
 * ------------------------------------------------------------------------ *
 *          EldhaMUD code (C) 2003-2008 by Robert Powell (Tommi)            *
 * ------------------------------------------------------------------------ *
 *                                                                           *
 ****************************************************************************/
#define MAX_TRAVEL_LOCATION		8

struct travel_data
{
    char        * keyword;
    short	map;
    short	x;
    short	y;
    short	cost;
};

const struct travel_data travel_table[] =
{
        {	"yagonza",	3, 	50, 	50,	100    },
        {	"bangoin", 	5, 	50, 	50,	100	},
        {	"nyemo",	1, 	50, 	50,	100	},
	{	"zogang", 	7, 	50, 	50,	100	},
	{	"sangang",	0, 	50, 	50,	100    },
        {	"jaili", 	2, 	50, 	50,	100	},
        {	"konjo",	6, 	50, 	50,	100	},
	{	"tangmai", 	8, 	50, 	50,	100	},
	{	"NULL", 	0, 	0, 	0,	0	}
};

