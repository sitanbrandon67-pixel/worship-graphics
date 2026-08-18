#include "psd-bridge.h"

#include <openpsd/psd.h>

#include <stdlib.h>
#include <string.h>

struct wg_psd_doc {
  psd_document_t *doc;
};

static void copy_error(char *dst, size_t dst_size, const char *src)
{
  if (!dst || dst_size == 0)
    return;
  if (!src)
    src = "Error PSD desconocido";
#if defined(_MSC_VER)
  strncpy_s(dst, dst_size, src, _TRUNCATE);
#else
  strncpy(dst, src, dst_size - 1);
  dst[dst_size - 1] = '\0';
#endif
}

wg_psd_doc *wg_psd_open_memory(const void *data, size_t size,
                               char *error, size_t error_size)
{
  if (!data || size == 0) {
    copy_error(error, error_size, "Buffer PSD vacío");
    return NULL;
  }

  psd_stream_t *stream = psd_stream_create_buffer(NULL, data, size);
  if (!stream) {
    copy_error(error, error_size, "No se pudo crear el stream PSD");
    return NULL;
  }

  psd_status_t status = PSD_OK;
  psd_document_t *parsed = psd_parse_ex(stream, NULL, &status);
  psd_stream_destroy(stream);

  if (!parsed) {
    copy_error(error, error_size, psd_error_string(status));
    return NULL;
  }

  wg_psd_doc *wrapper = (wg_psd_doc *)calloc(1, sizeof(wg_psd_doc));
  if (!wrapper) {
    psd_document_free(parsed);
    copy_error(error, error_size, "Sin memoria para abrir PSD");
    return NULL;
  }

  wrapper->doc = parsed;
  return wrapper;
}

void wg_psd_close(wg_psd_doc *doc)
{
  if (!doc)
    return;
  if (doc->doc)
    psd_document_free(doc->doc);
  free(doc);
}

int wg_psd_dimensions(wg_psd_doc *doc, uint32_t *width, uint32_t *height)
{
  if (!doc || !doc->doc)
    return PSD_ERR_INVALID_ARGUMENT;
  return (int)psd_document_get_dimensions(doc->doc, width, height);
}

int wg_psd_layer_count(wg_psd_doc *doc, int32_t *count)
{
  if (!doc || !doc->doc)
    return PSD_ERR_INVALID_ARGUMENT;
  return (int)psd_document_get_layer_count(doc->doc, count);
}

int wg_psd_layer_type_at(wg_psd_doc *doc, int32_t index, int *type)
{
  psd_layer_type_t value = PSD_LAYER_TYPE_EMPTY;
  if (!doc || !doc->doc || !type)
    return PSD_ERR_INVALID_ARGUMENT;

  psd_status_t st = psd_document_get_layer_type(doc->doc, index, &value);
  if (st == PSD_OK)
    *type = (int)value;
  return (int)st;
}

int wg_psd_layer_name(wg_psd_doc *doc, int32_t index,
                      char *buffer, size_t buffer_size)
{
  const uint8_t *raw = NULL;
  size_t length = 0;

  if (!doc || !doc->doc || !buffer || buffer_size == 0)
    return PSD_ERR_INVALID_ARGUMENT;

  psd_status_t st =
      psd_document_get_layer_name(doc->doc, index, &raw, &length);
  if (st != PSD_OK)
    return (int)st;

  if (!raw || length == 0) {
    buffer[0] = '\0';
    return PSD_OK;
  }

  size_t amount = length;
  if (amount >= buffer_size)
    amount = buffer_size - 1;

  memcpy(buffer, raw, amount);
  buffer[amount] = '\0';
  return PSD_OK;
}

int wg_psd_layer_bounds(wg_psd_doc *doc, int32_t index,
                        int32_t *top, int32_t *left,
                        int32_t *bottom, int32_t *right)
{
  if (!doc || !doc->doc)
    return PSD_ERR_INVALID_ARGUMENT;
  return (int)psd_document_get_layer_bounds(
      doc->doc, index, top, left, bottom, right);
}

int wg_psd_layer_properties(wg_psd_doc *doc, int32_t index,
                            uint8_t *opacity, uint8_t *flags)
{
  if (!doc || !doc->doc)
    return PSD_ERR_INVALID_ARGUMENT;
  return (int)psd_document_get_layer_properties(
      doc->doc, index, opacity, flags);
}

int wg_psd_render_layer_required(wg_psd_doc *doc, int32_t index,
                                 size_t *required)
{
  if (!doc || !doc->doc || !required)
    return PSD_ERR_INVALID_ARGUMENT;

  *required = 0;
  return (int)psd_document_render_layer_rgba8(
      doc->doc, index, NULL, 0, required);
}

int wg_psd_render_layer(wg_psd_doc *doc, int32_t index,
                        uint8_t *rgba, size_t rgba_size)
{
  if (!doc || !doc->doc || !rgba || rgba_size == 0)
    return PSD_ERR_INVALID_ARGUMENT;

  return (int)psd_document_render_layer_rgba8(
      doc->doc, index, rgba, rgba_size, NULL);
}

int wg_psd_text(wg_psd_doc *doc, int32_t index,
                char *buffer, size_t buffer_size)
{
  if (!doc || !doc->doc || !buffer || buffer_size == 0 || index < 0)
    return PSD_ERR_INVALID_ARGUMENT;

  return (int)psd_text_layer_get_text(
      doc->doc, (uint32_t)index, buffer, buffer_size);
}

int wg_psd_text_style_at(wg_psd_doc *doc, int32_t index,
                         wg_psd_text_style *style)
{
  if (!doc || !doc->doc || !style || index < 0)
    return PSD_ERR_INVALID_ARGUMENT;

  psd_text_style_t native_style;
  memset(&native_style, 0, sizeof(native_style));

  psd_status_t st = psd_text_layer_get_default_style(
      doc->doc, (uint32_t)index, &native_style);
  if (st != PSD_OK)
    return (int)st;

  memset(style, 0, sizeof(*style));
#if defined(_MSC_VER)
  strncpy_s(style->font_name, sizeof(style->font_name),
            native_style.font_name, _TRUNCATE);
#else
  strncpy(style->font_name, native_style.font_name,
          sizeof(style->font_name) - 1);
#endif
  style->size = native_style.size;
  memcpy(style->color_rgba, native_style.color_rgba, 4);
  style->justification = (int)native_style.justification;
  return PSD_OK;
}

int wg_psd_text_geometry_at(wg_psd_doc *doc, int32_t index,
                            wg_psd_text_geometry *geometry)
{
  if (!doc || !doc->doc || !geometry || index < 0)
    return PSD_ERR_INVALID_ARGUMENT;

  psd_text_matrix_t matrix;
  psd_text_bounds_t bounds;
  memset(&matrix, 0, sizeof(matrix));
  memset(&bounds, 0, sizeof(bounds));

  psd_status_t st = psd_text_layer_get_matrix_bounds(
      doc->doc, (uint32_t)index, &matrix, &bounds);
  if (st != PSD_OK)
    return (int)st;

  geometry->tx = matrix.tx;
  geometry->ty = matrix.ty;
  geometry->left = bounds.left;
  geometry->top = bounds.top;
  geometry->right = bounds.right;
  geometry->bottom = bounds.bottom;
  return PSD_OK;
}

const char *wg_psd_status_text(int status)
{
  return psd_error_string((psd_status_t)status);
}
