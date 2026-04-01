#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "utils.h"
#include "xdg-shell-protocol.h"

void xdg_fullscreen(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg fullscreen called");
}

void xdg_set_parent(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg set parent called");
}

void xdg_set_title(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg set title called");
}

void xdg_set_app_id(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg set app id wlr surface client called");
}

void xdg_toplevel_destroy(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg toplevel destroy called");
  struct xdg_toplevel *toplevel =
      wl_container_of(listener, toplevel, destroy_listener);
  wl_list_remove(&toplevel->maximize_listener.link);
  wl_list_remove(&toplevel->fullscreen_listener.link);
  wl_list_remove(&toplevel->minimize_listener.link);
  wl_list_remove(&toplevel->move_listener.link);
  wl_list_remove(&toplevel->resize_listener.link);
  wl_list_remove(&toplevel->show_window_menu_listener.link);
  wl_list_remove(&toplevel->set_parent_listener.link);
  wl_list_remove(&toplevel->set_title_listener.link);
  wl_list_remove(&toplevel->set_app_id_listener.link);
  wl_list_remove(&toplevel->destroy_listener.link);
  wl_list_remove(&toplevel->link);
  free(toplevel);
}

void xdg_maximize(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg maximize called");
}

void xdg_minimize(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg minimize called");
}

void xdg_move(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg move called");
}

void xdg_resize(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg resize called");
}

void xdg_show_window_menu(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg show window menu called");
}

void handle_wlr_surface_client_commit(struct wl_listener *listener,
                                      void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface client commit called");
}

void handle_wlr_surface_commit(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface commit called");
  struct surface_xdg *surface_xdg =
      wl_container_of(listener, surface_xdg, wlr_surface_commit_listener);
  struct wlr_xdg_surface *surface = surface_xdg->surface;
  //
  //
  //  IS THEN DONE ONCE OR EVERYTIME?!?!?
  //
  //
  //    struct wlr_xdg_surface *surface = toplevel->wlr_xdg_toplevel->base;
  struct wlr_xdg_surface_configure *config = calloc(1, sizeof(*config));
  switch (surface->role) {
  case WLR_XDG_SURFACE_ROLE_TOPLEVEL:
    wlr_log(WLR_DEBUG, "setting config on toplevel surface");
    config->toplevel_configure =
        calloc(1, sizeof(struct wlr_xdg_toplevel_configure *));
    config->toplevel_configure->fields = WLR_XDG_TOPLEVEL_CONFIGURE_BOUNDS;
    config->toplevel_configure = &(struct wlr_xdg_toplevel_configure){
        .bounds = {.width = 400, .height = 200}};
    wlr_xdg_surface_schedule_configure(surface);
    break;
  default:
    break;
  }

  wlr_log(WLR_DEBUG, "xdg surface initialized is %d", surface->initialized);
}

void handle_wlr_surface_map(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface map called");
}

void handle_wlr_surface_unmap(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface unmap called");
}

void handle_wlr_surface_destroy(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "handle wlr surface destory called");
  struct surface_xdg *surface =
      wl_container_of(listener, surface, wlr_surface_destroy_listener);
  if (surface->surface->surface == data) {
    wl_list_remove(&surface->wlr_surface_client_commit_listener.link);
    wl_list_remove(&surface->wlr_surface_commit_listener.link);
    wl_list_remove(&surface->wlr_surface_map_listener.link);
    wl_list_remove(&surface->wlr_surface_unmap_listener.link);
    wl_list_remove(&surface->wlr_surface_destroy_listener.link);
  }
}

void new_xdg_toplevel(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "new xdg toplevel called");
  struct window_manager *wm =
      wl_container_of(listener, wm, new_xdg_toplevel_listener);
  struct wlr_xdg_toplevel *wlr_xdg_toplevel = data;
  struct xdg_toplevel *xdg_toplevel = calloc(1, sizeof(*xdg_toplevel));
  xdg_toplevel->wlr_xdg_toplevel = wlr_xdg_toplevel;
  xdg_toplevel->wm = wm;
  wl_list_insert(&wm->toplevels, &xdg_toplevel->link);
  add_signal_listener(&wlr_xdg_toplevel->events.request_maximize,
                      &xdg_toplevel->maximize_listener, xdg_maximize);
  add_signal_listener(&wlr_xdg_toplevel->events.request_fullscreen,
                      &xdg_toplevel->fullscreen_listener, xdg_fullscreen);
  add_signal_listener(&wlr_xdg_toplevel->events.request_minimize,
                      &xdg_toplevel->minimize_listener, xdg_minimize);
  add_signal_listener(&wlr_xdg_toplevel->events.request_move,
                      &xdg_toplevel->move_listener, xdg_move);
  add_signal_listener(&wlr_xdg_toplevel->events.request_resize,
                      &xdg_toplevel->resize_listener, xdg_resize);
  add_signal_listener(&wlr_xdg_toplevel->events.request_show_window_menu,
                      &xdg_toplevel->show_window_menu_listener,
                      xdg_show_window_menu);
  add_signal_listener(&wlr_xdg_toplevel->events.set_parent,
                      &xdg_toplevel->set_parent_listener, xdg_set_parent);
  add_signal_listener(&wlr_xdg_toplevel->events.set_title,
                      &xdg_toplevel->set_title_listener, xdg_set_title);
  add_signal_listener(&wlr_xdg_toplevel->events.set_app_id,
                      &xdg_toplevel->set_app_id_listener, xdg_set_app_id);
  add_signal_listener(&wlr_xdg_toplevel->events.destroy,
                      &xdg_toplevel->destroy_listener, xdg_toplevel_destroy);
}

void new_xdg_popup(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "new xdg popup called");
}

void new_xdg_surface(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "new xdg surface called");
  struct wlr_xdg_surface *surface = data;
  struct window_manager *wm =
      wl_container_of(listener, wm, new_xdg_surface_listener);
  struct surface_xdg *surface_container = calloc(1, sizeof(*surface_container));
  surface_container->surface = surface;
  struct wlr_surface *child = surface->surface;
  surface_container->child = child;
  surface_container->wm = wm;
  wl_list_insert(&wm->surfaces_xdg, &surface_container->link);
  add_signal_listener(&child->events.client_commit,
                      &surface_container->wlr_surface_client_commit_listener,
                      handle_wlr_surface_client_commit);
  add_signal_listener(&child->events.commit,
                      &surface_container->wlr_surface_commit_listener,
                      handle_wlr_surface_commit);
  add_signal_listener(&child->events.map,
                      &surface_container->wlr_surface_map_listener,
                      handle_wlr_surface_map);
  add_signal_listener(&child->events.unmap,
                      &surface_container->wlr_surface_unmap_listener,
                      handle_wlr_surface_unmap);
  add_signal_listener(&child->events.destroy,
                      &surface_container->wlr_surface_destroy_listener,
                      handle_wlr_surface_destroy);
}

void xdg_destroy(struct wl_listener *listener, void *data) {
  wlr_log(WLR_DEBUG, "xdg destroy called");
  // TODO free struct memory here!!!
}
