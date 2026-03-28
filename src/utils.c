#include <wayland-server-core.h>

void add_signal_listener(struct wl_signal *sig,
			 struct wl_listener *listener,
			 void (*ptr)(struct wl_listener*, void*)) {
	listener->notify = ptr;
	wl_signal_add(sig, listener);
}


