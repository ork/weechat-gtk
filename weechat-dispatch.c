/* See COPYING file for license and copyright information */

#include "weechat-dispatch.h"
#include "weechat-buffer.h"

void client_dispatch_buffer_line_added(client_t* client, GVariant* gv)
{
    gchar* message;
    gchar* buf_ptr;

    GVariant* gvline = g_variant_get_child_value(
        g_variant_get_child_value(gv, 0), 0);

    /* Extract */
    GVariantDict* dict = g_variant_dict_new(gvline);
    g_variant_dict_lookup(dict, "message", "ms", &message);
    g_variant_dict_lookup(dict, "buffer", "ms", &buf_ptr);
    g_variant_dict_unref(dict);

    /* Display */
    gchar* buf_name = g_hash_table_lookup(client->buf_ptrs, buf_ptr);
    buffer_t* buf = g_hash_table_lookup(client->buffers, buf_name);

    buffer_append_text(buf, message);

    g_free(message);
}