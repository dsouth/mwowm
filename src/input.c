#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/backend/session.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon-keysyms.h>
#include <xkbcommon/xkbcommon.h>

#include "inspect.h"
#include "mwowm.h"
#include "output.h"
#include "utils.h"

void spawn_command(const char *cmd) {
  if (fork() == 0) {
    setsid();
    execl("/bin/sh", "/bin/sh", "-c", cmd, (void *)NULL);
  }
}

void output_move_focus(struct window_manager *wm, enum wlr_direction dir) {
  struct output *output = output_get_focused(wm);
  struct wlr_scene_node node = output->background->node;
  if ((dir == WLR_DIRECTION_LEFT && node.x > 0) ||
      (dir == WLR_DIRECTION_RIGHT && node.x == 0)) {
    int width, height;
    wlr_output_effective_resolution(output->wlr_output, &width, &height);
    struct output *focus_output = wlr_output_layout_adjacent_output(
        wm->output_layout, dir, output->wlr_output, (node.x + width) / 2.0,
        (node.y + height) / 2.0)->data;
    output_update_focus(wm, focus_output);
  }
}

void key_press_modal(struct wlr_keyboard_key_event *event, int sym_size,
                     const xkb_keysym_t *syms, struct keyboard *keyboard) {
  if (event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    for (int i = 0; i < sym_size; i++) {
      switch (syms[i]) {
      case XKB_KEY_Escape:
        wl_display_terminate(keyboard->wm->display);
        break;
      case XKB_KEY_Return:
        spawn_command("foot");
        break;
      case XKB_KEY_H:
        output_move_focus(keyboard->wm, WLR_DIRECTION_LEFT);
        break;
      case XKB_KEY_L:
        output_move_focus(keyboard->wm, WLR_DIRECTION_RIGHT);
        break;
      }
    }
  }
}

bool keyboard_vt_switch(struct window_manager *wm, const xkb_keysym_t *syms,
                        size_t sym_size) {
  for (size_t i = 0; i < sym_size; i++) {
    xkb_keysym_t keysym = syms[i];
    if (keysym >= XKB_KEY_XF86Switch_VT_1 &&
        keysym <= XKB_KEY_XF86Switch_VT_12) {
      if (wm->session) {
        unsigned vt = keysym - XKB_KEY_XF86Switch_VT_1 + 1;
        wlr_session_change_vt(wm->session, vt);
      }
      return true;
    }
  }
  return false;
}

void key_press(struct wl_listener *listener, void *data) {
  struct keyboard *keyboard = wl_container_of(listener, keyboard, key_listener);
  struct window_manager *wm = keyboard->wm;
  struct wlr_keyboard_key_event *event = data;

  uint32_t keycode = event->keycode + 8;
  const xkb_keysym_t *syms;
  int sym_size =
      xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);
  if (!keyboard_vt_switch(wm, syms, sym_size)) {
    if (!wm->input_mode) {
      key_press_modal(event, sym_size, syms, keyboard);
    } else if (!keyboard_vt_switch(wm, syms, sym_size)) {
      struct wlr_seat *seat = wm->seat;
      wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
      wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode,
                                   event->state);
    }
  }
}

void modifier_press(struct wl_listener *listener, void *data) {
  struct keyboard *keyboard =
      wl_container_of(listener, keyboard, modifier_listener);
  struct window_manager *wm = keyboard->wm;
  struct wlr_keyboard *wlr_keyboard = data;

  uint32_t mod = wlr_keyboard_get_modifiers(wlr_keyboard);
  // assumes that only the logo key is every pressed and not a good thing?
  if (mod == WLR_MODIFIER_LOGO) {
    wm->input_mode = !wm->input_mode;
    struct output *output;
    wl_list_for_each(output, &wm->outputs, link) {
      wlr_output_schedule_frame(output->wlr_output);
    }
  } else {
    wlr_seat_set_keyboard(wm->seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_modifiers(wm->seat,
                                       &keyboard->wlr_keyboard->modifiers);
  }
}

void keyboard_destroy(struct wl_listener *listener, void *data) {
  struct keyboard *keyboard =
      wl_container_of(listener, keyboard, keyboard_destroy_listener);
  wl_list_remove(&keyboard->key_listener.link);
  wl_list_remove(&keyboard->modifier_listener.link);
  wl_list_remove(&keyboard->keyboard_destroy_listener.link);
  wl_list_remove(&keyboard->link);

  free(keyboard);
}

void new_keyboard(struct window_manager *wm, struct wlr_input_device *device) {
  struct keyboard *keyboard = calloc(1, sizeof(*keyboard));
  keyboard->wlr_keyboard = wlr_keyboard_from_input_device(device);
  keyboard->wm = wm;
  wl_list_insert(&wm->keyboards, &keyboard->link);

  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (!wlr_keyboard_set_keymap(keyboard->wlr_keyboard, keymap)) {
    wlr_log(WLR_ERROR, "faild to set keymap");
  }
  xkb_keymap_unref(keymap);
  xkb_context_unref(context);
  int32_t rate_hz = 25;
  int32_t delay_ms = 600;
  wlr_keyboard_set_repeat_info(keyboard->wlr_keyboard, rate_hz, delay_ms);
  add_signal_listener(&keyboard->wlr_keyboard->events.key,
                      &keyboard->key_listener, key_press);
  add_signal_listener(&keyboard->wlr_keyboard->events.modifiers,
                      &keyboard->modifier_listener, modifier_press);
  add_signal_listener(&device->events.destroy,
                      &keyboard->keyboard_destroy_listener, keyboard_destroy);

  wlr_seat_set_keyboard(wm->seat, keyboard->wlr_keyboard);
}

void new_pointer(struct window_manager *wm, struct wlr_input_device *device) {
  // todo look at what we can do with libinput or something else?
  wlr_cursor_attach_input_device(wm->cursor, device);
}

void new_input(struct wl_listener *listener, void *data) {
  struct wlr_input_device *device = data;
  struct window_manager *wm = wl_container_of(listener, wm, new_input_listener);

  // assuming JUST keyboards for the time being
  switch (device->type) {
  case WLR_INPUT_DEVICE_KEYBOARD:
    new_keyboard(wm, device);
    break;
  case WLR_INPUT_DEVICE_POINTER:
    new_pointer(wm, device);
    break;
  default:
    break;
  }

  uint32_t capabilities = WL_SEAT_CAPABILITY_POINTER;
  if (!wl_list_empty(&wm->keyboards)) {
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
