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

#include <string.h>
#include "addresses.h"
#include "game.h"
#include "peep.h"
#include "strings.h"
#include "sprite.h"
#include "sprites.h"
#include "widget.h"
#include "window.h"
#include "window_dropdown.h"

enum {
	PAGE_INDIVIDUAL,
	PAGE_SUMMARISED
};

enum WINDOW_GUEST_LIST_WIDGET_IDX {
	WIDX_BACKGROUND,
	WIDX_TITLE,
	WIDX_CLOSE,
	WIDX_TAB_CONTENT_PANEL,
	WIDX_PAGE_DROPDOWN,
	WIDX_PAGE_DROPDOWN_BUTTON,
	WIDX_INFO_TYPE_DROPDOWN,
	WIDX_INFO_TYPE_DROPDOWN_BUTTON,
	WIDX_MAP,
	WIDX_TAB_1,
	WIDX_TAB_2,
	WIDX_GUEST_LIST
};

enum {
	VIEW_ACTIONS,
	VIEW_THOUGHTS
};

static rct_widget window_guest_list_widgets[] = {
	{ WWT_FRAME,			0,	0,		349,	0,	329,	0x0FFFFFFFF,	STR_NONE },						// panel / background
	{ WWT_CAPTION,			0,	1,		348,	1,	14,		STR_GUESTS,		STR_WINDOW_TITLE_TIP },			// title bar
	{ WWT_CLOSEBOX,			0,	337,	347,	2,	13,		STR_CLOSE_X,	STR_CLOSE_WINDOW_TIP },			// close x button
	{ WWT_RESIZE,			1,	0,		349,	43,	329,	0x0FFFFFFFF,	STR_NONE },						// tab content panel
	{ WWT_DROPDOWN,			1,	5,		84,		59,	70,		STR_PAGE_1,		STR_NONE },						// page dropdown
	{ WWT_DROPDOWN_BUTTON,	1,	73,		83,		60,	69, 	876,			STR_NONE },						// page dropdown button
	{ WWT_DROPDOWN,			1,	126,	301,	59,	70, 	0x0FFFFFFFF,	STR_INFORMATION_TYPE_TIP },		// information type dropdown
	{ WWT_DROPDOWN_BUTTON,	1,	290,	300,	60,	69, 	876,			STR_INFORMATION_TYPE_TIP },		// information type dropdown button
	{ WWT_FLATBTN,			1,	321,	344,	46,	69, 	5192,			STR_SHOW_GUESTS_ON_MAP_TIP },	// map
	{ WWT_TAB,				1,	3,		33,		17,	43, 	0x02000144E,	STR_INDIVIDUAL_GUESTS_TIP },	// tab 1
	{ WWT_TAB,				1,	34,		64,		17,	43, 	0x02000144E,	STR_SUMMARISED_GUESTS_TIP },	// tab 2
	{ WWT_SCROLL,			1,	3,		346,	72,	326,	3,				STR_NONE },						// guest list
	{ WIDGETS_END },
};

static void window_guest_list_emptysub() { }
static void window_guest_list_mouseup();
static void window_guest_list_resize();
static void window_guest_list_mousedown();
static void window_guest_list_dropdown();
static void window_guest_list_update(rct_window *w);
static void window_guest_list_scrollgetsize();
static void window_guest_list_scrollmousedown();
static void window_guest_list_scrollmouseover();
static void window_guest_list_tooltip();
static void window_guest_list_invalidate();
static void window_guest_list_paint();
static void window_guest_list_scrollpaint();

static void* window_guest_list_events[] = {
	window_guest_list_emptysub,
	window_guest_list_mouseup,
	window_guest_list_resize,
	window_guest_list_mousedown,
	window_guest_list_dropdown,
	window_guest_list_emptysub,
	window_guest_list_update,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_scrollgetsize,
	window_guest_list_scrollmousedown,
	window_guest_list_emptysub,
	window_guest_list_scrollmouseover,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_tooltip,
	window_guest_list_emptysub,
	window_guest_list_emptysub,
	window_guest_list_invalidate,
	window_guest_list_paint,
	window_guest_list_scrollpaint
};

static int _window_guest_list_highlighted_index;
static int _window_guest_list_selected_tab;
static int _window_guest_list_selected_filter;
static int _window_guest_list_selected_page;
static int _window_guest_list_selected_view;
static int _window_guest_list_num_pages;
static int _window_guest_list_num_groups;

static uint16 _window_guest_list_groups_num_guests[240];
static uint32 _window_guest_list_groups_argument_1[240];
static uint32 _window_guest_list_groups_argument_2[240];
static uint8 _window_guest_list_groups_guest_faces[240 * 58];

static int window_guest_list_is_peep_in_filter(rct_peep* peep);
static void window_guest_list_find_groups();
static int get_guest_face_sprite_small(rct_peep *peep);
static int get_guest_face_sprite_large(rct_peep *peep);

/**
 * 
 *  rct2: 0x006992E3
 */
void window_guest_list_open()
{
	rct_window* window;

	// Check if window is already open
	window = window_bring_to_front_by_id(WC_GUEST_LIST, 0);
	if (window != NULL)
		return;

	window = window_create_auto_pos(350, 330, (uint32*)window_guest_list_events, WC_GUEST_LIST, 0x0400);
	window->widgets = window_guest_list_widgets;
	window->enabled_widgets =
		(1 << WIDX_CLOSE) |
		(1 << WIDX_PAGE_DROPDOWN) |
		(1 << WIDX_PAGE_DROPDOWN_BUTTON) |
		(1 << WIDX_INFO_TYPE_DROPDOWN) |
		(1 << WIDX_INFO_TYPE_DROPDOWN_BUTTON) |
		(1 << WIDX_MAP) |
		(1 << WIDX_TAB_1) |
		(1 << WIDX_TAB_2);

	window_init_scroll_widgets(window);
	_window_guest_list_highlighted_index = -1;
	window->var_490 = 0;
	_window_guest_list_selected_tab = PAGE_INDIVIDUAL;
	_window_guest_list_selected_filter = -1;
	_window_guest_list_selected_page = 0;
	_window_guest_list_num_pages = 1;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN].type = WWT_EMPTY;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN_BUTTON].type = WWT_EMPTY;
	window->var_492 = 0;
	window->min_width = 350;
	window->min_height = 330;
	window->max_width = 500;
	window->max_height = 450;
	window->flags = WF_RESIZABLE;
	window->colours[0] = 1;
	window->colours[1] = 15;
	window->colours[2] = 15;
}

/**
 * 
 *  rct2: 0x00699AAF
 */
static void window_guest_list_mouseup()
{
	short widgetIndex;
	rct_window *w;

	__asm mov widgetIndex, dx
	__asm mov w, esi

	switch (widgetIndex) {
	case WIDX_CLOSE:
		window_close(w);
		break;
	case WIDX_MAP:
		window_map_open();
		break;
	}
}

/**
 * 
 *  rct2: 0x00699EA3
 */
static void window_guest_list_resize()
{
	rct_window *w;

	__asm mov w, esi

	w->min_width = 350;
	w->min_height = 330;
	if (w->width < w->min_width) {
		window_invalidate(w);
		w->width = w->min_width;
	}
	if (w->height < w->min_height) {
		window_invalidate(w);
		w->height = w->min_height;
	}
}

/**
 * 
 *  rct2: 0x00699AC4
 */
static void window_guest_list_mousedown()
{
	int i;
	short widgetIndex;
	rct_window *w;
	rct_widget *widget;

	__asm mov widgetIndex, dx
	__asm mov w, esi
	__asm mov widget, edi

	switch (widgetIndex) {
	case WIDX_TAB_1:
	case WIDX_TAB_2:
		if (_window_guest_list_selected_filter == -1)
			if (_window_guest_list_selected_tab == widgetIndex - WIDX_TAB_1)
				break;
		_window_guest_list_selected_tab = widgetIndex - WIDX_TAB_1;
		_window_guest_list_selected_page = 0;
		_window_guest_list_num_pages = 1;
		window_guest_list_widgets[WIDX_PAGE_DROPDOWN].type = WWT_EMPTY;
		window_guest_list_widgets[WIDX_PAGE_DROPDOWN_BUTTON].type = WWT_EMPTY;
		w->var_490 = 0;
		_window_guest_list_selected_filter = -1;
		window_invalidate(w);
		w->scrolls[0].v_top = 0;
		break;
	case WIDX_PAGE_DROPDOWN_BUTTON:
		widget = &w->widgets[widgetIndex - 1];

		window_dropdown_show_text_custom_width(
			w->x + widget->left,
			w->y + widget->top,
			widget->bottom - widget->top + 1,
			w->colours[1],
			0x80,
			_window_guest_list_num_pages,
			widget->right - widget->left - 3
		);

		for (i = 0; i < 2; i++) {
			gDropdownItemsFormat[i] = 1142;
			gDropdownItemsArgs[i] = STR_PAGE_1 + i;
		}
		RCT2_GLOBAL(0x009DED38, uint32) |= (1 << _window_guest_list_selected_view);
		break;
	case WIDX_INFO_TYPE_DROPDOWN_BUTTON:
		widget = &w->widgets[widgetIndex - 1];

		window_dropdown_show_text_custom_width(
			w->x + widget->left,
			w->y + widget->top,
			widget->bottom - widget->top + 1,
			w->colours[1],
			0x80,
			2,
			widget->right - widget->left - 3
		);

		for (i = 0; i < 2; i++) {
			gDropdownItemsFormat[i] = 1142;
			gDropdownItemsArgs[i] = STR_ACTIONS + i;
		}
		RCT2_GLOBAL(0x009DED38, uint32) |= (1 << _window_guest_list_selected_view);
		break;
	}
}

/**
 * 
 *  rct2: 0x00699AE1
 */
static void window_guest_list_dropdown()
{
	short dropdownIndex, widgetIndex;
	rct_window *w;

	__asm mov dropdownIndex, ax
	__asm mov widgetIndex, dx
	__asm mov w, esi

	switch (widgetIndex) {
	case WIDX_PAGE_DROPDOWN_BUTTON:
		if (dropdownIndex == -1)
			break;
		_window_guest_list_selected_page = dropdownIndex;
		window_invalidate(w);
		break;
	case WIDX_INFO_TYPE_DROPDOWN_BUTTON:
		if (dropdownIndex == -1)
			break;
		_window_guest_list_selected_view = dropdownIndex;
		window_invalidate(w);
		break;
	}
}

/**
 * 
 *  rct2: 0x00699E54
 */
static void window_guest_list_update(rct_window *w)
{
	if (RCT2_GLOBAL(0x00F1AF20, uint16) != 0)
		RCT2_GLOBAL(0x00F1AF20, uint16)--;
	w->var_490++;
	if (w->var_490 >= (_window_guest_list_selected_tab == PAGE_INDIVIDUAL ? 24 : 32))
		w->var_490 = 0;
	widget_invalidate(WC_GUEST_LIST, 0, WIDX_TAB_1 + _window_guest_list_selected_tab);
}

/**
 * 
 *  rct2: 0x00699C55
 */
static void window_guest_list_scrollgetsize()
{
	int i, y, numGuests, spriteIdx;
	rct_window *w;
	rct_peep *peep;

	__asm mov w, esi

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		// Count the number of guests
		numGuests = 0;

		spriteIdx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
		while (spriteIdx != SPRITE_INDEX_NULL) {
			peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx].peep);
			spriteIdx = peep->next;

			if (peep->type != PEEP_TYPE_GUEST)
				continue;
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1)
				if (window_guest_list_is_peep_in_filter(peep))
					continue;
			numGuests++;
		}
		w->var_492 = numGuests;
		y = numGuests * 10;
		break;
	case PAGE_SUMMARISED:
		// Find the groups
		window_guest_list_find_groups();
		w->var_492 = _window_guest_list_num_groups;
		y = _window_guest_list_num_groups * 21;
		break;
	}

	RCT2_GLOBAL(0x00F1EE09, uint32) = numGuests;
	i = _window_guest_list_selected_page;
	for (i = _window_guest_list_selected_page - 1; i >= 0; i--)
		y -= 0x7BF2;
	if (y < 0)
		y = 0;
	if (y > 0x7BF2)
		y = 0x7BF2;
	if (_window_guest_list_highlighted_index != -1) {
		_window_guest_list_highlighted_index = -1;
		window_invalidate(w);
	}

	i = y - window_guest_list_widgets[WIDX_GUEST_LIST].bottom + window_guest_list_widgets[WIDX_GUEST_LIST].top + 21;
	if (i < 0)
		i = 0;
	if (i < w->scrolls[0].v_top) {
		w->scrolls[0].v_top = i;
		window_invalidate(w);
	}

	__asm mov ecx, 447
	__asm mov edx, y
}

/**
 * 
 *  rct2: 0x00699D7D
 */
static void window_guest_list_scrollmousedown()
{
	int i, spriteIdx;
	short y;
	rct_window *w;
	rct_peep *peep;

	__asm mov y, dx
	__asm mov w, esi

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		i = y / 10;
		i += _window_guest_list_selected_page * 3173;
		spriteIdx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
		while (spriteIdx != SPRITE_INDEX_NULL) {
			peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx].peep);
			spriteIdx = peep->next;

			if (peep->type != PEEP_TYPE_GUEST)
				continue;
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1)
				if (window_guest_list_is_peep_in_filter(peep))
					continue;

			if (i == 0) {
				// Open guest window
				RCT2_CALLPROC_X(0x006989E9, 0, 0, 0, (int)peep, 0, 0, 0);
				break;
			} else {
				i--;
			}
		}
		break;
	case PAGE_SUMMARISED:
		i = y / 21;
		if (i < _window_guest_list_num_groups) {
			RCT2_GLOBAL(0x00F1EDF6, uint32) = _window_guest_list_groups_argument_1[i];
			RCT2_GLOBAL(0x00F1EDFA, uint32) = _window_guest_list_groups_argument_2[i];
			_window_guest_list_selected_filter = _window_guest_list_selected_view;
			_window_guest_list_selected_tab = PAGE_INDIVIDUAL;
			window_invalidate(w);
			w->scrolls[0].v_top = 0;
		}
		break;
	}
}

/**
 * 
 *  rct2: 0x00699D3B
 */
static void window_guest_list_scrollmouseover()
{
	int i;
	short y;
	rct_window *w;

	__asm mov y, dx
	__asm mov w, esi

	i = y / (_window_guest_list_selected_tab == PAGE_INDIVIDUAL ? 10 : 21);
	i += _window_guest_list_selected_page * 3173;
	if (i != _window_guest_list_highlighted_index) {
		_window_guest_list_highlighted_index = i;
		window_invalidate(w);
	}
}

/**
 * 
 *  rct2: 0x00699E4A
 */
static void window_guest_list_tooltip()
{
	RCT2_GLOBAL(0x013CE952, uint16) = STR_LIST;
}

/**
 * 
 *  rct2: 0x00699511
 */
static void window_guest_list_invalidate()
{
	rct_window *w;

	__asm mov w, esi

	w->pressed_widgets &= ~(1 << WIDX_TAB_1);
	w->pressed_widgets &= ~(1 << WIDX_TAB_2);
	w->pressed_widgets |= (1LL << (_window_guest_list_selected_tab + WIDX_TAB_1));

	window_guest_list_widgets[WIDX_INFO_TYPE_DROPDOWN].image = STR_ACTIONS + _window_guest_list_selected_view;
	window_guest_list_widgets[WIDX_MAP].type = WWT_EMPTY;
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL && _window_guest_list_selected_filter != -1)
		window_guest_list_widgets[WIDX_MAP].type = WWT_FLATBTN;

	window_guest_list_widgets[WIDX_BACKGROUND].right = w->width - 1;
	window_guest_list_widgets[WIDX_BACKGROUND].bottom = w->height - 1;
	window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].right = w->width - 1;
	window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].bottom = w->height - 1;
	window_guest_list_widgets[WIDX_TITLE].right = w->width - 2;
	window_guest_list_widgets[WIDX_CLOSE].left = w->width - 13;
	window_guest_list_widgets[WIDX_CLOSE].right = w->width - 3;

	window_guest_list_widgets[WIDX_GUEST_LIST].right = w->width - 4;
	window_guest_list_widgets[WIDX_GUEST_LIST].bottom = w->height - 15;
	window_guest_list_widgets[WIDX_PAGE_DROPDOWN].image = _window_guest_list_selected_page + 3440;
}

/**
 * 
 *  rct2: 0x006995CC
 */
static void window_guest_list_paint()
{
	int i, x, y, format;
	rct_window *w;
	rct_drawpixelinfo *dpi;

	__asm mov w, esi
	__asm mov dpi, edi

	// Widgets
	window_draw_widgets(w, dpi);
	// Tab 1 image
	i = (_window_guest_list_selected_tab == 0 ? w->var_490 & 0x0FFFFFFFC : 0);
	i += RCT2_ADDRESS(RCT2_GLOBAL(0x00982708, int), int)[0] + 1;
	i |= 0xA1600000;
	gfx_draw_sprite(
		dpi,
		i,
		(window_guest_list_widgets[WIDX_TAB_1].left + window_guest_list_widgets[WIDX_TAB_1].right) / 2 + w->x,
		window_guest_list_widgets[WIDX_TAB_1].bottom - 6 + w->y
	);

	// Tab 2 image
	i = (_window_guest_list_selected_tab == 1 ? w->var_490 / 4 : 0);
	gfx_draw_sprite(
		dpi,
		5568 + i,
		window_guest_list_widgets[WIDX_TAB_2].left + w->x,
		window_guest_list_widgets[WIDX_TAB_2].top + w->y
	);

	// Filter description
	x = w->x + 6;
	y = w->y + window_guest_list_widgets[WIDX_TAB_CONTENT_PANEL].top + 3;
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL) {
		if (_window_guest_list_selected_filter != -1) {
			if (RCT2_GLOBAL(0x00F1EDF6, sint16) != -1)
				format = STR_GUESTS_FILTER + _window_guest_list_selected_filter;
			else
				format = STR_GUESTS_FILTER_THINKING_ABOUT;
		} else {
			format = STR_ALL_GUESTS;
		}
	} else {
		format = STR_ALL_GUESTS_SUMMARISED;
	}
	gfx_draw_string_left_clipped(dpi, format, (void*)0x00F1EDF6, 0, x, y, 310);

	// Number of guests (list items)
	if (_window_guest_list_selected_tab == PAGE_INDIVIDUAL) {
		x = w->x + 4;
		y = w->y + window_guest_list_widgets[WIDX_GUEST_LIST].bottom + 2;
		RCT2_GLOBAL(0x013CE952, sint16) = w->var_492;
		gfx_draw_string_left(dpi, (w->var_492 == 1 ? 1755 : 1754), (void*)0x013CE952, 0, x, y);
	}
}

/**
 * 
 *  rct2: 0x00699701
 */
static void window_guest_list_scrollpaint()
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	int spriteIdx, format, numGuests, i, j, y;
	rct_window *w;
	rct_drawpixelinfo *dpi;
	rct_peep *peep;
	rct_peep_thought *thought;

	__asm mov w, esi
	__asm mov dpi, edi

	// Background fill
	gfx_fill_rect(dpi, dpi->x, dpi->y, dpi->x + dpi->width - 1, dpi->y + dpi->height - 1, ((char*)0x0141FC48)[w->colours[1] * 8]);

	switch (_window_guest_list_selected_tab) {
	case PAGE_INDIVIDUAL:
		i = 0;
		y = _window_guest_list_selected_page * -0x7BF2;

		// For each guest
		spriteIdx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
		while (spriteIdx != SPRITE_INDEX_NULL) {
			peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx].peep);
			spriteIdx = peep->next;

			if (peep->type != PEEP_TYPE_GUEST)
				continue;
			peep->var_0C &= ~0x200;
			if (peep->var_2A != 0)
				continue;
			if (_window_guest_list_selected_filter != -1) {
				if (window_guest_list_is_peep_in_filter(peep))
					continue;
				RCT2_GLOBAL(0x009AC861, uint16) |= 1;
				peep->var_0C |= 0x200;
			}

			// 
			if (y + 11 >= -0x7FFF && y + 11 > dpi->y && y < 0x7FFF) {
				// Check if y is beyond the scroll control
				if (y > dpi->y + dpi->height)
					break;

				// Highlight backcolour and text colour (format)
				format = 1191;
				if (i == _window_guest_list_highlighted_index) {
					gfx_fill_rect(dpi, 0, y, 800, y + 9, 0x02000031);
					format = 1193;
				}

				// Guest name
				RCT2_GLOBAL(0x013CE952, uint16) = peep->name_string_idx;
				RCT2_GLOBAL(0x013CE954, uint32) = peep->id;
				gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 0, y - 1, 113);

				switch (_window_guest_list_selected_view) {
				case VIEW_ACTIONS:
					// Guest face
					gfx_draw_sprite(dpi, get_guest_face_sprite_small(peep), 118, y);

					// Tracking icon
					if (peep->flags & PEEP_FLAGS_TRACKING)
						gfx_draw_sprite(dpi, 5129, 112, y);
					
					// Action
					eax = peep->var_0A;
					RCT2_CALLFUNC_X(0x00698B0D, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
					ebx &= 0xFFFF;
					ecx &= 0xFFFF;
					
					RCT2_GLOBAL(0x013CE952, uint16) = ebx;
					RCT2_GLOBAL(0x013CE952 + 2, uint16) = ecx;
					RCT2_GLOBAL(0x013CE952 + 4, uint32) = edx;
					gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 133, y - 1, 314);
					break;
				case VIEW_THOUGHTS:
					// For each thought
					for (j = 0; j < PEEP_MAX_THOUGHTS; j++) {
						thought = &peep->thoughts[j];
						if (thought->type == PEEP_THOUGHT_TYPE_NONE)
							break;
						if (thought->pad_3 == 0)
							continue;
						if (thought->pad_3 > 5)
							break;

						ebx = thought->type;
						eax = thought->item;
						RCT2_CALLFUNC_X(0x00698342, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
						ebx &= 0xFFFF;

						RCT2_GLOBAL(0x013CE952, uint16) = ebx;
						RCT2_GLOBAL(0x013CE952 + 2, uint32) = *((uint32*)esi);
						RCT2_GLOBAL(0x013CE952 + 6, uint16) = *((uint16*)(esi + 4));
						gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 118, y - 1, 329);
						break;
					}
					break;
				}
			}

			// Increment list item index and y
			i++;
			y += 10;
		}
		break;
	case PAGE_SUMMARISED:
		y = 0;

		// For each group of guests
		for (i = 0; i < _window_guest_list_num_groups; i++) {
			// Check if y is beyond the scroll control
			if (y + 22 >= dpi->y) {
				// Check if y is beyond the scroll control
				if (y >= dpi->y + dpi->height)
					break;

				// Highlight backcolour and text colour (format)
				format = 1191;
				if (i == _window_guest_list_highlighted_index) {
					gfx_fill_rect(dpi, 0, y, 800, y + 20, 0x02000031);
					format = 1193;
				}

				// Draw guest faces
				numGuests = _window_guest_list_groups_num_guests[i];
				for (j = 0; j < 56 && j < numGuests; j++)
					gfx_draw_sprite(dpi, _window_guest_list_groups_guest_faces[numGuests * 56 + j] + 5486, j * 8, y + 9);

				// Draw action
				RCT2_GLOBAL(0x013CE952, uint16) = _window_guest_list_groups_argument_1[i] & 0xFFFF;
				RCT2_GLOBAL(0x013CE952 + 2, uint16) = _window_guest_list_groups_argument_1[i] >> 16;
				RCT2_GLOBAL(0x013CE952 + 4, uint32) = _window_guest_list_groups_argument_2[i];
				RCT2_GLOBAL(0x013CE952 + 10, uint32) = numGuests;
				gfx_draw_string_left_clipped(dpi, format, (void*)0x013CE952, 0, 0, y - 1, 414);

				// Draw guest count
				RCT2_GLOBAL(0x013CE95A, uint16) = STR_GUESTS_COUNT_COMMA_SEP;
				gfx_draw_string_right(dpi, format, (void*)0x0013CE95A, 0, 326, y - 1);
			}
			y += 21;
		}
		break;
	}
}

/**
 * 
 *  rct2: 0x0069B865
 */
static int window_guest_list_is_peep_in_filter(rct_peep* peep)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;
	char temp;

	temp = _window_guest_list_selected_view;
	_window_guest_list_selected_view = _window_guest_list_selected_filter;
		
	esi = (int)peep;
	RCT2_CALLFUNC_X(0x0069B7EA, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
	ebx &= 0xFFFF;

	_window_guest_list_selected_view = temp;
	eax = (RCT2_GLOBAL(0x013CE952, uint16) << 16) | ebx;

	if (((RCT2_GLOBAL(0x00F1EDF6, uint32) >> 16) & 0xFFFF) == 0xFFFF && _window_guest_list_selected_filter == 1)
		eax |= 0xFFFF;

	if (eax == RCT2_GLOBAL(0x00F1EDF6, uint32) && RCT2_GLOBAL(0x013CE954, uint32) == RCT2_GLOBAL(0x00F1EDFA, uint32))
		return 0;
	return 1;
}

static int sub_69B7EA(rct_peep *peep, int *outEAX)
{
	int eax, ebx, ecx, edx, esi, edi, ebp;

	switch (_window_guest_list_selected_view) {
	case VIEW_ACTIONS:
		eax = peep->var_0A;
		RCT2_CALLFUNC_X(0x00698B0D, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
		RCT2_GLOBAL(0x013CE952, uint16) = ecx & 0xFFFF;
		RCT2_GLOBAL(0x013CE952 + 2, uint32) = edx;

		*outEAX = eax;
		return ebx & 0xFFFF;
	case VIEW_THOUGHTS:
		if (peep->thoughts[0].pad_3 <= 5) {
			eax = peep->thoughts[0].item;
			ebx = peep->thoughts[0].type;
			if (peep->thoughts[0].type != PEEP_THOUGHT_TYPE_NONE) {
				RCT2_CALLFUNC_X(0x00698342, &eax, &ebx, &ecx, &edx, &esi, &edi, &ebp);
				RCT2_GLOBAL(0x013CE952, uint16) = *((uint16*)esi);
				RCT2_GLOBAL(0x013CE952 + 2, uint32) = *((uint32*)(esi + 2));

				*outEAX = eax;
				return ebx & 0xFFFF;
			}
		}

		RCT2_GLOBAL(0x013CE952, uint16) = 0;
		RCT2_GLOBAL(0x013CE952 + 2, uint32) = 0;

		*outEAX = 0;
		return 0;
	}

	return 0;
}

/**
 * 
 *  rct2: 0x0069B5AE
 */
static void window_guest_list_find_groups()
{
	int spriteIdx, spriteIdx2, groupIndex, faceIndex;
	rct_peep *peep, *peep2;

	int eax = RCT2_GLOBAL(0x00F663AC, uint32) & 0xFFFFFF00;
	if (_window_guest_list_selected_view == RCT2_GLOBAL(0x00F1EE02, uint32))
		if (RCT2_GLOBAL(0x00F1AF20, uint16) != 0 || eax == RCT2_GLOBAL(0x00F1AF1C, uint32))
			return;

	RCT2_GLOBAL(0x00F1AF1C, uint32) = eax;
	RCT2_GLOBAL(0x00F1EE02, uint32) = _window_guest_list_selected_view;
	RCT2_GLOBAL(0x00F1AF20, uint16) = 320;
	_window_guest_list_num_groups = 0;

	// Set all guests to unassigned
	spriteIdx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
	while (spriteIdx != SPRITE_INDEX_NULL) {
		peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx].peep);
		spriteIdx = peep->next;

		if (peep->type != PEEP_TYPE_GUEST || peep->var_2A != 0)
			continue;

		peep->var_0C |= (1 << 8);
	}

	// For each guest / group
	spriteIdx = RCT2_GLOBAL(RCT2_ADDRESS_SPRITES_START_PEEP, uint16);
	while (spriteIdx != SPRITE_INDEX_NULL) {
		peep = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx].peep);
		spriteIdx = peep->next;

		if (peep->type != PEEP_TYPE_GUEST || peep->var_2A != 0 || !(peep->var_0C & (1 << 8)))
			continue;

		// New group, cap at 240 though
		groupIndex = _window_guest_list_num_groups;
		if (groupIndex >= 240)
			break;

		int ax = peep->var_0A;
		_window_guest_list_num_groups++;
		_window_guest_list_groups_num_guests[groupIndex] = 1;
		peep->var_0C &= ~(1 << 8);
		int bx = sub_69B7EA(peep, &eax);
		eax = (RCT2_GLOBAL(0x013CE952, uint16) << 16) | bx;
		RCT2_GLOBAL(0x00F1EDF6, uint32) = eax;
		_window_guest_list_groups_argument_1[groupIndex] = eax;
		RCT2_GLOBAL(0x00F1EDFA, uint32) = RCT2_GLOBAL(0x013CE952 + 2, uint32);
		_window_guest_list_groups_argument_2[groupIndex] = RCT2_GLOBAL(0x013CE952 + 2, uint32);
		RCT2_ADDRESS(0x00F1AF26, uint8)[groupIndex] = groupIndex;
		faceIndex = groupIndex * 56;
		_window_guest_list_groups_guest_faces[faceIndex++] = get_guest_face_sprite_small(peep) - 5486;

		// Find more peeps that belong to same group
		spriteIdx2 = peep->next;
		while (spriteIdx2 != SPRITE_INDEX_NULL) {
			peep2 = &(RCT2_ADDRESS(RCT2_ADDRESS_SPRITE_LIST, rct_sprite)[spriteIdx2].peep);
			spriteIdx2 = peep2->next;

			if (peep2->type != PEEP_TYPE_GUEST || peep2->var_2A != 0 || !(peep2->var_0C & (1 << 8)))
				continue;

			// Get and check if in same group
			// BUG this doesn't work!
			bx = sub_69B7EA(peep2, &eax);
			if (bx != RCT2_GLOBAL(0x00F1EDF6, uint32) || (eax & 0xFFFF) != RCT2_GLOBAL(0x013CE952, uint16) || eax != RCT2_GLOBAL(0x013CE952 + 2, uint32))
				continue;

			// Assign guest
			_window_guest_list_groups_num_guests[groupIndex]++;
			peep2->var_0C &= ~(1 << 8);

			// Add face sprite, cap at 56 though
			if (_window_guest_list_groups_num_guests[groupIndex] < 56)
				continue;
			_window_guest_list_groups_guest_faces[faceIndex++] = get_guest_face_sprite_small(peep2) - 5486;
		}

		if (RCT2_GLOBAL(0x00F1EDF6, uint16) == 0) {
			_window_guest_list_num_groups--;
			continue;
		}

		ax = _window_guest_list_groups_num_guests[groupIndex];
		int edi = 0;

		while (1) {
			if (edi >= groupIndex)
				goto nextPeep;
			if (ax > _window_guest_list_groups_num_guests[edi])
				break;
			edi++;
		}

		int ecx = _window_guest_list_groups_argument_1[groupIndex];
		int edx = _window_guest_list_groups_argument_2[groupIndex];
		int bl = RCT2_ADDRESS(0x00F1AF26, uint8)[groupIndex];
		int temp;

		do {
			temp = ax;
			ax = _window_guest_list_groups_num_guests[edi];
			_window_guest_list_groups_num_guests[edi] = temp;

			temp = ecx;
			ecx = _window_guest_list_groups_argument_1[edi];
			_window_guest_list_groups_argument_1[edi] = temp;

			temp = edx;
			edx = _window_guest_list_groups_argument_2[edi];
			_window_guest_list_groups_argument_2[edi] = temp;

			RCT2_ADDRESS(0x00F1AF26, uint8)[edi] = bl;
			bl = RCT2_ADDRESS(0x00F1AF26, uint8)[edi];
		} while (++edi <= groupIndex);

	nextPeep:
		;
	}
}

/**
 *  Function split into large and small sprite
 *  rct2: 0x00698721
 */
static int get_guest_face_sprite_small(rct_peep *peep)
{
	int sprite;
	sprite = SPR_PEEP_SMALL_FACE_ANGRY;

	if (peep->var_F3) return sprite;
	sprite = SPR_PEEP_SMALL_FACE_VERY_VERY_SICK;

	if (peep->nausea > 200) return sprite;
	sprite--; //VERY_SICK

	if (peep->nausea > 170) return sprite;
	sprite--; //SICK

	if (peep->nausea > 140) return sprite;
	sprite = SPR_PEEP_SMALL_FACE_VERY_TIRED;

	if (peep->energy < 46) return sprite;
	sprite--; //TIRED

	if (peep->energy < 70) return sprite;
	sprite = SPR_PEEP_SMALL_FACE_VERY_VERY_UNHAPPY;

	//There are 7 different happiness based faces
	for (int i = 37; peep->happiness >= i; i += 37)
	{
		sprite++;
	}

	return sprite;
}

/**
*  Function split into large and small sprite
*  rct2: 0x00698721
*/
static int get_guest_face_sprite_large(rct_peep* peep){
	int sprite;
	sprite = SPR_PEEP_LARGE_FACE_ANGRY;

	if (peep->var_F3) return sprite;
	sprite = SPR_PEEP_LARGE_FACE_VERY_VERY_SICK;

	if (peep->nausea > 200) return sprite;
	sprite = SPR_PEEP_LARGE_FACE_VERY_SICK;

	if (peep->nausea > 170) return sprite;
	sprite = SPR_PEEP_LARGE_FACE_SICK;

	if (peep->nausea > 140) return sprite;
	sprite = SPR_PEEP_LARGE_FACE_VERY_TIRED;

	if (peep->energy < 46) return sprite;
	sprite--; //TIRED

	if (peep->energy < 70) return sprite;
	sprite = SPR_PEEP_LARGE_FACE_VERY_VERY_UNHAPPY;

	//There are 7 different happiness based faces
	for (int i = 37; peep->happiness >= i; i += 37)
	{
		sprite++;
	}

	return sprite;
}
