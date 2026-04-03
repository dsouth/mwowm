#ifndef WMOWM_CURSOR_HEADER
#define WMOWM_CURSOR_HEADER

#include "mwowm.h"
#include <wayland-server-core.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output_layout.h>

void cursor_motion(struct wl_listener *, void *);
void cursor_motion_absolute(struct wl_listener *, void *);
void cursor_button(struct wl_listener *, void *);
void cursor_axis(struct wl_listener *, void *);
void cursor_frame(struct wl_listener *, void *);

void center_cursor_on_output(struct wlr_cursor *,
                             struct wlr_output_layout_output *);
void center_cursor_on_primary_monitor(struct wlr_cursor *,
                                      struct wlr_output_layout *);
struct output *get_primary_output(struct window_manager *);

#endif
