#ifndef MWOWM_OUTPUT_HEADER
#define MWOWM_OUTPUT_HEADER

#include "mwowm.h"
#include <wayland-server-core.h>

void output_new(struct wl_listener*, void*);
struct output* output_get_focused(struct window_manager*);

#endif
