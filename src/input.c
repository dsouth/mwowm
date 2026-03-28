#include "mwowm.h"
#include <stdint.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "utils.h"

void key_press(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "key press called");
	struct window_manager *wm = wl_container_of(listener, wm, key_listener);
	struct wlr_keyboard_key_event *event = data;

	uint32_t keycode = event->keycode + 8;
	const xkb_keysym_t *syms;
	int sym_size = xkb_state_key_get_syms(wm->wlr_keyboard->xkb_state, keycode, &syms);
	if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		for (int i = 0; i < sym_size; i++) {
			if (syms[i] == XKB_KEY_Escape) {
				wl_display_terminate(wm->display);
			}
		}
	}
}

void modifier_press(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "modifier press called");
}

void keyboard_destroy(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "keyboard destroy called");

	struct window_manager *wm = wl_container_of(listener, wm, keyboard_destroy_listener);
	wl_list_remove(&wm->key_listener.link);
	wl_list_remove(&wm->modifier_listener.link);
	wl_list_remove(&wm->keyboard_destroy_listener.link);
}

void new_keyboard(struct window_manager *wm, struct wlr_input_device *device) {
	wm->wlr_keyboard = wlr_keyboard_from_input_device(device);
	struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	struct xkb_keymap* keymap = xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
	if (!wlr_keyboard_set_keymap(wm->wlr_keyboard, keymap)) {
		wlr_log(WLR_ERROR, "faild to set keymap");
	}
	xkb_keymap_unref(keymap);
	xkb_context_unref(context);
	int32_t rate_hz = 25;
	int32_t delay_ms = 600;
	wlr_keyboard_set_repeat_info(wm->wlr_keyboard, rate_hz, delay_ms);
	add_signal_listener(&wm->wlr_keyboard->events.key, &wm->key_listener, key_press);
	add_signal_listener(&wm->wlr_keyboard->events.modifiers, &wm->modifier_listener, modifier_press);
	add_signal_listener(&device->events.destroy, &wm->keyboard_destroy_listener, keyboard_destroy);

	wlr_seat_set_keyboard(wm->seat, wm->wlr_keyboard);
}

void new_input(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "new input called");
	struct wlr_input_device *device = data;
	struct window_manager *wm = wl_container_of(listener, wm, new_input_listener);

	// assuming JUST keyboards for the time being
	// also assuming we only ever just have the ONE keyboard :/
	if (device->type == WLR_INPUT_DEVICE_KEYBOARD) {
		new_keyboard(wm, device);
	}

	uint32_t capabilities = WL_SEAT_CAPABILITY_POINTER;
	if (wm->wlr_keyboard != NULL) {
		capabilities |= WL_SEAT_CAPABILITY_KEYBOARD;
	}
	wlr_seat_set_capabilities(wm->seat, capabilities);
}

void request_set_cursor(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "request set cursor called");
}

void focus_change(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "focus change called");
}

void request_set_selection(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "request set selection called");
}

