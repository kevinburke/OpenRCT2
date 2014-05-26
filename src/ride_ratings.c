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
#include "ride_data.h"
#include "ride_ratings.h"

#include <stdint.h>

void go_karts_excitement(rct_ride *ride) {
	if (ride->lifecycle_flags & RIDE_LIFECYCLE_TESTED == 0) {
		return;
	}

	ride->var_198 = 16;
	sub_655FD6(ride);

	uint32 runtime_seconds = ride_get_runtime(ride);

	if (runtime_seconds > 700) {
		runtime_seconds = 700;
	}

	uint32 excitement = runtime_seconds / 2;

	if (ride->mode == RIDE_MODE_RACE) {
		if (ride->num_trains >= 3) {
			excitement += 140;
			uint32 intensity = 50;

			uint32 laps = ride->laps - 1;
			excitement += laps * 30;
			intensity += laps * 15;

			rating_tuple tup = sub_65DDD1(ride);
			tup.excitement = tup.excitement * 0x116A;
			tup.intensity = tup.intensity * 0x0D94;
			tup.nausea = tup.nausea * 0x1656;

			tup.excitement = tup.excitement >> 16;
			tup.intensity = tup.intensity >> 16;
			tup.nausea = tup.nausea >> 16;

			excitement += tup.excitement;
			intensity += tup.intensity;
			uint32 nausea = tup.nausea;

			tup = sub_65E139(ride);

			tup.excitement = tup.excitement * 0x2222;
			tup.intensity = tup.intensity * 0x1555;
			tup.nausea = tup.nausea * 0x1999;
			tup.excitement = tup.excitement >> 16;
			tup.intensity = tup.intensity >> 16;
			tup.nausea = tup.nausea >> 16;

			excitement += tup.excitement;
			intensity += tup.intensity;
			nausea += tup.nausea;

			tup = sub_65E1C2(ride);

			tup.excitement = tup.excitement * 0x0A0A;
			tup.intensity = tup.intensity * 0x2222;
			tup.nausea = tup.nausea * 0x924;
			tup.excitement = tup.excitement >> 16;
			tup.intensity = tup.intensity >> 16;
			tup.nausea = tup.nausea >> 16;
			excitement += tup.excitement;
			intensity += tup.intensity;
			nausea += tup.nausea;

			// 658E36
		}

	}
}

/**
 * rct2: 0x0065E1C2
 */
rating_tuple sub_65E1C2(rct_ride *ride) {
	uint32 excitement = 0;
	uint32 intensity = 0;
	uint32 nausea = 0;

	excitement += shift_flag_max_mult_shift(ride->var_118, 16, 0xffffffff, 1000, 0x23D7);
	intensity += shift_flag_max_mult_shift(ride->var_118, 16, 0xffffffff, 2000, 0x2666);
	nausea += shift_flag_max_mult_shift(ride->var_118, 16, 0xffffffff, 1000, 0x4000);

	uint32 eax = ride->var_11C;
	eax = eax * 0x7684B;
	eax = eax >> 16;
	excitement += eax;

	if (ride->var_11E & 0x40) {
		excitement += 20;
		nausea += 15;
	}

	if (ride->var_11E & 0x20) {
		excitement += 20;
		nausea += 15;
	}

	excitement += shift_flag_max_mult_shift(ride->var_11E, 0, 0x1F, 11, 0xBD174);

	rating_tuple tup = { excitement, intensity, nausea };
	return tup;
}

uint32 shift_flag_max_mult_shift(uint32 value, uint32 initial_shift, uint8 flag, uint8 max_val, uint32 mult) {
	uint32 eax = value >> initial_shift;
	eax = eax & flag;
	if (eax > max_val) {
		eax = max_val;
	}
	eax = eax * mult;
	return eax >> 16;
}

/**
 * rct2: 0x0065E139
 */
rating_tuple sub_65E139(rct_ride *ride) {
	uint32 excitement = 0;
	uint32 intensity = 0;
	uint32 nausea = 0;

	excitement += shift_flag_max_mult_shift(ride->var_115, 0, 0x3F, 9, 0x0B1C71);
	intensity += shift_flag_max_mult_shift(ride->var_115, 0, 0x3F, UINT32_MAX, 0xE2AAA);
	nausea += shift_flag_max_mult_shift(ride->var_115, 0, 0x3F, UINT32_MAX, 0xA0000);

	uint32 eax = ride->var_117;
	eax = eax << 1;
	eax = eax * 0x3E80;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_117;
	eax = eax << 1;
	eax = eax * 0x7D00;
	eax = eax >> 16;
	intensity += eax;

	eax = ride->var_117;
	eax = eax << 1;
	eax = eax * 0x2800;
	eax = eax >> 16;
	nausea += eax;

	rating_tuple tup = { excitement, intensity, nausea };
	return tup;
}

/**
 * rct2: 0x0065DDD1
 */
rating_tuple sub_65DDD1(rct_ride *ride) {
	uint32 excitement = 0;
	uint32 intensity = 0;
	uint32 nausea = 0;
	if (ride->type == RIDE_TYPE_HAUNTED_HOUSE) {
		if (ride->var_0D5 & 0x20) {
			excitement += 40;
			intensity += 25;
			nausea += 55;
		}
	} else if (ride->type == RIDE_TYPE_LOG_FLUME) {
		if (ride->var_0D5 & 0x40) {
			excitement += 48;
			intensity += 55;
			nausea += 65;
		}
	} else {
		// gentle? thrill? maybe?
		if (ride->var_0D5 & 0x20) {
			excitement += 50;
			intensity += 30;
			nausea += 20;
		} 
		if (ride->var_0D5 & 0x40) {
			excitement += 55;
			intensity += 30;
		}
		if (ride->var_0D5 & 0x80) {
			excitement += 35;
			intensity += 20;
			nausea += 23;
		}
	}
	// 65DE3C
	uint32 eax = ride->var_0D5;
	// set lower 5 bits
	eax &= 0x1F;
	if (eax > 9) {
		eax = 9;
	}
	// multiply it by 3, roughly
	eax = (eax * 0x3E38E) >> 0x10;
	excitement += eax;

	eax = ride->var_0D5;
	// set lower 5 bits
	eax &= 0x1F;
	if (eax > 0x0B) {
		eax = 0x0B;
	}

	// multiply by 2, roughly
	eax = (eax * 0x245D1) >> 0x10;
	intensity += eax;

	eax = ride->var_0D5;
	eax &= 0x1F;
	eax -= 5;
	if (eax < 0) {
		eax = 0;
	}

	if (eax > 0x0A) {
		eax = 0x0A;
	}
	eax = (eax * 0x140000) >> 0x10;
	nausea += eax;

	// 65DE9D
	excitement += apply_10E_multiplier(ride->var_10E, 256, 0x28000, 7);
	intensity += apply_10E_multiplier(ride->var_10E, 256, 0x14000, 7);
	nausea += apply_10E_multiplier(ride->var_10E, 256, 0x50000, 7);

	excitement += apply_10E_multiplier(ride->var_10E, 32, 0x30000, 7);
	intensity += apply_10E_multiplier(ride->var_10E, 32, 0xC000, 7);
	nausea += apply_10E_multiplier(ride->var_10E, 32, 0x32000, 7);

	excitement += apply_10E_multiplier(ride->var_10E, 1, 0x0F7BD, 0x1F);
	intensity += apply_10E_multiplier(ride->var_10E, 1, 0x5294, 0x1F);
	nausea += apply_10E_multiplier(ride->var_10E, 1, 0xA529, 0x1F);

	excitement += apply_10E_multiplier(ride->var_110, 256, 0x3C000, 7);
	intensity += apply_10E_multiplier(ride->var_110, 256, 0x14000, 7);
	nausea += apply_10E_multiplier(ride->var_110, 256, 0x50000, 7);

	excitement += apply_10E_multiplier(ride->var_110, 32, 0x3C000, 7);
	intensity += apply_10E_multiplier(ride->var_110, 32, 0xC000, 7);
	nausea += apply_10E_multiplier(ride->var_110, 32, 0x32000, 7);

	excitement += apply_10E_multiplier(ride->var_10E, 1, 0x12108, 0x1F);
	intensity += apply_10E_multiplier(ride->var_10E, 1, 0x5294, 0x1F);
	nausea += apply_10E_multiplier(ride->var_10E, 1, 0xBDEF, 0x1F);

	eax = ride->var_112;
	eax = eax >> 11;
	eax = eax & 0x3F;
	if (eax > 4) {
		eax = 4;
	}
	eax = eax * 0x78000;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_112;
	eax = eax >> 11;
	eax = eax & 0x3F;
	if (eax > 8) {
		eax = 8;
	}
	eax = eax * 0x78000;
	eax = eax >> 16;
	nausea += eax;

	eax = ride->var_112;
	eax = eax >> 8;
	eax = eax & 7;
	if (eax > 6) {
		eax = 6;
	}
	eax = eax * 0x42AAA;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_112;
	eax = eax / 32;
	eax = eax & 7;
	if (eax > 6) {
		eax = 6;
	}
	eax = eax * 0x3AAAA;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_112;
	eax = eax & 0x1F;
	if (eax > 7) {
		eax = 7;
	}
	eax = eax * 0x2DB6D;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_114;
	eax = eax & 0x1F;
	if (eax > 6) {
		eax = 6;
	}
	eax = eax * 0x1AAAAA;
	eax = eax >> 16;
	excitement += eax;

	eax = ride->var_114;
	eax = eax & 0x1F;
	eax = eax * 0x320000;
	eax = eax >> 16;
	intensity += eax;

	eax = ride->var_114;
	eax = eax & 0x1F;
	eax = eax * 0x15AAAA;
	eax = eax >> 16;
	nausea += eax;

	rating_tuple tup = { excitement, intensity, nausea };
	return tup;
}

uint32 apply_10E_multiplier(uint16 var_10E, uint16 quotient, uint32 multiplier, uint8 low_bits) {
	uint32 eax = var_10E / quotient;
	eax &= low_bits;
	eax = eax * multiplier;
	return eax >> 16;
}

/**
 * rct2: 0x0065C4D4
 *
 * Compute excitement, intensity, etc. for a crooked house ride.
 */
void crooked_house_excitement(rct_ride *ride)
{
	// Set lifecycle bits
	ride->lifecycle_flags |= RIDE_LIFECYCLE_TESTED;
	ride->lifecycle_flags |= RIDE_LIFECYCLE_NO_RAW_STATS;
	ride->var_198 = 5;
	sub_655FD6(ride);

	ride_rating excitement	= RIDE_RATING(2,15);
	ride_rating intensity	= RIDE_RATING(0,62);
	ride_rating nausea		= RIDE_RATING(0,34);

	// NB this should get marked out by the compiler, if it's smart.
	excitement = apply_intensity_penalty(excitement, intensity);
	rating_tuple tup = per_ride_rating_adjustments(ride, excitement, intensity, nausea);
	excitement = tup.excitement;
	intensity = tup.intensity;
	nausea = tup.nausea;

	ride->excitement = excitement;
	ride->intensity = intensity;
	ride->nausea = nausea;

	ride->upkeep_cost = compute_upkeep(ride);
	// Upkeep flag? or a dirtiness flag
	ride->var_14D |= 2;

	// clear all bits except lowest 5
	ride->var_114 &= 0x1f;
	// set 6th,7th,8th bits
	ride->var_114 |= 0xE0;
}

/**
 * rct2: sub_65E621
 *
 * I think this function computes ride upkeep? Though it is weird that the
 *
 * inputs
 * - edi: ride ptr
 */
uint16 compute_upkeep(rct_ride *ride)
{
	// data stored at 0x0057E3A8, incrementing 18 bytes at a time
	uint16 upkeep = initialUpkeepCosts[ride->type];

	uint16 trackCost = costPerTrackPiece[ride->type];
	uint8 dl = ride->var_115;

	dl = dl >> 6;
	dl = dl & 3;
	upkeep += trackCost * dl;

	uint32 runtime_seconds = ride_get_runtime(ride);

	// The data originally here was 20's and 0's. The 20's all represented
	// rides that had tracks. The 0's were fixed rides like crooked house or
	// bumper cars.
	// Data source is 0x0097E3AC
	uint32 runtime_upkeep = runtime_seconds;
	if (hasRunningTrack[ride->type]) {
		runtime_upkeep = runtime_seconds * 20;
	}
	runtime_upkeep = runtime_upkeep >> 0x0A;
	upkeep += (uint16)runtime_upkeep;

	if (ride->lifecycle_flags & RIDE_LIFECYCLE_ON_RIDE_PHOTO) {
		// The original code read from a table starting at 0x0097E3AE and
		// incrementing by 0x12 bytes between values. However, all of these
		// values were 40. I have replaced the table lookup with the constant
		// 40 in this case.
		upkeep += 40;
	}

	// Originally this data was at 0x0097E3B0 and incrementing in 18 byte
	// offsets. The value here for every ride is 80, except for the reverser,
	// which is 10
	uint16 eax;
	if (ride->type == RIDE_TYPE_REVERSER_ROLLER_COASTER) {
		eax = 10;
	} else {
		eax = 80;
	}

	// not sure what this value is; it's only written to in one place, where
	// it's incremented.
	sint16 dx = RCT2_GLOBAL(0x0138B5CC, sint16);
	upkeep += eax * dx;

	dx = RCT2_GLOBAL(0x0138B5CA, sint16);
	// Originally there was a lookup into a table at 0x0097E3B0 and
	// incrementing in 18 byte offsets. The value here for every ride was 20,
	// so it's been replaced here by the constant.
	upkeep += 20 * dx;

	// these seem to be adhoc adjustments to a ride's upkeep/cost, times
	// various variables set on the ride itself.

	// https://gist.github.com/kevinburke/e19b803cd2769d96c540
	upkeep += rideUnknownData1[ride->type] * ride->num_trains;

	// either set to 3 or 0, extra boosts for some rides including mini golf
	if (rideUnknownData2[ride->type]) {
		upkeep += 3 * ride->cars_per_train;
	}

	// slight upkeep boosts for some rides - 5 for mini railroad, 10 for log
	// flume/rapids, 10 for roller coaster, 28 for giga coaster
	upkeep += rideUnknownData3[ride->type] * ride->num_stations;

	if (ride->mode == RIDE_MODE_REVERSE_INCLINED_SHUTTLE) {
		upkeep += 30;
	} else if (ride->mode == RIDE_MODE_POWERED_LAUNCH) {
		upkeep += 160;
	} else if (ride->mode == RIDE_MODE_LIM_POWERED_LAUNCH) {
		upkeep += 320;
	} else if (ride->mode == RIDE_MODE_POWERED_LAUNCH2 || 
			ride->mode == RIDE_MODE_POWERED_LAUNCH_BLOCK_SECTIONED) {
		upkeep += 220;
	}

	// multiply by 5/8
	upkeep = upkeep * 10;
	upkeep = upkeep >> 4;
    return upkeep;
}

/**
 * rct2: 0x0065E7FB
 *
 * inputs
 * - bx: excitement
 * - cx: intensity
 * - bp: nausea
 * - edi: ride ptr
 */
rating_tuple per_ride_rating_adjustments(rct_ride *ride, ride_rating excitement, 
		ride_rating intensity, ride_rating nausea)
{
	// NB: The table here is allocated dynamically. Reading the exe will tell
	// you nothing
	uint32 subtype_p = RCT2_GLOBAL(0x009ACFA4 + ride->subtype * 4, uint32);

	// example value here: 12 (?)
	sint16 ctr = RCT2_GLOBAL(subtype_p + 0x1b2, sint16);
	excitement = excitement + ((excitement * ctr) >> 7);

	ctr = RCT2_GLOBAL(subtype_p + 0x1b3, sint16);
	intensity = intensity + ((intensity * ctr) >> 7);

	ctr = RCT2_GLOBAL(subtype_p + 0x1b4, sint16);
	nausea = nausea + ((nausea * ctr) >> 7);

	// As far as I can tell, this flag detects whether the ride is a roller
	// coaster, or a log flume or rapids. Everything else it's not set.
	// more detail: https://gist.github.com/kevinburke/d951e74e678b235eef3e
	uint16 ridetype_var = RCT2_GLOBAL(0x0097D4F2 + ride->type * 8, uint16);
	if (ridetype_var & 0x80) {
		uint16 ax = ride->var_1F4;
		if (RCT2_GLOBAL(subtype_p + 8, uint32) & 0x800) {
			// 65e86e
			ax = ax - 96;
			if (ax >= 0) {
				ax = ax >> 3;
				excitement = excitement - ax;
				ax = ax >> 1;
				nausea = nausea - ax;
			}
		} else {
			ax = ax >> 3;
			excitement = excitement + ax;
			ax = ax >> 1;
			nausea += ax;
		}
	}
	rating_tuple tup = { excitement, intensity, nausea };
	return tup;
}

/**
 * rct2: 0x0065E7A3
 *
 * inputs from x86
 * - bx: excitement
 * - cx: intensity
 * - bp: nausea
 *
 * returns: the excitement level, with intensity penalties applied
 */
ride_rating apply_intensity_penalty(ride_rating excitement, ride_rating intensity)
{
    // intensity penalty
    if (intensity >= 1000) {
        excitement = excitement - (excitement >> 2);
    }
    if (intensity >= 1100) {
        excitement = excitement - (excitement >> 2);
    }
    if (intensity >= 1200) {
        excitement = excitement - (excitement >> 2);
    }
    if (intensity >= 1320) {
        excitement = excitement - (excitement >> 2);
    }
    if (intensity >= 1450) {
        excitement = excitement - (excitement >> 2);
    }
    return excitement;
}

/**
 * rct2: 0x00655FD6
 *
 * Take ride property 1CD, make some modifications, store the modified value in
 * property 198.
 */
void sub_655FD6(rct_ride *ride)
{
	// Satisfaction decrease, or something, based on age, maybe
    uint8 al = ride->var_1CD;
    // No idea what this address is; maybe like compensation of some kind? The
    // maximum possible value?
    // List of ride names/values is here: 
    // https://gist.github.com/kevinburke/5eebcda14d94e6ee99c0
    al -= RCT2_ADDRESS(0x0097D7C9, uint8)[4 * ride->type];
    al = al << 1;
    ride->var_198 += al;
}
