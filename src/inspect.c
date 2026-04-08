#include <stdio.h>
#include <stdlib.h>

#include <wlr/types/wlr_scene.h>

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
