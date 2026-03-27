#ifndef WMOWM_XDG_SHELL_HEADER
#define WMOWM_XDG_SHELL_HEADER

#include <wayland-server-core.h>

void new_xdg_toplevel(struct wl_listener *listener, void *data);
void new_xdg_popup(struct wl_listener *listener, void *data);
void new_xdg_surface(struct wl_listener *listener, void *data);
void xdg_destroy(struct wl_listener *listener, void *data);
	
#endif
