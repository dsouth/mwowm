#ifndef MWOWM_INPUT_HEADER
#define MWOWM_INPUT_HEADER

#include <wayland-server-core.h>

void new_input(struct wl_listener *listener, void *data);
void request_set_cursor(struct wl_listener *listener, void *data);
void focus_change(struct wl_listener *listener, void *data);
void request_set_selection(struct wl_listener *listener, void *data);

#endif
