#ifndef WMOWM_XDG_SHELL_HEADER
#define WMOWM_XDG_SHELL_HEADER

#include <wayland-server-core.h>
#include <wlr/types/wlr_compositor.h>

#include "mwowm.h"

void new_xdg_toplevel(struct wl_listener *listener, void *data);
void new_xdg_popup(struct wl_listener *listener, void *data);
void new_xdg_surface(struct wl_listener *listener, void *data);
void xdg_destroy(struct wl_listener *listener, void *data);
void toplevel_focus(struct xdg_toplevel *);
struct xdg_toplevel *toplevel_at(struct window_manager *, double, double,
                                 struct wlr_surface **, double *, double *);
struct xdg_toplevel *toplevel_at_cursor(struct window_manager *,
                                                struct wlr_cursor *,
                                                struct wlr_surface **, double *,
                                                double *);

#endif
