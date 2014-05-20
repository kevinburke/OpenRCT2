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

#include "addresses.h"
#include "ride.h"
#include "sprite.h"
#include "peep.h"
#include "window.h"

#include "stdlib.h"

#define GET_RIDE(x) (&(RCT2_ADDRESS(RCT2_ADDRESS_RIDE_LIST, rct_ride)[x]))
#define GET_RIDE_MEASUREMENT(x) (&(RCT2_ADDRESS(RCT2_ADDRESS_RIDE_MEASUREMENTS, rct_ride_measurement)[x]))

#pragma region Ride classification table

const uint8 gRideClassifications[255] = {
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_SHOP_OR_STALL,
	RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_RIDE, RIDE_CLASS_SHOP_OR_STALL,
	RIDE_CLASS_KIOSK_OR_FACILITY, RIDE_CLASS_KIOSK_OR_FACILITY, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_KIOSK_OR_FACILITY, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_KIOSK_OR_FACILITY, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_RIDE, RIDE_CLASS_SHOP_OR_STALL,
	RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_SHOP_OR_STALL, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE,
	RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE, RIDE_CLASS_RIDE
};

#pragma endregion

int ride_get_count()
{
	rct_ride *ride;
	int i, count = 0;

	for (i = 0; i < MAX_RIDES; i++) {
		ride = GET_RIDE(i);
		if (ride->type != RIDE_TYPE_NULL)
			count++;
	}

	return count;
}

int ride_get_total_queue_length(rct_ride *ride)
{
	int i, queueLength = 0;
	for (i = 0; i < 4; i++)
		if (ride->entrances[i] != 0xFFFF)
			queueLength += ride->queue_length[i];
	return queueLength;
}

int ride_get_max_queue_time(rct_ride *ride)
{
	int i, queueTime = 0;
	for (i = 0; i < 4; i++)
		if (ride->entrances[i] != 0xFFFF)
			queueTime = max(queueTime, ride->queue_time[i]);
	return queueTime;
}

/**
 * rct2: 0x006B6456
 *
 * in this fn:
 *     dl = i incrementing from 0 to MAX_RIDES
 *     edi = ride measurement index (i multiplied by 0x4B0C)
 *     ebx = ride_measurement object (p. sure only var_00 is used)
 *     esi = not sure? sprite pointer
 */
void update_ride_measurements()
{
	int i;
	int si;
	int edx;
	int plus50;
	int ebp;
	int ax;

	rct_ride_measurement *ride_measurement;
	if (RCT2_GLOBAL(RCT2_ADDRESS_SCREEN_FLAGS, uint8) & 2) {
		return;
	}
	for (i = 0; i < MAX_RIDE_MEASUREMENTS; i++) {
		ride_measurement = GET_RIDE_MEASUREMENT(i);
		if (ride_measurement->var_00 == 0xff) {
			continue;
		}
		if (RCT2_ADDRESS(0x01362AC8, uint32)[ride_measurement->var_00] & 1 == 0) {
			continue;
		}
		if (RCT2_ADDRESS(0x0138B60D, uint8)[i*0x4B0C] & 1 == 0) {
			edx = 0;
			// maybe this check is because some rides don't use all 8 ride
			// measurements
			while(i >= RCT2_GLOBAL(0x013629C0, uint8)[ride_measurement->var_00]) {
				si = RCT2_GLOBAL(0x0136297E, uint32)[ride_measurement->var_00 + 2*edx];
				if (si == 0xffff) {
					// loc 6B64D2
					edx += 1;
					// XXX goto check_12629C0
					continue;
				} else {
					// loc 6B64D5
					si = si<<8;
					si = si + RCT2_ADDRESS_SPRITE_LIST;
					plus50 = RCT2_GLOBAL(si+0x50, uint8);
					if (plus50 == 3 || plus50 == 0x16) {
						edx += 1;
						// XXX goto check_12629C0
						continue;
					}
					RCT2_GLOBAL(0x0138B616, uint8)[i*0x4B0C] = i;
					RCT2_GLOBAL(0x0138B617, uint8) = *(si+0x4b);

					// set the first bit
					RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] |= 1;
					// clear the second bit
					RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] &= 0xFD;
					break;
				}
			}

			// XXX, duplicates the load/comparison above
			si = RCT2_GLOBAL(0x0136297E, uint32)[ride_measurement->var_00 + 2*edx];
			if (si == 0xffff) {
				continue;
			}

			// XXX also duplicates from above
			si = si << 8;
			si = si + RCT2_ADDRESS_SPRITE_LIST;
			ebp = RCT2_GLOBAL(0x0138B614, uint32)[i*0x4B0C];
			plus50 = RCT2_GLOBAL(si+0x50, uint8);
			if (RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] & 2 != 0) {
				if (plus50 == 3 || plus50 == 0x16) {
					// clear 2nd bit
					RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] &= 0xFD;
					if (i == RCT2_GLOBAL(0x0138B617, uint8)[i*0x4B0C]) {
						RCT2_GLOBAL(0x0138B614, uint8)[i*0x4B0C] = 0;
						ebp = 0;
					}
				} else {
					continue;
				}
			}
			// Code has a comparison for plus50 and 6 here.
			if (plus50 != 6) {
				// 6b6566
				ax = *(si+0x36);
				ax = ax >> 2;
				if (ax == 0xD8 || ax == 0x7B || ax == 9 || ax == 0x3F || 
						ax == 0x93 || ax == 0x9B) {
					if (*(si+0x28) == 0) {
						continue;
					}
				}
				if (ebp > 0x12C0) {
					// This variable is referred to a lot.
					if (RCT2_GLOBAL(0x00F663AC, uint32) & 1) {
						continue;
					}
				} else {
					if (RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] & 4) {
						// 6B662A
						sint32 eax = *(si+0x28);
						eax = eax * 5;
						eax = eax >> 0x10;
						eax = abs(eax);

						sint16 ax = eax & 0xffff;
						if (ax > 0xff) {
							ax = 0xff;

							// Set lower 16 bits, without affecting higher ones
							eax &= 0xffff00ff;
							eax |= 0x00ff;
						}
						sint16 dx = *(si+0x12);
						dx = dx >> 3;
						if (dx < 0xff) {
							dx = 0xff;
						}
						if (RCT2_GLOBAL(0x00F663AC, uint32) & 1) {
							add_store_measurement(0x0138DB98, ax, ebp+i*0x4B0C);
							add_store_measurement(0x0138EE58, dx, ebp+i*0x4B0C);
						} else {
							sint8 al = ax & 0xff;
							sint8 dl = dx & 0xff;
							// difference in addresses is exactly 0x4b0c
							RCT2_GLOBAL(0x0138DB98, sint8)[ebp + i*0x4B0C] = al;
							RCT2_GLOBAL(0x0138EE58, sint8)[ebp + i*0x4B0C] = dl;
							ebp++;
							uint16 bp = ebp & 0xffff;
							if (bp <= RCT2_GLOBAL(0x0138B612, uint16)[i*0x4B0C]) {
								RCT2_GLOBAL(0x0138B612, uint16)[i*0x4B0C] = bp;
							}
						}

						if (RCT2_GLOBAL(0x00F663AC, uint32) & 1) {
							RCT2_GLOBAL(0x0138B614, uint16)++;
						}
						continue;

					} else {
						// crazy branching subroutine
						uint32 eax;
						RCT2_CALLPROC_X(0x006D73D0, eax, 0, 0, edx, si, 0, 0);
						sint16 ax = eax & 0xffff;
						ax = ax >> 3;

						sint16 dx = edx & 0xffff;
						dx = dx >> 3;

						if (ax >= 0xff81) {
							ax = 0xff81;
						}
						if (ax < 0x7f) {
							ax = 0x7f;
						}

						if (dx >= 0xff81) {
							dx = 0xff81;
						}
						// XXX ?? this doesn't make any sense.
						if (dx <= 0x7f) {
							dx = 0x7f;
						}
					}
				}
			} else {
				// set the second bit
				RCT2_GLOBAL(0x0138B60D, uint8)[i*0x4B0C] |= 2;
				continue;
			}
		}
	}
}

void add_store_measurement(int addr, sint16 initial_value, int idx);
/**
 * convenience function
 *
 * addr: address to read, store the value
 * sint16 initial_value: what we've computed so far
 * idx: ride measurement offset
 */
void add_store_measurement(int addr, sint16 initial_value, int idx) 
{
	uint16 cx = RCT2_GLOBAL(addr, uint16)[idx];
	uint16 uax = initial_value;
	uax += cx;
	uax = uax >> 1;

	uint8 al = uax & 0xff;
	RCT2_GLOBAL(addr, uint8)[idx] = al;

}

/**
 *
 *  rct2: 0x006ACA89
 */
void ride_init_all()
{
	int i;
	rct_ride *ride;
	rct_ride_measurement *ride_measurement;

	for (i = 0; i < MAX_RIDES; i++) {
		ride = GET_RIDE(i);
		ride->type = RIDE_TYPE_NULL;
	}

	RCT2_GLOBAL(0x0138B590, sint8) = 0;
	RCT2_GLOBAL(0x0138B591, sint8) = 0;

	for (i = 0; i < MAX_RIDE_MEASUREMENTS; i++) {
		ride_measurement = GET_RIDE_MEASUREMENT(i);
		ride_measurement->var_00 = 0xFF;
	}
}

/**
*
*  rct2: 0x006B7A38
*/
void reset_all_ride_build_dates() {
	int i;
	rct_ride *ride;
	for (i = 0; i < MAX_RIDES; i++) {
		ride = GET_RIDE(i);
		if (ride->type != RIDE_TYPE_NULL) {
			//mov	ax, current_month_year
			//sub	[esi + 180h], ax
			ride->build_date -= RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, uint16);
		}
	}
}

/**
  * rct2: 0x006AC916
  */
void ride_update_favourited_stat()
{
	rct_ride *ride;
	rct_peep* peep;

	for (int i = 0; i < MAX_RIDES; i++) {
		ride = GET_RIDE(i);
		if (ride->type != RIDE_TYPE_NULL)
			ride->guests_favourite = 0;

	}
	for (int sprite_idx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16); sprite_idx != SPRITE_INDEX_NULL; sprite_idx = peep->next) {
		peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[sprite_idx].peep);
		if (peep->var_08 != 4)
			return;
		if (peep->favourite_ride != 0xff) {
			ride = GET_RIDE(peep->favourite_ride);
			ride->guests_favourite++;
			ride->var_14D |= 1;

		}

	}
	window_invalidate_by_id(WC_RIDE_LIST, 0);
}

