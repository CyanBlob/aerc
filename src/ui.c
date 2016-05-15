#include <termbox.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include "render.h"
#include "state.h"
#include "ui.h"

void init_ui() {
	tb_init();
	tb_select_input_mode(TB_INPUT_ESC | TB_INPUT_MOUSE);
}

void teardown_ui() {
	tb_shutdown();
}

int tb_printf(int x, int y, struct tb_cell *basis, const char *fmt, ...) {
	va_list args;
	va_start(args, fmt);
	int len = vsnprintf(NULL, 0, fmt, args);
	va_end(args);

	char *buf = malloc(len + 1);
	va_start(args, fmt);
	vsnprintf(buf, len + 1, fmt, args);
	va_end(args);

	int _x = x, _y = y;
	char *b = buf;
	while (*b) {
		b += tb_utf8_char_to_unicode(&basis->ch, b);
		switch (basis->ch) {
		case '\n':
			_x = x;
			_y++;
			break;
		case '\r':
			_x = x;
			break;
		default:
			tb_put_cell(_x, _y, basis);
			_x++;
			break;
		}
	}

	free(buf);
	return len;
}

void rerender() {
	tb_clear();
	int width = tb_width(), height = tb_height();
	int folder_width = 20;

	render_account_bar(0, 0, width, folder_width);
	render_folder_list(0, 1, folder_width, height);
	render_status(folder_width, height - 1, width - folder_width);
	render_items(folder_width + 1, 1, height - 2, width - folder_width - 1);

	tb_present();
}

bool ui_tick() {
	struct tb_event event;
	switch (tb_peek_event(&event, 0)) {
	case TB_EVENT_RESIZE:
		rerender();
		break;
	case TB_EVENT_KEY:
		if (event.key == TB_KEY_ESC
				|| event.ch == 'q'
				|| event.key == TB_KEY_CTRL_C) {
			return false;
		}
		if (event.key == TB_KEY_CTRL_L) {
			rerender();
		}
		if (event.key == TB_KEY_ARROW_RIGHT) {
			state->selected_account++;
			state->selected_account %= state->accounts->length;
			rerender();
		}
		if (event.key == TB_KEY_ARROW_LEFT) {
			state->selected_account--;
			state->selected_account %= state->accounts->length;
			rerender();
		}
		// TODO: Handle other keys
		break;
	case -1:
		return false;
	}
	return true;
}