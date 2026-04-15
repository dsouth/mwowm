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

#include "inspect.h"
#include "mwowm.h"
#include "output.h"
#include "xdg-shell.h"

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

void cursor_center_on_primary_monitor(struct wlr_cursor *cursor,
                                      struct wlr_output_layout *layouts) {
  center_cursor_on_output(cursor, output_get_primary_output_layout(layouts));
}

void cursor_process_motion(struct window_manager *wm, uint32_t time) {
  double tx, ty;
  struct wlr_seat *seat = wm->seat;
  struct wlr_surface *surface = NULL;
  struct xdg_toplevel *toplevel =
      toplevel_at_cursor(wm, wm->cursor, &surface, &tx, &ty);
  if (!toplevel) {
    wlr_cursor_set_xcursor(wm->cursor, wm->cursor_manager, "default");
  }
  if (surface) {
    wlr_seat_pointer_notify_enter(seat, surface, tx, ty);
    wlr_seat_pointer_notify_motion(seat, time, tx, ty);
  } else {
    wlr_seat_pointer_clear_focus(seat);
  }
  struct wlr_cursor *cursor = wm->cursor;
  struct wlr_output_layout_output *layout_output;
  wl_list_for_each(layout_output, &wm->output_layout->outputs, link) {
    struct output *output = layout_output->output->data;
    if (cursor->x >= layout_output->x &&
        cursor->x < layout_output->x + layout_output->output->width &&
        cursor->y >= layout_output->y &&
        cursor->y < layout_output->y + layout_output->output->height) {
      output->focused = true;
    } else {
      output->focused = false;
    }
  }
}

void cursor_motion(struct wl_listener *listener, void *data) {
  struct window_manager *wm =
      wl_container_of(listener, wm, cursor_motion_listener);
  struct wlr_pointer_motion_event *event = data;
  wlr_cursor_move(wm->cursor, &event->pointer->base, event->delta_x,
                  event->delta_y);
  cursor_process_motion(wm, event->time_msec);
}

void cursor_motion_absolute(struct wl_listener *listener, void *data) {
  struct window_manager *wm =
      wl_container_of(listener, wm, cursor_motion_absolute_listener);
  struct wlr_pointer_motion_absolute_event *event = data;
  wlr_cursor_warp_absolute(wm->cursor, &event->pointer->base, event->x,
                           event->y);
  cursor_process_motion(wm, event->time_msec);
}

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
