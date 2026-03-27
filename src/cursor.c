#include <wayland-server-core.h>
#include <wlr/util/log.h>

void cursor_motion(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "cursor motion called");
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
}
