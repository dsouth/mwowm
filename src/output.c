#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "utils.h"

float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};

void output_frame(struct wl_listener *listener, void *data) {
	wlr_log(WLR_DEBUG, "output frame called");
	struct output *output = wl_container_of(listener, output, frame_listener);
	struct wlr_output *wlr_output = output->wlr_output;

	struct wlr_output_state output_state;
	wlr_output_state_init(&output_state);
	struct wlr_render_pass *pass = wlr_output_begin_render_pass(wlr_output, &output_state, NULL);
	wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options){
		.box = { .width = wlr_output->width,
			 .height = wlr_output->height },
		.color = {
			background_color[0],
			background_color[1],
			background_color[2],
			background_color[3],
		},
	});
	wlr_output_add_software_cursors_to_render_pass(wlr_output, pass, NULL);
	wlr_render_pass_submit(pass);
	wlr_output_commit_state(wlr_output, &output_state);
	wlr_output_state_finish(&output_state);
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
	add_signal_listener(&wlr_output->events.destroy, &output->destroy_listener, output_destroy);
	add_signal_listener(&wlr_output->events.frame, &output->frame_listener, output_frame);
	add_signal_listener(&wlr_output->events.request_state, &output->request_state_listener, output_request_state);

	wl_list_insert(&wm->outputs, &output->link);

//	struct wlr_output_layout_output *layer_output;
	wlr_output_layout_add(wm->output_layout, wlr_output, x, y);
}

