#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "utils.h"

void output_frame(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output frame called");
}

void output_request_state(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output request state called");
}

void output_destroy(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output destroy called");
	struct output *output = wl_container_of(listener, output, destroy_listener);
	wl_list_remove(&output->frame_listener.link);
	wl_list_remove(&output->request_state_listener.link);
	wl_list_remove(&output->destroy_listener.link);
	free(output);
}

void output_new(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output new called");
	struct window_manager *wm = wl_container_of(listener, wm, new_output_listener);
	struct wlr_output *wlr_output = data;
	wlr_output_init_render(wlr_output, wm->allocator, wm->renderer);
	struct wlr_output_state state;
	wlr_output_state_init(&state);
	wlr_output_state_set_enabled(&state, true);

	// set up multi monitors here since my is always wrong ;)
	int x, y;
	x = y = 0;

	if (strstr(wlr_output->name, "HDMI-A-1")) {
		x = 1920;
	}

	struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
	if (mode != NULL) {
		wlr_output_state_set_mode(&state, mode);
	}
	wlr_output_commit_state(wlr_output, &state);
	wlr_output_state_finish(&state);

	struct output *output = calloc(1, sizeof(*output));
	output->wlr_output = wlr_output;
	output->wm = wm;
	add_signal_listener(&wlr_output->events.description, &output->destroy_listener, output_destroy);
	add_signal_listener(&wlr_output->events.frame, &output->frame_listener, output_frame);
	add_signal_listener(&wlr_output->events.request_state, &output->request_state_listener, output_request_state);

	wl_list_insert(&wm->outputs, &output->link);

//	struct wlr_output_layout_output *layer_output;
	wlr_output_layout_add(wm->output_layout, wlr_output, x, y);
}

