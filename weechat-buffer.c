#include <glib/gprintf.h>
#include "weechat-buffer.h"

gboolean buffer_equal(gconstpointer a, gconstpointer b)
{
    return g_strcmp0(((buffer_t*)a)->full_name, ((buffer_t*)b)->full_name) == 0;
}

buffer_t* buffer_create(GVariant* buf)
{
    buffer_t* buffer = g_try_malloc0(sizeof(buffer_t));

    if (buffer == NULL) {
        return NULL;
    }
    GVariantDict* dict = g_variant_dict_new(buf);

    g_variant_dict_lookup(dict, "full_name", "ms", &buffer->full_name);
    g_variant_dict_lookup(dict, "short_name", "ms", &buffer->short_name);
    g_variant_dict_lookup(dict, "title", "ms", &buffer->title);
    g_variant_dict_lookup(dict, "notify", "i", &buffer->notify);
    g_variant_dict_lookup(dict, "number", "i", &buffer->number);
    buffer->pointers = g_variant_dup_strv(
        g_variant_dict_lookup_value(dict, "__path", NULL), NULL);

    buffer->text_buf = gtk_text_buffer_new(NULL);

    g_variant_dict_unref(dict);

    return buffer;
}

void buffer_delete(buffer_t* buffer)
{
    g_free(buffer->full_name);
    g_free(buffer->short_name);
    g_free(buffer->title);
    g_strfreev(buffer->pointers);
    g_free(buffer);
}

const gchar* buffer_get_canonical_name(buffer_t* buffer)
{
    if (buffer->short_name != NULL) {
        return buffer->short_name;
    } else {
        return buffer->full_name;
    }
}
