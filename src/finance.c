/*****************************************************************************
* Copyright (c) 2014 Matthias Lanzinger
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
#include "finance.h"
#include "sprite.h"
#include "park.h"
#include "peep.h"
#include "ride.h"
#include "window.h"

// Monthly staff wages
const money32 wage_table[4] = {
	MONEY(50,00),		// Handyman
	MONEY(80,00),		// Mechanic
	MONEY(60,00),		// Security guard
	MONEY(55,00)		// Entertainer
};

// Monthly research funding costs
const money32 research_cost_table[4] = {
	MONEY(  0,00),		// No funding
	MONEY(100,00),		// Minimum funding
	MONEY(200,00),		// Normal funding
	MONEY(400,00)		// Maximum funding
};

/**
 * 
 * rct2: 0x069C674
 * @param amount (eax)
 * @param type passed via global var 0x0141F56C, our type is that var/4.
 **/
void finance_payment(money32 amount, rct_expenditure_type type)
{
	money32 cur_money = DECRYPT_MONEY(RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32));
	money32 new_money = cur_money - amount;

	//overflow check
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32) = ENCRYPT_MONEY(new_money);
	RCT2_ADDRESS(RCT2_ADDRESS_EXPENDITURE_TABLE, money32)[type] -= amount;
	if (RCT2_ADDRESS(0x00988E60, uint32)[type] & 1)
		RCT2_GLOBAL(0x0135832C, money32) -= amount;
	

	RCT2_GLOBAL(0x009A9804, uint32) |= 1; // money diry flag
	window_invalidate_by_id(WC_FINANCES, 0);
}

/**
 * Pays the wages of all active staff members in the park.
 * rct2: 0x006C18A9
 **/
void finance_pay_wages()
{
	rct_peep* peep;
	uint16 sprite_idx;

	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & 0x800)
		return;

	for (sprite_idx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16); sprite_idx != SPRITE_INDEX_NULL; sprite_idx = peep->next) {
		peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[sprite_idx].peep);
		if (peep->type == PEEP_TYPE_STAFF) 
			finance_payment(wage_table[peep->staff_type] / 4, RCT_EXPENDITURE_TYPE_WAGES);
	}
}

/**
* Pays the current research level's cost.
* rct2: 0x00684DA5
**/
void finance_pay_research()
{
	uint8 level;

	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & 0x800)
		return;

	level = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_RESEARCH_LEVEL, uint8);
	finance_payment(research_cost_table[level] / 4, RCT_EXPENDITURE_TYPE_RESEARCH);
}

/**
 * Pay interest on current loans.
 * rct2: 0x0069E092
 **/
void finance_pay_interest()
{
	money32 current_loan = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, sint32);
	sint16 current_interest = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_INTEREST_RATE, sint16);
	money32 tempcost = (current_loan * 5 * current_interest) >> 14; // (5 * interest) / 2^14 is pretty close to

	if (RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & 0x800)
		return;

	finance_payment(tempcost, RCT_EXPENDITURE_TYPE_INTEREST);
}

/**
 *
 * rct2: 0x006AC885
 */
void finance_pay_ride_upkeep()
{
	rct_ride* ride;
	for (int i = 0; i < MAX_RIDES; i++) {
		ride = &(RCT2_ADDRESS(RCT2_ADDRESS_RIDE_LIST, rct_ride)[i]);
		if (ride->type == RIDE_TYPE_NULL)
			continue;

		if (!(ride->lifecycle_flags & RIDE_LIFECYCLE_EVER_BEEN_OPENED)) {
			ride->build_date = RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONTH_YEAR, uint16);
			ride->reliability = 100;
			ride->reliability_countdown = 0xff;
		}
		if (ride->status != RIDE_STATUS_CLOSED && !(RCT2_GLOBAL(RCT2_ADDRESS_PARK_FLAGS, uint32) & 0x800)) {
			sint16 upkeep = ride->upkeep_cost;
			if (upkeep != -1) {
				ride->var_158 -= upkeep;
				ride->var_14D |= 2;
				finance_payment(upkeep, RCT2_EXPENDITURE_TYPE_RIDE_UPKEEP);
			}
		}
	}
}


/**
*
*  rct2: 0x0069DEFB
*/
void finance_init() {

	for (short i = 0; i < 56; i++) {
		RCT2_ADDRESS(RCT2_ADDRESS_EXPENDITURE_TABLE, money32)[i] = 0;
	}

	RCT2_GLOBAL(0x0135832C, uint32) = 0;

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PROFIT, money32) = 0;

	RCT2_GLOBAL(0x01358334, uint32) = 0;
	RCT2_GLOBAL(0x01358338, uint16) = 0;

	RCT2_GLOBAL(0x013573DC, money32) = MONEY(10000,00); // Cheat detection

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_MONEY_ENCRYPTED, sint32) = ENCRYPT_MONEY(MONEY(10000,00));
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32) = MONEY(10000,00);
	RCT2_GLOBAL(RCT2_ADDRESS_MAXIMUM_LOAN, money32) = MONEY(20000,00);

	RCT2_GLOBAL(0x013587D0, uint32) = 0;

	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_INTEREST_RATE, uint8) = 10;
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_PARK_VALUE, money32) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_COMPANY_VALUE, money32) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_COMPLETED_COMPANY_VALUE, money32) = MONEY32_UNDEFINED;
	RCT2_GLOBAL(RCT2_ADDRESS_TOTAL_ADMISSIONS, uint32) = 0;
	RCT2_GLOBAL(RCT2_ADDRESS_INCOME_FROM_ADMISSIONS, uint32) = 0;

	RCT2_GLOBAL(0x013587D8, uint16) = 0x3F;

	sub_69E869();
}

void sub_69E869()
{
	// This subroutine is loan related and is used for cheat detection
	sint32 value = 0x70093A;
	value -= RCT2_GLOBAL(0x013573DC, money32); // Cheat detection
	value = ror32(value, 5);
	value -= RCT2_GLOBAL(RCT2_ADDRESS_CURRENT_LOAN, money32);
	value = ror32(value, 7);
	value += RCT2_GLOBAL(RCT2_ADDRESS_MAXIMUM_LOAN, money32);
	value = ror32(value, 3);
	RCT2_GLOBAL(0x013587C4, sint32) = value;
}
