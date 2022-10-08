/*
 * Copyright 1999-2006 Trent Gamblin
 *
 * This file is part of Stax.
 *
 * Stax is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Stax is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Stax; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <new>
#include <vector>
#include <cstdlib>
#include <cctype>
using namespace std;

#include <allegro.h>
#include "stax.h"

static const int GUI_OUTER_BORDER_WIDTH = 3;
static const int GUI_INNER_BORDER_WIDTH = 3;
static const int GUI_SPACING = 20;
static const int GUI_WINDOW_RADIUS = 12;   // The radius of the circle used for the corners
static const int GUI_BUTTON_HEIGHT = 30;
static const int GUI_BUTTON_RADIUS = (GUI_BUTTON_HEIGHT - (GUI_INNER_BORDER_WIDTH * 2)) / 2;
static const int GUI_BUTTON_PADDING = 15;

static const int GUI_CHECKBOX_WIDTH = 20;
static const int GUI_CHECKBOX_SPACING = 5;
static const int GUI_CHAR_SELECTOR_WIDTH = 70;
static const int GUI_ARROW_WIDTH = 20;
static const int GUI_OPTION_SPACING = 10;
static const int GUI_CHANGE_WAIT_RESET = -1;
static const int GUI_CHANGE_WAIT_FIRST = 500;
static const int GUI_CHANGE_WAIT = 150;
static const int GUI_SCROLLBAR_MIN_SIZE = 20;
static const int GUI_SCROLLBAR_WIDTH = 15;

static int change_wait;

int GUI_TOP_COLOR;
int GUI_BOTTOM_COLOR;
int GUI_BORDER_COLOR;
int GUI_BORDER_COLOR_MIDDLE;
int GUI_SELECTED_BORDER_COLOR;
int GUI_SELECTED_BORDER_COLOR_MIDDLE;
int GUI_UNSELECTED_COLOR;
int GUI_TEXT_COLOR;
int GUI_SELECTED_TEXT_COLOR;
int GUI_DEPRESSED_TEXT_COLOR;
int GUI_BASE_COLOR;
int GUI_TEXT_OUTLINE_COLOR;

BITMAP *arrow_left = NULL, *arrow_left_selected = NULL, *arrow_left_depressed = NULL;
BITMAP *arrow_right = NULL, *arrow_right_selected = NULL, *arrow_right_depressed = NULL;
BITMAP *check_on = NULL, *check_on_selected = NULL, *check_on_depressed = NULL;
BITMAP *check_off = NULL, *check_off_selected = NULL, *check_off_depressed = NULL;
BITMAP *radio_on = NULL, *radio_on_selected = NULL, *radio_on_depressed = NULL;
BITMAP *radio_off = NULL, *radio_off_selected = NULL, *radio_off_depressed = NULL;
BITMAP *slider_tab = NULL, *slider_tab_selected = NULL, *slider_tab_depressed = NULL;

bool in_gui = false;

static int *outer_border_colors;
static int *inner_border_colors;

static int char_width;

static void StripColorCodes(char *dest, char *src)
{
	int i, j = 0;

	for (i = 0; src[i]; i++) {
		if (src[i] == '|') {
			i++;
			if (isdigit(src[i])) {
				i += 8;
				continue;
			}
			else if (src[i] == 'O') {
				continue;
			}
		}
		dest[j++] = src[i];
	}
	dest[j] = 0;
}

/*
 * Draw a string to the screen using various colors for different parts of
 * the string. the string "s" can have color codes in it. To specify a color
 * write a '|' followed by 9 digits in the form RRRGGGBBB. Each color component
 * (red, green and blue) should be a value from 0 to 255. To use the original
 * color (the one passed to this function) write "|O". To draw a '|' character
 * use two of them. For example to write a string "FOO BAR" with "BAR"
 * appearing red, pass "FOO |255000000BAR" as the "s" parameter.
 */
void textout_color(BITMAP *b, FONT *f, char *s, int x, int y, int color, int bg)
{
	int i, j, k, a[3];
	char ch[2];
	ch[1] = 0;
	char comp[4];
	comp[3] = 0;
	int cc = color;

	for (i = 0; s[i]; i++) {
		if (s[i] == '|') {
			i++;
			if (isdigit(s[i])) {
				for (j = 0; j < 3; j++) {
					for (k = 0; k < 3; k++) {
						comp[k] = s[i++];
					}
					a[j] = atoi(comp);
				}
				cc = makecol(a[0], a[1], a[2]);
				i--;
				continue;
			}
			else if (s[i] == 'O') {
				cc = color;
				continue;
			}
		}
		ch[0] = s[i];
		textout_ex(b, f, ch, x, y, cc, bg);
		x += text_length(f, ch);
	}
}

void textout_outline(BITMAP *b, FONT *f, char *s, int x, int y, int color, int outline_color, int bg)
{
	int i;
	
	char stripped[MAX_STRING+1];
	StripColorCodes(stripped, s);

	textout_ex(b, f, stripped, x-1, y, outline_color, bg);
	textout_ex(b, f, stripped, x, y-1, outline_color, bg);
	textout_ex(b, f, stripped, x, y+1, outline_color, bg);
	textout_ex(b, f, stripped, x-1, y-1, outline_color, bg);
	textout_ex(b, f, stripped, x-1, y+1, outline_color, bg);
	textout_ex(b, f, stripped, x+1, y-1, outline_color, bg);
	textout_ex(b, f, stripped, x+1, y, outline_color, bg);
	textout_ex(b, f, stripped, x+1, y+1, outline_color, bg);

	textout_color(b, f, s, x, y, color, bg);
}

static int text_length_color(FONT *f, char *s)
{
	char tmp[MAX_STRING+1];
	StripColorCodes(tmp, s);
	return text_length(f, tmp);
}

static int find_first_x(BITMAP *b, int y)
{
	for (int i = 0; i < b->w; i++) {
		if (getpixel(b, i, y) != makecol(255, 0, 255)) {
			return i;
		}
	}

	return -1;
}

static int find_last_x(BITMAP *b, int y)
{
	for (int i = b->w-1; i >= 0; i--) {
		if (getpixel(b, i, y) != makecol(255, 0, 255)) {
			return i;
		}
	}

	return -1;
}

static int find_first_y(BITMAP *b, int x)
{
	for (int i = 0; i < b->h; i++) {
		if (getpixel(b, x, i) != makecol(255, 0, 255)) {
			return i;
		}
	}

	return -1;
}

static int find_last_y(BITMAP *b, int x)
{
	for (int i = b->h-1; i >= 0; i--) {
		if (getpixel(b, x, i) != makecol(255, 0, 255)) {
			return i;
		}
	}

	return -1;
}

/*
 * Call with percent = 0.0 - 100.0 
 */ 
int GUI_GetGradientColor(float percent, int top_color, int bottom_color)
{
	percent /= 100.0;
	if (get_color_depth() == 8) {
		percent = 0.5;
	}
	int r = +(int)(getr(bottom_color) + ((getr(top_color) - getr(bottom_color)) * percent));
	int g = +(int)(getg(bottom_color) + ((getg(top_color) - getg(bottom_color)) * percent));
	int b = +(int)(getb(bottom_color) + ((getb(top_color) - getb(bottom_color)) * percent));

	return makecol(r, g, b);
}

static void GUI_Outline(BITMAP *b, int color)
{
	std::vector<Point> border;
	Point p;
	int x, y;

	for (x = 0; x < b->w; x++) {
		y = find_first_y(b, x) - 1;
		if (y < 0) {
			continue;
		}
		p.x = x;
		p.y = y;
		border.push_back(p);
		y = find_last_y(b, x) + 1;
		p.y = y;
		border.push_back(p);
	}

	for (x = 0; (unsigned int)x < border.size(); x++) {
		putpixel(b, border[x].x, border[x].y, color);
	}
	border.clear();

	for (y = 0; y < b->h; y++) {
		x = find_first_x(b, y) - 1;
		if (x < 0) {
			continue;
		}
		p.x = x;
		p.y = y;
		border.push_back(p);
		x = find_last_x(b, y) + 1;
		p.x = x;
		border.push_back(p);
	}

	for (x = 0; (unsigned int)x < border.size(); x++) {
		putpixel(b, border[x].x, border[x].y, color);
	}

	border.clear();
}

typedef struct {
	int size;
	int options;
	int lines;
	int start;
	float inc_multiplier;
} ListData;

static int GUI_GetMinButtonWidth(char *s)
{
	return (GUI_BUTTON_RADIUS * 2) + (GUI_INNER_BORDER_WIDTH * 2) + (GUI_BUTTON_PADDING * 2) + text_length_color(font, s);
}


/*
 * Pass -1 width to let this function figure out the minimum width
 */
static BITMAP *GUI_CreateButton(int w, char *s, WidgetState state) throw (std::bad_alloc)
{
	bool selected = state == WIDGET_STATE_SELECTED || state == WIDGET_STATE_DEPRESSED;

	if (w < 0) {
		w = GUI_GetMinButtonWidth(s);
	}

	BITMAP *b = create_bitmap(w, GUI_BUTTON_HEIGHT);
	if (!b) {
		throw std::bad_alloc();
	}
	clear_to_color(b, makecol(255, 0, 255));

	circlefill(b, GUI_BUTTON_RADIUS+GUI_INNER_BORDER_WIDTH,
		        GUI_BUTTON_RADIUS+GUI_INNER_BORDER_WIDTH, 
			GUI_BUTTON_RADIUS, GUI_BASE_COLOR);
	circlefill(b, w-GUI_INNER_BORDER_WIDTH-GUI_BUTTON_RADIUS-1,
		        GUI_INNER_BORDER_WIDTH+GUI_BUTTON_RADIUS,
			GUI_BUTTON_RADIUS, GUI_BASE_COLOR);
	rectfill(b, GUI_INNER_BORDER_WIDTH+GUI_BUTTON_RADIUS,
		       	GUI_INNER_BORDER_WIDTH,
			w-GUI_INNER_BORDER_WIDTH-GUI_BUTTON_RADIUS-1,
			GUI_BUTTON_HEIGHT-GUI_INNER_BORDER_WIDTH,
			GUI_BASE_COLOR);
	line(b, 0, GUI_BUTTON_HEIGHT-GUI_INNER_BORDER_WIDTH, w-1,
			GUI_BUTTON_HEIGHT-GUI_INNER_BORDER_WIDTH, makecol(255, 0, 255));


	int y, f, l, i, color;
	int top_color = selected ? GUI_TOP_COLOR : GUI_UNSELECTED_COLOR;
	int bottom_color = selected ? GUI_BOTTOM_COLOR : GUI_UNSELECTED_COLOR;
	
	for (y = GUI_INNER_BORDER_WIDTH; y < GUI_BUTTON_HEIGHT-GUI_INNER_BORDER_WIDTH+1; y++) {
		f = find_first_x(b, y);
		l = find_last_x(b, y);
		if (get_color_depth() == 8) {
			if (state == WIDGET_STATE_SELECTED) {
				color = GUI_TOP_COLOR;
			}
			else if (state == WIDGET_STATE_DEPRESSED) {
				color = GUI_BOTTOM_COLOR;
			}
			else {
				color = GUI_GetGradientColor(50.0, top_color, bottom_color);
			}
		}
		else {
			color = GUI_GetGradientColor(100.0 - ((float)y / (float)(GUI_BUTTON_HEIGHT-2) * 100.0),
					top_color, bottom_color);
		}
		line(b, f, y, l, y, color);
	}
	
	if (state == WIDGET_STATE_SELECTED) {
		for (i = 0; i < GUI_INNER_BORDER_WIDTH; i++) {
			color = inner_border_colors[i];
			GUI_Outline(b, color);
		}
	}

	int tw = text_length_color(font, s);
	int th = text_height(font);

	if (state == WIDGET_STATE_SELECTED) {
		textout_outline(b, font, s, (w/2)-(tw/2), (GUI_BUTTON_HEIGHT/2)-(th/2), GUI_SELECTED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else if (state ==  WIDGET_STATE_DEPRESSED) {
		textout_outline(b, font, s, (w/2)-(tw/2), (GUI_BUTTON_HEIGHT/2)-(th/2), GUI_DEPRESSED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else { 
		textout_outline(b, font, s, (w/2)-(tw/2), (GUI_BUTTON_HEIGHT/2)-(th/2), GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}

	return b;
}

// Create a circular button just big enough to fit one character
static BITMAP *GUI_CreateCharButton(int ch, WidgetState state) throw (std::bad_alloc)
{
	const int r = 11;
	const int w = 30, h = 30;

	BITMAP *b = create_bitmap(w, h);
	clear_to_color(b, makecol(255, 0, 255));
	circlefill(b, w/2, h/2, r, GUI_BASE_COLOR);

	int y, f, l, c;

	int top_color = state == WIDGET_STATE_UNSELECTED ? GUI_UNSELECTED_COLOR : GUI_TOP_COLOR;
	int bottom_color = state == WIDGET_STATE_UNSELECTED ? GUI_UNSELECTED_COLOR : GUI_BOTTOM_COLOR;
	
	for (y = 0; y < h; y++) {
		f = find_first_x(b, y);
		if (f < 0) {
			continue;
		}
		l = find_last_x(b, y);
		if (get_color_depth() == 8) {
			if (state == WIDGET_STATE_SELECTED) {
				c = GUI_TOP_COLOR;
			}
			else if (state == WIDGET_STATE_DEPRESSED) {
				c = GUI_BOTTOM_COLOR;
			}
			else {
				c = GUI_GetGradientColor(50.0, top_color, bottom_color);
			}
		}
		else {
			c = GUI_GetGradientColor(100.0 - ((float)y / (float)h * 100.0),
				top_color, bottom_color);
		}
		line(b, f, y, l, y, c);
	}

	if (state == WIDGET_STATE_SELECTED) {
		for (y = 0; y < GUI_INNER_BORDER_WIDTH; y++) {
			c = inner_border_colors[y];
			GUI_Outline(b, c);
		}
	}

	char s[2];
	s[0] = ch;
	s[1] = 0;

	if (state == WIDGET_STATE_SELECTED) {
		textout_outline(b, font, s, (w/2)-(text_length_color(font, s)/2)+1, (h/2)-(text_height(font)/2)+2, GUI_SELECTED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else if (state == WIDGET_STATE_DEPRESSED) {
		textout_outline(b, font, s, (w/2)-(text_length_color(font, s)/2)+1, (h/2)-(text_height(font)/2)+2, GUI_DEPRESSED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else {
		textout_outline(b, font, s, (w/2)-(text_length_color(font, s)/2)+1, (h/2)-(text_height(font)/2)+2, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}

	return b;
}

static int GUI_GetChangeWait(int wait = GUI_CHANGE_WAIT)
{
	if (change_wait <= GUI_CHANGE_WAIT_RESET) {
		return GUI_CHANGE_WAIT_FIRST;
	}
	return wait;
}

static int GUI_WidgetWidth(Widget *w)
{
	int i, len, n;
	OptionGetter g;

	switch (w->type) {
		case WIDGET_EDITOR:
			return w->d1 * char_width + 6;
		case WIDGET_SLIDER:
			return w->w;
		case WIDGET_TEXT:
			return text_length_color(font, w->s);
		case WIDGET_CHECKBOX:
			return GUI_CHECKBOX_WIDTH + GUI_CHECKBOX_SPACING + text_length_color(font, w->s);
		case WIDGET_BUTTON:
			return w->w;
		case WIDGET_TEXTBUTTON:
			return text_length_color(font, w->s);
		case WIDGET_ICON:
			return ((BITMAP *)w->p1)->w;
		case WIDGET_CHAR_SELECTOR:
			return GUI_CHAR_SELECTOR_WIDTH;
		case WIDGET_OPTION:
			g = (OptionGetter)w->p1;
			g(-1, &n);
			len = 0;
			for (i = 0; i < n; i++) {
				if (text_length_color(font, g(i, NULL)) > len) {
					len = text_length_color(font, g(i, NULL));
				}
			}
			len += (GUI_ARROW_WIDTH * 2) + (GUI_OPTION_SPACING * 2);
			// for 320x240 (place on an even boundry)
			if (len % 2) {
				len++;
			}
			if ((len / 2) % 2) {
				len += 2;
			}
			return len;
		case WIDGET_LIST:
			return w->w;
		default:
			break;
	}
	return 0;
}

static int GUI_WidgetHeight(Widget *w)
{
	switch (w->type) {
		case WIDGET_EDITOR:
			return text_height(font) + 6;
		case WIDGET_SLIDER:
			return slider_tab->h;
		case WIDGET_TEXT:
			return text_height(font);
		case WIDGET_CHECKBOX:
			return text_height(font);
		case WIDGET_BUTTON:
			return GUI_BUTTON_HEIGHT;
		case WIDGET_TEXTBUTTON:
			return text_height(font);
		case WIDGET_ICON:
			return ((BITMAP *)w->p1)->h;
		case WIDGET_CHAR_SELECTOR:
			return text_height(font);
		case WIDGET_OPTION:
			return text_height(font);
		case WIDGET_LIST:
			return w->h;
		default:
			break;
	}
	return 0;
}

static int GUI_GetSliderTabX(Widget *w)
{
	int x = ((w->d1 / w->d2) * w->d2) - (slider_tab->w/2);
	if (x < 0) {
		x = 0;
	}
	if (x > w->w-slider_tab->w) {
		x = w->w-slider_tab->w;
	}
	return x;
}

static void GUI_IncSlider(Widget *w)
{
	w->d1 += w->d2;
	if (w->d1 > w->w) {
		w->d1 = w->w;
	}
}

static void GUI_DecSlider(Widget *w)
{
	w->d1 -= w->d2;
	if (w->d1 < 0) {
		w->d1 = 0;
	}
}

static int GUI_ListSBStart(Widget *w)
{
	ListData *l = (ListData *)w->p3;
	return (int)(((float)w->d2 / (float)(l->options - l->lines)) * (float)((w->h - 4) - l->size));
}

static void GUI_ListInc(Widget *w)
{
	ListData *l = (ListData *)w->p3;
	w->d1++;
	if (w->d1 >= l->options) {
		w->d1 = l->options-1;
	}
	if (w->d2 + l->lines <= w->d1) {
		w->d2++;
	}
}

static void GUI_ListDec(Widget *w)
{
	w->d1--;
	if (w->d1 < 0) {
		w->d1 = 0;
	}
	if (w->d1 < w->d2) {
		w->d2 = w->d1;
	}
}

static void GUI_ListPGUP(Widget *w)
{
	ListData *l = (ListData *)w->p3;

	w->d2 -= l->lines;
	if (w->d2 < 0) {
		w->d2 = 0;
	}

	w->d1 -= l->lines;
	if (w->d1 < 0) {
		w->d1 = 0;
	}
}

static void GUI_ListPGDN(Widget *w)
{
	ListData *l = (ListData *)w->p3;

	w->d2 += l->lines;
	if (w->d2 > l->options-l->lines) {
		w->d2 = l->options - l->lines;
	}

	w->d1 += l->lines;
	if (w->d1 >= l->options) {
		w->d1 = l->options-1;
	}
}

static const int num_name_chars = 72;
static int name_chars[num_name_chars] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',
       	'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
       	'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 
	'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
       	'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7',
       	'8', '9', '*', '!', '?', '$', '@', '.', '+', '-', '\'', ' '
};

static const int num_uppercase_chars = 26;
static int uppercase_chars[] = {
	'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
	'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'
};

static const int num_numeric_chars = 10;
static int numeric_chars[] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0'
};

static int *valid_chars = uppercase_chars;
static int num_valid_chars = num_uppercase_chars;

static void SetCharWidth(void)
{
	int i;
	int max = 0;
	int w;
	char s[2];
	s[1] = 0;

	for (i = 0; i < num_valid_chars; i++) {
		s[0] = valid_chars[i];
		w = text_length(font, s);
		if (w > max) {
			max = w;
		}
	}
	char_width = max;
}

static bool GUI_IsValidChar(int k)
{
	int i;

	for (i = 0; i < num_valid_chars; i++) {
		if (k == valid_chars[i]) {
			return true;
		}
	}

	return false;
}

static void GUI_UseUppercaseChars(void)
{
	valid_chars = uppercase_chars;
	num_valid_chars = num_uppercase_chars;
	SetCharWidth();
}

static void GUI_UseNumericChars(void)
{
	valid_chars = numeric_chars;
	num_valid_chars = num_numeric_chars;
	SetCharWidth();
}

static void GUI_UseNameChars(void)
{
	valid_chars = name_chars;
	num_valid_chars = num_name_chars;
	SetCharWidth();
}

static void GUI_SelectValidChars(ValidCharType v)
{
	switch (v) {
	case VALID_NUM:
		GUI_UseNumericChars();
		break;
	case VALID_UPPERCASE:
		GUI_UseUppercaseChars();
		break;
	default:
		GUI_UseNameChars();
		break;
	}
}

static void GUI_IncCharSelector(Widget *w)
{
	GUI_SelectValidChars((ValidCharType)w->d3);
	w->d2++;
	if (w->d1 < 0 && w->d2 >= num_valid_chars)
		w->d2 = 0;
	else if (w->d2 > num_valid_chars) {
		w->d2 = 0;
	}
}

static void GUI_DecCharSelector(Widget *w)
{
	GUI_SelectValidChars((ValidCharType)w->d3);
	w->d2--;
	if (w->d2 < 0) {
		if (w->d1 < 0)
			w->d2 = num_valid_chars - 1;
		else
			w->d2 = num_valid_chars;
	}
}

static bool GUI_IsSelectable(Widget *w)
{
	switch (w->type) {
		case WIDGET_EDITOR:
			if (w->d2) {
				return true;
			}
			else {
				return false;
			}
			break;
		case WIDGET_SLIDER:
		case WIDGET_CHECKBOX:
		case WIDGET_BUTTON:
		case WIDGET_TEXTBUTTON:
		case WIDGET_ICON:
		case WIDGET_CHAR_SELECTOR:
		case WIDGET_OPTION:
		case WIDGET_LIST:
			return true;
		default:
			break;
	}
	return false;
}

static void GUI_CopyWidgets(Widget *dest, Widget *src)
{
	int i = 0;

	do {
		dest[i].type = src[i].type;
		dest[i].x = src[i].x;
		dest[i].y = src[i].y;
		dest[i].w = src[i].w;
		dest[i].h = src[i].h;
	} while (src[i++].type != WIDGET_END);
}

static int GUI_SelectPrevious(Widget *widgets, int selected)
{
	int prev;

	if (selected) {
		prev = selected;
		widgets[selected].state = WIDGET_STATE_UNSELECTED;
		do {
			selected--;
		} while (selected && !GUI_IsSelectable(&widgets[selected]));
		if (!GUI_IsSelectable(&widgets[selected])) {
			selected = prev;
		}
	}
	widgets[selected].state = WIDGET_STATE_SELECTED;
	return selected;
}

static int GUI_SelectNext(Widget *widgets, int selected)
{
	int prev = selected;
	widgets[selected].state = WIDGET_STATE_UNSELECTED;
	do {
		selected++;
	} while (widgets[selected].type != WIDGET_END && !GUI_IsSelectable(&widgets[selected]));
	if (widgets[selected].type == WIDGET_END || !GUI_IsSelectable(&widgets[selected])) {
		selected = prev;
	}
	widgets[selected].state = WIDGET_STATE_SELECTED;
	return selected;
}

static void GUI_DrawWindow(BITMAP *b)
{
	if (!b) {
		return;
	}
	clear_to_color(b, makecol(255, 0, 255));

	circle(b, GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH, 
			GUI_WINDOW_RADIUS, GUI_BASE_COLOR);
	circle(b, b->w-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			GUI_WINDOW_RADIUS, GUI_BASE_COLOR);
	circle(b, b->w-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			b->h-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_WINDOW_RADIUS, GUI_BASE_COLOR);
	circle(b, GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			b->h-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_WINDOW_RADIUS, GUI_BASE_COLOR);
	line(b, GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			GUI_OUTER_BORDER_WIDTH,
			b->w-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_OUTER_BORDER_WIDTH, GUI_BASE_COLOR);
	line(b, b->w-GUI_OUTER_BORDER_WIDTH-1, 
			GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			b->w-GUI_OUTER_BORDER_WIDTH-1,
			b->h-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_BASE_COLOR);
	line(b, GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			b->h-GUI_OUTER_BORDER_WIDTH-1,
			b->w-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			b->h-GUI_OUTER_BORDER_WIDTH-1, GUI_BASE_COLOR);
	line(b, GUI_OUTER_BORDER_WIDTH, 
			GUI_WINDOW_RADIUS+GUI_OUTER_BORDER_WIDTH,
			GUI_OUTER_BORDER_WIDTH,
			b->h-GUI_WINDOW_RADIUS-GUI_OUTER_BORDER_WIDTH-1,
			GUI_BASE_COLOR);

	int y, f, l, i, color;
	
	for (y = GUI_OUTER_BORDER_WIDTH; y < b->h-GUI_OUTER_BORDER_WIDTH; y++) {
		f = find_first_x(b, y);
		l = find_last_x(b, y);
		color = GUI_GetGradientColor(100.0 - ((float)y / (float)(b->h-2) * 100.0),
				GUI_TOP_COLOR, GUI_BOTTOM_COLOR);
		line(b, f, y, l, y, color);
	}

	for (i = 0; i < GUI_OUTER_BORDER_WIDTH; i++) {
		GUI_Outline(b, outer_border_colors[i]);
	}
}

static void GUI_DrawEditor(BITMAP *b, Widget *w)
{
	int bc;

	if (w->state != WIDGET_STATE_UNSELECTED) {
		bc = GUI_SELECTED_TEXT_COLOR;
	}
	else {
		bc = GUI_UNSELECTED_COLOR;
	}

	if (w->state == WIDGET_STATE_SELECTED) {
		rect(b, w->x, w->y, w->x+w->w, w->y+w->h, bc);
	}
	rect(b, w->x+1, w->y+1, w->x+w->w-1, w->y+w->h-1, bc);
	rectfill(b, w->x+2, w->y+2, w->x+w->w-2, w->y+w->h-2, GUI_UNSELECTED_COLOR);
	textout_outline(b, font, w->s, w->x+4, w->y+4, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);

	// Draw a cursor
	if ((w->state == WIDGET_STATE_SELECTED || !w->d2) && (currentTimeMillis() % 1000 < 500)) {
		int xx;
		if (text_length_color(font, w->s) == 0) {
			if (w->state == WIDGET_STATE_SELECTED)
				xx = w->x + 3;
			else
				xx = w->x + 2;
		}
		else
			xx = w->x + 4 + text_length_color(font, w->s);
		line(b, xx, w->y+3, xx, w->y+w->h-4, GUI_SELECTED_BORDER_COLOR_MIDDLE);
	}
}

static void GUI_DrawSlider(BITMAP *b, Widget *w)
{
	int line_y = w->y + (slider_tab->h / 2);

	line(b, w->x, line_y, w->x+w->w-1, line_y, makecol(0, 0, 0));
	line(b, w->x, line_y+1, w->x+w->w-1, line_y+1, makecol(0, 0, 0));

	BITMAP *bmp;
	if (w->state == WIDGET_STATE_DEPRESSED) {
		bmp = slider_tab_depressed;
	}
	else if (w->state == WIDGET_STATE_SELECTED) {
		bmp = slider_tab_selected;
	}
	else {
		bmp = slider_tab;
	}

	draw_sprite(b, bmp, w->x+GUI_GetSliderTabX(w), w->y);
}

static void GUI_DrawText(BITMAP *b, Widget *w)
{
	if (w->d1 < 0)
		textout_outline(b, font, w->s, w->x, w->y, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	else
		textout_outline(b, font, w->s, w->x, w->y, w->d1, GUI_TEXT_OUTLINE_COLOR, -1);
}

static void GUI_DrawCheckBox(BITMAP *b, Widget *w)
{
	int c;
	BITMAP *bmp;

	if (w->state == WIDGET_STATE_SELECTED) {
		c = GUI_SELECTED_TEXT_COLOR;
	}
	else if (w->state == WIDGET_STATE_DEPRESSED) {
		c = GUI_DEPRESSED_TEXT_COLOR;
	}
	else {
		c = GUI_TEXT_COLOR;
	}

	if (w->d2) {
		if (w->d1) {
			if (w->state == WIDGET_STATE_SELECTED) {
				bmp = radio_on_selected;
			}
			else if (w->state == WIDGET_STATE_DEPRESSED) {
				bmp = radio_on_depressed;
			}
			else {
				bmp = radio_on;
			}
		}
		else {
			if (w->state == WIDGET_STATE_SELECTED) {
				bmp = radio_off_selected;
			}
			else if (w->state == WIDGET_STATE_DEPRESSED) {
				bmp = radio_off_depressed;
			}
			else {
				bmp = radio_off;
			}
		}
	}
	else {
		if (w->d1) {
			if (w->state == WIDGET_STATE_SELECTED) {
				bmp = check_on_selected;
			}
			else if (w->state == WIDGET_STATE_DEPRESSED) {
				bmp = check_on_depressed;
			}
			else {
				bmp = check_on;
			}
		}
		else {
			if (w->state == WIDGET_STATE_SELECTED) {
				bmp = check_off_selected;
			}
			else if (w->state == WIDGET_STATE_DEPRESSED) {
				bmp = check_off_depressed;
			}
			else {
				bmp = check_off;
			}
		}
	}

	draw_sprite(b, bmp, w->x, w->y);
	if (w->s) {
		textout_outline(b, font, w->s, w->x+GUI_CHECKBOX_WIDTH+GUI_CHECKBOX_SPACING, w->y, c, GUI_TEXT_OUTLINE_COLOR, -1);
	}
}

static void GUI_DrawButton(BITMAP *b, Widget *w)
{
	if (w->state == WIDGET_STATE_UNSELECTED) {
		draw_sprite(b, (BITMAP *)w->p2, w->x, w->y);
	}
	else if (w->state == WIDGET_STATE_SELECTED) {
		draw_sprite(b, (BITMAP *)w->p1, w->x, w->y);
	}
	else if (w->state == WIDGET_STATE_DEPRESSED) {
		draw_sprite(b, (BITMAP *)w->p3, w->x, w->y);
	}
}

static void GUI_DrawTextButton(BITMAP *b, Widget *w)
{
	if (w->state == WIDGET_STATE_SELECTED) {
		textout_outline(b, font, w->s, w->x, w->y, GUI_SELECTED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else if (w->state == WIDGET_STATE_DEPRESSED) {
		textout_outline(b, font, w->s, w->x, w->y, GUI_DEPRESSED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	else {
		textout_outline(b, font, w->s, w->x, w->y, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
}

static void GUI_DrawIcon(BITMAP *b, Widget *w)
{
	if (w->state == WIDGET_STATE_SELECTED) {
		draw_sprite(b, (BITMAP *)w->p2, w->x, w->y);
	}
	else if (w->state == WIDGET_STATE_DEPRESSED) {
		draw_sprite(b, (BITMAP *)w->p3, w->x, w->y);
	}
	else {
		draw_sprite(b, (BITMAP *)w->p1, w->x, w->y);
	}
}

static void GUI_DrawList(BITMAP *b, Widget *w)
{
	BITMAP *bmp = (BITMAP *)w->p2;
	clear_to_color(bmp, GUI_UNSELECTED_COLOR);

	ListData *l = (ListData *)w->p3;
	
	int i, j, x, y;

	x = 3;
	y = 2;

	OptionGetter g = (OptionGetter)w->p1;

	for (i = 0, j = w->d2; i < l->lines+1 && j < l->options; i++, j++) {
		if (j == w->d1) {
			if (w->state == WIDGET_STATE_SELECTED || w->state == WIDGET_STATE_DEPRESSED) {
				rectfill(bmp, x, y, x + w->w - 5, y + text_height(font)-2, GUI_SELECTED_BORDER_COLOR_MIDDLE);
			}
			else {
				rectfill(bmp, x, y, x + w->w - 5, y + text_height(font)-2, GUI_SELECTED_BORDER_COLOR);
			}
			textout_outline(bmp, font, g(j, NULL), x, y, GUI_SELECTED_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
		}
		else {
			textout_outline(bmp, font, g(j, NULL), x, y, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
		}

		y += text_height(font);
	}

	if (w->state != WIDGET_STATE_UNSELECTED) {
		i = GUI_SELECTED_TEXT_COLOR;
	}
	else {
		i = makecol(0, 0, 0);
	}
	rect(bmp, 0, 0, w->w-1, w->h-1, i);
	rect(bmp, 1, 1, w->w-2, w->h-2, i);

	// draw a scrollbar
	if (w->d3) {
		int sb_start = GUI_ListSBStart(w) + 2;
		if (w->d2) {
			sb_start = MAX(sb_start, 1);
		}
		int sb_end = sb_start + l->size;
		if (w->d2 < l->options - l->lines) {
			sb_end = MIN(sb_end, w->h - 4);
		}
		if (sb_end > w->h - 3) {
			sb_end = w->h - 3;
		}
		rectfill(bmp, w->w-GUI_SCROLLBAR_WIDTH-4, 2, w->w-3, w->h-3, GUI_UNSELECTED_COLOR);
		line(bmp, w->w-GUI_SCROLLBAR_WIDTH-3, 2, w->w-GUI_SCROLLBAR_WIDTH-3, w->h-3, i);
		line(bmp, w->w-GUI_SCROLLBAR_WIDTH-4, 2, w->w-GUI_SCROLLBAR_WIDTH-4, w->h-3, i);
		if (w->state == WIDGET_STATE_RIGHT || w->state == WIDGET_STATE_RIGHT_DEPRESSED) {
			rectfill(bmp, w->w-GUI_SCROLLBAR_WIDTH, sb_start+2, w->w-5, sb_end-2, GUI_SELECTED_BORDER_COLOR_MIDDLE);
		}
		else {
			rectfill(bmp, w->w-GUI_SCROLLBAR_WIDTH, sb_start+2, w->w-5, sb_end-2, GUI_SELECTED_BORDER_COLOR);
		}
	}

	blit(bmp, b, 0, 0, w->x, w->y, w->w, w->h);
}

static void GUI_DrawCharSelector(BITMAP *b, Widget *w)
{
	GUI_SelectValidChars((ValidCharType)w->d3);

	char s[3];
	if (w->d2 == num_valid_chars) {
		strcpy(s, "BS");
	}
	else {
		s[0] = valid_chars[w->d2];
		s[1] = 0;
	}

	int c;
	BITMAP *b1, *b2;

	if (w->state == WIDGET_STATE_DEPRESSED) {
		c = GUI_DEPRESSED_TEXT_COLOR;
	}
	else if (w->state == WIDGET_STATE_UNSELECTED) {
		c = GUI_TEXT_COLOR;
	}
	else {
		c = GUI_SELECTED_TEXT_COLOR;
	}

	if (w->state == WIDGET_STATE_LEFT) {
		b1 = arrow_left_selected;
		b2 = arrow_right;
	}
	else if (w->state == WIDGET_STATE_RIGHT) {
		b1 = arrow_left;
		b2 = arrow_right_selected;
	}
	else if (w->state == WIDGET_STATE_LEFT_DEPRESSED) {
		b1 = arrow_left_depressed;
		b2 = arrow_right;
	}
	else if (w->state == WIDGET_STATE_RIGHT_DEPRESSED) {
		b1 = arrow_left;
		b2 = arrow_right_depressed;
	}
	else {
		b1 = arrow_left;
		b2 = arrow_right;
	}

	draw_sprite(b, b1, w->x, w->y);
	textout_outline(b, font, s, w->x+(GUI_CHAR_SELECTOR_WIDTH/2)-(text_length_color(font, s)/2), w->y, c, GUI_TEXT_OUTLINE_COLOR, -1);
	draw_sprite(b, b2, w->x+GUI_CHAR_SELECTOR_WIDTH-GUI_ARROW_WIDTH-3, w->y);
}

static void GUI_ChangeOption(Widget *w, int n)
{
	if (n < 0 || n >= w->d2) {
		return;
	}

	w->d1 = n;
	int new_w = GUI_WidgetWidth(w);

	if (w->align == ALIGN_CENTER) {
		w->x = w->x + (w->w/2) - (new_w/2);
	}
	else if (w->align == ALIGN_RIGHT) {
		w->x = w->x + w->w - new_w;
	}
	w->w = new_w;
}

static void GUI_DrawOption(BITMAP *b, Widget *w)
{
	int c;
	BITMAP *b1, *b2;

	if (w->state == WIDGET_STATE_SELECTED) {
		c = GUI_SELECTED_TEXT_COLOR;
	}
	else if (w->state == WIDGET_STATE_DEPRESSED) {
		c = GUI_DEPRESSED_TEXT_COLOR;
	}
	else {
		c = GUI_TEXT_COLOR;
	}

	if (w->state == WIDGET_STATE_UNSELECTED) {
		c = GUI_TEXT_COLOR;
	}
	else {
		c = GUI_SELECTED_TEXT_COLOR;
	}

	if (w->state == WIDGET_STATE_LEFT) {
		b1 = arrow_left_selected;
		b2 = arrow_right;
	}
	else if (w->state == WIDGET_STATE_RIGHT) {
		b1 = arrow_left;
		b2 = arrow_right_selected;
	}
	else if (w->state == WIDGET_STATE_LEFT_DEPRESSED) {
		b1 = arrow_left_depressed;
		b2 = arrow_right;
	}
	else if (w->state == WIDGET_STATE_RIGHT_DEPRESSED) {
		b1 = arrow_left;
		b2 = arrow_right_depressed;
	}
	else {
		b1 = arrow_left;
		b2 = arrow_right;
	}

	draw_sprite(b, b1, w->x, w->y);
	draw_sprite(b, b2, w->x+w->w-GUI_ARROW_WIDTH, w->y);
	OptionGetter g = (OptionGetter)w->p1;
	textout_outline(b, font, g(w->d1, NULL), w->x + ((w->w/2) - (text_length_color(font, g(w->d1, NULL))/2)), w->y, c, GUI_TEXT_OUTLINE_COLOR, -1);
}

static void GUI_DrawWidget(BITMAP *b, Widget *w)
{
	switch (w->type) {
		case WIDGET_EDITOR:
			GUI_DrawEditor(b, w);
			break;
		case WIDGET_SLIDER:
			GUI_DrawSlider(b, w);
			break;
		case WIDGET_TEXT:
			GUI_DrawText(b, w);
			break;
		case WIDGET_CHECKBOX:
			GUI_DrawCheckBox(b, w);
			break;
		case WIDGET_BUTTON:
			GUI_DrawButton(b, w);
			break;
		case WIDGET_TEXTBUTTON:
			GUI_DrawTextButton(b, w);
			break;
		case WIDGET_ICON:
			GUI_DrawIcon(b, w);
			break;
		case WIDGET_CHAR_SELECTOR:
			GUI_DrawCharSelector(b, w);
			break;
		case WIDGET_OPTION:
			GUI_DrawOption(b, w);
			break;
		case WIDGET_LIST:
			GUI_DrawList(b, w);
			break;
		default:
			break;
	}
}

static void GUI_SetColors(void)
{
	GUI_TOP_COLOR = makecol(configuration.colors[COLOR_TOP].r, configuration.colors[COLOR_TOP].g, configuration.colors[COLOR_TOP].b);
	GUI_BOTTOM_COLOR = makecol(configuration.colors[COLOR_BOTTOM].r, configuration.colors[COLOR_BOTTOM].g, configuration.colors[COLOR_BOTTOM].b);
	GUI_BORDER_COLOR = makecol(configuration.colors[COLOR_BORDER].r, configuration.colors[COLOR_BORDER].g, configuration.colors[COLOR_BORDER].b);
	GUI_BORDER_COLOR_MIDDLE = makecol(configuration.colors[COLOR_BORDER_MIDDLE].r, configuration.colors[COLOR_BORDER_MIDDLE].g, configuration.colors[COLOR_BORDER_MIDDLE].b);
	GUI_SELECTED_BORDER_COLOR = makecol(configuration.colors[COLOR_SELECTED_BORDER].r, configuration.colors[COLOR_SELECTED_BORDER].g, configuration.colors[COLOR_SELECTED_BORDER].b);
	GUI_SELECTED_BORDER_COLOR_MIDDLE = makecol(configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].r, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].g, configuration.colors[COLOR_SELECTED_BORDER_MIDDLE].b);
	GUI_UNSELECTED_COLOR = makecol(configuration.colors[COLOR_UNSELECTED].r, configuration.colors[COLOR_UNSELECTED].g, configuration.colors[COLOR_UNSELECTED].b);
	GUI_TEXT_COLOR = makecol(configuration.colors[COLOR_TEXT].r, configuration.colors[COLOR_TEXT].g, configuration.colors[COLOR_TEXT].b);
	GUI_SELECTED_TEXT_COLOR = makecol(configuration.colors[COLOR_TEXT_SELECTED].r, configuration.colors[COLOR_TEXT_SELECTED].g, configuration.colors[COLOR_TEXT_SELECTED].b);
	GUI_DEPRESSED_TEXT_COLOR = makecol(configuration.colors[COLOR_TEXT_DEPRESSED].r, configuration.colors[COLOR_TEXT_DEPRESSED].g, configuration.colors[COLOR_TEXT_DEPRESSED].b);
	GUI_TEXT_OUTLINE_COLOR = makecol(configuration.colors[COLOR_TEXT_OUTLINE].r, configuration.colors[COLOR_TEXT_OUTLINE].g, configuration.colors[COLOR_TEXT_OUTLINE].b);
	GUI_BASE_COLOR = GUI_GetGradientColor(50, 50, 50);
}

int GUI_Go(int x, int y, int w, int h, Widget *widgets, int selected, void (*callback)(bool)) throw (std::bad_alloc)
{
	if (!GUI_IsSelectable(&widgets[selected])) {
		GUI_WaitMessage("|255000000*Error*\n|255255000GUI_Go:\nCannot select an unselectable widget");
		return -1;
	}

	// Save a copy of the widgets x, y, w, and h since we will modify them
	int num_widgets;
	for (num_widgets = 0; widgets[num_widgets].type != WIDGET_END; num_widgets++)
		; // count
	num_widgets++; // for the WIDGET_END
	Widget *copy = new Widget[num_widgets];
	if (!copy) throw std::bad_alloc();
	GUI_CopyWidgets(copy, widgets);

	float x_ratio = (float)BUFFER_WIDTH / (float)SCREEN_W;
	float y_ratio = (float)BUFFER_HEIGHT / (float)SCREEN_H;

	in_gui = true;

	int i;
	int first_editor = -1;

	for (i = 0; widgets[i].type != WIDGET_END; i++) {
		if (widgets[i].type == WIDGET_BUTTON) {
			if (widgets[i].w < 0) {
				widgets[i].p1 = (void *)GUI_CreateButton(-1, widgets[i].s, WIDGET_STATE_SELECTED);
				widgets[i].p2 = (void *)GUI_CreateButton(-1, widgets[i].s, WIDGET_STATE_UNSELECTED);
				widgets[i].p3 = (void *)GUI_CreateButton(-1, widgets[i].s, WIDGET_STATE_DEPRESSED);
				widgets[i].w = ((BITMAP *)widgets[i].p1)->w;
				widgets[i].h = ((BITMAP *)widgets[i].p1)->h;
			}
			else {
				widgets[i].p1 = (void *)GUI_CreateButton(widgets[i].w, widgets[i].s, WIDGET_STATE_SELECTED);
				widgets[i].p2 = (void *)GUI_CreateButton(widgets[i].w, widgets[i].s, WIDGET_STATE_UNSELECTED);
				widgets[i].p3 = (void *)GUI_CreateButton(widgets[i].w, widgets[i].s, WIDGET_STATE_DEPRESSED);
				widgets[i].h = ((BITMAP *)widgets[i].p1)->h;
			}
		}
		else {
			widgets[i].w = GUI_WidgetWidth(&widgets[i]);
			widgets[i].h = GUI_WidgetHeight(&widgets[i]);
		}
		if (widgets[i].type == WIDGET_EDITOR) {
			if (first_editor < 0) {
				first_editor = i;
			}
		}
		else if (widgets[i].type == WIDGET_LIST) {
			widgets[i].p2 = (void *)create_bitmap(widgets[i].w, widgets[i].h);
			if (!widgets[i].p2) throw std::bad_alloc();
			widgets[i].p3 = (void *)(new ListData);
			if (!widgets[i].p3) throw std::bad_alloc();
			ListData *l = (ListData *)widgets[i].p3;
			OptionGetter g = (OptionGetter)widgets[i].p1;
			g(-1, &l->options);
			l->lines = (widgets[i].h - 4) / text_height(font);
			l->size = MAX((widgets[i].h - 4) - (l->options - l->lines), GUI_SCROLLBAR_MIN_SIZE);
			l->start = GUI_ListSBStart(&widgets[i]);
			l->inc_multiplier = (float)l->options / (float)(widgets[i].h - l->size);
			widgets[selected].d3 = l->options > l->lines;
		}
		else if (widgets[i].type == WIDGET_OPTION) {
			OptionGetter og = (OptionGetter)widgets[i].p1;
			og(-1, &widgets[i].d2);
		}
		if (i == selected) {
			widgets[i].state = WIDGET_STATE_SELECTED;
		}
		else {
			widgets[i].state = WIDGET_STATE_UNSELECTED;
		}
		switch (widgets[i].align) {
			case ALIGN_LEFT:
				break;
			case ALIGN_CENTER:
				widgets[i].x -= (widgets[i].w / 2);
				break;
			case ALIGN_RIGHT:
				widgets[i].x -= widgets[i].w;
				break;
		}
	}

	BITMAP *window = create_bitmap(w, h);
	if (!window) {
		throw std::bad_alloc();
	}
	GUI_DrawWindow(window);
	BITMAP *dialog = create_bitmap(w, h);
	if (!dialog) {
		throw std::bad_alloc();
	}
	BITMAP *bg = create_bitmap(buffer->w, buffer->h);
	if (!bg) {
		throw std::bad_alloc();
	}

	blit(buffer, bg, 0, 0, 0, 0, buffer->w, buffer->h);

	bool b1 = false, key_wait = false, click_was_mouse = false, click_in_range;
	bool sb_paging_up = false, sb_paging_down = false;
	bool arrow_pressed = false, scrollbar_clicked = false;
	bool done = false;
	int k, prev;
	int slider_click = -1, slider_x = -1, bar_click = -1;
	int click_x = -1, click_y = -1, sb_click_y = -1;
	int il, ir, iu, id;
	
	int old_mouse_x = -1;
	int old_mouse_y = -1;

	long t = currentTimeMillis();
	change_wait = GUI_CHANGE_WAIT_RESET;
	int last_draw = currentTimeMillis();
	bool button;

	for (i = 0; widgets[i].type != WIDGET_END; i++) {
		if (widgets[i].type == WIDGET_SLIDER) {
			if (widgets[i].w % widgets[i].d2) {
				GUI_WaitMessage("|255000000*Error*\n|255255000GUI_Go:\nSlider width not a multiple of increment");
				goto end;
			}
		}
	}

	int last_mouse_x;
	int last_mouse_y;

	while (!done) {
		long current_time = currentTimeMillis();
		while (t < current_time) {
			last_mouse_x = mouse_x;
			last_mouse_y = mouse_y;
			if (keyboard_needs_poll())
				poll_keyboard();
			if (mouse_needs_poll())
				poll_mouse();
			poll_joystick();
			if (!LeftPressed() && !RightPressed() && !DownPressed() && !UpPressed() && !mouse_b) {
				change_wait = GUI_CHANGE_WAIT_RESET;
				bar_click = -1;
				scrollbar_clicked = false;
			}
			else {
				change_wait--;
			}
			if (arrow_pressed && !LeftPressed() && !RightPressed()) {
				widgets[selected].state = WIDGET_STATE_SELECTED;
				arrow_pressed = false;
			}
			if (!b1 && (old_mouse_x != last_mouse_x || old_mouse_y != last_mouse_y) && !mouse_b) {
				old_mouse_x = last_mouse_x;
				old_mouse_y = last_mouse_y;
				for (i = 0; widgets[i].type != WIDGET_END; i++) {
					int mx = (int)((float)last_mouse_x * x_ratio);
					int my = (int)((float)last_mouse_y * y_ratio);
					if (mx >= widgets[i].x + x && 
							mx < widgets[i].x + widgets[i].w + x &&
							my >= widgets[i].y + y &&
							my < widgets[i].y + widgets[i].h + y &&
							GUI_IsSelectable(&widgets[i])) {
						widgets[selected].state = WIDGET_STATE_UNSELECTED;
						selected = i;
						if (widgets[i].type == WIDGET_CHAR_SELECTOR || widgets[i].type == WIDGET_OPTION) {
							mx = mx - widgets[selected].x - x;
							if (mx < GUI_ARROW_WIDTH) {
								widgets[selected].state = WIDGET_STATE_LEFT;
							}
							else if (mx >= widgets[selected].w - GUI_ARROW_WIDTH) {
								widgets[selected].state = WIDGET_STATE_RIGHT;
							}
							else {
								widgets[selected].state = WIDGET_STATE_SELECTED;
							}
						}
						else if (widgets[i].type == WIDGET_LIST) {
							mx = mx - widgets[selected].x - x;
							if (widgets[selected].d3 && mx > widgets[selected].w-GUI_SCROLLBAR_WIDTH-2) {
								widgets[i].state = WIDGET_STATE_RIGHT;
							}
							else {
								widgets[i].state = WIDGET_STATE_SELECTED;
							}
						}
						else {
							widgets[i].state = WIDGET_STATE_SELECTED; 
						}
					}
				}
			}
			else {
				if (widgets[selected].type == WIDGET_SLIDER) {
					if (mouse_b) {
						int tx = GUI_GetSliderTabX(&widgets[selected]);
						int mx = (int)(((float)last_mouse_x * x_ratio) - widgets[selected].x - x);
						if (slider_click == 0 && bar_click <= 0 && change_wait <= 0) {
							if (mx < tx) {
								GUI_DecSlider(&widgets[selected]);
								change_wait = GUI_GetChangeWait(widgets[selected].d3);
								bar_click = 0;
							}
						}
						else if (slider_click == 1) {
							int c = mx - slider_x;
							widgets[selected].d1 += c;
							if (widgets[selected].d1 < 0) {
								widgets[selected].d1 = slider_x = 0;
							}
							else if (widgets[selected].d1 > widgets[selected].w) {
								widgets[selected].d1 = slider_x = widgets[selected].w;
							}
							else {
								slider_x = mx;
							}
						}
						else if (change_wait <= 0 && bar_click != 0) {
							if (mx > tx+slider_tab->w) {
								GUI_IncSlider(&widgets[selected]);
								change_wait = GUI_GetChangeWait(widgets[selected].d3);
								bar_click = 2;
							}
						}
					}
				}
				else if (widgets[selected].type == WIDGET_LIST) {
					ListData *l = (ListData *)widgets[selected].p3;
					if (widgets[selected].d3 && click_x > (widgets[selected].w-GUI_SCROLLBAR_WIDTH-2)) {
						int inc = 0;
						if (sb_click_y >= 0) {
							inc = (int)((click_y - sb_click_y) * l->inc_multiplier);
							if (inc) {
								sb_click_y = click_y;
								if (inc < 0) {
									inc = -inc;
									for (i = 0; i < inc; i++) {
										if (widgets[selected].d2) {
											widgets[selected].d2--;
											GUI_ListDec(&widgets[selected]);
										}
									}
								}
								else {
									for (i = 0; i < inc; i++) {
										if (widgets[selected].d2 + l->lines < l->options) {
											widgets[selected].d2++;
											GUI_ListInc(&widgets[selected]);
										}
									}
								}
							}
							if (widgets[selected].d2 < 0) {
								widgets[selected].d2 = 0;
							}
							else if (widgets[selected].d2 > (l->options - l->lines)) {
								widgets[selected].d2 = l->options - l->lines;
							}
						}
					}
					else if (sb_click_y < 0 && click_x >= 0 && mouse_b) {
						if (!widgets[selected].d3 || (click_x < widgets[selected].w - GUI_SCROLLBAR_WIDTH - 4)) {
							widgets[selected].d1 = (int)((float)(click_y - 2) / (float)text_height(font)) + widgets[selected].d2;
							if (widgets[selected].d1 >= l->options) {
								widgets[selected].d1 = l->options - 1;
							}
						}
					}
				}
					
			}
			if (!b1 && LeftPressed())  {
				if (widgets[selected].type == WIDGET_SLIDER && change_wait <= 0) {
					GUI_DecSlider(&widgets[selected]);
					change_wait = GUI_GetChangeWait(widgets[selected].d3);
				}
				else if (widgets[selected].type == WIDGET_CHAR_SELECTOR && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_LEFT_DEPRESSED;
					GUI_DecCharSelector(&widgets[selected]);
					change_wait = GUI_GetChangeWait();
					arrow_pressed = true;
				}
				else if (widgets[selected].type == WIDGET_OPTION && change_wait <= 0) {
					GUI_ChangeOption(&widgets[selected], widgets[selected].d1 - 1);
					widgets[selected].state = WIDGET_STATE_LEFT_DEPRESSED;
					change_wait = GUI_GetChangeWait();
					arrow_pressed = true;
				}
				else if (widgets[selected].type == WIDGET_ICON && widgets[selected].s && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					sscanf(widgets[selected].s, "L%dR%dU%dD%d", &il, &ir, &iu, &id);
					selected = il;
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
				}
				else if (change_wait <= 0) {
					selected = GUI_SelectPrevious(widgets, selected);
					change_wait = GUI_GetChangeWait();
				}
			}
			if (!b1 && RightPressed())  {
				if (widgets[selected].type == WIDGET_SLIDER && change_wait <= 0) {
					GUI_IncSlider(&widgets[selected]);
					change_wait = GUI_GetChangeWait(widgets[selected].d3);
				}
				else if (widgets[selected].type == WIDGET_CHAR_SELECTOR && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_RIGHT_DEPRESSED;
					GUI_IncCharSelector(&widgets[selected]);
					change_wait = GUI_GetChangeWait();
					arrow_pressed = true;
				}
				else if (widgets[selected].type == WIDGET_OPTION && change_wait <= 0) {
					GUI_ChangeOption(&widgets[selected], widgets[selected].d1 + 1);
					widgets[selected].state = WIDGET_STATE_RIGHT_DEPRESSED;
					change_wait = GUI_GetChangeWait();
					arrow_pressed = true;
				}
				else if (widgets[selected].type == WIDGET_ICON && widgets[selected].s && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					sscanf(widgets[selected].s, "L%dR%dU%dD%d", &il, &ir, &iu, &id);
					selected = ir;
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
				}
				else if (change_wait <= 0) {
					selected = GUI_SelectNext(widgets, selected);
					change_wait = GUI_GetChangeWait();
				}
			}
			if (!b1 && DownPressed()) {
				if (widgets[selected].type == WIDGET_ICON && widgets[selected].s && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					sscanf(widgets[selected].s, "L%dR%dU%dD%d", &il, &ir, &iu, &id);
					selected = id;
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
				}
				else if (widgets[selected].type == WIDGET_LIST && change_wait <= 0) {
					GUI_ListInc(&widgets[selected]);
					change_wait = GUI_GetChangeWait();
				}
				else if (change_wait <= 0) {
					selected = GUI_SelectNext(widgets, selected);
					change_wait = GUI_GetChangeWait();
					/*
					prev = selected;
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					do {
						selected++;
					} while (widgets[selected].type != WIDGET_END && !GUI_IsSelectable(&widgets[selected]));
					if (widgets[selected].type == WIDGET_END || !GUI_IsSelectable(&widgets[selected])) {
						selected = prev;
					}
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
					*/
				}
			}
			if (!b1 && UpPressed()) {
				if (widgets[selected].type == WIDGET_ICON && widgets[selected].s && change_wait <= 0) {
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					sscanf(widgets[selected].s, "L%dR%dU%dD%d", &il, &ir, &iu, &id);
					selected = iu;
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
				}
				else if (widgets[selected].type == WIDGET_LIST && change_wait <= 0) {
					GUI_ListDec(&widgets[selected]);
					change_wait = GUI_GetChangeWait();
				}
				else if (selected && change_wait <= 0) {
					selected = GUI_SelectPrevious(widgets, selected);
					change_wait = GUI_GetChangeWait();
					/*
					prev = selected;
					widgets[selected].state = WIDGET_STATE_UNSELECTED;
					do {
						selected--;
					} while (selected && !GUI_IsSelectable(&widgets[selected]));
					if (!GUI_IsSelectable(&widgets[selected])) {
						selected = prev;
					}
					widgets[selected].state = WIDGET_STATE_SELECTED;
					change_wait = GUI_GetChangeWait();
					*/
				}
			}
			if (b1 && !ButtonPressed() && !mouse_b) {
				play_sample(toggle_sample, 255, 0, 1000, 0);
				if (click_was_mouse) {
					int mx = (int)((float)last_mouse_x * x_ratio);
					int my = (int)((float)last_mouse_y * y_ratio);
					if (mx >= widgets[selected].x + x && 
						mx < widgets[selected].x + widgets[selected].w + x &&
						my >= widgets[selected].y + y &&
						my < widgets[selected].y + widgets[selected].h + y) {
						click_in_range = true;
					}
					else {
						click_in_range = false;
					}
				}
				WidgetType wt = widgets[selected].type;
				if (!click_was_mouse || click_in_range) {
					if (wt == WIDGET_BUTTON || wt == WIDGET_TEXTBUTTON || wt == WIDGET_ICON || (!click_was_mouse && wt == WIDGET_LIST)) {
						done = true;
						goto end;
					}
					else if (wt == WIDGET_CHECKBOX) {
						if (widgets[selected].d2 != 0) {
							int g = widgets[selected].d2;
							for (i = 0; widgets[i].type != WIDGET_END; i++) {
								if (widgets[i].type == WIDGET_CHECKBOX) {
									if (widgets[i].d2 == g)  {
										widgets[i].d1 = 0;
									}
								}
							}
							widgets[selected].d1 = 1;
						}
						else {
							widgets[selected].d1 = !widgets[selected].d1;
						}
						widgets[selected].state = WIDGET_STATE_SELECTED;
					}
					else if (wt == WIDGET_CHAR_SELECTOR) {
						if (widgets[selected].state == WIDGET_STATE_DEPRESSED) {
							int e = widgets[selected].d1;
							k = widgets[selected].d2;
							GUI_SelectValidChars((ValidCharType)widgets[selected].d3);
							if (e < 0) {
								widgets[selected].d2 = valid_chars[k];
								goto end;
							}
							if (widgets[e].type != WIDGET_EDITOR) {
								GUI_WaitMessage("|255000000*Error*\n|255255000GUI_Go:\nWidget indexed by CharSelector not an editor");
								selected = -1;
								goto end;
							}
							int len = strlen(widgets[e].s);
							if (k == num_valid_chars) {
								if (widgets[e].s[0]) {
									widgets[e].s[len-1] = 0;
								}
							}
							else {
								if (len < widgets[e].d1) {
									widgets[e].s[len] = valid_chars[k];
									widgets[e].s[len+1] = 0;
								}
							}
						}
						int mx = (int)(((float)last_mouse_x * x_ratio) - widgets[selected].x - x);
						if (mx < GUI_ARROW_WIDTH) {
							widgets[selected].state = WIDGET_STATE_LEFT;
						}
						else if (mx > widgets[selected].w-GUI_ARROW_WIDTH) {
							widgets[selected].state = WIDGET_STATE_RIGHT;
						}
						else {
							widgets[selected].state = WIDGET_STATE_SELECTED;
						}
					}
					else if (widgets[selected].type == WIDGET_OPTION) {
						int mx = (int)(((float)last_mouse_x * x_ratio) - widgets[selected].x - x);
						if (mx < GUI_ARROW_WIDTH) {
							widgets[selected].state = WIDGET_STATE_LEFT;
						}
						else if (mx > widgets[selected].w-GUI_ARROW_WIDTH) {
							widgets[selected].state = WIDGET_STATE_RIGHT;
						}
						else {
							widgets[selected].state = WIDGET_STATE_SELECTED;
						}
					}
					else if (widgets[selected].type == WIDGET_LIST) {
						sb_click_y = -1;
						sb_paging_up = sb_paging_down = false;
						if (widgets[selected].d3 && click_x > widgets[selected].w-GUI_SCROLLBAR_WIDTH-2) {
							widgets[selected].state = WIDGET_STATE_RIGHT;
						}
						else {
							widgets[selected].state = WIDGET_STATE_SELECTED;
						}
					}
					else if (widgets[selected].type == WIDGET_EDITOR) {
						if (widgets[selected].d4 < 0) {
							done = true;
							goto end;
						}
					}
					else {
						widgets[selected].state = WIDGET_STATE_SELECTED;
					}
				}
				click_x = -1;
				click_was_mouse = false;
				slider_click = -1;
				b1 = false;
			}
			else if ((button = ButtonPressed()) || mouse_b) {
				click_was_mouse = !button;
				if (!scrollbar_clicked) {
					click_x = -1;
				}
				click_y = -1;
				int mx = (int)((float)last_mouse_x * x_ratio);
				int my = (int)((float)last_mouse_y * y_ratio);
				if (button || scrollbar_clicked || (mx >= widgets[selected].x + x && 
					mx < widgets[selected].x + widgets[selected].w + x &&
					my >= widgets[selected].y + y &&
					my < widgets[selected].y + widgets[selected].h + y)) {
					if (widgets[selected].type != WIDGET_LIST || !scrollbar_clicked) {
						click_x = mx - widgets[selected].x - x;
					}
					click_y = my - widgets[selected].y - y;
					if (click_was_mouse && (widgets[selected].type == WIDGET_CHAR_SELECTOR || widgets[selected].type == WIDGET_OPTION)) {
						if (click_x < GUI_ARROW_WIDTH) {
							if (widgets[selected].type == WIDGET_CHAR_SELECTOR && change_wait <= 0) {
								GUI_DecCharSelector(&widgets[selected]);
								change_wait = GUI_GetChangeWait();
							}
							else if (change_wait <= 0) {
								GUI_ChangeOption(&widgets[selected], widgets[selected].d1-1);
								change_wait = GUI_GetChangeWait();
							}
							widgets[selected].state = WIDGET_STATE_LEFT_DEPRESSED;
						}
						else if (click_x >= widgets[selected].w - GUI_ARROW_WIDTH) {
							if (widgets[selected].type == WIDGET_CHAR_SELECTOR && change_wait <= 0) {
								GUI_IncCharSelector(&widgets[selected]);
								change_wait = GUI_GetChangeWait();
							}
							else if (change_wait <= 0) {
								GUI_ChangeOption(&widgets[selected], widgets[selected].d1+1);
								change_wait = GUI_GetChangeWait();
							}
							widgets[selected].state = WIDGET_STATE_RIGHT_DEPRESSED;
						}
						else if (widgets[selected].type == WIDGET_CHAR_SELECTOR) {
							widgets[selected].state = WIDGET_STATE_DEPRESSED;
						}
					}
					else if (click_was_mouse && widgets[selected].type == WIDGET_LIST && click_x > widgets[selected].w-GUI_SCROLLBAR_WIDTH-2) {
						ListData *l = (ListData *)widgets[selected].p3;
						int sb_start = GUI_ListSBStart(&widgets[selected]);
						if (!sb_paging_up && !sb_paging_down && sb_click_y < 0 && click_y >= sb_start && click_y < (sb_start+l->size)) {
							scrollbar_clicked = true;
							sb_click_y = click_y;
						}
						else if (!sb_paging_down && sb_click_y < 0 && click_y < sb_start && change_wait <= 0) {
							GUI_ListPGUP(&widgets[selected]);
							change_wait = GUI_GetChangeWait();
							sb_paging_up = true;
						}
						else if (!sb_paging_up && sb_click_y < 0 && click_y > (sb_start+l->size) && change_wait <= 0) {
							GUI_ListPGDN(&widgets[selected]);
							change_wait = GUI_GetChangeWait();
							sb_paging_down = true;
						}
						if (widgets[selected].d3) {
							widgets[selected].state = WIDGET_STATE_RIGHT_DEPRESSED;
						}
						else {
							widgets[selected].state = WIDGET_STATE_DEPRESSED;
						}
					}
					else {
						widgets[selected].state = WIDGET_STATE_DEPRESSED;
					}
					if (widgets[selected].type == WIDGET_SLIDER && slider_click < 0) {
						int tx = GUI_GetSliderTabX(&widgets[selected]);
						if (click_x < tx) {
							slider_click = 0;
						}
						else if (click_x < tx+slider_tab->w) {
							slider_click = 1;
						}
						else {
							slider_click = 2;
						}
						slider_x = click_x;
					}
					b1 = true;
				}
				else {
					widgets[selected].state = WIDGET_STATE_SELECTED;
				}
			}
			else if (key[KEY_ESC]) {
				selected = -1;
				goto end;
			}
			if (key_wait && !keypressed()) {
				key_wait = false;
			}
			else if (!key_wait) {
				k = keypressed() ? readkey() : -1;
				if ((k >> 8) == KEY_ESC) {
					selected = -1;
					goto end;
				}
				else if ((k >> 8) == KEY_TAB) {
				       if (key_shifts & KB_SHIFT_FLAG) {
					       selected = GUI_SelectPrevious(widgets, selected);
				       }
				       else {
					       selected = GUI_SelectNext(widgets, selected);
				       }
				       key_wait = true;
				}
				else if (k >= 0) {
					if (first_editor >= 0 || widgets[selected].type == WIDGET_EDITOR || widgets[selected].type == WIDGET_CHAR_SELECTOR) {
						int e;
						if (widgets[selected].type == WIDGET_CHAR_SELECTOR) {
							e = widgets[selected].d1;
						}
						else if (widgets[selected].type == WIDGET_EDITOR) {
							e = selected;
						}
						else {
							e = first_editor;
						}
						if (e < 0) {
						       	if (GUI_IsValidChar(k & 0xff)) {
								int i;
								for (i = 0; i < num_valid_chars; i++) {
									if (valid_chars[i] == (k & 0xff)) {
										break;
									}
								}
								widgets[selected].d2 = valid_chars[i];
								goto end;
							}
						}
						else {
							GUI_SelectValidChars((ValidCharType)widgets[e].d3);
							int len = strlen(widgets[e].s);
							if ((k >> 8) == KEY_BACKSPACE) {
								if (widgets[e].s[0]) {
									widgets[e].s[len-1] = 0;
								}
							}
							else {
								k &= 0xff;
								if ((ValidCharType)widgets[e].d3 == VALID_UPPERCASE)
									k = toupper(k);
								if (GUI_IsValidChar(k) && len < widgets[e].d1) {
									widgets[e].s[len] = k;
									widgets[e].s[len+1] = 0;
								}
							}
						}
					}
					else if (widgets[selected].type == WIDGET_LIST) {
						ListData *l = (ListData *)widgets[selected].p3;
						if ((k >> 8) == KEY_PGUP) {
							GUI_ListPGUP(&widgets[selected]);
						}
						else if ((k >> 8) == KEY_PGDN) {
							GUI_ListPGDN(&widgets[selected]);
						}
						else if ((k >> 8) == KEY_HOME) {
							widgets[selected].d1 = widgets[selected].d2 = 0;
						}
						else if ((k >> 8) == KEY_END) {
							widgets[selected].d1 = l->options-1;
							widgets[selected].d2 = MAX(l->options - l->lines, 0);
						}
					}
					key_wait = true;
				}
			}
			clear_keybuf();
			if (callback) {
				(*callback)(false);
			}
			t++;
			if ((t+1000) < current_time)
				t = current_time;
		}
		if (done) {
			break;
		}
		if (callback) {
			(*callback)(true);
			blit(buffer, bg, 0, 0, 0, 0, buffer->w, buffer->h);
		}
		clear(buffer);
		blit(bg, buffer, 0, 0, 0, 0, bg->w, bg->h);
		blit(window, dialog, 0, 0, 0, 0, w, h);
		for (i = 0; widgets[i].type != WIDGET_END; i++) {
			GUI_DrawWidget(dialog, &widgets[i]);
		}
		masked_blit(dialog, buffer, 0, 0, x, y, w, h);
		BlitToScreen();
		current_time = currentTimeMillis();
		if ((current_time-last_draw) < 30) {
			rest(30-(current_time-last_draw));
		}
		last_draw = current_time;
	}

end:

	for (i = 0; widgets[i].type != WIDGET_END; i++) {
		if (widgets[i].type == WIDGET_BUTTON) {
			destroy_bitmap((BITMAP *)widgets[i].p1);
			destroy_bitmap((BITMAP *)widgets[i].p2);
			destroy_bitmap((BITMAP *)widgets[i].p3);
		}
		else if (widgets[i].type == WIDGET_LIST) {
			destroy_bitmap((BITMAP *)widgets[i].p2);
			delete (ListData *)widgets[i].p3;
		}
		else if (widgets[i].type == WIDGET_SLIDER) {
			int tx = GUI_GetSliderTabX(&widgets[i]);
			if (tx == 0) {
				widgets[i].d1 = 0;
			}
			else if (tx == widgets[i].w-slider_tab->w) {
				widgets[i].d1 = widgets[i].w;
			}
		}
	}

	blit(bg, buffer, 0, 0, 0, 0, buffer->w, buffer->h);

	destroy_bitmap(window);
	destroy_bitmap(dialog);
	destroy_bitmap(bg);

	in_gui = false;

	GUI_CopyWidgets(widgets, copy);
	delete[] copy;

	WaitForRelease();

	return selected;
}

void GUI_Message(char *s1, char *s2, char *s3) throw (std::bad_alloc)
{
	if (!s1) {
		GUI_WaitMessage("|255000000*Error*\n|255255000GUI_Message:\nMust pass at least 1 string");
		return;
	}

	printf("%s\n", s1);
	if (s2) {
		printf("%s\n", s2);
	}
	if (s3) {
		printf("%s\n", s3);
	}

	if (!configuration.graphics_mode_set) 
		return;

	int w, h;

	w = text_length_color(font, s1);
	if (s2 && text_length_color(font, s2) > w) {
		w = text_length_color(font, s2);
	}
	if (s3 && text_length_color(font, s3) > w) {
		w = text_length_color(font, s3);
	}
	w += GUI_SPACING * 2;

	h = (GUI_SPACING * 2);
	if (s3) {
		h += (text_height(font) * 3);
	}
	else if (s2) {
		h += (text_height(font) * 2);
	}
	else {
		h += text_height(font);
	}

	BITMAP *window = create_bitmap(w, h);
	if (!window) throw std::bad_alloc();
	GUI_DrawWindow(window);
	textout_outline(window, font, s1, (w / 2) - (text_length_color(font, s1) / 2), GUI_SPACING, GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	if (s2) {
		textout_outline(window, font, s2, (w / 2) - (text_length_color(font, s2) / 2), GUI_SPACING + text_height(font), GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}
	if (s3) {
		textout_outline(window, font, s3, (w / 2) - (text_length_color(font, s3) / 2), GUI_SPACING + (text_height(font) * 2), GUI_TEXT_COLOR, GUI_TEXT_OUTLINE_COLOR, -1);
	}

	if (SCREEN_W != BUFFER_WIDTH || SCREEN_H != BUFFER_HEIGHT) {
		float x_ratio = (float)SCREEN_W / (float)BUFFER_WIDTH;
		float y_ratio = (float)SCREEN_H / (float)BUFFER_HEIGHT;
		int newx = (int)((((float)BUFFER_WIDTH / 2.0) - (float)(w / 2)) * x_ratio);
		int newy = (int)((((float)BUFFER_HEIGHT / 2.0) - (float)(h / 2)) * y_ratio);
		int neww = (int)((float)window->w * x_ratio);
		int newh = (int)((float)window->h * y_ratio);
		masked_stretch_blit(window, screen, 0, 0, window->w, window->h,
				newx, newy, neww, newh);
	}
	else {
		masked_blit(window, screen, 0, 0, (SCREEN_W / 2) - (w / 2), (SCREEN_H / 2) - (h / 2), w, h);
	}

	destroy_bitmap(window);
}

void GUI_WaitMessage(char *s, ...) throw (std::bad_alloc)
{
	va_list list;
	int i, bi = 0, line = 0, lines;
	
	char *buf = new char[MAX_STRING+1];
	buf[0] = 0;

	std::vector<char *> all;

	va_start(list, s);

	for (i = 0; s[i]; i++) {
		if (s[i] == '%') {
			i++;
			if (s[i] == 'd') {
				int ii = va_arg(list, int);
				buf[bi] = 0;
				sprintf(buf, "%s%d", buf, ii);
				bi = strlen(buf);
			}
			else if (s[i] == 'g') {
				double d = va_arg(list, double);
				buf[bi] = 0;
				sprintf(buf, "%s%.4g", buf, d);
				bi = strlen(buf);
			}
			else if (s[i] == 's') {
				char *st = va_arg(list, char *);
				buf[bi] = 0;
				sprintf(buf, "%s%s", buf, st);
				bi = strlen(buf);
			}
			else if (s[i] == 'c') {
				char c = (char)va_arg(list, int);
				buf[bi++] = c;
			}
			else if (s[i] == '%') {
				buf[bi++] = s[i];
			}
			else {
				GUI_WaitMessage("|255000000*Error*\n|255255000GUI_WaitMessage:\nInvalid argument type '%c'", s[i]);
				return;
			}
		}
		else if (s[i] == '\n') {
			buf[bi] = 0;
			all.push_back(buf);
			buf = new char[MAX_STRING+1];
			if (!buf) throw std::bad_alloc();
			bi = 0;
			line++;
		}
		else {
			buf[bi++] = s[i];
		}
	}
	buf[bi] = 0;
	all.push_back(buf);

	va_end(list);

	lines = all.size();

	int width = 0;
	int height;

	if (!configuration.graphics_mode_set) {
		char stripped[MAX_STRING+1];
		char full[MAX_STRING+1];
		full[0] = 0;
		for (i = 0; i < lines; i++) {
			StripColorCodes(stripped, all[i]);
			strcat(full, stripped);
			strcat(full, "\n");
		}
		allegro_message(full);
		return;
	}

	for (i = 0; i < lines; i++) {
		if (text_length_color(font, all[i]) > width) {
			width = text_length_color(font, all[i]);
		}
	}

	width += GUI_SPACING * 2;
	
	height = (lines * text_height(font)) + (GUI_SPACING * 3) + GUI_BUTTON_HEIGHT;

	int num_widgets = lines + 2;

	Widget *w = new Widget[num_widgets];
	if (!w) throw std::bad_alloc();

	for (i = 0; i < lines; i++) {
		w[i].type = WIDGET_TEXT;
		w[i].align = ALIGN_CENTER;
		w[i].d1 = -1;
		w[i].x = width / 2;
		w[i].y = GUI_SPACING + (i * text_height(font));
		w[i].s = all[i];
	}

	w[i].type = WIDGET_BUTTON;
	w[i].align = ALIGN_CENTER;
	w[i].x = width/2;
	w[i].y = (lines * text_height(font)) + (GUI_SPACING * 2);
	w[i].w = -1;
	w[i].s = "OK";
	w[i].d1 = w[i].d2 = w[i].d3 = w[i].d4 = -1;

	i++;
	w[i].type = WIDGET_END;

	GUI_Go((BUFFER_WIDTH/2)-(width/2), (BUFFER_HEIGHT/2)-(height/2), width, height, w, lines, NULL);

	delete w;
	
	for (i = 0; i < lines; i++) {
		delete all[i];
	}
	all.clear();
}

bool GUI_Prompt(char *message, char *b1, char *b2) throw (std::bad_alloc)
{
	Widget widgets[4];
	
	int w = text_length_color(font, message);
	int bw = GUI_GetMinButtonWidth(b1) + GUI_GetMinButtonWidth(b2) + GUI_SPACING;
	if (bw > w) {
		w = bw;
	}
	w += GUI_SPACING * 2;
	int h = (GUI_SPACING * 3) + GUI_BUTTON_HEIGHT + text_height(font);

	widgets[0].type = WIDGET_TEXT;
	widgets[0].align = ALIGN_CENTER;
	widgets[0].d1 = -1;
	widgets[0].x = w / 2;
	widgets[0].y = GUI_SPACING;
	widgets[0].s = message;
	widgets[1].type = widgets[2].type = WIDGET_BUTTON;
	widgets[1].align = widgets[2].align = ALIGN_LEFT;
	widgets[1].d1 = widgets[2].d1 = -1;
	widgets[1].d2 = widgets[2].d2 = -1;
	widgets[1].d3 = widgets[2].d3 = -1;
	widgets[1].d4 = widgets[2].d4 = -1;
	widgets[1].x = (w/2) - (bw/2);
	widgets[1].y = widgets[2].y = (GUI_SPACING * 2) + text_height(font);
	widgets[1].w = widgets[2].w = -1;
	widgets[1].s = b1;
	widgets[2].x = widgets[1].x + GUI_GetMinButtonWidth(b1) + GUI_SPACING;
	widgets[2].s = b2;
	widgets[3].type = WIDGET_END;

	int ret = GUI_Go((BUFFER_WIDTH/2)-(w/2), (BUFFER_HEIGHT/2)-(h/2), w, h, widgets, 1, NULL);

	return ret == 1 ? true : false;
}

void GUI_Initialize(void) throw (std::bad_alloc)
{
	GUI_SetColors();	

	if (outer_border_colors) {
		delete outer_border_colors;
	}
	if (inner_border_colors) {
		delete inner_border_colors;
	}

	outer_border_colors = new int[GUI_OUTER_BORDER_WIDTH];
	inner_border_colors = new int[GUI_INNER_BORDER_WIDTH];

	int i;
	float p;

	if (GUI_OUTER_BORDER_WIDTH == 2) {
		outer_border_colors[0] = outer_border_colors[1] = GUI_BORDER_COLOR_MIDDLE;
	}
	else {
		float d = ((float)GUI_OUTER_BORDER_WIDTH/2.0) - 1.0;
		for (i = 0; i < GUI_OUTER_BORDER_WIDTH/2; i++) {
			p = 100.0 - ((float)i / d * 100.0);
			outer_border_colors[i] = GUI_GetGradientColor(p, GUI_BORDER_COLOR, GUI_BORDER_COLOR_MIDDLE);
			outer_border_colors[GUI_OUTER_BORDER_WIDTH-i-1] = GUI_GetGradientColor(p, GUI_BORDER_COLOR, GUI_BORDER_COLOR_MIDDLE);
		}
		if (GUI_OUTER_BORDER_WIDTH % 2) {
			outer_border_colors[GUI_OUTER_BORDER_WIDTH/2] = GUI_BORDER_COLOR_MIDDLE;
		}
	}

	if (GUI_INNER_BORDER_WIDTH == 2) {
		inner_border_colors[0] = inner_border_colors[1] = GUI_SELECTED_BORDER_COLOR_MIDDLE;
	}
	else {
		float d = ((float)GUI_INNER_BORDER_WIDTH/2.0) - 1.0;
		for (i = 0; i < GUI_INNER_BORDER_WIDTH/2; i++) {
			p = 100.0 - ((float)i / d * 100.0);
			inner_border_colors[i] = GUI_GetGradientColor(p, GUI_SELECTED_BORDER_COLOR, GUI_SELECTED_BORDER_COLOR_MIDDLE);
			inner_border_colors[GUI_INNER_BORDER_WIDTH-i-1] = GUI_GetGradientColor(p, GUI_SELECTED_BORDER_COLOR, GUI_SELECTED_BORDER_COLOR_MIDDLE);
		}
		if (GUI_INNER_BORDER_WIDTH % 2) {
			inner_border_colors[GUI_INNER_BORDER_WIDTH/2] = GUI_SELECTED_BORDER_COLOR_MIDDLE;
		}
	}

	SetCharWidth();
}

