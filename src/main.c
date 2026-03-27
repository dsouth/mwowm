#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/util/log.h>

#include "mwowm.h"

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
}
