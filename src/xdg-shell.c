#include <wayland-server-core.h>
#include <wlr/util/log.h>

void new_xdg_toplevel(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "new xdg toplevel called");
}

void new_xdg_popup(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "new xdg popup called");
}

void new_xdg_surface(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "new xdg surface called");
}

void xdg_destroy(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "xdg destroy called");
}

