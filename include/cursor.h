#ifndef WMOWM_CURSOR_HEADER
#define WMOWM_CURSOR_HEADER

#include <wayland-server-core.h>

void cursor_motion(struct wl_listener *listener, void *data);
void cursor_motion_absolute(struct wl_listener *listener, void *data);
void cursor_button(struct wl_listener *listener, void *data);
void cursor_axis(struct wl_listener *listener, void *data);
void cursor_frame(struct wl_listener *listener, void *data);

#endif
