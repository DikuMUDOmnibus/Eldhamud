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
 *			      Regular update module			    *
 ****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "./Headers/mud.h"
int global_exp;
int global_qp;
int global_quad;
short display;
short qpdisplay;
short quaddisplay;
extern void start_arena( );
extern void do_game( );
extern int in_start_arena;
extern int ppl_in_arena;
extern int ppl_challenged;
extern int num_in_arena( );
void mob_reset_map ( int map_number );
/*
 * Local functions.
 */
int hit_gain args ( ( CHAR_DATA * ch ) );
int mana_gain args ( ( CHAR_DATA * ch ) );
int move_gain args ( ( CHAR_DATA * ch ) );
void mobile_update args ( ( void ) );
void time_update args ( ( void ) ); /* FB */
void char_update args ( ( void ) );
void obj_update args ( ( void ) );
void aggr_update args ( ( void ) );
void room_act_update args ( ( void ) );
void obj_act_update args ( ( void ) );
void char_check args ( ( void ) );
void hallucinations args ( ( CHAR_DATA * ch ) );
void subtract_times args ( ( struct timeval * etime, struct timeval * sttime ) );
/* Overland Map movement - Samson 7-31-99 */
bool map_wander ( CHAR_DATA * ch, short map, short x, short y, short sector );
/* weather functions - FB */
void adjust_vectors args ( ( WEATHER_DATA * weather ) );
void get_weather_echo args ( ( WEATHER_DATA * weather ) );
void get_time_echo args ( ( WEATHER_DATA * weather ) );
void quest_update args ( ( void ) ); /* Questmaster */
void update_all_qobjs ( CHAR_DATA * ch );
void mud_recv_message( );
void update_bots ( CHAR_DATA * ch );
/*
 * Global Variables
 */
CHAR_DATA *gch_prev;
OBJ_DATA *gobj_prev;
CHAR_DATA *timechar;
char *corpse_descs[] =
{
	"The corpse of %s is in the last stages of decay.",
	"The corpse of %s is crawling with vermin.",
	"The corpse of %s fills the air with a foul stench.",
	"The corpse of %s is buzzing with flies.",
	"The corpse of %s lies here."
};
extern int top_exit;
/*
 * Advancement stuff.
 */
void advance_level ( CHAR_DATA * ch )
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	int add_hp;
	int add_mana;
	int add_move;
	int add_prac;
	
	update_all_qobjs ( ch );
	add_hp =   number_range ( Class_table[ch->Class]->hp_min, ( Class_table[ch->Class]->hp_max + get_curr_con ( ch ) ) );
	add_mana = number_range ( Class_table[ch->Class]->mana_min, 
		( Class_table[ch->Class]->mana_max + get_curr_int ( ch ) + get_curr_wis ( ch ) / 2 ) );
	add_move = ( number_range (Class_table[ch->Class]->move_min, Class_table[ch->Class]->move_max) +
			        ( get_curr_con ( ch ) + get_curr_dex ( ch ) ) / 2 );
	add_prac = ( get_curr_wis ( ch ) / 3 ) + 1;
	add_hp = UMAX ( 1, add_hp );
	add_mana = UMAX ( 0, add_mana );
	add_move = UMAX ( 10, add_move );
	
	ch->max_hit += add_hp;
	ch->max_mana += add_mana;
	ch->max_move += add_move;
	ch->practice += add_prac;
	if ( !IS_NPC ( ch ) )
		xREMOVE_BIT ( ch->act, PLR_BOUGHT_PET );
	if ( ch->level == LEVEL_AVATAR )
	{
		DESCRIPTOR_DATA *d;
		sprintf ( buf, "%s has just achieved Avatarhood!", ch->name );
		for ( d = first_descriptor; d; d = d->next )
			if ( d->connected == CON_PLAYING && d->character != ch )
			{
				set_char_color ( AT_IMMORT, d->character );
				send_to_char ( buf, d->character );
				send_to_char ( "\r\n", d->character );
			}
		set_char_color ( AT_WHITE, ch );
		do_help ( ch, "M_ADVHERO_" );
	}
	if ( ch->level < LEVEL_IMMORTAL )
	{
		sprintf ( buf, "Your gain is: %d/%d hp, %d/%d mana, %d/%d mv %d/%d prac.\r\n",
		          add_hp, ch->max_hit, add_mana, ch->max_mana, add_move, ch->max_move, add_prac, ch->practice );
		set_char_color ( AT_WHITE, ch );
		send_to_char ( buf, ch );
		if ( !IS_NPC ( ch ) )
		{
			sprintf ( buf2, "&G%-13s  ->&w%-2d  &G-&w  %-5d&G   Rvnum: %-5d   %s %s",
			          ch->name,
			          ch->level, get_age ( ch ), ch->in_room == NULL ? 0 : ch->in_room->vnum, capitalize ( race_table[ch->race]->race_name ), Class_table[ch->Class]->who_name );
			append_to_file ( PLEVEL_FILE, buf2 );
		}
	}
	return;
}
void gain_exp ( CHAR_DATA * ch, int gain )
{
	int modgain;
	char buf[MAX_STRING_LENGTH];
	if ( IS_NPC ( ch ) || ch->level >= LEVEL_AVATAR )
		return;
	
	modgain = gain;
	if ( modgain > 0 && IS_PKILL ( ch ) )
	{
		if ( !str_cmp ( ch->in_room->area->filename, "house.are" ) && ( ch->fighting ) )
			ch_printf ( ch, "You receive %d experience for your effort.\r\n", gain );
	}
	
	/*
	 * xp cap to prevent any one event from giving enuf xp to gain more than one level - FB
	 */
	modgain = UMIN ( modgain, ( exp_level ( ch, ch->level + 2 ) - 1 ) );
	ch->exp = UMAX ( 0, ch->exp + modgain );
	while ( ch->level < LEVEL_AVATAR && ch->exp >= exp_level ( ch, ch->level + 1 ) )
	{
		set_char_color ( AT_WHITE + AT_BLINK, ch );
		ch_printf ( ch, "You have now obtained experience level %d!\r\n", ++ch->level );
		sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c %s has now obtained experience level %d!\r\n", ch->name, ch->level );
		talk_info ( AT_GREEN, buf, TRUE );
		ch->exp = ( ch->exp - exp_level ( ch, ( ch->level ) ) );
		advance_level ( ch );
		affect_strip ( ch, gsn_poison );
		affect_strip ( ch, gsn_blindness );
		affect_strip ( ch, gsn_curse );
		ch->hit = ch->max_hit;
		ch->mana = ch->max_mana;
		ch->move = ch->max_move;
		update_pos ( ch );
		send_to_char ( "The God's have blessed your acheivement and given you the power to continue.", ch );
	}
	return;
}

/*
 * Regeneration stuff.
 */
int hit_gain ( CHAR_DATA * ch )
{
	int gain;
	if ( IS_NPC ( ch ) )
	{
		gain = ch->level * 3 / 2;
	}
	else
	{
		gain = UMIN ( 5, ch->level );
		switch ( ch->position )
		{
			case POS_DEAD:
				return 0;
			case POS_MORTAL:
				return -1;
			case POS_INCAP:
				return -1;
			case POS_STUNNED:
				return 1;
			case POS_SLEEPING:
				gain += get_curr_con ( ch ) * 2.0;
				break;
			case POS_RESTING:
				gain += get_curr_con ( ch ) * 1.25;
				break;
		}
	}
	if ( IS_AFFECTED ( ch, AFF_POISON ) )
		gain /= 4;
	if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && ( ch->on->value[3] > 0 ) )
		gain = gain * ch->on->value[3] / 100;
	return UMIN ( gain, ch->max_hit - ch->hit );
}

int mana_gain ( CHAR_DATA * ch )
{
	int gain;
	if ( IS_NPC ( ch ) )
	{
		gain = ch->level;
	}
	else
	{
		gain = UMIN ( 5, ch->level / 2 );
		if ( ch->position < POS_SLEEPING )
			return 0;
		switch ( ch->position )
		{
			case POS_SLEEPING:
				gain += get_curr_int ( ch ) * 3.25;
				break;
			case POS_RESTING:
				gain += get_curr_int ( ch ) * 1.75;
				break;
		}
	}
	if ( IS_AFFECTED ( ch, AFF_POISON ) )
		gain /= 4;
	if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && ( ch->on->value[3] > 0 ) )
		gain = gain * ch->on->value[3] / 100;
	return UMIN ( gain, ch->max_mana - ch->mana );
}

int move_gain ( CHAR_DATA * ch )
{
	int gain;
	if ( IS_NPC ( ch ) )
	{
		gain = ch->level;
	}
	else
	{
		gain = UMAX ( 15, 2 * ch->level );
		switch ( ch->position )
		{
			case POS_DEAD:
				return 0;
			case POS_MORTAL:
				return -1;
			case POS_INCAP:
				return -1;
			case POS_STUNNED:
				return 1;
			case POS_SLEEPING:
				gain += get_curr_dex ( ch ) * 4.5;
				break;
			case POS_RESTING:
				gain += get_curr_dex ( ch ) * 2.5;
				break;
		}
	}
	if ( IS_AFFECTED ( ch, AFF_POISON ) )
		gain /= 4;
	if ( ch->on != NULL && ch->on->item_type == ITEM_FURNITURE && ( ch->on->value[3] > 0 ) )
		gain = gain * ch->on->value[3] / 100;
	return UMIN ( gain, ch->max_move - ch->move );
}

/*
 * Put this in a seperate function so it isn't called three times per tick
 * This was added after a suggestion from Cronel	--Shaddai
 */
void check_alignment ( CHAR_DATA * ch )
{
	/*
	 *  Race alignment restrictions, h
	 */
	if ( ch->alignment < race_table[ch->race]->minalign )
	{
		set_char_color ( AT_BLOOD, ch );
		send_to_char ( "Your actions have been incompatible with the ideals of your race.  This troubles you.", ch );
	}
	if ( ch->alignment > race_table[ch->race]->maxalign )
	{
		set_char_color ( AT_BLOOD, ch );
		send_to_char ( "Your actions have been incompatible with the ideals of your race.  This troubles you.", ch );
	}
}

void pract_dummy_update ( void )
{
	CHAR_DATA *ch;
	
	for ( ch = first_char; ch; ch = ch->next  )
	{
		if( IS_NPC( ch ) )
			continue;
		if ( !str_cmp ( ch->in_room->area->filename, "house.are" ) && ( ch->fighting && ch->level < 30 ) )
		{
			gain_exp ( ch, 10 * ch->level );
		}
	}
	return;
}

/*
 * Mob autonomous action.
 * This function takes 25% to 35% of ALL Mud cpu time.
 */
void mobile_update ( void )
{
	char buf[MAX_STRING_LENGTH];
	CHAR_DATA *ch;
	EXIT_DATA *pexit;
	int door;
	ch_ret retcode;
	retcode = rNONE;
	/*
	 * Examine all mobs.
	 */
	for ( ch = last_char; ch; ch = gch_prev )
	{
		set_cur_char ( ch );
		if ( ch == first_char && ch->prev )
		{
			bug ( "mobile_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		if ( gch_prev && gch_prev->next != ch )
		{
			sprintf ( buf, "FATAL: Mobile_update: %s->prev->next doesn't point to ch.", ch->name );
			bug ( buf, 0 );
			bug ( "Short-cutting here", 0 );
			gch_prev = NULL;
			ch->prev = NULL;
			do_yell ( ch, "The_Fury says, 'Prepare for the worst!'" );
		}
		if ( !IS_NPC ( ch ) )
		{
			continue;
		}
		if ( !ch->in_room || IS_AFFECTED ( ch, AFF_CHARM ) || IS_AFFECTED ( ch, AFF_PARALYSIS ) )
			continue;
		/*
		 * Clean up 'animated corpses' that are not charmed' - Scryn
		 */
		if ( ch->pIndexData->vnum == 5 && !IS_AFFECTED ( ch, AFF_CHARM ) )
		{
			if ( ch->in_room->first_person )
				act ( AT_MAGIC, "$n returns to the dust from whence $e came.", ch, NULL, NULL, TO_ROOM );
			if ( IS_NPC ( ch ) ) /* Guard against purging switched? */
				extract_char ( ch, TRUE );
			continue;
		}

		/*
		 * Examine call for special procedure
		 */
		if ( !xIS_SET ( ch->act, ACT_RUNNING ) && ch->spec_fun )
		{
			if ( ( *ch->spec_fun ) ( ch ) )
				continue;
			if ( char_died ( ch ) )
				continue;
		}
		/*
		 * Check for mudprogram script on mob
		 */
		if ( HAS_PROG ( ch->pIndexData, SCRIPT_PROG ) )
		{
			mprog_script_trigger ( ch );
			continue;
		}
		if ( ch != cur_char )
		{
			bug ( "Mobile_update: ch != cur_char after spec_fun", 0 );
			continue;
		}
		/*
		 * That's all for sleeping / busy monster
		 */
		if ( ch->position != POS_STANDING )
			continue;
		if ( xIS_SET ( ch->act, ACT_MOUNTED ) )
		{
			if ( xIS_SET ( ch->act, ACT_AGGRESSIVE ) || xIS_SET ( ch->act, ACT_META_AGGR ) )
				do_emote ( ch, "snarls and growls." );
			continue;
		}
		if ( xIS_SET ( ch->in_room->room_flags, ROOM_SAFE ) && ( xIS_SET ( ch->act, ACT_AGGRESSIVE ) || xIS_SET ( ch->act, ACT_META_AGGR ) ) )
			do_emote ( ch, "glares around and snarls." );
		/*
		 * MOBprogram random trigger
		 */
		if ( ch->in_room->area->nplayer > 0 )
		{
			mprog_random_trigger ( ch );
			if ( char_died ( ch ) )
				continue;
			if ( ch->position < POS_STANDING )
				continue;
		}
		/*
		 * MOBprogram hour trigger: do something for an hour
		 */
		mprog_hour_trigger ( ch );
		if ( char_died ( ch ) )
			continue;
		rprog_hour_trigger ( ch );
		if ( char_died ( ch ) )
			continue;
		if ( ch->position < POS_STANDING )
			continue;
		/*
		 * Scavenge
		 */
		if ( xIS_SET ( ch->act, ACT_SCAVENGER ) && ch->in_room->first_content && number_bits ( 2 ) == 0 )
		{
			OBJ_DATA *obj;
			OBJ_DATA *obj_best;
			int max;
			max = 1;
			obj_best = NULL;
			for ( obj = ch->in_room->first_content; obj; obj = obj->next_content )
			{
				if ( CAN_WEAR ( obj, ITEM_TAKE ) && obj->cost > max && !IS_OBJ_STAT ( obj, ITEM_BURIED ) )
				{
					obj_best = obj;
					max = obj->cost;
				}
			}
			if ( obj_best )
			{
				obj_from_room ( obj_best );
				obj_to_char ( obj_best, ch );
				act ( AT_ACTION, "$n gets $p.", ch, obj_best, NULL, TO_ROOM );
			}
		}
		if ( IS_ACT_FLAG ( ch, ACT_ONMAP ) )
		{
			short sector = get_terrain ( ch->map, ch->x, ch->y );
			short map = ch->map;
			short x = ch->x;
			short y = ch->y;
			short dir = number_bits ( 5 );
			if ( dir < DIR_SOMEWHERE && dir != DIR_UP && dir != DIR_DOWN )
			{
				switch ( dir )
				{
					case DIR_NORTH:
						if ( map_wander ( ch, map, x, y - 1, sector ) )
							move_char ( ch, NULL, 0, DIR_NORTH );
						break;
					case DIR_NORTHEAST:
						if ( map_wander ( ch, map, x + 1, y - 1, sector ) )
							move_char ( ch, NULL, 0, DIR_NORTHEAST );
						break;
					case DIR_EAST:
						if ( map_wander ( ch, map, x + 1, y, sector ) )
							move_char ( ch, NULL, 0, DIR_EAST );
						break;
					case DIR_SOUTHEAST:
						if ( map_wander ( ch, map, x + 1, y + 1, sector ) )
							move_char ( ch, NULL, 0, DIR_SOUTHEAST );
						break;
					case DIR_SOUTH:
						if ( map_wander ( ch, map, x, y + 1, sector ) )
							move_char ( ch, NULL, 0, DIR_SOUTH );
						break;
					case DIR_SOUTHWEST:
						if ( map_wander ( ch, map, x - 1, y + 1, sector ) )
							move_char ( ch, NULL, 0, DIR_SOUTHWEST );
						break;
					case DIR_WEST:
						if ( map_wander ( ch, map, x - 1, y, sector ) )
							move_char ( ch, NULL, 0, DIR_WEST );
						break;
					case DIR_NORTHWEST:
						if ( map_wander ( ch, map, x - 1, y - 1, sector ) )
							move_char ( ch, NULL, 0, DIR_NORTHWEST );
						break;
				}
			}
			if ( char_died ( ch ) )
				continue;
		}
		/*
		 * Wander
		 */
		if ( !xIS_SET ( ch->act, ACT_RUNNING )
		        && !xIS_SET ( ch->act, ACT_SENTINEL )
		        && !xIS_SET ( ch->act, ACT_PROTOTYPE ) && ( door = number_bits ( 5 ) ) <= 9 && ( pexit = get_exit ( ch->in_room, door ) ) != NULL && pexit->to_room
		        /*
		         * Map wanderers - Samson 7-29-00
		         */
		        && !IS_SET ( pexit->exit_info, EX_CLOSED )
		        && !xIS_SET ( pexit->to_room->room_flags, ROOM_NO_MOB )
		        && !xIS_SET ( pexit->to_room->room_flags, ROOM_DEATH ) && ( !xIS_SET ( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
		{
			retcode = move_char ( ch, pexit, 0, pexit->vdir );
			/*
			 * If ch changes position due
			 * to it's or someother mob's
			 * movement via MOBProgs,
			 * continue - Kahn
			 */
			if ( char_died ( ch ) )
				continue;
			if ( retcode != rNONE || xIS_SET ( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
				continue;
		}
		/*
		 * Flee
		 */
		if ( ch->hit < ch->max_hit / 2
		        && ( door = number_bits ( 4 ) ) <= 9
		        && ( pexit = get_exit ( ch->in_room, door ) ) != NULL
		        && pexit->to_room && !IS_SET ( pexit->exit_info, EX_CLOSED ) && !xIS_SET ( pexit->to_room->room_flags, ROOM_NO_MOB ) )
		{
			CHAR_DATA *rch;
			bool found;
			found = FALSE;
			for ( rch = ch->in_room->first_person; rch; rch = rch->next_in_room )
			{
				if ( is_fearing ( ch, rch ) )
				{
					switch ( number_bits ( 2 ) )
					{
						case 0:
							sprintf ( buf, "Get away from me, %s!", rch->name );
							break;
						case 1:
							sprintf ( buf, "Leave me be, %s!", rch->name );
							break;
						case 2:
							sprintf ( buf, "%s is trying to kill me!  Help!", rch->name );
							break;
						case 3:
							sprintf ( buf, "Someone save me from %s!", rch->name );
							break;
					}
					do_yell ( ch, buf );
					found = TRUE;
					break;
				}
			}
			if ( found )
				retcode = move_char ( ch, pexit, 0, pexit->vdir );
		}
	}
	return;
}

/*
 * Update all chars, including mobs.
 * This function is performance sensitive.
 */
void char_update ( void )
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_save;
	short save_count = 0;
	ch_save = NULL;
	for ( ch = last_char; ch; ch = gch_prev )
	{
		if ( ch == first_char && ch->prev )
		{
			bug ( "char_update: first_char->prev != NULL... fixed", 0 );
			ch->prev = NULL;
		}
		gch_prev = ch->prev;
		set_cur_char ( ch );
		if ( gch_prev && gch_prev->next != ch )
		{
			bug ( "char_update: ch->prev->next != ch", 0 );
			return;
		}
		/*
		 *  Do a room_prog rand check right off the bat
		 *   if ch disappears (rprog might wax npc's), continue
		 */
		if ( !IS_NPC ( ch ) )
			rprog_random_trigger ( ch );
		if ( char_died ( ch ) )
			continue;
		if ( IS_NPC ( ch ) )
			mprog_time_trigger ( ch );
		if ( char_died ( ch ) )
			continue;
		rprog_time_trigger ( ch );
		if ( char_died ( ch ) )
			continue;
		/*
		 * See if player should be auto-saved.
		 */
		if ( !IS_NPC ( ch ) && ( !ch->desc || ch->desc->connected == CON_PLAYING ) && ch->level >= 2 && current_time - ch->save_time > ( sysdata.save_frequency * 60 ) )
			ch_save = ch;
		else
			ch_save = NULL;
		if ( ch->position >= POS_STUNNED )
		{
			if ( ch->hit < ch->max_hit )
				ch->hit += hit_gain ( ch );
			if ( ch->mana < ch->max_mana )
				ch->mana += mana_gain ( ch );
			if ( ch->move < ch->max_move )
				ch->move += move_gain ( ch );
		}
		if ( ch->position == POS_STUNNED )
			update_pos ( ch );

		if ( !IS_NPC ( ch ) && ch->level < LEVEL_IMMORTAL )
		{
			OBJ_DATA *obj;
			if ( ( obj = get_eq_char ( ch, WEAR_LIGHT ) ) != NULL && obj->item_type == ITEM_LIGHT && obj->value[2] > 0 )
			{
				if ( --obj->value[2] == 0 && ch->in_room )
				{
					ch->in_room->light -= obj->count;
					if ( ch->in_room->light < 0 )
						ch->in_room->light = 0;
					act ( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_ROOM );
					act ( AT_ACTION, "$p goes out.", ch, obj, NULL, TO_CHAR );
					if ( obj->serial == cur_obj )
						global_objcode = rOBJ_EXPIRED;
					extract_obj ( obj );
				}
			}
			if ( ++ch->timer >= 30 )
			{
				if ( !IS_IDLE ( ch ) )
				{
					if ( ch->fighting )
						stop_fighting ( ch, TRUE );
					act ( AT_ACTION, "$n disappears into the void.", ch, NULL, NULL, TO_ROOM );
					send_to_char ( "You disappear into the void.\r\n", ch );
					if ( IS_SET ( sysdata.save_flags, SV_IDLE ) )
						save_char_obj ( ch );
					SET_BIT ( ch->pcdata->flags, PCFLAG_IDLE );
					char_from_room ( ch );
					char_to_room ( ch, get_room_index ( ROOM_VNUM_LIMBO ) );
				}
			}

			/*
			 * Function added on suggestion from Cronel
			 */
			check_alignment ( ch );
		}
		if ( !IS_NPC ( ch ) && !IS_IMMORTAL ( ch ) && ch->pcdata->release_date > 0 && ch->pcdata->release_date <= current_time )
		{
			ROOM_INDEX_DATA *location;
			if ( ch->pcdata->clan )
				location = get_room_index ( ch->pcdata->clan->recall );
			else
				location = get_room_index ( ROOM_VNUM_TEMPLE );
			if ( !location )
				location = ch->in_room;
			MOBtrigger = FALSE;
			char_from_room ( ch );
			char_to_room ( ch, location );
			send_to_char ( "The gods have released you from hell as your sentance is up!\r\n", ch );
			do_look ( ch, "auto" );
			STRFREE ( ch->pcdata->helled_by );
			ch->pcdata->helled_by = NULL;
			ch->pcdata->release_date = 0;
			save_char_obj ( ch );
		}
		if ( !char_died ( ch ) )
		{
			OBJ_DATA *arrow = NULL;
			int dam = 0;
			if ( ( arrow = get_eq_char ( ch, WEAR_LODGE_RIB ) ) != NULL )
			{
				dam = number_range ( ( 2 * arrow->value[1] ), ( 2 * arrow->value[2] ) );
				act ( AT_CARNAGE, "$n suffers damage from $p stuck in $s rib.", ch, arrow, NULL, TO_ROOM );
				act ( AT_CARNAGE, "You suffer damage from $p stuck in your rib.", ch, arrow, NULL, TO_CHAR );
				damage ( ch, ch, dam, TYPE_UNDEFINED );
			}
			else if ( ( arrow = get_eq_char ( ch, WEAR_LODGE_LEG ) ) != NULL )
			{
				dam = number_range ( arrow->value[1], arrow->value[2] );
				act ( AT_CARNAGE, "$n suffers damage from $p stuck in $s leg.", ch, arrow, NULL, TO_ROOM );
				act ( AT_CARNAGE, "You suffer damage from $p stuck in your leg.", ch, arrow, NULL, TO_CHAR );
				damage ( ch, ch, dam, TYPE_UNDEFINED );
			}
			else if ( ( arrow = get_eq_char ( ch, WEAR_LODGE_ARM ) ) != NULL )
			{
				dam = number_range ( arrow->value[1], arrow->value[2] );
				act ( AT_CARNAGE, "$n suffers damage from $p stuck in $s arm.", ch, arrow, NULL, TO_ROOM );
				act ( AT_CARNAGE, "You suffer damage from $p stuck in your arm.", ch, arrow, NULL, TO_CHAR );
				damage ( ch, ch, dam, TYPE_UNDEFINED );
			}
			if ( char_died ( ch ) )
				continue;
			/*
			 * Careful with the damages here,
			 *   MUST NOT refer to ch after damage taken, without checking
			 *   return code and/or char_died as it may be lethal damage.
			 */
			if ( IS_AFFECTED ( ch, AFF_POISON ) )
			{
				act ( AT_POISON, "$n shivers and suffers.", ch, NULL, NULL, TO_ROOM );
				act ( AT_POISON, "You shiver and suffer.", ch, NULL, NULL, TO_CHAR );
				damage ( ch, ch, 6, gsn_poison );
			}
			else if ( ch->position == POS_INCAP )
				damage ( ch, ch, 1, TYPE_UNDEFINED );
			else if ( ch->position == POS_MORTAL )
				damage ( ch, ch, 4, TYPE_UNDEFINED );
			if ( char_died ( ch ) )
				continue;
			/*
			 * Recurring spell affect
			 */
			if ( IS_AFFECTED ( ch, AFF_RECURRINGSPELL ) )
			{
				AFFECT_DATA *paf, *paf_next;
				SKILLTYPE *skill;
				bool found = FALSE, died = FALSE;
				for ( paf = ch->first_affect; paf; paf = paf_next )
				{
					paf_next = paf->next;
					if ( paf->location == APPLY_RECURRINGSPELL )
					{
						found = TRUE;
						if ( IS_VALID_SN ( paf->modifier ) && ( skill = skill_table[paf->modifier] ) != NULL && skill->type == SKILL_SPELL )
						{
							if ( ( *skill->spell_fun ) ( paf->modifier, ch->level, ch, ch ) == rCHAR_DIED || char_died ( ch ) )
							{
								died = TRUE;
								break;
							}
						}
					}
				}
				if ( died )
					continue;
				if ( !found )
					xREMOVE_BIT ( ch->affected_by, AFF_RECURRINGSPELL );
			}
			if ( ch == ch_save && IS_SET ( sysdata.save_flags, SV_AUTO ) && ++save_count < 10 ) /* save max of 10 per tick */
				save_char_obj ( ch );
		}
	}
	return;
}

void update_bonuses( )
{
	char buf[MAX_INPUT_LENGTH];
	if ( global_exp-- >= -1 )
	{
		display++;
		if ( display >= 3 && global_exp > 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BThere are %d Minutes of double exp left.{x\r\n", global_exp );
			talk_info ( AT_GREEN, buf, FALSE );
			display = 0;
		}
		if ( global_exp == 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BDouble exp has run out!{x\r\n" );
			talk_info ( AT_GREEN, buf, FALSE );
			double_exp = FALSE;
		}
	}
	if ( global_qp-- >= 0 )
	{
		qpdisplay++;
		if ( qpdisplay >= 3 && global_qp > 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BThere are %d Minutes of double questpoints left.{x\r\n", global_qp );
			talk_info ( AT_GREEN, buf, FALSE );
			qpdisplay = 0;
		}
		if ( global_qp == 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BDouble questpoints has run out!{x\r\n" );
			talk_info ( AT_GREEN, buf, FALSE );
			double_qp = FALSE;
		}
	}
	if ( global_quad-- >= 0 )
	{
		quaddisplay++;
		if ( quaddisplay >= 3 && global_quad > 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BThere are %d Minutes of quad damage left.{x\r\n", global_quad );
			talk_info ( AT_GREEN, buf, FALSE );
			quaddisplay = 0;
		}
		if ( global_quad == 0 )
		{
			sprintf ( buf, "&R[&CANNOUNCEMENT&R]&c:{x {BQuad damage has run out!{x\r\n" );
			talk_info ( AT_GREEN, buf, FALSE );
			quad_damage = FALSE;
		}
	}
	return;
}

/*
 * Update all objs.
 * This function is performance sensitive.
 */
void obj_update ( void )
{
	OBJ_DATA *obj;
	short AT_TEMP;
	for ( obj = last_object; obj; obj = gobj_prev )
	{
		CHAR_DATA *rch;
		char *message;
		if ( obj == first_object && obj->prev )
		{
			bug ( "obj_update: first_object->prev != NULL... fixed", 0 );
			obj->prev = NULL;
		}
		gobj_prev = obj->prev;
		if ( gobj_prev && gobj_prev->next != obj )
		{
			bug ( "obj_update: obj->prev->next != obj", 0 );
			return;
		}
		set_cur_obj ( obj );
		if ( obj->carried_by )
			oprog_random_trigger ( obj );
		else if ( obj->in_room && obj->in_room->area->nplayer > 0 )
			oprog_random_trigger ( obj );
		if ( obj_extracted ( obj ) )
			continue;
		if ( obj->item_type == ITEM_PIPE )
		{
			if ( IS_SET ( obj->value[3], PIPE_LIT ) )
			{
				if ( --obj->value[1] <= 0 )
				{
					obj->value[1] = 0;
					REMOVE_BIT ( obj->value[3], PIPE_LIT );
				}
				else if ( IS_SET ( obj->value[3], PIPE_HOT ) )
					REMOVE_BIT ( obj->value[3], PIPE_HOT );
				else
				{
					if ( IS_SET ( obj->value[3], PIPE_GOINGOUT ) )
					{
						REMOVE_BIT ( obj->value[3], PIPE_LIT );
						REMOVE_BIT ( obj->value[3], PIPE_GOINGOUT );
					}
					else
						SET_BIT ( obj->value[3], PIPE_GOINGOUT );
				}
				if ( !IS_SET ( obj->value[3], PIPE_LIT ) )
					SET_BIT ( obj->value[3], PIPE_FULLOFASH );
			}
			else
				REMOVE_BIT ( obj->value[3], PIPE_HOT );
		}
		/*
		 * Corpse decay (npc corpses decay at 8 times the rate of pc corpses) - Narn
		 */
		if ( obj->item_type == ITEM_CORPSE_PC || obj->item_type == ITEM_CORPSE_NPC )
		{
			short timerfrac = UMAX ( 1, obj->timer - 1 );
			if ( obj->item_type == ITEM_CORPSE_PC )
				timerfrac = ( int ) ( obj->timer / 8 + 1 );
			if ( obj->timer > 0 && obj->value[2] > timerfrac )
			{
				char buf[MAX_STRING_LENGTH];
				char name[MAX_STRING_LENGTH];
				char *bufptr;
				bufptr = one_argument ( obj->short_descr, name );
				bufptr = one_argument ( bufptr, name );
				bufptr = one_argument ( bufptr, name );
				separate_obj ( obj );
				obj->value[2] = timerfrac;
				sprintf ( buf, corpse_descs[UMIN ( timerfrac - 1, 4 ) ], bufptr );
				STRFREE ( obj->description );
				obj->description = STRALLOC ( buf );
			}
		}
		/*
		 * don't let inventory decay
		 */
		if ( IS_OBJ_STAT ( obj, ITEM_INVENTORY ) )
			continue;
		/*
		 * groundrot items only decay on the ground
		 */
		if ( IS_OBJ_STAT ( obj, ITEM_GROUNDROT ) && !obj->in_room )
			continue;
		if ( ( obj->timer <= 0 || --obj->timer > 0 ) )
			continue;
		/*
		 * if we get this far, object's timer has expired.
		 */
		AT_TEMP = AT_PLAIN;
		switch ( obj->item_type )
		{
			default:
				message = "$p mysteriously vanishes.";
				AT_TEMP = AT_PLAIN;
				break;
			case ITEM_CONTAINER:
				message = "$p falls apart, tattered from age.";
				AT_TEMP = AT_OBJECT;
				break;
			case ITEM_PORTAL:
				message = "$p unravels and winks from existence.";
				remove_portal ( obj );
				obj->item_type = ITEM_TRASH;  /* so extract_obj  */
				AT_TEMP = AT_MAGIC;  /* doesn't remove_portal */
				break;
			case ITEM_FOUNTAIN:
				message = "$p dries up.";
				AT_TEMP = AT_BLUE;
				break;
			case ITEM_CORPSE_NPC:
				message = "$p decays into dust and blows away.";
				AT_TEMP = AT_OBJECT;
				break;
			case ITEM_CORPSE_PC:
				message = "$p is sucked into a swirling vortex of colors...";
				AT_TEMP = AT_MAGIC;
				break;
			case ITEM_COOK:
			case ITEM_FOOD:
				message = "$p is devoured by a swarm of maggots.";
				AT_TEMP = AT_HUNGRY;
				break;
			case ITEM_BLOOD:
				message = "$p slowly seeps into the ground.";
				AT_TEMP = AT_BLOOD;
				break;
			case ITEM_BLOODSTAIN:
				message = "$p dries up into flakes and blows away.";
				AT_TEMP = AT_BLOOD;
				break;
			case ITEM_SCRAPS:
				message = "$p crumble and decay into nothing.";
				AT_TEMP = AT_OBJECT;
				break;
			case ITEM_FIRE:
				/*
				 * This is removed because it is done in obj_from_room
				 * * Thanks to gfinello@mail.karmanet.it for pointing this out.
				 * * --Shaddai
				 * if (obj->in_room)
				 * {
				 * --obj->in_room->light;
				 * if ( obj->in_room->light < 0 )
				 * obj->in_room->light = 0;
				 * }
				 */
				message = "$p burns out.";
				AT_TEMP = AT_FIRE;
		}
		if ( obj->carried_by )
		{
			act ( AT_TEMP, message, obj->carried_by, obj, NULL, TO_CHAR );
		}
		else if ( obj->in_room && ( rch = obj->in_room->first_person ) != NULL && !IS_OBJ_STAT ( obj, ITEM_BURIED ) )
		{
			act ( AT_TEMP, message, rch, obj, NULL, TO_ROOM );
			act ( AT_TEMP, message, rch, obj, NULL, TO_CHAR );
		}
		if ( obj->serial == cur_obj )
			global_objcode = rOBJ_EXPIRED;
		extract_obj ( obj );
	}
	return;
}

/*
 * Function to check important stuff happening to a player
 * This function should take about 5% of mud cpu time
 */
void char_check ( void )
{
	CHAR_DATA *ch, *ch_next;
	OBJ_DATA *obj;
	EXIT_DATA *pexit;
	static int cnt = 0;
	int door, retcode;
	/*
	 * This little counter can be used to handle periodic events
	 */
	cnt = ( cnt + 1 ) % SECONDS_PER_TICK;
	for ( ch = first_char; ch; ch = ch_next )
	{
		set_cur_char ( ch );
		ch_next = ch->next;
		will_fall ( ch, 0 );
		if ( char_died ( ch ) )
			continue;
		if ( IS_NPC ( ch ) )
		{
			if ( ( cnt & 1 ) )
				continue;
			/*
			 * running mobs  -Thoric
			 */
			if ( xIS_SET ( ch->act, ACT_RUNNING ) )
			{
				if ( ch->spec_fun )
				{
					if ( ( *ch->spec_fun ) ( ch ) )
						continue;
					if ( char_died ( ch ) )
						continue;
				}
				/*
				 * Map wanderers - Samson 7-28-00
				 */
				if ( IS_ACT_FLAG ( ch, ACT_ONMAP ) )
				{
					short sector = ch->sector;
					short map = ch->map;
					short x = ch->x;
					short y = ch->y;
					int dir = number_bits ( 5 );
					if ( dir < DIR_SOMEWHERE && dir != DIR_UP && dir != DIR_DOWN )
					{
						switch ( dir )
						{
							case DIR_NORTH:
								if ( map_wander ( ch, map, x, y - 1, sector ) )
									move_char ( ch, NULL, 0, DIR_NORTH );
								break;
							case DIR_NORTHEAST:
								if ( map_wander ( ch, map, x + 1, y - 1, sector ) )
									move_char ( ch, NULL, 0, DIR_NORTHEAST );
								break;
							case DIR_EAST:
								if ( map_wander ( ch, map, x + 1, y, sector ) )
									move_char ( ch, NULL, 0, DIR_EAST );
								break;
							case DIR_SOUTHEAST:
								if ( map_wander ( ch, map, x + 1, y + 1, sector ) )
									move_char ( ch, NULL, 0, DIR_SOUTHEAST );
								break;
							case DIR_SOUTH:
								if ( map_wander ( ch, map, x, y + 1, sector ) )
									move_char ( ch, NULL, 0, DIR_SOUTH );
								break;
							case DIR_SOUTHWEST:
								if ( map_wander ( ch, map, x - 1, y + 1, sector ) )
									move_char ( ch, NULL, 0, DIR_SOUTHWEST );
								break;
							case DIR_WEST:
								if ( map_wander ( ch, map, x - 1, y, sector ) )
									move_char ( ch, NULL, 0, DIR_WEST );
								break;
							case DIR_NORTHWEST:
								if ( map_wander ( ch, map, x - 1, y - 1, sector ) )
									move_char ( ch, NULL, 0, DIR_NORTHWEST );
								break;
						}
					}
					if ( char_died ( ch ) )
						continue;
				}
				if ( !xIS_SET ( ch->act, ACT_SENTINEL )
				        && !xIS_SET ( ch->act, ACT_PROTOTYPE )
				        && ( door = number_bits ( 4 ) ) <= 9
				        && ( pexit = get_exit ( ch->in_room, door ) ) != NULL
				        && pexit->to_room
				        && !IS_SET ( pexit->exit_info, EX_CLOSED )
				        && !xIS_SET ( pexit->to_room->room_flags, ROOM_NO_MOB )
				        && !xIS_SET ( pexit->to_room->room_flags, ROOM_DEATH ) && ( !xIS_SET ( ch->act, ACT_STAY_AREA ) || pexit->to_room->area == ch->in_room->area ) )
				{
					retcode = move_char ( ch, pexit, 0, pexit->vdir );
					if ( char_died ( ch ) )
						continue;
					if ( retcode != rNONE || xIS_SET ( ch->act, ACT_SENTINEL ) || ch->position < POS_STANDING )
						continue;
				}
			}
			continue;
		}
		else
		{
			if ( ch->mount && ch->in_room != ch->mount->in_room )
			{
				xREMOVE_BIT ( ch->mount->act, ACT_MOUNTED );
				ch->mount = NULL;
				ch->position = POS_STANDING;
				send_to_char ( "No longer upon your mount, you fall to the ground...\r\nOUCH!\r\n", ch );
			}
			if ( ( ch->in_room && ch->in_room->sector_type == SECT_UNDERWATER ) || ( ch->in_room && ch->in_room->sector_type == SECT_OCEANFLOOR ) )
			{
				if ( !IS_AFFECTED ( ch, AFF_AQUA_BREATH ) )
				{
					if ( ch->level < LEVEL_IMMORTAL )
					{
						int dam;
						/*
						 * Changed level of damage at Brittany's request. -- Narn
						 */
						dam = number_range ( ch->max_hit / 100, ch->max_hit / 50 );
						dam = UMAX ( 1, dam );
						if ( number_bits ( 3 ) == 0 )
							send_to_char ( "You cough and choke as you try to breathe water!\r\n", ch );
						damage ( ch, ch, dam, TYPE_UNDEFINED );
					}
				}
			}
			if ( char_died ( ch ) )
				continue;
			if ( ch->in_room && ( ( ch->in_room->sector_type == SECT_WATER_NOSWIM ) || ( ch->in_room->sector_type == SECT_WATER_SWIM ) ) )
			{
				if ( !IS_AFFECTED ( ch, AFF_FLYING ) && !IS_AFFECTED ( ch, AFF_FLOATING ) && !IS_AFFECTED ( ch, AFF_AQUA_BREATH ) && !ch->mount )
				{
					for ( obj = ch->first_carrying; obj; obj = obj->next_content )
						if ( obj->item_type == ITEM_BOAT )
							break;
					if ( !obj )
					{
						if ( ch->level < LEVEL_IMMORTAL )
						{
							int mov;
							int dam;
							if ( ch->move > 0 )
							{
								mov = number_range ( ch->max_move / 20, ch->max_move / 5 );
								mov = UMAX ( 1, mov );
								if ( ch->move - mov < 0 )
									ch->move = 0;
								else
									ch->move -= mov;
							}
							else
							{
								dam = number_range ( ch->max_hit / 20, ch->max_hit / 5 );
								dam = UMAX ( 1, dam );
								if ( number_bits ( 3 ) == 0 )
									send_to_char ( "Struggling with exhaustion, you choke on a mouthful of water.\r\n", ch );
								damage ( ch, ch, dam, TYPE_UNDEFINED );
							}
						}
					}
				}
			}
			/*
			 * beat up on link dead players
			 */
			if ( !ch->desc )
			{
				CHAR_DATA *wch, *wch_next;
				for ( wch = ch->in_room->first_person; wch; wch = wch_next )
				{
					wch_next = wch->next_in_room;
					if ( !IS_NPC ( wch )
					        || wch->fighting
					        || IS_AFFECTED ( wch, AFF_CHARM ) || !IS_AWAKE ( wch ) || ( xIS_SET ( wch->act, ACT_WIMPY ) && IS_AWAKE ( ch ) ) || !can_see ( wch, ch, FALSE ) )
						continue;

					if ( ( !xIS_SET ( wch->act, ACT_AGGRESSIVE )
					        && !xIS_SET ( wch->act, ACT_META_AGGR ) ) || xIS_SET ( wch->act, ACT_MOUNTED ) || xIS_SET ( wch->in_room->room_flags, ROOM_SAFE ) )
						continue;
					global_retcode = multi_hit ( wch, ch, TYPE_UNDEFINED );
				}
			}
		}
	}
}

/*
 * Aggress.
 *
 * for each descriptor
 *     for each mob in room
 *         aggress on some random PC
 *
 * This function should take 5% to 10% of ALL mud cpu time.
 * Unfortunately, checking on each PC move is too tricky,
 *   because we don't the mob to just attack the first PC
 *   who leads the party into the room.
 *
 */
void aggr_update ( void )
{
	DESCRIPTOR_DATA *d, *dnext;
	CHAR_DATA *wch;
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	CHAR_DATA *victim;
	struct act_prog_data *apdtmp;
#ifdef UNDEFD
	/*
	 *  GRUNT!  To do
	 *
	 */
	if ( IS_NPC ( wch ) && wch->mpactnum > 0 && wch->in_room->area->nplayer > 0 )
	{
		MPROG_ACT_LIST *tmp_act, *tmp2_act;
		for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp_act->next )
		{
			oprog_wordlist_check ( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
			DISPOSE ( tmp_act->buf );
		}
		for ( tmp_act = wch->mpact; tmp_act; tmp_act = tmp2_act )
		{
			tmp2_act = tmp_act->next;
			DISPOSE ( tmp_act );
		}
		wch->mpactnum = 0;
		wch->mpact = NULL;
	}
#endif
	/*
	 * check mobprog act queue
	 */
	while ( ( apdtmp = mob_act_list ) != NULL )
	{
		wch = mob_act_list->vo;
		if ( !char_died ( wch ) && wch->mpactnum > 0 )
		{
			MPROG_ACT_LIST *tmp_act;
			while ( ( tmp_act = wch->mpact ) != NULL )
			{
				if ( tmp_act->obj && obj_extracted ( tmp_act->obj ) )
					tmp_act->obj = NULL;
				if ( tmp_act->ch && !char_died ( tmp_act->ch ) )
					mprog_wordlist_check ( tmp_act->buf, wch, tmp_act->ch, tmp_act->obj, tmp_act->vo, ACT_PROG );
				wch->mpact = tmp_act->next;
				DISPOSE ( tmp_act->buf );
				DISPOSE ( tmp_act );
			}
			wch->mpactnum = 0;
			wch->mpact = NULL;
		}
		mob_act_list = apdtmp->next;
		DISPOSE ( apdtmp );
	}
	/*
	 * Just check descriptors here for victims to aggressive mobs
	 * We can check for linkdead victims in char_check  -Thoric
	 */
	for ( d = first_descriptor; d; d = dnext )
	{
		dnext = d->next;
		if ( d->connected != CON_PLAYING || ( wch = d->character ) == NULL )
			continue;
		if ( char_died ( wch ) || IS_NPC ( wch ) || wch->level >= LEVEL_IMMORTAL || !wch->in_room )
			continue;
		for ( ch = wch->in_room->first_person; ch; ch = ch_next )
		{
			int count;
			ch_next = ch->next_in_room;
			if ( !IS_NPC ( ch )
			        || ch->fighting || IS_AFFECTED ( ch, AFF_CHARM ) || !IS_AWAKE ( ch ) || ( xIS_SET ( ch->act, ACT_WIMPY ) && IS_AWAKE ( wch ) ) || !can_see ( ch, wch, FALSE ) )
				continue;

			if ( ( !xIS_SET ( ch->act, ACT_AGGRESSIVE )
			        && !xIS_SET ( ch->act, ACT_META_AGGR ) ) || xIS_SET ( ch->act, ACT_MOUNTED ) || xIS_SET ( ch->in_room->room_flags, ROOM_SAFE ) )
				continue;
			/*
			 * Ok we have a 'wch' player character and a 'ch' npc aggressor.
			 * Now make the aggressor fight a RANDOM pc victim in the room,
			 *   giving each 'vch' an equal chance of selection.
			 *
			 * Depending on flags set, the mob may attack another mob
			 */
			count = 0;
			victim = NULL;
			for ( vch = wch->in_room->first_person; vch; vch = vch_next )
			{
				vch_next = vch->next_in_room;
				if ( ( !IS_NPC ( vch ) || xIS_SET ( ch->act, ACT_META_AGGR )
				        || xIS_SET ( vch->act, ACT_ANNOYING ) )
				        && vch->level < LEVEL_IMMORTAL && ( !xIS_SET ( ch->act, ACT_WIMPY ) || !IS_AWAKE ( vch ) ) && can_see ( ch, vch, FALSE ) )
				{
					if ( number_range ( 0, count ) == 0 )
						victim = vch;
					count++;
				}
			}
			if ( !victim )
			{
				bug ( "Aggr_update: null victim.", count );
				continue;
			}
			global_retcode = multi_hit ( ch, victim, TYPE_UNDEFINED );
		}
	}
	return;
}

/* From interp.c */
bool check_social args ( ( CHAR_DATA * ch, char *command, char *argument ) );

void tele_update ( void )
{
	TELEPORT_DATA *tele, *tele_next;
	if ( !first_teleport )
		return;
	for ( tele = first_teleport; tele; tele = tele_next )
	{
		tele_next = tele->next;
		if ( --tele->timer <= 0 )
		{
			if ( tele->room->first_person )
			{
				if ( xIS_SET ( tele->room->room_flags, ROOM_TELEPORT ) )
					teleport ( tele->room->first_person, tele->room->tele_vnum, TELE_SHOWDESC | TELE_TRANSALL );
				else
					teleport ( tele->room->first_person, tele->room->tele_vnum, TELE_TRANSALL );
			}
			UNLINK ( tele, first_teleport, last_teleport, next, prev );
			DISPOSE ( tele );
		}
	}
}

/*
 * Handle all kinds of updates.
 * Called once per pulse from game loop.
 * Random times to defeat tick-timing clients and players.
 */
void update_handler ( void )
{
	static int pulse_area;
	static int pulse_pract_dummy;
	static int pulse_mobile;
	static int pulse_violence;
	static int pulse_point;
	static int pulse_second;
	static int pulse_houseauc;
	static int pulse_start_arena = PULSE_ARENA;
	static int pulse_arena = PULSE_ARENA;

	struct timeval sttime;
	struct timeval etime;
	if ( timechar )
	{
		set_char_color ( AT_PLAIN, timechar );
		send_to_char ( "Starting update timer.\r\n", timechar );
		gettimeofday ( &sttime, NULL );
	}
	if ( --pulse_houseauc <= 0 )
	{
		pulse_houseauc = 1800 * PULSE_PER_SECOND;
		homebuy_update( );
	}
	if ( --pulse_pract_dummy <= 0 )
	{
		pulse_pract_dummy =  PULSE_TICK;
		pract_dummy_update( );
	}
	if ( --pulse_area <= 0 )
	{
		pulse_area = PULSE_AREA;
		area_update( );
	}
	if ( --pulse_mobile <= 0 )
	{
		pulse_mobile = PULSE_MOBILE;
		mobile_update( );
	}
	if ( --pulse_violence <= 0 )
	{
		pulse_violence = PULSE_VIOLENCE;
		violence_update( );
	}
	if ( --pulse_point <= 0 )
	{
		pulse_point = PULSE_TICK;
		time_update( );
		weather_update( );
		char_update( );
		obj_update( );
		clear_vrooms( );  /* remove virtual rooms */
		quest_update( );
		update_bonuses( );
	}
	if ( --pulse_second <= 0 )
	{
		pulse_second = PULSE_PER_SECOND;
		char_check( );
		check_pfiles ( 0 );
	}
	if ( in_start_arena || ppl_challenged )
		if ( --pulse_start_arena <= 0 )
		{
			pulse_start_arena = PULSE_ARENA;
			start_arena( );
		}
	if ( ppl_in_arena )
		if ( ( --pulse_arena <= 0 ) || ( num_in_arena( ) == 1 ) )
		{
			pulse_arena = PULSE_ARENA;
			do_game( );
		}
	if ( --auction->pulse <= 0 )
	{
		auction->pulse = PULSE_AUCTION;
		auction_update( );
	}
	mpsleep_update( );   /* Check for sleeping mud progs -rkb */
	tele_update( );
	aggr_update( );
	obj_act_update( );
	room_act_update( );
	clean_obj_queue( );  /* dispose of extracted objects */
	clean_char_queue( );    /* dispose of dead mobs/quitting chars */
	if ( timechar )
	{
		gettimeofday ( &etime, NULL );
		set_char_color ( AT_PLAIN, timechar );
		send_to_char ( "Update timing complete.\r\n", timechar );
		subtract_times ( &etime, &sttime );
		ch_printf ( timechar, "Timing took %ld.%06ld seconds.\r\n", ( time_t ) etime.tv_sec, ( time_t ) etime.tv_usec );
		timechar = NULL;
	}
	tail_chain( );
	return;
}

void remove_portal ( OBJ_DATA * portal )
{
	ROOM_INDEX_DATA *fromRoom, *toRoom;
	EXIT_DATA *pexit;
	bool found;
	if ( !portal )
	{
		bug ( "remove_portal: portal is NULL", 0 );
		return;
	}
	fromRoom = portal->in_room;
	found = FALSE;
	if ( !fromRoom )
	{
		bug ( "remove_portal: portal->in_room is NULL", 0 );
		return;
	}
	for ( pexit = fromRoom->first_exit; pexit; pexit = pexit->next )
		if ( IS_SET ( pexit->exit_info, EX_PORTAL ) )
		{
			found = TRUE;
			break;
		}
	if ( !found )
	{
		bug ( "remove_portal: portal not found in room %d!", fromRoom->vnum );
		return;
	}
	if ( pexit->vdir != DIR_PORTAL )
		bug ( "remove_portal: exit in dir %d != DIR_PORTAL", pexit->vdir );
	if ( ( toRoom = pexit->to_room ) == NULL )
		bug ( "remove_portal: toRoom is NULL", 0 );
	extract_exit ( fromRoom, pexit );
	return;
}

/* the auction update*/
void auction_update ( void )
{
	int tax, pay;
	char buf[MAX_STRING_LENGTH];
	if ( !auction->item )
	{
		if ( AUCTION_MEM > 0 && auction->history[0] && ++auction->hist_timer == 6 * AUCTION_MEM )
		{
			int i;
			for ( i = AUCTION_MEM - 1; i >= 0; i-- )
			{
				if ( auction->history[i] )
				{
					auction->history[i] = NULL;
					auction->hist_timer = 0;
					break;
				}
			}
		}
		return;
	}
	switch ( ++auction->going )   /* increase the going state */
	{
		case 1: /* going once */
		case 2: /* going twice */
			if ( auction->bet > auction->starting )
				sprintf ( buf, "%s: going %s for %s.", auction->item->short_descr, ( ( auction->going == 1 ) ? "once" : "twice" ), num_punct ( auction->bet ) );
			else
				sprintf ( buf, "%s: going %s (bid not received yet).", auction->item->short_descr, ( ( auction->going == 1 ) ? "once" : "twice" ) );
			talk_auction ( buf );
			break;
		case 3: /* SOLD! */
			if ( !auction->buyer && auction->bet )
			{
				bug ( "Auction code reached SOLD, with NULL buyer, but %d gold bid", auction->bet );
				auction->bet = 0;
			}
			if ( auction->bet > 0 && auction->buyer != auction->seller )
			{
				sprintf ( buf, "%s sold to %s for %s.",
				          auction->item->short_descr, IS_NPC ( auction->buyer ) ? auction->buyer->short_descr : auction->buyer->name, num_punct ( auction->bet ) );
				talk_auction ( buf );
				act ( AT_ACTION, "The auctioneer materializes before you, and hands you $p.", auction->buyer, auction->item, NULL, TO_CHAR );
				act ( AT_ACTION, "The auctioneer materializes before $n, and hands $m $p.", auction->buyer, auction->item, NULL, TO_ROOM );
				if ( ( auction->buyer->carry_weight + get_obj_weight ( auction->item ) ) > can_carry_w ( auction->buyer ) )
				{
					act ( AT_PLAIN, "$p is too heavy for you to carry with your current inventory.", auction->buyer, auction->item, NULL, TO_CHAR );
					act ( AT_PLAIN, "$n is carrying too much to also carry $p, and $e drops it.", auction->buyer, auction->item, NULL, TO_ROOM );
					obj_to_room ( auction->item, auction->buyer->in_room, NULL );
				}
				else
					obj_to_char ( auction->item, auction->buyer );
				pay = ( int ) auction->bet * 0.9;
				tax = ( int ) auction->bet * 0.1;
				boost_economy ( auction->seller->in_room->area, tax );
				auction->seller->gold += pay; /* give him the money, tax 10 % */
				ch_printf ( auction->seller, "The auctioneer pays you %s gold, charging an auction fee of ", num_punct ( pay ) );
				ch_printf ( auction->seller, "%s.\r\n", num_punct ( tax ) );
				auction->item = NULL;   /* reset item */
				if ( IS_SET ( sysdata.save_flags, SV_AUCTION ) )
				{
					save_char_obj ( auction->buyer );
					save_char_obj ( auction->seller );
				}
			}
			else  /* not sold */
			{
				sprintf ( buf, "No bids received for %s - removed from auction.\r\n", auction->item->short_descr );
				talk_auction ( buf );
				act ( AT_ACTION, "The auctioneer appears before you to return $p to you.", auction->seller, auction->item, NULL, TO_CHAR );
				act ( AT_ACTION, "The auctioneer appears before $n to return $p to $m.", auction->seller, auction->item, NULL, TO_ROOM );
				if ( ( auction->seller->carry_weight + get_obj_weight ( auction->item ) ) > can_carry_w ( auction->seller ) )
				{
					act ( AT_PLAIN, "You drop $p as it is just too much to carry" " with everything else you're carrying.", auction->seller, auction->item, NULL, TO_CHAR );
					act ( AT_PLAIN, "$n drops $p as it is too much extra weight" " for $m with everything else.", auction->seller, auction->item, NULL, TO_ROOM );
					obj_to_room ( auction->item, auction->seller->in_room, NULL );
				}
				else
					obj_to_char ( auction->item, auction->seller );
				tax = ( int ) auction->item->cost * 0.05;
				boost_economy ( auction->seller->in_room->area, tax );
				sprintf ( buf, "The auctioneer charges you an auction fee of %s.\r\n", num_punct ( tax ) );
				send_to_char ( buf, auction->seller );
				if ( ( auction->seller->gold - tax ) < 0 )
					auction->seller->gold = 0;
				else
					auction->seller->gold -= tax;
				if ( IS_SET ( sysdata.save_flags, SV_AUCTION ) )
					save_char_obj ( auction->seller );
			}  /* else */
			auction->item = NULL;   /* clear auction */
	}  /* switch */
}  /* func */
void subtract_times ( struct timeval *etime, struct timeval *sttime )
{
	etime->tv_sec -= sttime->tv_sec;
	etime->tv_usec -= sttime->tv_usec;
	while ( etime->tv_usec < 0 )
	{
		etime->tv_usec += 1000000;
		etime->tv_sec--;
	}
	return;
}

/*
 * Function to update weather vectors according to climate
 * settings, random effects, and neighboring areas.
 * Last modified: July 18, 1997
 * - Fireblade
 */
void adjust_vectors ( WEATHER_DATA * weather )
{
	NEIGHBOR_DATA *neigh;
	double dT, dP, dW;
	if ( !weather )
	{
		bug ( "adjust_vectors: NULL weather data.", 0 );
		return;
	}
	dT = 0;
	dP = 0;
	dW = 0;
	/*
	 * Add in random effects
	 */
	dT += number_range ( -rand_factor, rand_factor );
	dP += number_range ( -rand_factor, rand_factor );
	dW += number_range ( -rand_factor, rand_factor );
	/*
	 * Add in climate effects
	 */
	dT += climate_factor * ( ( ( weather->climate_temp - 2 ) * weath_unit ) - ( weather->temp ) ) / weath_unit;
	dP += climate_factor * ( ( ( weather->climate_precip - 2 ) * weath_unit ) - ( weather->precip ) ) / weath_unit;
	dW += climate_factor * ( ( ( weather->climate_wind - 2 ) * weath_unit ) - ( weather->wind ) ) / weath_unit;
	/*
	 * Add in effects from neighboring areas
	 */
	for ( neigh = weather->first_neighbor; neigh; neigh = neigh->next )
	{
		/*
		 * see if we have the area cache'd already
		 */
		if ( !neigh->address )
		{
			/*
			 * try and find address for area
			 */
			neigh->address = get_area ( neigh->name );
			/*
			 * if couldn't find area ditch the neigh
			 */
			if ( !neigh->address )
			{
				NEIGHBOR_DATA *temp;
				bug ( "adjust_weather: " "invalid area name.", 0 );
				temp = neigh->prev;
				UNLINK ( neigh, weather->first_neighbor, weather->last_neighbor, next, prev );
				STRFREE ( neigh->name );
				DISPOSE ( neigh );
				neigh = temp;
				continue;
			}
		}
		dT += ( neigh->address->weather->temp - weather->temp ) / neigh_factor;
		dP += ( neigh->address->weather->precip - weather->precip ) / neigh_factor;
		dW += ( neigh->address->weather->wind - weather->wind ) / neigh_factor;
	}
	/*
	 * now apply the effects to the vectors
	 */
	weather->temp_vector += ( int ) dT;
	weather->precip_vector += ( int ) dP;
	weather->wind_vector += ( int ) dW;
	/*
	 * Make sure they are within the right range
	 */
	weather->temp_vector = URANGE ( -max_vector, weather->temp_vector, max_vector );
	weather->precip_vector = URANGE ( -max_vector, weather->precip_vector, max_vector );
	weather->wind_vector = URANGE ( -max_vector, weather->wind_vector, max_vector );
	return;
}

/*
 * function updates weather for each area
 * Last Modified: July 31, 1997
 * Fireblade
 */
void weather_update( )
{
	AREA_DATA *pArea;
	DESCRIPTOR_DATA *d;
	int limit;
	limit = 3 * weath_unit;
	for ( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
	{
		/*
		 * Apply vectors to fields
		 */
		pArea->weather->temp += pArea->weather->temp_vector;
		pArea->weather->precip += pArea->weather->precip_vector;
		pArea->weather->wind += pArea->weather->wind_vector;
		/*
		 * Make sure they are within the proper range
		 */
		pArea->weather->temp = URANGE ( -limit, pArea->weather->temp, limit );
		pArea->weather->precip = URANGE ( -limit, pArea->weather->precip, limit );
		pArea->weather->wind = URANGE ( -limit, pArea->weather->wind, limit );
		/*
		 * get an appropriate echo for the area
		 */
		get_weather_echo ( pArea->weather );
	}
	for ( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
	{
		adjust_vectors ( pArea->weather );
	}
	/*
	 * display the echo strings to the appropriate players
	 */
	for ( d = first_descriptor; d; d = d->next )
	{
		WEATHER_DATA *weath;
		if ( d->connected == CON_PLAYING && xIS_OUTSIDE ( d->character ) && !NO_WEATHER_SECT ( d->character->in_room->sector_type ) && IS_AWAKE ( d->character ) )
		{
			weath = d->character->in_room->area->weather;
			if ( !weath->echo )
				continue;
			set_char_color ( weath->echo_color, d->character );
			send_to_char ( weath->echo, d->character );
			send_mip_sound ( d->character, weath->echo_weather_snd );
		}
	}
	return;
}

/*
 * get weather echo messages according to area weather...
 * stores echo message in weath_data.... must be called before
 * the vectors are adjusted
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_weather_echo ( WEATHER_DATA * weath )
{
	int n;
	int temp, precip, wind;
	int dT, dP, dW;
	int tindex, pindex, windex;
	/*
	 * set echo to be nothing
	 */
	weath->echo = NULL;
	weath->echo_color = AT_GREY;
	/*
	 * get the random number
	 */
	n = number_bits ( 2 );
	/*
	 * variables for convenience
	 */
	temp = weath->temp;
	precip = weath->precip;
	wind = weath->wind;
	dT = weath->temp_vector;
	dP = weath->precip_vector;
	dW = weath->wind_vector;
	tindex = ( temp + 3 * weath_unit - 1 ) / weath_unit;
	pindex = ( precip + 3 * weath_unit - 1 ) / weath_unit;
	windex = ( wind + 3 * weath_unit - 1 ) / weath_unit;
	/*
	 * get the echo string... mainly based on precip
	 */
	switch ( pindex )
	{
		case 0:
			if ( precip - dP > -2 * weath_unit )
			{
				char *echo_strings[4] =
				{
					"The clouds disappear.\r\n",
					"The clouds disappear.\r\n",
					"The sky begins to break through " "the clouds.\r\n",
					"The clouds are slowly " "evaporating.\r\n"
				};
				weath->echo = echo_strings[n];
				weath->echo_color = AT_WHITE;
			}
			break;
		case 1:
			if ( precip - dP <= -2 * weath_unit )
			{
				char *echo_strings[4] =
				{
					"The sky is getting cloudy.\r\n",
					"The sky is getting cloudy.\r\n",
					"Light clouds cast a haze over " "the sky.\r\n",
					"Billows of clouds spread through " "the sky.\r\n"
				};
				weath->echo = echo_strings[n];
				weath->echo_color = AT_GREY;
			}
			break;
		case 2:
			if ( precip - dP > 0 )
			{
				if ( tindex > 1 )
				{
					char *echo_strings[4] =
					{
						"The rain stops.\r\n",
						"The rain stops.\r\n",
						"The rainstorm tapers " "off.\r\n",
						"The rain's intensity " "breaks.\r\n"
					};
					weath->echo = echo_strings[n];
					weath->echo_color = AT_CYAN;
				}
				else
				{
					char *echo_strings[4] =
					{
						"The snow stops.\r\n",
						"The snow stops.\r\n",
						"The snow showers taper " "off.\r\n",
						"The snow flakes disappear " "from the sky.\r\n"
					};
					weath->echo = echo_strings[n];
					weath->echo_color = AT_WHITE;
				}
			}
			break;
		case 3:
			if ( precip - dP <= 0 )
			{
				if ( tindex > 1 )
				{
					char *echo_strings[4] =
					{
						"It starts to rain.\r\n",
						"It starts to rain.\r\n",
						"A droplet of rain falls " "upon you.\r\n",
						"The rain begins to " "patter.\r\n"
					};
					weath->echo_weather_snd = "fury_weather_rain1.wav";
					weath->echo = echo_strings[n];
					weath->echo_color = AT_CYAN;
				}
				else
				{
					char *echo_strings[4] =
					{
						"It starts to snow.\r\n",
						"It starts to snow.\r\n",
						"Crystal flakes begin to " "fall from the " "sky.\r\n",
						"Snow flakes drift down " "from the clouds.\r\n"
					};
					weath->echo_weather_snd = "fury_weather_rain1.wav";
					weath->echo = echo_strings[n];
					weath->echo_color = AT_WHITE;
				}
			}
			else if ( tindex < 2 && temp - dT > -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The temperature drops and the rain " "becomes a light snow.\r\n",
					"The temperature drops and the rain " "becomes a light snow.\r\n",
					"Flurries form as the rain freezes.\r\n",
					"Large snow flakes begin to fall " "with the rain.\r\n"
				};
				weath->echo_weather_snd = "fury_weather_rain1.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_WHITE;
			}
			else if ( tindex > 1 && temp - dT <= -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The snow flurries are gradually " "replaced by pockets of rain.\r\n",
					"The snow flurries are gradually " "replaced by pockets of rain.\r\n",
					"The falling snow turns to a cold drizzle.\r\n",
					"The snow turns to rain as the air warms.\r\n"
				};
				weath->echo_weather_snd = "fury_weather_rain1.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_CYAN;
			}
			break;
		case 4:
			if ( precip - dP > 2 * weath_unit )
			{
				if ( tindex > 1 )
				{
					char *echo_strings[4] =
					{
						"The lightning has stopped.\r\n",
						"The lightning has stopped.\r\n",
						"The sky settles, and the " "thunder surrenders.\r\n",
						"The lightning bursts fade as " "the storm weakens.\r\n"
					};
					weath->echo_weather_snd = "rain.wav";
					weath->echo = echo_strings[n];
					weath->echo_color = AT_GREY;
				}
			}
			else if ( tindex < 2 && temp - dT > -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The cold rain turns to snow.\r\n",
					"The cold rain turns to snow.\r\n",
					"Snow flakes begin to fall " "amidst the rain.\r\n",
					"The driving rain begins to freeze.\r\n"
				};
				weath->echo_weather_snd = "fury_ambiance_wind.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_WHITE;
			}
			else if ( tindex > 1 && temp - dT <= -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The snow becomes a freezing rain.\r\n",
					"The snow becomes a freezing rain.\r\n",
					"A cold rain beats down on you " "as the snow begins to melt.\r\n",
					"The snow is slowly replaced by a heavy " "rain.\r\n"
				};
				weath->echo_weather_snd = "fury_weather_rain2.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_CYAN;
			}
			break;
		case 5:
			if ( precip - dP <= 2 * weath_unit )
			{
				if ( tindex > 1 )
				{
					char *echo_strings[4] =
					{
						"Lightning flashes in the " "sky.\r\n",
						"Lightning flashes in the " "sky.\r\n",
						"A flash of lightning splits " "the sky.\r\n",
						"The sky flashes, and the " "ground trembles with " "thunder.\r\n"
					};
					weath->echo_weather_snd = "fury_weather_thunder1.wav";
					weath->echo = echo_strings[n];
					weath->echo_color = AT_YELLOW;
				}
			}
			else if ( tindex > 1 && temp - dT <= -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The sky rumbles with thunder as " "the snow changes to rain.\r\n",
					"The sky rumbles with thunder as " "the snow changes to rain.\r\n",
					"The falling turns to freezing rain " "amidst flashes of " "lightning.\r\n",
					"The falling snow begins to melt as " "thunder crashes overhead.\r\n"
				};
				weath->echo_weather_snd = "fury_weather_thunder4.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_WHITE;
			}
			else if ( tindex < 2 && temp - dT > -weath_unit )
			{
				char *echo_strings[4] =
				{
					"The lightning stops as the rainstorm " "becomes a blinding " "blizzard.\r\n",
					"The lightning stops as the rainstorm " "becomes a blinding " "blizzard.\r\n",
					"The thunder dies off as the " "pounding rain turns to " "heavy snow.\r\n",
					"The cold rain turns to snow and " "the lightning stops.\r\n"
				};
				weath->echo_weather_snd = "fury_weather_windy_vf.wav";
				weath->echo = echo_strings[n];
				weath->echo_color = AT_CYAN;
			}
			break;
		default:
			bug ( "echo_weather: invalid precip index" );
			weath->precip = 0;
			break;
	}
	return;
}

/*
 * get echo messages according to time changes...
 * some echoes depend upon the weather so an echo must be
 * found for each area
 * Last Modified: August 10, 1997
 * Fireblade
 */
void get_time_echo ( WEATHER_DATA * weath )
{
	int n;
	int pindex;
	n = number_bits ( 2 );
	pindex = ( weath->precip + 3 * weath_unit - 1 ) / weath_unit;
	weath->echo = NULL;
	weath->echo_color = AT_GREY;
	switch ( time_info.hour )
	{
		case 5:
		{
			char *echo_strings[4] =
			{
				"The day has begun.\r\n",
				"The day has begun.\r\n",
				"The sky slowly begins to glow.\r\n",
				"The sun slowly embarks upon a new day.\r\n"
			};
			weath->echo_time_snd = "fury_ambience_rooster.wav";
			time_info.sunlight = SUN_RISE;
			weath->echo = echo_strings[n];
			weath->echo_color = AT_YELLOW;
			break;
		}
		case 6:
		{
			char *echo_strings[4] =
			{
				"The sun rises in the east.\r\n",
				"The sun rises in the east.\r\n",
				"The hazy sun rises over the horizon.\r\n",
				"Day breaks as the sun lifts into the sky.\r\n"
			};
			weath->echo_time_snd = "fury_ambience_birds_2.wav";
			time_info.sunlight = SUN_LIGHT;
			weath->echo = echo_strings[n];
			weath->echo_color = AT_ORANGE;
			break;
		}
		case 12:
		{
			if ( pindex > 0 )
			{
				weath->echo = "It's noon.\r\n";
			}
			else
			{
				char *echo_strings[2] =
				{
					"The intensity of the sun " "heralds the noon hour.\r\n",
					"The sun's bright rays beat down " "upon your shoulders.\r\n"
				};
				weath->echo = echo_strings[n % 2];
			}
			weath->echo_time_snd = "fury_ambiance_birds.wav";
			time_info.sunlight = SUN_LIGHT;
			weath->echo_color = AT_WHITE;
			break;
		}
		case 19:
		{
			char *echo_strings[4] =
			{
				"The sun slowly disappears in the west.\r\n",
				"The reddish sun sets past the horizon.\r\n",
				"The sky turns a reddish orange as the sun " "ends its journey.\r\n",
				"The sun's radiance dims as it sinks in the " "sky.\r\n"
			};
			weath->echo_time_snd = "fury_ambience_night.wav";
			time_info.sunlight = SUN_SET;
			weath->echo = echo_strings[n];
			weath->echo_color = AT_RED;
			break;
		}
		case 20:
		{
			if ( pindex > 0 )
			{
				char *echo_strings[2] =
				{
					"The night begins.\r\n",
					"Twilight descends around you.\r\n"
				};
				weath->echo = echo_strings[n % 2];
			}
			else
			{
				char *echo_strings[2] =
				{
					"The moon's gentle glow diffuses " "through the night sky.\r\n",
					"The night sky gleams with " "glittering starlight.\r\n"
				};
				weath->echo = echo_strings[n % 2];
			}
			weath->echo_time_snd = "fury_ambiance_crickets.wav";
			time_info.sunlight = SUN_DARK;
			weath->echo_color = AT_DBLUE;
			break;
		}
	}
	return;
}

/*
 * update the time
 */
void time_update( )
{
	AREA_DATA *pArea;
	DESCRIPTOR_DATA *d;
	WEATHER_DATA *weath;
	switch ( ++time_info.hour )
	{
		case 5:
		case 6:
		case 12:
		case 19:
		case 20:
			for ( pArea = first_area; pArea; pArea = ( pArea == last_area ) ? first_build : pArea->next )
			{
				get_time_echo ( pArea->weather );
			}
			for ( d = first_descriptor; d; d = d->next )
			{
				if ( d->connected == CON_PLAYING && xIS_OUTSIDE ( d->character ) && IS_AWAKE ( d->character ) )
				{
					weath = d->character->in_room->area->weather;
					if ( !weath->echo )
						continue;
					set_char_color ( weath->echo_color, d->character );
					send_to_char ( weath->echo, d->character );
					send_mip_sound ( d->character, weath->echo_time_snd );
				}
			}
			break;
		case 24:
			time_info.hour = 0;
			time_info.day++;
			break;
	}
	if ( time_info.day >= 29 )
	{
		time_info.day = 0;
		time_info.month++;
	}
	if ( time_info.month >= 12 )
	{
		time_info.month = 0;
		time_info.year++;
	}
	/*
	 * if( time_info.day == 29 )
	 * {
	 * if( double_exp == FALSE )
	 * {
	 * do_double( NULL, "exp on 15" );
	 * }
	 *
	 * }
	 * if( time_info.day == 19 )
	 * {
	 * if( double_qp == FALSE )
	 * {
	 * do_double( NULL, "quest on 15" );
	 * }
	 *
	 * }
	 * if( time_info.day == 9 )
	 * {
	 * if( quad_damage == FALSE )
	 * {
	 * do_double( NULL, "quad on 15" );
	 * }
	 *
	 * }
	 *///TODO: Turn back on bonuses.
	return;
}
