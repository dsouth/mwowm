#ifndef MWOWM_OUTPUT_HEADER
#define MWOWM_OUTPUT_HEADER

#include <wayland-server-core.h>
#include <wlr/types/wlr_output_layout.h>

#include "mwowm.h"

void output_new(struct wl_listener *, void *);
struct output *output_get_focused(struct window_manager *);
void output_update_focus(struct window_manager *, struct output *);
void output_move_focus(struct window_manager *, enum wlr_direction);
struct output *output_get_primary(struct window_manager *wm);
struct wlr_output_layout_output *
output_get_primary_output_layout(struct wlr_output_layout *);

#endif
