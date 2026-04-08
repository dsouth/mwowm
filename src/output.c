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

float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float focused_color[] = {0.15f, 0.15f, 0.75f, 1.0f};
float input_color[] = {0.05f, 0.85f, 0.05f, 1.0f};

void indent_debug_print(int indent) {
  for (int i = 0; i < indent; i++) {
    printf("  ");
  }
}

void print_node(struct wlr_scene_node *, int);

void print_node_tree(struct wlr_scene_tree *tree, int indent) {
  struct wlr_scene_node *n;
  wl_list_for_each(n, &tree->children, link) { print_node(n, indent + 1); }
}

void print_tree(struct wlr_scene_node *node, int indent) {
  struct wlr_scene_tree *tree = wlr_scene_tree_from_node(node);
  print_node_tree(tree, indent);
}

void print_node(struct wlr_scene_node *node, int indent) {
  indent_debug_print(indent);
  printf("node: type - ");
  switch (node->type) {
  case WLR_SCENE_NODE_TREE:
    printf("tree");
    break;
  case WLR_SCENE_NODE_RECT:
    printf("rectangle");
    break;
  case WLR_SCENE_NODE_BUFFER:
    printf("buffer");
    break;
  default:
    printf("<unknown>");
  }
  printf(", enabled: [%d] x: [%d], y: [%d]\n", node->enabled, node->x, node->y);
  if (node->type == WLR_SCENE_NODE_TREE) {
    print_tree(node, indent);
  } else if (node->type == WLR_SCENE_NODE_RECT) {
    indent_debug_print(indent);
    struct wlr_scene_rect *rect = wlr_scene_rect_from_node(node);
    printf("width: [%d], height: [%d]\n", rect->width, rect->height);
  } else if (node->type == WLR_SCENE_NODE_BUFFER) {
    struct wlr_scene_buffer *buffer = wlr_scene_buffer_from_node(node);
    if (buffer != NULL) {
      struct wlr_buffer *b = buffer->buffer;
      if (b != NULL) {
        indent_debug_print(indent);
        printf("width: [%d], height: [%d]\n", b->width, b->height);
      }
    }
  }
}

void print_scene_tree(struct wlr_scene *scene) {
  if (!wl_list_empty(&scene->tree.children)) {
    printf("\n--- start ---\n");
    print_node_tree(&scene->tree, 1);
    printf("---- end ----\n\n");
  }
}

void output_frame(struct wl_listener *listener, void *data) {
  //  wlr_log(WLR_DEBUG, "output frame");
  struct output *output = wl_container_of(listener, output, frame_listener);
  struct wlr_scene *scene = output->wm->scene;
  print_scene_tree(scene);
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

void output_new(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "output new called");
  struct window_manager *wm =
      wl_container_of(listener, wm, new_output_listener);
  wlr_log(WLR_DEBUG, "1 - new output's root tree's children pointer %p",
          (void *)&wm->scene->tree.children);
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
  wlr_log(WLR_DEBUG, "output created pointer is %p", (void *)output);
  output->wlr_output = wlr_output;
  output->wm = wm;
  add_signal_listener(&wlr_output->events.destroy, &output->destroy_listener,
                      output_destroy);
  add_signal_listener(&wlr_output->events.frame, &output->frame_listener,
                      output_frame);
  add_signal_listener(&wlr_output->events.request_state,
                      &output->request_state_listener, output_request_state);

  wl_list_insert(&wm->outputs, &output->link);

  output->background = wlr_scene_tree_create(&wm->scene->tree);

  struct wlr_scene_rect *rect = wlr_scene_rect_create(output->background, wlr_output->width,
                        wlr_output->height, (float[4]){1.0f, 0.0f, 0.0f, 1.0f});
  if (x == 0)
    wlr_scene_node_lower_to_bottom(&rect->node);
  struct wlr_scene_node *background = &output->background->node;
  background->x = 2* x;
  background->y = y;

  struct wlr_output_layout_output *layout_output =
      wlr_output_layout_add(wm->output_layout, wlr_output, x, y);
  wlr_log(WLR_DEBUG, "1 - new output root tree's pointer %p",
          (void *)&(wm->scene->tree));
  struct wlr_scene_output *scene_output =
      wlr_scene_output_create(wm->scene, wlr_output);
  wlr_log(WLR_DEBUG, "2 - new output root tree's pointer %p",
          (void *)&(wm->scene->tree));
  wlr_scene_output_layout_add_output(wm->scene_layout, layout_output,
                                     scene_output);

  wlr_log(WLR_DEBUG, "2 - new output's root tree's children pointer %p",
          (void *)&wm->scene->tree.children);
  // add background rect???
  // struct wlr_scene_tree tree = scene_output->scene->tree;
}
