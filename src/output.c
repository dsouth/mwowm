#include <wayland-server-core.h>
#include <wlr/util/log.h>

void output_frame(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output frame called");
}

void output_request_state(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output request state called");
}

void output_destroy(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output destroy called");
}

void output_new(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output new called");
}

