#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wg_psd_doc wg_psd_doc;

enum wg_psd_layer_type {
  WG_PSD_GROUP_END = 0,
  WG_PSD_GROUP_START = 1,
  WG_PSD_PIXEL = 2,
  WG_PSD_TEXT = 3,
  WG_PSD_SMART_OBJECT = 4,
  WG_PSD_ADJUSTMENT = 5,
  WG_PSD_FILL = 6,
  WG_PSD_EFFECTS = 7,
  WG_PSD_3D = 8,
  WG_PSD_VIDEO = 9,
  WG_PSD_EMPTY = 10
};

typedef struct wg_psd_text_style {
  char font_name[128];
  double size;
  uint8_t color_rgba[4];
  int justification;
} wg_psd_text_style;

typedef struct wg_psd_text_geometry {
  double tx;
  double ty;
  double left;
  double top;
  double right;
  double bottom;
} wg_psd_text_geometry;

wg_psd_doc *wg_psd_open_memory(const void *data, size_t size,
                               char *error, size_t error_size);
void wg_psd_close(wg_psd_doc *doc);

int wg_psd_dimensions(wg_psd_doc *doc, uint32_t *width, uint32_t *height);
int wg_psd_layer_count(wg_psd_doc *doc, int32_t *count);
int wg_psd_layer_type_at(wg_psd_doc *doc, int32_t index, int *type);
int wg_psd_layer_name(wg_psd_doc *doc, int32_t index,
                      char *buffer, size_t buffer_size);
int wg_psd_layer_bounds(wg_psd_doc *doc, int32_t index,
                        int32_t *top, int32_t *left,
                        int32_t *bottom, int32_t *right);
int wg_psd_layer_properties(wg_psd_doc *doc, int32_t index,
                            uint8_t *opacity, uint8_t *flags);

int wg_psd_render_layer_required(wg_psd_doc *doc, int32_t index,
                                 size_t *required);
int wg_psd_render_layer(wg_psd_doc *doc, int32_t index,
                        uint8_t *rgba, size_t rgba_size);

int wg_psd_text(wg_psd_doc *doc, int32_t index,
                char *buffer, size_t buffer_size);
int wg_psd_text_style_at(wg_psd_doc *doc, int32_t index,
                         wg_psd_text_style *style);
int wg_psd_text_geometry_at(wg_psd_doc *doc, int32_t index,
                            wg_psd_text_geometry *geometry);

const char *wg_psd_status_text(int status);

#ifdef __cplusplus
}
#endif
