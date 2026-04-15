#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "output.h"
#include "utils.h"

struct xdg_toplevel *toplevel_at(struct window_manager *wm, double x, double y,
                                 struct wlr_surface **surface, double *px,
                                 double *py) {
  struct wlr_scene_node *node =
      wlr_scene_node_at(&wm->scene->tree.node, x, y, px, py);
  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }
  struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_try_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }
  // TODO why is this happening???
  *surface = scene_surface->surface;
  struct wlr_scene_tree *tree = node->parent;
  // only the root of the toplevel node will have data set
  // to the toplevel struct
  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }
  if (tree == NULL) {
    return NULL;
  }
  return tree->node.data;
}

struct xdg_toplevel *toplevel_at_cursor(struct window_manager *wm,
                                        struct wlr_cursor *cursor,
                                        struct wlr_surface **surface, double *x,
                                        double *y) {
  return toplevel_at(wm, cursor->x, cursor->y, surface, x, y);
}

void toplevel_focus(struct xdg_toplevel *toplevel) {
  if (toplevel == NULL) {
    return;
  }
  struct window_manager *wm = toplevel->wm;
  struct wlr_seat *seat = wm->seat;
  struct wlr_surface *previous_surface = seat->keyboard_state.focused_surface;
  struct wlr_surface *surface = toplevel->wlr_xdg_toplevel->base->surface;
  if (previous_surface == surface) {
    // already focused
    return;
  }
  if (previous_surface) {
    struct wlr_xdg_toplevel *previous_toplevel =
        wlr_xdg_toplevel_try_from_wlr_surface(previous_surface);
    if (previous_toplevel) {
      wlr_xdg_toplevel_set_activated(previous_toplevel, false);
    }
  }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  wlr_scene_node_raise_to_top(&toplevel->scene_tree->node);
  // move toplevel to the "front" of the list
  wl_list_remove(&toplevel->link);
  wl_list_insert(&wm->toplevels, &toplevel->link);
  wlr_xdg_toplevel_set_activated(toplevel->wlr_xdg_toplevel, true);
  if (keyboard) {
    wlr_seat_keyboard_notify_enter(seat, surface, keyboard->keycodes,
                                   keyboard->num_keycodes,
                                   &keyboard->modifiers);
  }
}

void xdg_fullscreen(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg fullscreen called");
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg toplevel destroy called");
  struct xdg_toplevel *toplevel =
      wl_container_of(listener, toplevel, destroy_listener);
  wl_list_remove(&toplevel->wlr_surface_map_listener.link);
  wl_list_remove(&toplevel->wlr_surface_unmap_listener.link);
  wl_list_remove(&toplevel->wlr_surface_commit_listener.link);
  wl_list_remove(&toplevel->maximize_listener.link);
  wl_list_remove(&toplevel->fullscreen_listener.link);
  wl_list_remove(&toplevel->move_listener.link);
  wl_list_remove(&toplevel->resize_listener.link);
  wl_list_remove(&toplevel->destroy_listener.link);

  wl_list_remove(&toplevel->link);
  free(toplevel);
}

void xdg_maximize(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg maximize called");
}

void xdg_move(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg move called");
}

void xdg_resize(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg resize called");
}

void handle_xdg_toplevel_commit(struct wl_listener *listener, void *data) {
//  wlr_log(WLR_DEBUG, "handle wlr surface commit called");
  struct xdg_toplevel *toplevel =
      wl_container_of(listener, toplevel, wlr_surface_commit_listener);
  if (toplevel->wlr_xdg_toplevel->base->initial_commit) {
    struct window_manager *wm = toplevel->wm;
    struct output *output = output_get_focused(wm);
    int width, height;
    wlr_output_effective_resolution(output->wlr_output, &width, &height);
    wlr_xdg_toplevel_set_size(toplevel->wlr_xdg_toplevel, width - 2 * MARGIN,
                              height - 2 * MARGIN);
  }
}

void handle_xdg_toplevel_map(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface map called");
  struct xdg_toplevel *toplevel =
      wl_container_of(listener, toplevel, wlr_surface_map_listener);
  //  wl_list_insert(&toplevel->wm->toplevels, &toplevel->link);
  toplevel_focus(toplevel);
}

void handle_xdg_toplevel_unmap(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface unmap called");
  struct xdg_toplevel *toplevel =
      wl_container_of(listener, toplevel, wlr_surface_unmap_listener);
  //  wl_list_remove(&toplevel->link);
}

void xdg_toplevel_initialize_data() {}

void new_xdg_toplevel(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "new xdg toplevel called");
  struct window_manager *wm =
      wl_container_of(listener, wm, new_xdg_toplevel_listener);
  struct wlr_xdg_toplevel *wlr_xdg_toplevel = data;
  struct xdg_toplevel *xdg_toplevel = calloc(1, sizeof(*xdg_toplevel));
  xdg_toplevel->wlr_xdg_toplevel = wlr_xdg_toplevel;
  xdg_toplevel->wm = wm;
  xdg_toplevel->scene_tree =
      wlr_scene_xdg_surface_create(&wm->scene->tree, wlr_xdg_toplevel->base);
  xdg_toplevel->scene_tree->node.data = xdg_toplevel;
  wlr_xdg_toplevel->base->data = xdg_toplevel->scene_tree;
  add_signal_listener(&wlr_xdg_toplevel->base->surface->events.map,
                      &xdg_toplevel->wlr_surface_map_listener,
                      handle_xdg_toplevel_map);
  add_signal_listener(&wlr_xdg_toplevel->base->surface->events.unmap,
                      &xdg_toplevel->wlr_surface_unmap_listener,
                      handle_xdg_toplevel_unmap);
  add_signal_listener(&wlr_xdg_toplevel->base->surface->events.commit,
                      &xdg_toplevel->wlr_surface_commit_listener,
                      handle_xdg_toplevel_commit);
  add_signal_listener(&wlr_xdg_toplevel->events.request_maximize,
                      &xdg_toplevel->maximize_listener, xdg_maximize);
  add_signal_listener(&wlr_xdg_toplevel->events.request_fullscreen,
                      &xdg_toplevel->fullscreen_listener, xdg_fullscreen);
  add_signal_listener(&wlr_xdg_toplevel->events.request_move,
                      &xdg_toplevel->move_listener, xdg_move);
  add_signal_listener(&wlr_xdg_toplevel->events.request_resize,
                      &xdg_toplevel->resize_listener, xdg_resize);
  add_signal_listener(&wlr_xdg_toplevel->events.destroy,
                      &xdg_toplevel->destroy_listener, xdg_toplevel_destroy);
  wl_list_insert(&xdg_toplevel->wm->toplevels, &xdg_toplevel->link);

  struct output *output = output_get_focused(wm);
  struct wlr_scene_tree *background = output->background;
  struct wlr_scene_node node = background->node;
  int x = node.x + MARGIN;
  int y = node.y + MARGIN;
  wlr_scene_node_set_position(&xdg_toplevel->scene_tree->node, x, y);
}

void new_xdg_popup(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "new xdg popup called");
}
