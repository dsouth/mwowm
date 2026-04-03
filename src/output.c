#include <stdlib.h>
#include <wayland-server-core.h>
#include <wayland-util.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/box.h>
#include <wlr/util/log.h>

#include "mwowm.h"
#include "utils.h"

float background_color[4] = {0.0f, 0.0f, 0.0f, 1.0f};
float focused_color[] = {0.15f, 0.15f, 0.75f, 1.0f};
float input_color[] = {0.05f, 0.85f, 0.05f, 1.0f};

void output_frame(struct wl_listener *listener, void *data) {
  // wlr_log(WLR_DEBUG, "output frame");
  struct output *output = wl_container_of(listener, output, frame_listener);
  struct wlr_output *wlr_output = output->wlr_output;
  struct window_manager *wm = output->wm;
  struct wlr_output_state output_state;
  wlr_output_state_init(&output_state);
  struct wlr_render_pass *pass =
      wlr_output_begin_render_pass(wlr_output, &output_state, NULL);
  float *color = wm->input_mode && output->focused ? input_color
                 : wm->input_mode                  ? background_color
                                                   : focused_color;
  wlr_render_pass_add_rect(pass, &(struct wlr_render_rect_options){
                                     .box = {.width = wlr_output->width,
                                             .height = wlr_output->height},
                                     .color =
                                         {
                                             color[0],
                                             color[1],
                                             color[2],
                                             color[3],
                                         },
                                 });

  struct wl_list *toplevels = &wm->toplevels;
  // toplevels = wl_container_of(wm, toplevels, prev);
  if (!wl_list_empty(toplevels)) {
    struct xdg_toplevel *toplevel;
    toplevel = wl_container_of(toplevels, toplevel, link);
    wl_list_for_each(toplevel, toplevels, link) {
      struct wlr_surface *wlr_surface =
          toplevel->wlr_xdg_toplevel->base->surface;
      if (wlr_surface_has_buffer(wlr_surface)) {
        struct wlr_box render_box = {.x = 200,
                                     .y = 100,
                                     .width = wlr_surface->current.width,
                                     .height = wlr_surface->current.height};
        struct wlr_texture *toplevel_texture =
            wlr_surface_get_texture(wlr_surface);
        const struct wlr_render_texture_options options = {
            .texture = toplevel_texture,
            .dst_box = render_box,
        };
        wlr_render_pass_add_texture(pass, &options);
      };
    }
  }
  wlr_output_add_software_cursors_to_render_pass(wlr_output, pass, NULL);
  wlr_render_pass_submit(pass);
  wlr_output_commit_state(wlr_output, &output_state);
  wlr_output_state_finish(&output_state);
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
  output->wlr_output = wlr_output;
  /*
  bool focused = false;
  if (wl_list_empty(&wm->outputs)) {
    struct output *previous_output;
    wl_list_for_each(previous_output, &wm->outputs, link) {
      if (previous_output->focused) {
        focused = true;
        break;
      }
    }
  }

  if (!focused) {
    output->focused = true;
    int32_t width = output->wlr_output->width / 2;
    int32_t height = output->wlr_output->height / 2;
    wlr_cursor_warp_absolute(wm->cursor, NULL, width, height);
  }
*/
  output->wm = wm;
  add_signal_listener(&wlr_output->events.destroy, &output->destroy_listener,
                      output_destroy);
  add_signal_listener(&wlr_output->events.frame, &output->frame_listener,
                      output_frame);
  add_signal_listener(&wlr_output->events.request_state,
                      &output->request_state_listener, output_request_state);

  wl_list_insert(&wm->outputs, &output->link);

  wlr_output_layout_add(wm->output_layout, wlr_output, x, y);
}
