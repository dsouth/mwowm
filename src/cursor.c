#include "mwowm.h"
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/util/log.h>

void cursor_motion(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor motion called");
	struct window_manager *wm = wl_container_of(listener, wm, cursor_motion_listener);
	struct wlr_pointer_motion_event *event = data;

	wlr_cursor_move(wm->cursor, &event->pointer->base, event->delta_x, event->delta_y);
	wlr_log(WLR_DEBUG, "%lf, %lf event", event->delta_x, event->delta_y);
	wlr_log(WLR_DEBUG, "%lf, %lf cursor", wm->cursor->x, wm->cursor->y);

	wlr_cursor_set_xcursor(wm->cursor, wm->cursor_manager, "default");
}

void cursor_motion_absolute(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor motion absolute called");
}

void cursor_button(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor button called");
}

void cursor_axis(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor axis called");
}

void cursor_frame(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor frame called");
	struct window_manager *wm = wl_container_of(listener, wm, cursor_frame_listener);
	wlr_seat_pointer_notify_frame(wm->seat);
}
