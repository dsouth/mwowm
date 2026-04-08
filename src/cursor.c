#include <linux/input-event-codes.h>
#include <stdint.h>
#include <strings.h>
#include <wayland-server-core.h>
#include <wayland-server-protocol.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "inspect.h"

void center_cursor_on_output(struct wlr_cursor *cursor,
                             struct wlr_output_layout_output *layout) {
  int32_t x, y;
  double width, height;
  x = layout->x;
  y = layout->y;
  struct wlr_output *output = layout->output;
  width = output->width;
  height = output->height;
  wlr_cursor_warp(cursor, NULL, x + width / 2, y + height / 2);
}

struct wlr_output_layout_output *
get_primary_output_layout(struct wlr_output_layout *layouts) {
  // assuming 0, 0 is the primary monitor, is this a good assumption?
  struct wlr_output_layout_output *layout = NULL;
  wl_list_for_each(layout, &layouts->outputs, link) {
    if (layout->x == 0 && layout->y == 0) {
      break;
    }
  }
  // if we don't find 0, 0 then we're just returning the last configured
  // output... :/
  return layout;
}

struct output *get_primary_output(struct window_manager *wm) {
  struct wlr_output_layout_output *layout =
      get_primary_output_layout(wm->output_layout);
  struct wlr_output *wlr_output = layout->output;
  struct output *output;
  wl_list_for_each(output, &wm->outputs, link) {
    if (output->wlr_output == wlr_output) {
      break;
    }
  }
  wlr_log(WLR_DEBUG, "primary output is %s", output->wlr_output->name);
  return output;
}

void center_cursor_on_primary_monitor(struct wlr_cursor *cursor,
                                      struct wlr_output_layout *layouts) {
  center_cursor_on_output(cursor, get_primary_output_layout(layouts));
}

void cursor_motion(struct wl_listener *listener, void *data) {
  struct window_manager *wm =
      wl_container_of(listener, wm, cursor_motion_listener);
  struct wlr_pointer_motion_event *event = data;
  wlr_cursor_move(wm->cursor, &event->pointer->base, event->delta_x,
                  event->delta_y);
  wlr_cursor_set_xcursor(wm->cursor, wm->cursor_manager, "default");
}

void cursor_motion_absolute(struct wl_listener *listener, void *data) {}

void cursor_button(struct wl_listener *listener, void *data) {
  struct wlr_pointer_button_event *event = data;
  if (event->state == WL_POINTER_BUTTON_STATE_PRESSED &&
      event->button == BTN_LEFT) {
    struct window_manager *wm =
        wl_container_of(listener, wm, cursor_button_listener);
    print_scene_tree(wm->scene);
  }
}

void cursor_axis(struct wl_listener *listener, void *data) {}

void cursor_frame(struct wl_listener *listener, void *data) {
  struct window_manager *wm =
      wl_container_of(listener, wm, cursor_frame_listener);
  wlr_seat_pointer_notify_frame(wm->seat);
}
