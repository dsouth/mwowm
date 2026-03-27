#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "output.h"
#include "xdg-shell.h"
#include "cursor.h"

void add_signal_listener(struct wl_signal *sig,
			 struct wl_listener *listener,
			 void (*ptr)(struct wl_listener*, void*)) {
	listener->notify = ptr;
	wl_signal_add(sig, listener);
}

int main(int argc, char* argv[]) {
	wlr_log_init(WLR_DEBUG, NULL);
	struct window_manager wm = {0};
	wm.display = wl_display_create();
	wm.backend = wlr_backend_autocreate(wl_display_get_event_loop(wm.display), NULL);
	if (wm.backend == NULL) {
		wlr_log(WLR_ERROR, "failed to autocreate backend");
	}
	wm.renderer = wlr_renderer_autocreate(wm.backend);
	if (wm.renderer == NULL) {
		wlr_log(WLR_ERROR, "faild to autocreate renderer");
	}
	wlr_renderer_init_wl_display(wm.renderer, wm.display);
	wm.allocator = wlr_allocator_autocreate(wm.backend, wm.renderer);
	if (wm.allocator == NULL) {
		wlr_log(WLR_ERROR, "failed to autocreate allocatoer");
	}
	wlr_compositor_create(wm.display, 5, wm.renderer);
	wlr_subcompositor_create(wm.display);
	wlr_data_device_manager_create(wm.display);
	wm.output_layout = wlr_output_layout_create(wm.display);
	wl_list_init(&wm.output.link);
	add_signal_listener(&wm.backend->events.new_output, &wm.new_output_listener, output_new);
	wm.xdg_shell = wlr_xdg_shell_create(wm.display, 3);
	add_signal_listener(&wm.xdg_shell->events.new_toplevel, &wm.new_xdg_toplevel_listener, new_xdg_toplevel);
	add_signal_listener(&wm.xdg_shell->events.new_popup, &wm.new_xdg_popup_listener, new_xdg_popup);
	add_signal_listener(&wm.xdg_shell->events.new_surface, &wm.new_xdg_surface_listener, new_xdg_surface);
	add_signal_listener(&wm.xdg_shell->events.destroy, &wm.xdg_destroy_listener, xdg_destroy);
	wm.cursor = wlr_cursor_create();
	wlr_cursor_attach_output_layout(wm.cursor, wm.output_layout);
	wm.cursor_manager = wlr_xcursor_manager_create(NULL, 24);
	add_signal_listener(&wm.cursor->events.motion, &wm.cursor_motion_listener, cursor_motion);
	add_signal_listener(&wm.cursor->events.motion_absolute, &wm.cursor_motion_absolute_listener, cursor_motion_absolute);
	add_signal_listener(&wm.cursor->events.button, &wm.cursor_button_listener, cursor_button);
	add_signal_listener(&wm.cursor->events.axis, &wm.cursor_axis_listener, cursor_axis);
	add_signal_listener(&wm.cursor->events.frame, &wm.cursor_frame_listener, cursor_frame);
}
