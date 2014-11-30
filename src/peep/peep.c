/*****************************************************************************
* Copyright (c) 2014 Ted John
* OpenRCT2, an open source clone of Roller Coaster Tycoon 2.
*
* This file is part of OpenRCT2.
*
* OpenRCT2 is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "../addresses.h"
#include "../audio/audio.h"
#include "../audio/mixer.h"
#include "../interface/window.h"
#include "../localisation/localisation.h"
#include "../management/news_item.h"
#include "../ride/ride.h"
#include "../scenario.h"
#include "../sprites.h"
#include "../world/sprite.h"
#include "../world/scenery.h"
#include "peep.h"
#include "staff.h"

static void peep_update(rct_peep *peep);

const char *gPeepEasterEggNames[] = {
	"MICHAEL SCHUMACHER",
	"JACQUES VILLENEUVE",
	"DAMON HILL",
	"MR BEAN",
	"CHRIS SAWYER",
	"KATIE BRAYSHAW",
	"MELANIE WARN",
	"SIMON FOSTER",
	"JOHN WARDLEY",
	"LISA STIRLING",
	"DONALD MACRAE",
	"KATHERINE MCGOWAN",
	"FRANCES MCGOWAN",
	"CORINA MASSOURA",
	"CAROL YOUNG",
	"MIA SHERIDAN",
	"KATIE RODGER",
	"EMMA GARRELL",
	"JOANNE BARTON",
	"FELICITY ANDERSON",
	"KATIE SMITH",
	"EILIDH BELL",
	"NANCY STILLWAGON",
	"ANDY HINE",
	"ELISSA WHITE",
	"DAVID ELLIS"
};

int peep_get_staff_count()
{
	uint16 spriteIndex;
	rct_peep *peep;
	int count = 0;

	FOR_ALL_STAFF(spriteIndex, peep)
		count++;

	return count;
}

/**
 *
 *  rct2: 0x0068F0A9
 */
void peep_update_all()
{
	int i;
	uint16 spriteIndex;
	rct_peep* peep;	

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 0x0E)
		return;

	spriteIndex = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
	i = 0;
	while (spriteIndex != SPRITE_INDEX_NULL) {
		peep = &(g_sprite_list[spriteIndex].peep);
		spriteIndex = peep->next;

		if ((i & 0x7F) != (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) & 0x7F)) {
			peep_update(peep);
		} else {
			RCT2_CALLPROC_X(0x0068F41A, 0, 0, 0, i, (int)peep, 0, 0);
			if (peep->linked_list_type_offset == SPRITE_LINKEDLIST_OFFSET_PEEP)
				peep_update(peep);
		}

		i++;
	}
}

/* some sort of check to see if peep is connected to the ground?? */
int sub_68F3AE(rct_peep* peep){
	peep->var_C4++;
	if ((peep->var_C4 & 0xF) != (peep->sprite_index & 0xF))return 1;

	uint16 ebx = (peep->next_x | (peep->next_y << 8)) >> 5;
	rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER(ebx);

	uint8 map_type = MAP_ELEMENT_TYPE_PATH;
	if ((peep->next_z >> 8) & ((1 << 4) | (1 << 3))){
		map_type = MAP_ELEMENT_TYPE_SURFACE;
	}

	int z = peep->next_z & 0xFF;

	for (;; map_element++){
		if ((map_element->type & MAP_ELEMENT_TYPE_MASK) == map_type){
			if (z == map_element->base_height)return 1;
		}
		if (map_element->flags & MAP_ELEMENT_FLAG_LAST_TILE)break;
	}
	
	peep_decrement_num_riders(peep);
	peep->state = PEEP_STATE_FALLING;
	peep_window_state_update(peep);
	return 0;
}

void sub_693B58(rct_peep* peep){
	int ebx;
	if (peep->action >= 0xFE){
		ebx = RCT2_ADDRESS(0x981D8C, uint8)[peep->var_6D];
	}
	else{
		ebx = RCT2_ADDRESS(0x981D8F, uint8)[peep->action];
	}
	if (ebx == peep->var_6E)return;

	invalidate_sprite((rct_sprite*)peep);
	peep->var_6E = ebx;

	uint8* edx = RCT2_ADDRESS(0x98270C, uint8*)[peep->sprite_type * 2];
	peep->var_14 = edx[ebx * 4];
	peep->var_09 = edx[ebx * 4 + 1];
	peep->var_15 = edx[ebx * 4 + 2];
	// This is pointless as nothing will have changed.
	invalidate_sprite((rct_sprite*)peep);
}

static void peep_state_reset(rct_peep* peep){
	peep_decrement_num_riders(peep);
	peep->state = PEEP_STATE_1;
	peep_window_state_update(peep);

	RCT2_CALLPROC_X(0x00693BE5, 0, 0, 0, 0, (int)peep, 0, 0);
}

/* rct2: 0x6939EB 
 * Possibly peep update action frame.
 * Also used to move peeps to the correct position to
 * start an action. Returns 0 if the correct destination 
 * has not yet been reached.
 */
int sub_6939EB(sint16* x, sint16* y, rct_peep* peep){
	RCT2_GLOBAL(0xF1AEF0, uint8) = peep->var_70;
	if (peep->action == 0xFE){
		peep->action = 0xFF;
	}

	*x = peep->x - peep->destination_x;
	*y = peep->y - peep->destination_y;

	int x_delta = abs(*x);
	int y_delta = abs(*y);

	if (peep->action >= 0xFE){
		if (x_delta + y_delta <= peep->destination_tolerence){
			return 0;
		}
		int direction = 0;
		if (x_delta < y_delta){
			direction = 8;
			if (*y >= 0){
				direction = 24;
			}
		}
		else{
			direction = 16;
			if (*x >= 0){
				direction = 0;
			}
		}
		peep->sprite_direction = direction;
		*x = peep->x + RCT2_ADDRESS(0x981D7C, uint16)[direction / 4];
		*y = peep->y + RCT2_ADDRESS(0x981D7E, uint16)[direction / 4];
		int ebx = peep->var_E0 + 1;
		uint32* edi = RCT2_ADDRESS(0x982708, uint32*)[peep->sprite_type * 2];
		uint8* _edi = (uint8*)(edi[peep->var_6E * 2 + 1]);
		if (ebx >= *_edi){
			ebx = 0;
		}
		peep->var_E0 = ebx;
		peep->var_70 = _edi[ebx + 1];
		return 1;
	}
	
	int* edi = RCT2_ADDRESS(0x982708, uint32*)[peep->sprite_type * 2];
	uint8* _edi = (uint8*)(edi[peep->var_6E * 2 + 1]);
	peep->action_frame++;
	int ebx = _edi[peep->action_frame + 1];

	// If last frame of action
	if (ebx == 0xFF){
		peep->var_70 = 0;
		peep->action = 0xFF;
		sub_693B58(peep);
		invalidate_sprite((rct_sprite*)peep);
		*x = peep->x;
		*y = peep->y;
		return 1;
	}
	peep->var_70 = ebx;

	// If not throwing up and not at the frame where sick appears.
	if (peep->action != PEEP_ACTION_THROW_UP || peep->action_frame != 15){
		invalidate_sprite((rct_sprite*)peep);
		*x = peep->x;
		*y = peep->y;
		return 1;
	}

	// We are throwing up
	peep->hunger /= 2;
	peep->nausea_growth_rate /= 2;

	if (peep->nausea < 30)
		peep->nausea = 0;
	else
		peep->nausea -= 30;

	peep->var_45 |= (1 << 2);

	// Create sick at location
	RCT2_CALLPROC_X(0x67375D, peep->x, peep->sprite_direction, peep->y, peep->z, 0, 0, peep->sprite_index & 1);

	int sound_id = (scenario_rand() & 3) + 24;

	sound_play_panned(sound_id, 0x8001, peep->x, peep->y, peep->z);

	invalidate_sprite((rct_sprite*)peep);
	*x = peep->x;
	*y = peep->y;
	return 1;
}
/**
*  rct2: 0x0069A409
* Decreases rider count if on/entering a ride.
*/
void peep_decrement_num_riders(rct_peep* peep){
	if (peep->state == PEEP_STATE_ON_RIDE 
		|| peep->state == PEEP_STATE_ENTERING_RIDE){

		rct_ride* ride = GET_RIDE(peep->current_ride);
		ride->num_riders--;
		ride->var_14D |= 0xC;
	}
}

/**
 *  rct2: 0x0069A42F
 * Call after changing a peeps state to insure that
 * all relevant windows update. Note also increase ride
 * count if on/entering a ride.
 */
void peep_window_state_update(rct_peep* peep){
	
	rct_window* w = window_find_by_number(WC_PEEP, peep->sprite_index);
	if (w){
		RCT2_CALLPROC_X(w->event_handlers[WE_INVALIDATE], 0, 0, 0, 0, (int)w, 0, 0);
	}

	if (peep->type == PEEP_TYPE_GUEST){
		// Update action label
		widget_invalidate_by_number(WC_PEEP, peep->sprite_index, 12);

		if (peep->state == PEEP_STATE_ON_RIDE || peep->state == PEEP_STATE_ENTERING_RIDE){
			rct_ride* ride = GET_RIDE(peep->current_ride);
			ride->num_riders++;
			ride->var_14D |= 0xC;
		}

		window_invalidate_by_class(WC_GUEST_LIST);
	}
	else{
		// Update action label
		widget_invalidate_by_number(WC_PEEP, peep->sprite_index, 9);
		window_invalidate_by_class(WC_STAFF_LIST);
	}
}

/** New function removes peep from 
 * park existance. Works with staff.
 */
void peep_remove(rct_peep* peep){
	if (peep->type == PEEP_TYPE_GUEST){
		if (peep->var_2A == 0){
			RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16)--;
			RCT2_GLOBAL(0x9A9804, uint16) |= (1 << 2);
		}
		if (peep->state == PEEP_STATE_ENTERING_PARK){
			RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_HEADING_FOR_PARK, uint16)--;
		}
	}
	RCT2_CALLPROC_X(0x69A535, 0, 0, 0, 0, (int)peep, 0, 0);
}

/**
 * rct2: 0x690028
 * Falling and its subset drowning
 */
void peep_update_falling(rct_peep* peep){
	if (peep->action == PEEP_ACTION_DROWNING){
		// Check to see if we are ready to drown.
		sint16 x, y;
		sub_6939EB(&x, &y, peep);
		//RCT2_CALLPROC_X(0x6939EB, 0, 0, 0, 0, (int)peep, 0, 0);
		if (peep->action == PEEP_ACTION_DROWNING) return;
		if (!(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & 0x80000)){
			RCT2_GLOBAL(0x13CE952, uint16) = peep->name_string_idx;
			RCT2_GLOBAL(0x13CE954, uint32) = peep->id;
			news_item_add_to_queue(NEWS_ITEM_BLANK, 2347, peep->x | (peep->y << 16));
		}
		RCT2_GLOBAL(0x135882E, uint16) += 25;
		if (RCT2_GLOBAL(0x135882E, uint16) > 1000){
			RCT2_GLOBAL(0x135882E, uint16) = 1000;
		}
		peep_remove(peep);
		return;
	}

	// If not drowning then falling. Note: peeps 'fall' after leaving a ride/enter the park.

	rct_map_element *map_element = TILE_MAP_ELEMENT_POINTER((peep->y / 32) * 256 + (peep->x /32));
	rct_map_element *saved_map = NULL;
	int saved_height = 0;

	for (int final_element = 0; !final_element; map_element++){
		final_element = map_element->flags & MAP_ELEMENT_FLAG_LAST_TILE;

		// If a path check if we are on it
		if (map_element->type == MAP_ELEMENT_TYPE_PATH){
			int height = map_height_from_slope(peep->x, peep->y, map_element->properties.surface.slope)
				+ map_element->base_height * 8;

			if (height < peep->z - 1 || height - 4 > peep->z) continue;

			saved_height = height;
			saved_map = map_element;
			break;
		} // If a surface get the height and see if we are on it
		else if (map_element->type == MAP_ELEMENT_TYPE_SURFACE){
			// If the surface is water check to see if we could be drowning
			if (map_element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK){
				int height = (map_element->properties.surface.terrain & MAP_ELEMENT_WATER_HEIGHT_MASK) * 16;

				if (height - 4 >= peep->z && height < peep->z + 20){
					// Looks like we are drowning!
					invalidate_sprite((rct_sprite*)peep);
					sprite_move(peep->x, peep->y, height, (rct_sprite*)peep);
					// Drop balloon if held
					if (peep->item_standard_flags & PEEP_ITEM_BALLOON){
						peep->item_standard_flags &= ~PEEP_ITEM_BALLOON;

						if (peep->sprite_type == 19 && peep->x != 0x8000){
							create_balloon(peep->x, peep->y, height, peep->balloon_colour);
							peep->var_45 |= (1 << 3);
							RCT2_CALLPROC_X(0x0069B8CC, 0, 0, 0, 0, (int)peep, 0, 0);
						}
					}

					peep_insert_new_thought(peep, PEEP_THOUGHT_TYPE_DROWNING, -1);

					peep->action = PEEP_ACTION_DROWNING;
					peep->action_frame = 0;
					peep->var_70 = 0;

					sub_693B58(peep);
					invalidate_sprite((rct_sprite*)peep);
					peep_window_state_update(peep);
					return;
				}
			}
			int map_height = map_element_height(0xFFFF & peep->x, 0xFFFF & peep->y) & 0xFFFF;
			if (map_height < peep->z || map_height - 4 > peep->z) continue;
			saved_height = map_height;
			saved_map = map_element;
		} // If not a path or surface go see next element
		else continue;
	}

	// This will be null if peep is falling
	if (saved_map == NULL){
		invalidate_sprite((rct_sprite*)peep);
		if (peep->z <= 1){
			// Remove peep if it has gone to the void
			peep_remove(peep);
			return;
		}
		sprite_move(peep->x, peep->y, peep->z - 2, (rct_sprite*)peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}
	
	invalidate_sprite((rct_sprite*)peep);
	sprite_move(peep->x, peep->y, saved_height, (rct_sprite*)peep);
	invalidate_sprite((rct_sprite*)peep);

	peep->next_x = peep->x & 0xFFE0;
	peep->next_y = peep->y & 0xFFE0;
	peep->next_z = saved_map->base_height;

	int edx = saved_map->properties.surface.slope & 0x7;
	if (saved_map->type != MAP_ELEMENT_TYPE_PATH){
		edx = 8;
	}
	peep->next_z += edx << 8;
	peep_decrement_num_riders(peep);
	peep->state = PEEP_STATE_1;
	peep_window_state_update(peep);
}

/**
 * rct2: 0x00691677
 */
void peep_try_get_up_from_sitting(rct_peep* peep){
	// Eats all food first
	if (peep_has_food(peep))return;

	peep->time_to_sitdown--;
	if (peep->time_to_sitdown) return;

	peep_decrement_num_riders(peep);
	peep->state = PEEP_STATE_WALKING;
	peep_window_state_update(peep);

	// Set destination to the center of the tile.
	peep->destination_x = (peep->x & 0xFFE0) + 16;
	peep->destination_y = (peep->y & 0xFFE0) + 16;
	peep->destination_tolerence = 5;
	sub_693B58(peep);
}

/**
 * rct2: 0x0069152B 
 */
void peep_update_sitting(rct_peep* peep){
	if (peep->var_2C == 0){
		if (!sub_68F3AE(peep))return;
		//691541

		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 1))return;

		int ebx = peep->var_37 & 0x7;
		int x = (peep->x & 0xFFE0) + RCT2_ADDRESS(0x981F2C, uint16)[ebx * 2];
		int y = (peep->y & 0xFFE0) + RCT2_ADDRESS(0x981F2E, uint16)[ebx * 2];
		int z = peep->z;		
		
		invalidate_sprite((rct_sprite*)peep);
		sprite_move(x, y, z, (rct_sprite*)peep);

		peep->sprite_direction = ((peep->var_37 + 2) & 3) * 8;
		invalidate_sprite((rct_sprite*)peep);
		peep->action = 254;
		peep->var_6F = 7;
		RCT2_CALLPROC_X(0x693BAB, 0, 0, 0, 0, (int)peep, 0, 0);

		peep->var_2C++;

		// Sets time to sit on seat
		peep->time_to_sitdown = (129 - peep->energy) * 16 + 50;
	}
	else if (peep->var_2C == 1){
		if (peep->action < 0xFE){
			sint16 x, y;
			sub_6939EB(&x, &y, peep);
			//RCT2_CALLPROC_X(0x6939EB, 0, 0, 0, 0, (int)peep, 0, 0);
			if (peep->action != 0xFF) return;

			peep->action = 0xFE;
			peep_try_get_up_from_sitting(peep);
			return;
		}
		
		if ((peep->flags & PEEP_FLAGS_LEAVING_PARK)){
			peep_decrement_num_riders(peep);
			peep->state = PEEP_STATE_WALKING;
			peep_window_state_update(peep);

			// Set destination to the center of the tile
			peep->destination_x = (peep->x & 0xFFE0) + 16;
			peep->destination_y = (peep->y & 0xFFE0) + 16;
			peep->destination_tolerence = 5;
			sub_693B58(peep);
			return;
		}
		
		if (peep->sprite_type == 0x15){
			peep_try_get_up_from_sitting(peep);
			return;
		}

		if (peep_has_food(peep)){
			if ((scenario_rand() & 0xFFFF) > 1310){
				peep_try_get_up_from_sitting(peep);
				return;
			}
			peep->action = PEEP_ACTION_SITTING_EAT_FOOD;
			peep->action_frame = 0;
			peep->var_70 = 0;
			sub_693B58(peep);
			invalidate_sprite((rct_sprite*)peep);
			return;
		}

		int rand = scenario_rand();
		if ((rand & 0xFFFF) > 131){
			peep_try_get_up_from_sitting(peep);
			return;
		}
		if (peep->sprite_type == 0x13 || peep->sprite_type == 0x1E){
			peep_try_get_up_from_sitting(peep);
			return;
		}

		peep->action = PEEP_ACTION_SITTING_LOOK_AROUND_LEFT;
		if (rand & 0x80000000){
			peep->action = PEEP_ACTION_SITTING_LOOK_AROUND_RIGHT;
		}

		if (rand & 0x40000000){
			peep->action = PEEP_ACTION_SITTING_CHECK_WATCH;
		}
		peep->action_frame = 0;
		peep->var_70 = 0;
		sub_693B58(peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}
}

/* rct2: 0x691A30 
 * Also used by entering_ride and queueing_front */
static void peep_update_leaving_ride(rct_peep* peep){
	RCT2_CALLPROC_X(RCT2_ADDRESS(0x9820DC, int)[peep->var_2C], 0, 0, 0, 0, (int)peep, 0, 0);
}

/**
 * rct2: 0x69185D
 */
static void peep_update_queuing(rct_peep* peep){
	if (!sub_68F3AE(peep)){
		RCT2_CALLPROC_X(0x6966A9, 0, 0, 0, 0, (int)peep, 0, 0);
		return;
	}
	rct_ride* ride = GET_RIDE(peep->current_ride);
	if (ride->status == RIDE_STATUS_CLOSED || ride->status == RIDE_STATUS_TESTING){
		RCT2_CALLPROC_X(0x6966A9, 0, 0, 0, 0, (int)peep, 0, 0);
		peep_decrement_num_riders(peep);
		peep->state = PEEP_STATE_1;
		peep_window_state_update(peep);
		return;
	}

	if (peep->var_2C != 0xA){
		if (peep->var_74 == 0xFFFF){
			//Happens every time peep goes onto ride.
			peep->destination_tolerence = 0;
			peep_decrement_num_riders(peep);
			peep->state = PEEP_STATE_QUEUING_FRONT;
			peep_window_state_update(peep);
			peep->var_2C = 0;
			return;
		}
		//Give up queueing for the ride
		peep->sprite_direction ^= (1 << 4);
		invalidate_sprite((rct_sprite*)peep);
		RCT2_CALLPROC_X(0x6966A9, 0, 0, 0, 0, (int)peep, 0, 0);
		peep_decrement_num_riders(peep);
		peep->state = PEEP_STATE_1;
		peep_window_state_update(peep);
	}

	RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
	if (peep->action < 0xFE)return;
	if (peep->sprite_type == 0){
		if (peep->var_7A >= 2000 && (0xFFFF & scenario_rand()) <= 119){
			// Look at watch
			peep->action = PEEP_ACTION_CHECK_WATCH;
			peep->action_frame = 0;
			peep->var_70 = 0;
			sub_693B58(peep);
			invalidate_sprite((rct_sprite*)peep);
		}
		if (peep->var_7A >= 3500 && (0xFFFF & scenario_rand()) <= 93)
		{
			//Create the ive been waiting in line ages thought
			peep_insert_new_thought(peep, PEEP_THOUGHT_TYPE_QUEUING_AGES, peep->current_ride);
		}
	}
	else{
		if (!(peep->var_7A & 0x3F) && peep->action == 0xFE && peep->var_6F == 2){
			switch (peep->sprite_type){
			case 0xF:
			case 0x10:
			case 0x11:
			case 0x12:
			case 0x14:
			case 0x16:
			case 0x18:
			case 0x1F:
			case 0x20:
			case 0x21:
			case 0x22:
			case 0x23:
			case 0x24:
			case 0x25:
			case 0x27:
			case 0x29:
			case 0x2A:
			case 0x2B:
			case 0x2C:
			case 0x2D:
			case 0x2E:
			case 0x2F:
				// Look at watch
				peep->action = PEEP_ACTION_CHECK_WATCH;
				peep->action_frame = 0;
				peep->var_70 = 0;
				sub_693B58(peep);
				invalidate_sprite((rct_sprite*)peep);
				break;
			}
		}
	}
	if (peep->var_7A < 4300) return;

	if (peep->happiness <= 65 && (0xFFFF & scenario_rand()) < 2184){
		//Give up queueing for the ride
		peep->sprite_direction ^= (1 << 4);
		invalidate_sprite((rct_sprite*)peep);
		RCT2_CALLPROC_X(0x6966A9, 0, 0, 0, 0, (int)peep, 0, 0);
		peep_decrement_num_riders(peep);
		peep->state = PEEP_STATE_1;
		peep_window_state_update(peep);
	}
}

/* rct2: 0x006BF567 */
static void peep_update_mowing(rct_peep* peep){
	peep->var_E2 = 0;
	if (!sub_68F3AE(peep))return;

	invalidate_sprite((rct_sprite*)peep);
	while (1){
		sint16 x = 0, y = 0;
		if (sub_6939EB(&x, &y, peep)){
			int eax = x, ebx, ecx = y, z, ebp, edi;

			RCT2_CALLFUNC_X(0x662783, &eax, &ebx, &ecx, &z, (int*)&peep, &edi, &ebp);
			x = eax;
			y = ecx;
			sprite_move(x, y, z, (rct_sprite*)peep);
			invalidate_sprite((rct_sprite*)peep);
			return;
		}

		peep->var_37++;

		if (peep->var_37 == 1){
			RCT2_CALLPROC_X(0x00693BE5, 2, 0, 0, 0, (int)peep, 0, 0);
		}

		if (RCT2_ADDRESS(0x9929C8, uint16)[peep->var_37 * 2] == 0xFFFF){
			peep_state_reset(peep);
			return;
		}

		peep->destination_x = RCT2_ADDRESS(0x9929C8, uint16)[peep->var_37 * 2] + peep->next_x;
		peep->destination_y = RCT2_ADDRESS(0x9929CA, uint16)[peep->var_37 * 2] + peep->next_y;

		if (peep->var_37 != 7)continue;

		rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER((peep->next_x | (peep->next_y << 8)) >> 5);

		for (; ((map_element->type & MAP_ELEMENT_TYPE_MASK) != MAP_ELEMENT_TYPE_SURFACE); map_element++);

		if ((map_element->properties.surface.terrain & MAP_ELEMENT_SURFACE_TERRAIN_MASK) == (TERRAIN_GRASS << 5)){
			map_element->properties.surface.grass_length = 0;
			gfx_invalidate_scrollingtext(peep->next_x, peep->next_y, map_element->base_height * 8, map_element->base_height * 8 + 16);
		}
		peep->staff_lawns_mown++;
		peep->var_45 |= (1 << 5);
	}
}

/* rct2: 0x006BF7E6 */
static void peep_update_watering(rct_peep* peep){
	peep->var_E2 = 0;
	if (peep->var_2C == 0){
		if (!sub_68F3AE(peep))return;

		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 1))return;

		peep->sprite_direction = (peep->var_37 & 3) << 3;
		peep->action = PEEP_ACTION_STAFF_WATERING;
		peep->action_frame = 0;
		peep->var_70 = 0;
		sub_693B58(peep);
		invalidate_sprite((rct_sprite*)peep);		
		
		peep->var_2C = 1;
	}
	else if (peep->var_2C == 1){
		if (peep->action != PEEP_ACTION_NONE_2){
			sint16 x, y;
			sub_6939EB(&x, &y, peep);
			return;
		}

		int x = peep->next_x + RCT2_ADDRESS(0x993CCC, sint16)[peep->var_37 * 2];
		int y = peep->next_y + RCT2_ADDRESS(0x993CCE, sint16)[peep->var_37 * 2];

		rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER((x | (y << 8)) >> 5);

		for (;; map_element++){
			if ((map_element->type & MAP_ELEMENT_TYPE_MASK) == MAP_ELEMENT_TYPE_SCENERY){
				if (abs((peep->next_z & 0xFF) - map_element->base_height) <= 4){
					rct_scenery_entry* scenery_entry = g_smallSceneryEntries[map_element->properties.scenery.type];

					if (scenery_entry->small_scenery.flags& SMALL_SCENERY_FLAG6){
						map_element->properties.scenery.age = 0;
						gfx_invalidate_scrollingtext(x, y, map_element->base_height * 8, map_element->clearance_height * 8);
						peep->staff_gardens_watered++;
						peep->var_45 |= (1 << 4);
					}
				}
			}
			if (map_element->flags&MAP_ELEMENT_FLAG_LAST_TILE){
				peep_state_reset(peep);
				return;
			}
		}
	}
}

/* rct2: 0x006BF6C9 */
static void peep_update_emptying_bin(rct_peep* peep){
	peep->var_E2 = 0;

	if (peep->var_2C == 0){
		if (!sub_68F3AE(peep))return;

		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 1))return;

		peep->sprite_direction = (peep->var_37 & 3) << 3;
		peep->action = PEEP_ACTION_STAFF_EMPTY_BIN;
		peep->action_frame = 0;
		peep->var_70 = 0;
		sub_693B58(peep);
		invalidate_sprite((rct_sprite*)peep);

		peep->var_2C = 1;
	}
	else if (peep->var_2C == 1){

		if (peep->action == PEEP_ACTION_NONE_2){
			peep_state_reset(peep);
			return;
		}

		sint16 x = 0, y = 0;
		sub_6939EB(&x, &y, peep);

		if (peep->action_frame != 11)return;

		rct_map_element* map_element = TILE_MAP_ELEMENT_POINTER((peep->next_x | (peep->next_y << 8)) >> 5);

		for (;; map_element++){
			if ((map_element->type & MAP_ELEMENT_TYPE_MASK) == MAP_ELEMENT_TYPE_PATH){
				if ((peep->next_z & 0xFF) == map_element->base_height)break;
			}
			if (map_element->flags&MAP_ELEMENT_FLAG_LAST_TILE){
				peep_state_reset(peep);
				return;
			}
		}

		if ((map_element->properties.path.additions & 0xF) == 0){
			peep_state_reset(peep);
			return;
		}

		rct_scenery_entry* scenery_entry = g_pathBitSceneryEntries[(map_element->properties.path.additions & 0xF) - 1];
		if (!(scenery_entry->small_scenery.flags & SMALL_SCENERY_FLAG1)
			|| map_element->flags&(1 << 5)
			|| map_element->properties.path.additions & (1 << 7)){
			peep_state_reset(peep);
			return;
		}

		map_element->properties.path.addition_status = ((3 << peep->var_37) << peep->var_37);

		gfx_invalidate_scrollingtext(peep->next_x, peep->next_y, map_element->base_height * 8, map_element->clearance_height * 8);

		peep->staff_bins_emptied++;
		peep->var_45 |= (1 << 4);
	}
}

/* rct2: 0x6BF641 */
static void peep_update_sweeping(rct_peep* peep){
	peep->var_E2 = 0;
	if (!sub_68F3AE(peep))return;
	
	invalidate_sprite((rct_sprite*)peep);

	if (peep->action == PEEP_ACTION_STAFF_SWEEP && peep->action_frame == 8){
		//Remove sick at this location		
		RCT2_CALLPROC_X(0x6738E1, peep->x, 0, peep->y, peep->z, 0, 0, 0);
		peep->staff_litter_swept++;
		peep->var_45 |= (1 << 4);
	}
	sint16 x = 0, y = 0;
	if (sub_6939EB(&x, &y, peep)){
		int eax = x, ebx, ecx = y, z, ebp, edi;

		RCT2_CALLFUNC_X(0x694921, &eax, &ebx, &ecx, &z, (int*)&peep, &edi, &ebp);
		x = eax;
		y = ecx;
		sprite_move(x, y, z, (rct_sprite*)peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}

	peep->var_37++;
	if (peep->var_37 != 2){
		peep->action = PEEP_ACTION_STAFF_SWEEP;
		peep->action_frame = 0;
		peep->var_70 = 0;
		sub_693B58(peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}
	peep_state_reset(peep);
}

/* rct2: 0x6902A2 */
static void peep_update_1(rct_peep* peep){
	if (!sub_68F3AE(peep))return;

	peep_decrement_num_riders(peep);

	if (peep->type == PEEP_TYPE_GUEST){
		peep->state = PEEP_STATE_WALKING;
	}
	else{
		peep->state = PEEP_STATE_PATROLLING;
	}
	peep_window_state_update(peep);
	peep->destination_x = peep->x;
	peep->destination_y = peep->y;
	peep->destination_tolerence = 10;
	peep->var_76 = 0;
	peep->var_78 = peep->sprite_direction >> 3;
}

/**
 * rct2: 0x690009
 */
static void peep_update_picked(rct_peep* peep){
	if (RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_TICKS, uint32) & 0x1F) return;
	peep->var_2C++;
	if (peep->var_2C == 13){
		peep_insert_new_thought(peep, PEEP_THOUGHT_HELP, 0xFF);
	}
}

/* rct2: 0x6914CD */
static void peep_update_leaving_park(rct_peep* peep){
	if (peep->var_37 != 0){
		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 2))return;
		RCT2_CALLPROC_X(0x69A535, 0, 0, 0, 0, (int)peep, 0, 0);
		return;
	}

	sint16 x = 0, y = 0;
	if (sub_6939EB(&x, &y, peep)){
		invalidate_sprite((rct_sprite*)peep);
		sprite_move(x, y, peep->z, (rct_sprite*)peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}

	peep->var_2A = 1;
	peep->destination_tolerence = 5;
	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16)--;
	RCT2_GLOBAL(0x9A9804, uint16) |= (1 << 0);
	peep->var_37 = 1;

	window_invalidate_by_class(WC_GUEST_LIST);

	RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
	if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 2))return;
	RCT2_CALLPROC_X(0x69A535, 0, 0, 0, 0, (int)peep, 0, 0);
}

/* rct2: 0x6916D6 */
static void peep_update_watching(rct_peep* peep){
	if (peep->var_2C == 0){
		if (!sub_68F3AE(peep))return;

		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if (!(RCT2_GLOBAL(0xF1EE18, uint16) & 1))return;

		peep->destination_x = peep->x;
		peep->destination_y = peep->y;

		peep->sprite_direction = (peep->var_37 & 3) * 8;
		invalidate_sprite((rct_sprite*)peep);

		peep->action = 0xFE;
		peep->var_6F = 2;

		RCT2_CALLPROC_X(0x693BAB, 0, 0, 0, 0, (int)peep, 0, 0);

		peep->var_2C++;

		peep->time_to_stand = clamp(0, ((129 - peep->energy) * 16 + 50) / 2, 255);
		RCT2_CALLPROC_X(0x0069B8CC, 0, 0, 0, 0, (int)peep, 0, 0);
	}
	else if (peep->var_2C == 1){
		if (peep->action < 0xFE){
			//6917F6
			sint16 x = 0, y = 0;
			sub_6939EB(&x, &y, peep);

			if (peep->action != 0xFF)return;
			peep->action = 0xFE;
		}
		else{
			if (peep_has_food(peep)){
				if ((scenario_rand() & 0xFFFF) <= 1310){
					peep->action = PEEP_ACTION_CHECK_WATCH;
					peep->action_frame = 0;
					peep->var_70 = 0;
					sub_693B58(peep);
					invalidate_sprite((rct_sprite*)peep);
					return;
				}
			}
			
			if ((scenario_rand() & 0xFFFF) <= 655){
				peep->action = PEEP_ACTION_TAKE_PHOTO;
				peep->action_frame = 0;
				peep->var_70 = 0;
				sub_693B58(peep);
				invalidate_sprite((rct_sprite*)peep);
				return;
			}
				
			if ((peep->standing_flags & 1)){
				if ((scenario_rand() & 0xFFFF) <= 655){
					peep->action = PEEP_ACTION_WAVE;
					peep->action_frame = 0;
					peep->var_70 = 0;
					sub_693B58(peep);
					invalidate_sprite((rct_sprite*)peep);
					return;
				}
			}
		}

		peep->standing_flags ^= (1 << 7);
		if (!(peep->standing_flags & (1 << 7)))return;

		peep->time_to_stand--;
		if (peep->time_to_stand != 0)return;

		peep_decrement_num_riders(peep);
		peep->state = PEEP_STATE_WALKING;
		peep_window_state_update(peep);
		RCT2_CALLPROC_X(0x0069B8CC, 0, 0, 0, 0, (int)peep, 0, 0);
		// Send peep to the center of current tile.
		peep->destination_x = (peep->x & 0xFFE0) + 16;
		peep->destination_y = (peep->y & 0xFFE0) + 16;
		peep->destination_tolerence = 5;
		sub_693B58(peep);
	}
}

/**
* rct2: 0x691451
*/
static void peep_update_entering_park(rct_peep* peep){
	if (peep->var_37 != 1){
		RCT2_CALLPROC_X(0x693C9E, 0, 0, 0, 0, (int)peep, 0, 0);
		if ((RCT2_GLOBAL(0xF1EE18, uint16) & 2)){
			RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_HEADING_FOR_PARK, uint16)--;
			RCT2_CALLPROC_X(0x69A535, 0, 0, 0, 0, (int)peep, 0, 0);
		}
		return;
	}
	sint16 x = 0, y = 0;
	if (sub_6939EB(&x, &y, peep)){
		invalidate_sprite((rct_sprite*)peep); 
		sprite_move(x, y, peep->z, (rct_sprite*)peep);
		invalidate_sprite((rct_sprite*)peep);
		return;
	}
	peep_decrement_num_riders(peep);
	peep->state = PEEP_STATE_FALLING;
	peep_window_state_update(peep);

	peep->var_2A = 0;
	peep->time_in_park = RCT2_GLOBAL(RCT2_ADDRESS_SCENARIO_TICKS, sint32);
	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16)++;
	RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_HEADING_FOR_PARK, uint16)--;
	RCT2_GLOBAL(0x9A9804, uint16) |= (1 << 2);
	window_invalidate_by_class(WC_GUEST_LIST);
}

/* From peep_update */
static void peep_update_thoughts(rct_peep* peep){
	// Thoughts must always have a gap of at least
	// 220 ticks in age between them. In order to 
	// allow this when a thought is new it enters
	// a holding zone. Before it becomes fresh.
	int add_fresh = 1;
	int fresh_thought = -1;
	for (int i = 0; i < PEEP_MAX_THOUGHTS; i++) {
		if (peep->thoughts[i].type == PEEP_THOUGHT_TYPE_NONE)
			break;


		if (peep->thoughts[i].var_2 == 1) {
			add_fresh = 0;		
			// If thought is fresh we wait 220 ticks
			// before allowing a new thought to become fresh.
			if (++peep->thoughts[i].var_3 >= 220) {
				peep->thoughts[i].var_3 = 0;
				// Thought is no longer fresh
				peep->thoughts[i].var_2++;
				add_fresh = 1;
			}
		}
		else if (peep->thoughts[i].var_2 > 1) {
			if (++peep->thoughts[i].var_3 == 0) {			
				// When thought is older than ~6900 ticks remove it
				if (++peep->thoughts[i].var_2 >= 28) {
					peep->var_45 |= 1;

					// Clear top thought, push others up
					memmove(&peep->thoughts[i], &peep->thoughts[i + 1], sizeof(rct_peep_thought)*(PEEP_MAX_THOUGHTS - i - 1));
					peep->thoughts[PEEP_MAX_THOUGHTS - 1].type = PEEP_THOUGHT_TYPE_NONE;
				}
			}
		}
		else {
			fresh_thought = i;
		}
	}
	// If there are no fresh thoughts 
	// a previously new thought can become 
	// fresh.
	if (add_fresh && fresh_thought != -1) {
		peep->thoughts[fresh_thought].var_2 = 1;
		peep->var_45 |= 1;
	}
}

/**
 *
 *  rct2: 0x0068FC1E
 */
static void peep_update(rct_peep *peep)
{
	//RCT2_CALLPROC_X(0x0068FC1E, 0, 0, 0, 0, (int)peep, 0, 0); return;
	//return;

	if (peep->type == PEEP_TYPE_GUEST) {
		if (peep->var_AD != 255)
			if (++peep->var_AE < 720)
				peep->var_AD = 255;

		peep_update_thoughts(peep);
	}

	// Walking speed logic
	unsigned int stepsToTake = peep->energy;
	if (stepsToTake < 95 && peep->state == PEEP_STATE_QUEUING)
		stepsToTake = 95;
	if ((peep->flags & PEEP_FLAGS_SLOW_WALK) && peep->state != PEEP_STATE_QUEUING)
		stepsToTake /= 2;
	if (peep->action == 255 && ((peep->next_z >> 8) & 4)) {
		stepsToTake /= 2;
		if (peep->state == PEEP_STATE_QUEUING)
			stepsToTake += stepsToTake / 2;
	}

	unsigned int carryCheck = peep->var_73 + stepsToTake;
	peep->var_73 = carryCheck;
	if (carryCheck <= 255) {
		// loc_68FD3A
		RCT2_CALLPROC_X(0x0068FD3A, 0, 0, 0, 0, (int)peep, 0, 0);
	} else {
		// loc_68FD2F
		//RCT2_CALLPROC_X(0x68FD2F, 0, 0, 0, 0, (int)peep, 0, 0);
		//return;
		switch (peep->state) {
		case PEEP_STATE_FALLING:
			peep_update_falling(peep);
			break;
		case PEEP_STATE_1:
			peep_update_1(peep);
			break;
		case PEEP_STATE_QUEUING_FRONT:
			peep_update_leaving_ride(peep);
			break;
		case PEEP_STATE_ON_RIDE:
			// No action
			break;
		case PEEP_STATE_LEAVING_RIDE:
			peep_update_leaving_ride(peep);
			break;
		case PEEP_STATE_WALKING:
			RCT2_CALLPROC_X(0x0069030A, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_QUEUING:
			peep_update_queuing(peep);
			break;
		case PEEP_STATE_ENTERING_RIDE:
			// Calls the same function as leaving ride
			peep_update_leaving_ride(peep);
			break;
		case PEEP_STATE_SITTING:
			peep_update_sitting(peep);
			break;
		case PEEP_STATE_PICKED:
			peep_update_picked(peep);
			break;
		case PEEP_STATE_PATROLLING:
			RCT2_CALLPROC_X(0x006BF1FD, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_MOWING:
			peep_update_mowing(peep);
			break;
		case PEEP_STATE_SWEEPING:
			peep_update_sweeping(peep);
			break;
		case PEEP_STATE_ENTERING_PARK:
			peep_update_entering_park(peep);
			break;
		case PEEP_STATE_LEAVING_PARK:
			peep_update_leaving_park(peep);
			break;
		case PEEP_STATE_ANSWERING:
			RCT2_CALLPROC_X(0x006C0CB8, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_FIXING:
			RCT2_CALLPROC_X(0x006C0E8B, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_BUYING:
			RCT2_CALLPROC_X(0x006912A3, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_WATCHING:
			peep_update_watching(peep);
			break;
		case PEEP_STATE_EMPTYING_BIN:
			peep_update_emptying_bin(peep);
			break;
		case PEEP_STATE_20:
			RCT2_CALLPROC_X(0x00691089, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_WATERING:
			peep_update_watering(peep);
			break;
		case PEEP_STATE_HEADING_TO_INSPECTION:
			RCT2_CALLPROC_X(0x006C16D7, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		case PEEP_STATE_INSPECTING:
			RCT2_CALLPROC_X(0x006C0E8B, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
			//There shouldnt be any more
		default:		
			RCT2_CALLPROC_X(0x0068FD2F, 0, 0, 0, 0, (int)peep, 0, 0);
			break;
		}
	}
}


/**
 *
 * rct2: 0x0069BF41
 **/
void peep_problem_warnings_update()
{
	rct_peep* peep;
	rct_ride* ride;
	uint16 spriteIndex;
	uint16 guests_in_park = RCT2_GLOBAL(RCT2_ADDRESS_GUESTS_IN_PARK, uint16);
	int hunger_counter = 0, lost_counter = 0, noexit_counter = 0, thirst_counter = 0,
		litter_counter = 0, disgust_counter = 0, bathroom_counter = 0 ,vandalism_counter = 0;
	static int warning_throttle[7] = { 0, 0, 0, 0, 0, 0, 0 };

	RCT2_GLOBAL(RCT2_ADDRESS_RIDE_COUNT, sint16) = ride_get_count(); // refactor this to somewhere else

	FOR_ALL_GUESTS(spriteIndex, peep) {
		if (peep->var_2A != 0 || peep->thoughts[0].var_2 > 5)
			continue;

		switch (peep->thoughts[0].type) {
		case PEEP_THOUGHT_TYPE_LOST: //0x10
			lost_counter++;
			break;

		case PEEP_THOUGHT_TYPE_HUNGRY: // 0x14
			if (peep->guest_heading_to_ride_id == -1){
				hunger_counter++;
				break;
			}
			ride = &g_ride_list[peep->guest_heading_to_ride_id];
			if (!(RCT2_GLOBAL(RCT2_ADDRESS_RIDE_FLAGS + ride->type * 8, uint32) & 0x80000))
				hunger_counter++;
			break;

		case PEEP_THOUGHT_TYPE_THIRSTY:
			if (peep->guest_heading_to_ride_id == -1){
				thirst_counter++;
				break;
			}
			ride = &g_ride_list[peep->guest_heading_to_ride_id];
			if (!(RCT2_GLOBAL(RCT2_ADDRESS_RIDE_FLAGS + ride->type * 8, uint32) & 0x1000000))
				thirst_counter++;
			break;

		case PEEP_THOUGHT_TYPE_BATHROOM:
			if (peep->guest_heading_to_ride_id == -1){
				bathroom_counter++;
				break;
			}
			ride = &g_ride_list[peep->guest_heading_to_ride_id];
			if (!(RCT2_GLOBAL(RCT2_ADDRESS_RIDE_FLAGS + ride->type * 8, uint32) & 0x2000000))
				bathroom_counter++;
			break;

		case PEEP_THOUGHT_TYPE_BAD_LITTER: // 0x1a
			litter_counter++;
			break;
		case PEEP_THOUGHT_TYPE_CANT_FIND_EXIT: // 0x1b
			noexit_counter++;
			break;
		case PEEP_THOUGHT_TYPE_PATH_DISGUSTING: // 0x1f
			disgust_counter++;
			break;
		case PEEP_THOUGHT_TYPE_VANDALISM: //0x21
			vandalism_counter++;
			break;
		default:
			break;
		}
	}
	// could maybe be packed into a loop, would lose a lot of clarity though
	if (warning_throttle[0])
		--warning_throttle[0];
	else if ( hunger_counter >= PEEP_HUNGER_WARNING_THRESHOLD && hunger_counter >= guests_in_park / 16) {
		warning_throttle[0] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_ARE_HUNGRY, 20);
	}

	if (warning_throttle[1])
		--warning_throttle[1];
	else if (thirst_counter >= PEEP_THIRST_WARNING_THRESHOLD && thirst_counter >= guests_in_park / 16) {
		warning_throttle[1] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_ARE_THIRSTY, 21);
	}

	if (warning_throttle[2])
		--warning_throttle[2];
	else if (bathroom_counter >= PEEP_BATHROOM_WARNING_THRESHOLD && bathroom_counter >= guests_in_park / 16) {
		warning_throttle[2] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_CANT_FIND_BATHROOM, 22);
	}

	if (warning_throttle[3])
		--warning_throttle[3];
	else if (litter_counter >= PEEP_LITTER_WARNING_THRESHOLD && litter_counter >= guests_in_park / 32) {
		warning_throttle[3] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_DISLIKE_LITTER, 26);
	}

	if (warning_throttle[4])
		--warning_throttle[4];
	else if (disgust_counter >= PEEP_DISGUST_WARNING_THRESHOLD && disgust_counter >= guests_in_park / 32) {
		warning_throttle[4] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_DISGUSTED_BY_PATHS, 31);
	}

	if (warning_throttle[5])
		--warning_throttle[5];
	else if (vandalism_counter >= PEEP_VANDALISM_WARNING_THRESHOLD && vandalism_counter >= guests_in_park / 32) {
		warning_throttle[5] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_DISLIKE_VANDALISM, 33);
	}

	if (warning_throttle[6])
		--warning_throttle[6];
	else if (noexit_counter >= PEEP_NOEXIT_WARNING_THRESHOLD) {
		warning_throttle[6] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_GETTING_LOST_OR_STUCK, 27);
	} else if (lost_counter >= PEEP_LOST_WARNING_THRESHOLD) {
		warning_throttle[6] = 4;
		news_item_add_to_queue(NEWS_ITEM_PEEPS, STR_PEEPS_GETTING_LOST_OR_STUCK, 16);
	}
}

/**
 *
 *  rct2: 0x006BD18A
 */
void peep_update_crowd_noise()
{
	rct_viewport *viewport;
	uint16 spriteIndex;
	rct_peep *peep;
	int visiblePeeps;

	if (!(RCT2_GLOBAL(0x009AF284, uint32) & (1 << 0)))
		return;

	if (RCT2_GLOBAL(0x009AF59C, uint8) != 0)
		return;

	if (!(RCT2_GLOBAL(0x009AF59D, uint8) & (1 << 0)))
		return;

	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 2)
		return;

	viewport = RCT2_GLOBAL(0x00F438A4, rct_viewport*);
	if (viewport == (rct_viewport*)-1)
		return;

	// Count the number of peeps visible
	visiblePeeps = 0;

	FOR_ALL_GUESTS(spriteIndex, peep) {
		if (peep->sprite_left == (sint16)0x8000)
			continue;
		if (viewport->view_x > peep->sprite_right)
			continue;
		if (viewport->view_x + viewport->view_width < peep->sprite_left)
			continue;
		if (viewport->view_y > peep->sprite_bottom)
			continue;
		if (viewport->view_y + viewport->view_height < peep->sprite_top)
			continue;

		visiblePeeps += peep->state == PEEP_STATE_QUEUING ? 1 : 2;
	}

	// This function doesn't account for the fact that the screen might be so big that 100 peeps could potentially be very
	// spread out and therefore not produce any crowd noise. Perhaps a more sophisticated solution would check how many peeps
	// were in close proximity to each other.

	// Allows queuing peeps to make half as much noise, and at least 6 peeps must be visible for any crowd noise
	visiblePeeps = (visiblePeeps / 2) - 6;
	if (visiblePeeps < 0) {
		// Mute crowd noise
		if (RCT2_GLOBAL(0x009AF5FC, uint32) != 1) {
#ifdef USE_MIXER
			Mixer_Stop_Channel(gCrowdSoundChannel);
			gCrowdSoundChannel = 0;
#else
			sound_channel_stop(2); //RCT2_CALLPROC_1(0x00401A05, int, 2);
#endif
			RCT2_GLOBAL(0x009AF5FC, uint32) = 1;
		}
	} else {
		sint32 volume;

		// Formula to scale peeps to dB where peeps [0, 120] scales approximately logarithmically to [-3314, -150] dB/100
		// 207360000 maybe related to DSBVOLUME_MIN which is -10,000 (dB/100)
		volume = 120 - min(visiblePeeps, 120);
		volume = volume * volume * volume * volume;
		volume = (((207360000 - volume) >> viewport->zoom) - 207360000) / 65536 - 150;

		// Check if crowd noise is already playing
		if (RCT2_GLOBAL(0x009AF5FC, uint32) == 1) {
			// Load and play crowd noise
#ifdef USE_MIXER
			if (!gCrowdSoundChannel) {
				gCrowdSoundChannel = Mixer_Play_Music(PATH_ID_CSS2);
			}
			if (gCrowdSoundChannel) {
				Mixer_Channel_Volume(gCrowdSoundChannel, DStoMixerVolume(volume));
				RCT2_GLOBAL(0x009AF5FC, uint32) = volume;
			}
#else
			if (sound_channel_load_file2(2, (char*)get_file_path(PATH_ID_CSS2), 0)) {
				sound_channel_play(2, 1, volume, 0, 0);
				RCT2_GLOBAL(0x009AF5FC, uint32) = volume;
			}
#endif
		} else {
			// Alter crowd noise volume
			if (RCT2_GLOBAL(0x009AF5FC, uint32) != volume) {
#ifdef USE_MIXER
				Mixer_Channel_Volume(gCrowdSoundChannel, DStoMixerVolume(volume));
#else
				sound_channel_set_volume(2, volume);//RCT2_CALLPROC_2(0x00401AD3, int, int, 2, volume);
#endif
				RCT2_GLOBAL(0x009AF5FC, uint32) = volume;
			}
		}
	}
}

/**
 *
 *  rct2: 0x0069BE9B
 */
void peep_applause()
{
	uint16 spriteIndex;
	rct_peep* peep;	

	FOR_ALL_GUESTS(spriteIndex, peep) {
		if (peep->var_2A != 0)
			continue;

		// Release balloon
		if (peep->item_standard_flags & PEEP_ITEM_BALLOON) {
			peep->item_standard_flags &= ~PEEP_ITEM_BALLOON;
			if (peep->x != 0x8000) {
				create_balloon(peep->x, peep->y, peep->z + 9, peep->balloon_colour);
				peep->var_45 |= 8;
				RCT2_CALLPROC_X(0x0069B8CC, 0, 0, 0, 0, (int)peep, 0, 0);
			}
		}

		// Clap
		if ((peep->state == PEEP_STATE_WALKING || peep->state == PEEP_STATE_QUEUING) && peep->action >= 254) {
			peep->action = PEEP_ACTION_CLAP;
			peep->action_frame = 0;
			peep->var_70 = 0;
			sub_693B58(peep);
			invalidate_sprite((rct_sprite*)peep);
		}
	}

	// Play applause noise
	sound_play_panned(SOUND_APPLAUSE, RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_WIDTH, uint16) / 2, 0, 0, 0);
}

/**
 *
 *  rct2: 0x0069A05D
 */
rct_peep *peep_generate(int x, int y, int z)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	eax = x;
	ecx = y;
	edx = z;
	RCT2_CALLFUNC_X(0x0069A05D, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	return (rct_peep*)esi;
}

/**
* rct2: 0x00698B0D
* peep.sprite_index (eax)
* thought.type (ebx)
* argument_1 (ecx & ebx)
* argument_2 (edx)
*/
void get_arguments_from_action(rct_peep* peep, uint32 *argument_1, uint32* argument_2){
	rct_ride ride;

	switch (peep->state){
	case PEEP_STATE_FALLING:
		*argument_1 = peep->action == PEEP_ACTION_DROWNING ? STR_DROWNING : STR_WALKING;
		*argument_2 = 0;
		break;
	case PEEP_STATE_1:
		*argument_1 = STR_WALKING;
		*argument_2 = 0;
		break;
	case PEEP_STATE_ON_RIDE:
	case PEEP_STATE_LEAVING_RIDE:
	case PEEP_STATE_ENTERING_RIDE:
		*argument_1 = STR_ON_RIDE;
		ride = g_ride_list[peep->current_ride];
		if (RCT2_GLOBAL(RCT2_ADDRESS_RIDE_FLAGS + ride.type * 8, uint32) & 0x400000){
			*argument_1 = STR_IN_RIDE;
		}
		*argument_1 |= (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	case PEEP_STATE_BUYING:
		ride = g_ride_list[peep->current_ride];
		*argument_1 = STR_AT_RIDE | (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	case PEEP_STATE_WALKING:
	case PEEP_STATE_20:
		if (peep->guest_heading_to_ride_id != 0xFF){
			ride = g_ride_list[peep->guest_heading_to_ride_id];
			*argument_1 = STR_HEADING_FOR | (ride.name << 16);
			*argument_2 = ride.name_arguments;
		}
		else{
			*argument_1 = peep->flags & PEEP_FLAGS_LEAVING_PARK ? STR_LEAVING_PARK : STR_WALKING;
			*argument_2 = 0;
		}
		break;
	case PEEP_STATE_QUEUING_FRONT:
	case PEEP_STATE_QUEUING:
		ride = g_ride_list[peep->current_ride];
		*argument_1 = STR_QUEUING_FOR | (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	case PEEP_STATE_SITTING:
		*argument_1 = STR_SITTING;
		*argument_2 = 0;
		break;
	case PEEP_STATE_WATCHING:
		if (peep->current_ride != 0xFF){
			ride = g_ride_list[peep->current_ride];
			*argument_1 = STR_WATCHING_RIDE | (ride.name << 16);
			*argument_2 = ride.name_arguments;
			if (peep->current_seat & 0x1)
				*argument_1 = STR_WATCHING_CONSTRUCTION_OF | (ride.name << 16);
			else
				*argument_1 = STR_WATCHING_RIDE | (ride.name << 16);
		}
		else{
			*argument_1 = peep->current_seat & 0x1 ? STR_WATCHING_NEW_RIDE_BEING_CONSTRUCTED : STR_LOOKING_AT_SCENERY;
			*argument_2 = 0;
		}
		break;
	case PEEP_STATE_PICKED:
		*argument_1 = STR_SELECT_LOCATION;
		*argument_2 = 0;
		break;
	case PEEP_STATE_PATROLLING:
	case PEEP_STATE_ENTERING_PARK:
	case PEEP_STATE_LEAVING_PARK:
		*argument_1 = STR_WALKING;
		*argument_2 = 0;
		break;
	case PEEP_STATE_MOWING:
		*argument_1 = STR_MOWING_GRASS;
		*argument_2 = 0;
		break;
	case PEEP_STATE_SWEEPING:
		*argument_1 = STR_SWEEPING_FOOTPATH;
		*argument_2 = 0;
		break;
	case PEEP_STATE_WATERING:
		*argument_1 = STR_WATERING_GARDENS;
		*argument_2 = 0;
		break;
	case PEEP_STATE_EMPTYING_BIN:
		*argument_1 = STR_EMPTYING_LITTER_BIN;
		*argument_2 = 0;
		break;
	case PEEP_STATE_ANSWERING:
		if (peep->var_2C == 0){
			*argument_1 = STR_WALKING;
			*argument_2 = 0;
		}
		else if (peep->var_2C == 1){
			*argument_1 = STR_ANSWERING_RADIO_CALL;
			*argument_2 = 0;
		}
		else{
			ride = g_ride_list[peep->current_ride];
			*argument_1 = STR_RESPONDING_TO_RIDE_BREAKDOWN_CALL | (ride.name << 16);
			*argument_2 = ride.name_arguments;
		}
		break;
	case PEEP_STATE_FIXING:
		ride = g_ride_list[peep->current_ride];
		*argument_1 = STR_FIXING_RIDE | (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	case PEEP_STATE_HEADING_TO_INSPECTION:
		ride = g_ride_list[peep->current_ride];
		*argument_1 = STR_HEADING_TO_RIDE_FOR_INSPECTION | (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	case PEEP_STATE_INSPECTING:
		ride = g_ride_list[peep->current_ride];
		*argument_1 = STR_INSPECTING_RIDE | (ride.name << 16);
		*argument_2 = ride.name_arguments;
		break;
	}

}

/**
* rct2: 0x00698342
* thought.item (eax)
* thought.type (ebx)
* argument_1 (esi & ebx)
* argument_2 (esi+2)
*/
void get_arguments_from_thought(rct_peep_thought thought, uint32* argument_1, uint32* argument_2){
	int esi = 0x9AC86C;

	if ((RCT2_ADDRESS(0x981DB1, uint16)[thought.type] & 0xFF) & 1){
		rct_ride* ride = &g_ride_list[thought.item];
		esi = (int)(&(ride->name));
	}
	else if ((RCT2_ADDRESS(0x981DB1, uint16)[thought.type] & 0xFF) & 2){
		if (thought.item < 0x20){
			RCT2_GLOBAL(0x9AC86C, uint16) = thought.item + STR_ITEM_START;
		}
		else{
			RCT2_GLOBAL(0x9AC86C, uint16) = thought.item + STR_ITEM2_START;
		}
	}
	else if ((RCT2_ADDRESS(0x981DB1, uint16)[thought.type] & 0xFF) & 4){
		if (thought.item < 0x20){
			RCT2_GLOBAL(0x9AC86C, uint16) = thought.item + STR_ITEM_SINGULAR_START;
		}
		else
		{
			RCT2_GLOBAL(0x9AC86C, uint16) = thought.item + STR_ITEM2_SINGULAR_START;
		}
	}
	else{
		esi = 0x9AC864; //No thought?
	}
	*argument_1 = ((thought.type + STR_THOUGHT_START) & 0xFFFF) | (*((uint16*)esi) << 16);
	*argument_2 = *((uint32*)(esi + 2)); //Always 0 apart from on rides?
}

/**
 * rct2: 0x00698827 
 * returns 1 on pickup (CF not set)
 */
int peep_can_be_picked_up(rct_peep* peep){
	return RCT2_ADDRESS(0x982004, uint8)[peep->state] & 1;
}

enum{
	PEEP_FACE_OFFSET_ANGRY = 0,
	PEEP_FACE_OFFSET_VERY_VERY_SICK,
	PEEP_FACE_OFFSET_VERY_SICK,
	PEEP_FACE_OFFSET_SICK,
	PEEP_FACE_OFFSET_VERY_TIRED,
	PEEP_FACE_OFFSET_TIRED,
	PEEP_FACE_OFFSET_VERY_VERY_UNHAPPY,
	PEEP_FACE_OFFSET_VERY_UNHAPPY,
	PEEP_FACE_OFFSET_UNHAPPY,
	PEEP_FACE_OFFSET_NORMAL,
	PEEP_FACE_OFFSET_HAPPY,
	PEEP_FACE_OFFSET_VERY_HAPPY,
	PEEP_FACE_OFFSET_VERY_VERY_HAPPY,
};

const int face_sprite_small[] = {
	SPR_PEEP_SMALL_FACE_ANGRY,
	SPR_PEEP_SMALL_FACE_VERY_VERY_SICK,
	SPR_PEEP_SMALL_FACE_VERY_SICK,
	SPR_PEEP_SMALL_FACE_SICK,
	SPR_PEEP_SMALL_FACE_VERY_TIRED,
	SPR_PEEP_SMALL_FACE_TIRED,
	SPR_PEEP_SMALL_FACE_VERY_VERY_UNHAPPY,
	SPR_PEEP_SMALL_FACE_VERY_UNHAPPY,
	SPR_PEEP_SMALL_FACE_UNHAPPY,
	SPR_PEEP_SMALL_FACE_NORMAL,
	SPR_PEEP_SMALL_FACE_HAPPY,
	SPR_PEEP_SMALL_FACE_VERY_HAPPY,
	SPR_PEEP_SMALL_FACE_VERY_VERY_HAPPY,
};

const int face_sprite_large[] = {
	SPR_PEEP_LARGE_FACE_ANGRY,
	SPR_PEEP_LARGE_FACE_VERY_VERY_SICK,
	SPR_PEEP_LARGE_FACE_VERY_SICK,
	SPR_PEEP_LARGE_FACE_SICK,
	SPR_PEEP_LARGE_FACE_VERY_TIRED,
	SPR_PEEP_LARGE_FACE_TIRED,
	SPR_PEEP_LARGE_FACE_VERY_VERY_UNHAPPY,
	SPR_PEEP_LARGE_FACE_VERY_UNHAPPY,
	SPR_PEEP_LARGE_FACE_UNHAPPY,
	SPR_PEEP_LARGE_FACE_NORMAL,
	SPR_PEEP_LARGE_FACE_HAPPY,
	SPR_PEEP_LARGE_FACE_VERY_HAPPY,
	SPR_PEEP_LARGE_FACE_VERY_VERY_HAPPY,
};

int get_face_sprite_offset(rct_peep *peep){

	// ANGRY
	if (peep->var_F3) return PEEP_FACE_OFFSET_ANGRY;

	// VERY_VERY_SICK
	if (peep->nausea > 200) return PEEP_FACE_OFFSET_VERY_VERY_SICK;

	// VERY_SICK
	if (peep->nausea > 170) return PEEP_FACE_OFFSET_VERY_SICK;

	// SICK
	if (peep->nausea > 140) return PEEP_FACE_OFFSET_SICK;

	// VERY_TIRED
	if (peep->energy < 46) return PEEP_FACE_OFFSET_VERY_TIRED;

	// TIRED
	if (peep->energy < 70) return PEEP_FACE_OFFSET_TIRED;

	int offset = PEEP_FACE_OFFSET_VERY_VERY_UNHAPPY;
	//There are 7 different happiness based faces
	for (int i = 37; peep->happiness >= i; i += 37)
	{
		offset++;
	}

	return offset;
}

/**
 *  Function split into large and small sprite
 *  rct2: 0x00698721
 */
int get_peep_face_sprite_small(rct_peep *peep){
	return face_sprite_small[get_face_sprite_offset(peep)];
}

/**
 *  Function split into large and small sprite
 *  rct2: 0x00698721
 */
int get_peep_face_sprite_large(rct_peep *peep){
	return face_sprite_large[get_face_sprite_offset(peep)];
}

/**
 *
 *  rct2: 0x0069A5A0
 * tests if a peep's name matches a cheat code, normally returns using a register flag
 * @param index (eax)
 * @param ride (esi)
 */
int peep_check_easteregg_name(int index, rct_peep *peep)
{
	char buffer[256];

	format_string(buffer, peep->name_string_idx, &peep->id);
	return _stricmp(buffer, gPeepEasterEggNames[index]) == 0;
}

int peep_get_easteregg_name_id(rct_peep *peep)
{
	char buffer[256];
	int i;
	
	format_string(buffer, peep->name_string_idx, &peep->id);

	for (i = 0; i < countof(gPeepEasterEggNames); i++)
		if (_stricmp(buffer, gPeepEasterEggNames[i]) == 0)
			return i;

	return -1;

}

int peep_is_mechanic(rct_peep *peep)
{
	return (
		peep->sprite_identifier == SPRITE_IDENTIFIER_PEEP &&
		peep->type == PEEP_TYPE_STAFF &&
		peep->staff_type == STAFF_TYPE_MECHANIC
	);
}

/* To simplify check of 0x36BA3E0 and 0x11FF78 
 * returns 0 on no food.
 */
int peep_has_food(rct_peep* peep){
	return (peep->item_standard_flags &(
		PEEP_ITEM_DRINK |
		PEEP_ITEM_BURGER |
		PEEP_ITEM_FRIES |
		PEEP_ITEM_ICE_CREAM |
		PEEP_ITEM_COTTON_CANDY |
		PEEP_ITEM_PIZZA |
		PEEP_ITEM_POPCORN |
		PEEP_ITEM_HOT_DOG |
		PEEP_ITEM_TENTACLE |
		PEEP_ITEM_CANDY_APPLE |
		PEEP_ITEM_DONUT |
		PEEP_ITEM_COFFEE |
		PEEP_ITEM_CHICKEN |
		PEEP_ITEM_LEMONADE)) ||
		(peep->item_extra_flags &(
		PEEP_ITEM_PRETZEL |
		PEEP_ITEM_CHOCOLATE |
		PEEP_ITEM_ICED_TEA |
		PEEP_ITEM_FUNNEL_CAKE |
		PEEP_ITEM_BEEF_NOODLES |
		PEEP_ITEM_FRIED_RICE_NOODLES |
		PEEP_ITEM_WONTON_SOUP |
		PEEP_ITEM_MEATBALL_SOUP |
		PEEP_ITEM_FRUIT_JUICE |
		PEEP_ITEM_SOYBEAN_MILK |
		PEEP_ITEM_SU_JONGKWA |
		PEEP_ITEM_SUB_SANDWICH |
		PEEP_ITEM_COOKIE |
		PEEP_ITEM_ROAST_SAUSAGE
		));
}

/**
 * rct2: 0x699F5A
 * al:thought_type
 * ah:thought_arguments
 * esi: peep
 */
void peep_insert_new_thought(rct_peep *peep, uint8 thought_type, uint8 thought_arguments){
	int action = RCT2_ADDRESS(0x981DB0, uint16)[thought_type];

	if (action != 0xFF && peep->action >= 254){
			peep->action = action;
			peep->action_frame = 0;
			peep->var_70 = 0;
			sub_693B58(peep);
			invalidate_sprite((rct_sprite*)peep);
	}
	
	for (int i = 0; i < PEEP_MAX_THOUGHTS; ++i){
		rct_peep_thought* thought = &peep->thoughts[i];
		// Remove the oldest thought by setting it to NONE.
		if (thought->type == PEEP_THOUGHT_TYPE_NONE) break;

		if (thought->type == thought_type && thought->item == thought_arguments){
			// If the thought type has not changed then we need to move
			// it to the top of the thought list. This is done by first removing the
			// existing thought and placing it at the top.
			memmove(thought, thought + 1, sizeof(rct_peep_thought)*(PEEP_MAX_THOUGHTS - i - 1));
			break;
		}
	}

	memmove(&peep->thoughts[1], &peep->thoughts[0], sizeof(rct_peep_thought)*(PEEP_MAX_THOUGHTS - 1));

	peep->thoughts[0].type = thought_type;
	peep->thoughts[0].item = thought_arguments;
	peep->thoughts[0].var_2 = 0;
	peep->thoughts[0].var_3 = 0;

	peep->var_45 |= (1 << 0);
}

void peep_set_map_tooltip(rct_peep *peep)
{
	if (peep->type == PEEP_TYPE_GUEST) {
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = peep->flags & PEEP_FLAGS_TRACKING ? 1450 : 1449;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint32) = get_peep_face_sprite_small(peep);
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 6, uint16) = peep->name_string_idx;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 8, uint32) = peep->id;

		uint32 arg0, arg1;
		get_arguments_from_action(peep, &arg0, &arg1);
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 12, uint32) = arg0;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 16, uint32) = arg1;
	} else {
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 0, uint16) = 1451;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 2, uint16) = peep->name_string_idx;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 4, uint32) = peep->id;

		uint32 arg0, arg1;
		get_arguments_from_action(peep, &arg0, &arg1);
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 8, uint32) = arg0;
		RCT2_GLOBAL(RCT2_ADDRESS_MAP_TOOLTIP_ARGS + 12, uint32) = arg1;
	}
}