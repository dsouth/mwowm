#include <bits/time.h>
#include <stdlib.h>
#include <time.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "stdbool.h"
#include "utils.h"

float background_color[] = {0.0f, 0.0f, 0.0f, 1.0f};
float focused_color[] = {0.15f, 0.15f, 0.75f, 1.0f};
float modal_color[] = {0.05f, 0.85f, 0.05f, 1.0f};

void output_frame(struct wl_listener *listener, void *data) {
  struct output *output = wl_container_of(listener, output, frame_listener);
  struct window_manager *wm = output->wm;
  // assumes background will only have the one child
  struct wlr_scene_node *node;
  node = wl_container_of(output->background->children.next, node, link);
  wlr_log(WLR_DEBUG, "node type is %d", node->type);
  struct wlr_scene_rect *rect = wlr_scene_rect_from_node(node);
  float *color;
  if (output->focused) {
    color = focused_color;
  } else if (!wm->input_mode) {
    color = modal_color;
  } else {
    color = background_color;
  }
  wlr_scene_rect_set_color(rect, color);
  struct wlr_scene *scene = output->wm->scene;
  struct wlr_scene_output *scene_output =
      wlr_scene_get_scene_output(scene, output->wlr_output);
  wlr_scene_output_commit(scene_output, NULL);
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  wlr_scene_output_send_frame_done(scene_output, &now);
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

void output_setup_data(struct output *output, struct wlr_output *wlr_output, struct window_manager *wm) {
  output->wlr_output = wlr_output;
  output->wm = wm;
  wlr_output->data = output;
  add_signal_listener(&wlr_output->events.destroy, &output->destroy_listener,
                      output_destroy);
  add_signal_listener(&wlr_output->events.frame, &output->frame_listener,
                      output_frame);
  add_signal_listener(&wlr_output->events.request_state,
                      &output->request_state_listener, output_request_state);
  wl_list_insert(&wm->outputs, &output->link);

}

void output_intialize_monitor(struct window_manager *wm, struct wlr_output *wlr_output) {
  struct wlr_output_state state;
  wlr_output_init_render(wlr_output, wm->allocator, wm->renderer);
  wlr_output_state_init(&state);
  wlr_output_state_set_enabled(&state, true);
  struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
  if (mode != NULL) {
    wlr_output_state_set_mode(&state, mode);
  }
  wlr_output_commit_state(wlr_output, &state);
  wlr_output_state_finish(&state);
}

void output_intialize_scene(struct output *output, struct window_manager *wm, struct wlr_output *wlr_output) {
  // set up multi monitors here since mine is always wrong ;)
  int x, y;
  x = y = 0;
  if (strstr(wlr_output->name, "HDMI-A-1")) {
    x = 1920;
  }
  int width, height;
  wlr_output_effective_resolution(output->wlr_output, &width, &height);
  wlr_scene_rect_create(output->background, width, height,
                        (float[4]){0.0f, 0.0f, 0.0f, 1.0f});
  wlr_scene_node_set_position(&output->background->node, x, y);
  struct wlr_output_layout_output *layout_output =
      wlr_output_layout_add(wm->output_layout, wlr_output, x, y);
  struct wlr_scene_output *scene_output =
      wlr_scene_output_create(wm->scene, wlr_output);
  wlr_scene_output_layout_add_output(wm->scene_layout, layout_output,
                                     scene_output);

}

void output_new(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "output new called");
  struct window_manager *wm =
      wl_container_of(listener, wm, new_output_listener);
  struct wlr_output *wlr_output = data;
  output_intialize_monitor(wm, wlr_output);
  struct output *output = calloc(1, sizeof(*output));
  output_setup_data(output, wlr_output, wm);
  output->background = wlr_scene_tree_create(&wm->scene->tree);
  output_intialize_scene(output, wm, wlr_output);
}
