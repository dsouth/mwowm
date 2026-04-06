#ifndef MWOWM_MAIN_HEADER
#define MWOWM_MAIN_HEADER

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wayland-util.h>

struct xdg_toplevel {
  struct wl_list link;
  struct window_manager *wm;
  struct wlr_xdg_toplevel *wlr_xdg_toplevel;
  struct wlr_scene_tree *scene_tree;

  struct wl_listener maximize_listener;
  struct wl_listener fullscreen_listener;
  struct wl_listener move_listener;
  struct wl_listener resize_listener;
  struct wl_listener destroy_listener;
  
  struct wl_listener wlr_surface_commit_listener;
  struct wl_listener wlr_surface_map_listener;
  struct wl_listener wlr_surface_unmap_listener;
};

struct output {
  struct wl_list link;
  struct window_manager *wm;
  struct wlr_output *wlr_output;

  bool focused;

  struct wl_listener frame_listener;
  struct wl_listener request_state_listener;
  struct wl_listener destroy_listener;
};

struct surface_xdg {
  struct wl_list link;
  struct wlr_xdg_surface *surface;
  struct wlr_surface *child;
  struct window_manager *wm;

};

struct keyboard {
  struct wl_list link;
  struct window_manager *wm;
  struct wlr_keyboard *wlr_keyboard;
  struct wl_listener modifier_listener;
  struct wl_listener key_listener;
  struct wl_listener keyboard_destroy_listener;
};

struct window_manager {
  struct wl_display *display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;
  struct wlr_output_layout *output_layout;
  struct wlr_compositor *compositor;
  struct wlr_scene *scene;
  struct wlr_scene_output_layout *scene_layout;

  bool input_mode;

  struct wl_list outputs;
  struct wl_listener new_output_listener;

  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_toplevel_listener;
  struct wl_listener new_xdg_popup_listener;
  struct wl_list toplevels;
  struct wl_list surfaces_xdg;

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_manager;
  struct wl_listener cursor_motion_listener;
  struct wl_listener cursor_motion_absolute_listener;
  struct wl_listener cursor_button_listener;
  struct wl_listener cursor_axis_listener;
  struct wl_listener cursor_frame_listener;

  struct wl_list keyboards;
  struct wl_listener new_input_listener;

  struct wlr_seat *seat;
  struct wl_listener request_set_cursor_listener;
  struct wl_listener focus_change_listener;
  struct wl_listener request_set_selection_listener;
};

#endif
