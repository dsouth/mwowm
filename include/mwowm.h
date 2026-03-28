#ifndef MWOWM_MAIN_HEADER
#define MWOWM_MAIN_HEADER

#include <wayland-server-core.h>

struct output {
	struct wl_list link;
	struct window_manager *wm;
	struct wlr_output *wlr_output;

	struct wl_listener frame_listener;
	struct wl_listener request_state_listener;
	struct wl_listener destroy_listener;
};

struct window_manager {
	struct wl_display *display;
	struct wlr_backend *backend;
	struct wlr_renderer *renderer;
	struct wlr_allocator *allocator;
	struct wlr_output_layout *output_layout;

	struct wl_list outputs;
	struct wl_listener new_output_listener;

	struct wlr_xdg_shell *xdg_shell;
	struct wl_listener new_xdg_toplevel_listener;
	struct wl_listener new_xdg_popup_listener;
	struct wl_listener new_xdg_surface_listener;
	struct wl_listener xdg_destroy_listener;

	struct wlr_cursor *cursor;	
	struct wlr_xcursor_manager *cursor_manager;
	struct wl_listener cursor_motion_listener;
	struct wl_listener cursor_motion_absolute_listener;
	struct wl_listener cursor_button_listener;
	struct wl_listener cursor_axis_listener;
	struct wl_listener cursor_frame_listener;

	struct wl_listener new_input_listener;

	struct wlr_keyboard *wlr_keyboard;
	struct wl_listener modifier_listener;
	struct wl_listener key_listener;
	struct wl_listener keyboard_destroy_listener;

	struct wlr_seat *seat;
	struct wl_listener request_set_cursor_listener;
	struct wl_listener focus_change_listener;
	struct wl_listener request_set_selection_listener;
};

#endif
